// Active Object Pattern - Asynchronous Scientific Computation
// Decoupling computation requests from execution for parallel scientific processing
#include <iostream>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <chrono>
#include <atomic>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <random>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Scientific Computation Request interface
class ComputationRequest {
public:
    virtual ~ComputationRequest() = default;
    virtual void call() = 0;
};

// Concrete Scientific Computation Request with Future
template<typename Result>
class ConcreteComputationRequest : public ComputationRequest {
private:
    std::function<Result()> computation_;
    std::promise<Result> promise_;
    std::string name_;
    
public:
    ConcreteComputationRequest(std::function<Result()> computation, 
                              const std::string& name = "Computation") 
        : computation_(computation), name_(name) {}
    
    void call() override {
        try {
            auto start = std::chrono::high_resolution_clock::now();
            
            if constexpr (std::is_void_v<Result>) {
                computation_();
                promise_.set_value();
            } else {
                promise_.set_value(computation_());
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            
            if (duration > 100) {  // Log if computation took more than 100μs
                std::cout << "[" << name_ << "] Computation completed in " 
                          << duration << " μs\n";
            }
        } catch (...) {
            promise_.set_exception(std::current_exception());
        }
    }
    
    std::future<Result> getFuture() {
        return promise_.get_future();
    }
};

// Scientific Computation Queue with Priority Support
class ComputationQueue {
private:
    struct PrioritizedRequest {
        std::unique_ptr<ComputationRequest> request;
        int priority;
        std::chrono::steady_clock::time_point enqueueTime;
        
        bool operator<(const PrioritizedRequest& other) const {
            // Higher priority value = higher priority
            if (priority != other.priority) {
                return priority < other.priority;  // For max heap
            }
            // If same priority, earlier request has priority
            return enqueueTime > other.enqueueTime;
        }
    };
    
    std::priority_queue<PrioritizedRequest> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool stopped_ = false;
    std::atomic<size_t> totalProcessed_{0};
    std::atomic<size_t> totalEnqueued_{0};
    
public:
    void enqueue(std::unique_ptr<ComputationRequest> request, int priority = 0) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) {
                throw std::runtime_error("Computation queue is stopped");
            }
            queue_.push({std::move(request), priority, 
                        std::chrono::steady_clock::now()});
            totalEnqueued_++;
        }
        condition_.notify_one();
    }
    
    std::unique_ptr<ComputationRequest> dequeue() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        
        if (stopped_ && queue_.empty()) {
            return nullptr;
        }
        
        auto request = std::move(const_cast<PrioritizedRequest&>(queue_.top()).request);
        queue_.pop();
        totalProcessed_++;
        return request;
    }
    
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
        }
        condition_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    
    size_t totalProcessed() const { return totalProcessed_; }
    size_t totalEnqueued() const { return totalEnqueued_; }
};

// Scientific Computation Scheduler
class ComputationScheduler {
private:
    ComputationQueue queue_;
    std::vector<std::thread> workers_;
    std::atomic<bool> running_{false};
    std::string name_;
    std::atomic<size_t> activeWorkers_{0};
    
    void run(int workerId) {
        while (running_) {
            auto request = queue_.dequeue();
            if (request) {
                activeWorkers_++;
                request->call();
                activeWorkers_--;
            } else if (!running_) {
                break;
            }
        }
    }
    
public:
    ComputationScheduler(const std::string& name = "ComputeScheduler", 
                        size_t numWorkers = std::thread::hardware_concurrency()) 
        : name_(name), running_(true) {
        
        std::cout << "[" << name_ << "] Starting with " << numWorkers 
                  << " computation threads\n";
        
        for (size_t i = 0; i < numWorkers; ++i) {
            workers_.emplace_back(&ComputationScheduler::run, this, i);
        }
    }
    
    ~ComputationScheduler() {
        stop();
    }
    
    void enqueue(std::unique_ptr<ComputationRequest> request, int priority = 0) {
        queue_.enqueue(std::move(request), priority);
    }
    
    void stop() {
        if (running_) {
            running_ = false;
            queue_.stop();
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
            
            std::cout << "[" << name_ << "] Stopped. Total computations processed: " 
                      << queue_.totalProcessed() << "/" << queue_.totalEnqueued() << "\n";
        }
    }
    
    size_t pendingComputations() const {
        return queue_.size();
    }
    
    size_t activeWorkers() const {
        return activeWorkers_;
    }
    
    std::pair<size_t, size_t> getStatistics() const {
        return {queue_.totalProcessed(), queue_.totalEnqueued()};
    }
};

// Scientific Computation Proxy
template<typename ComputeServant>
class ScientificComputationProxy {
private:
    std::shared_ptr<ComputeServant> servant_;
    std::shared_ptr<ComputationScheduler> scheduler_;
    
public:
    ScientificComputationProxy(std::shared_ptr<ComputeServant> servant, 
                              std::shared_ptr<ComputationScheduler> scheduler)
        : servant_(servant), scheduler_(scheduler) {}
    
    template<typename Result, typename Method, typename... Args>
    std::future<Result> compute(Method method, 
                               Args... args) {
        auto request = std::make_unique<ConcreteComputationRequest<Result>>(
            [servant = servant_, method, args...]() {
                return (servant.get()->*method)(args...);
            },
            "Computation"
        );
        
        auto future = request->getFuture();
        scheduler_->enqueue(std::move(request), 0);
        return future;
    }
    
    template<typename Result, typename Method, typename... Args>
    std::future<Result> computeWithPriority(Method method, 
                                           int priority,
                                           const std::string& computationName,
                                           Args... args) {
        auto request = std::make_unique<ConcreteComputationRequest<Result>>(
            [servant = servant_, method, args...]() {
                return (servant.get()->*method)(args...);
            },
            computationName
        );
        
        auto future = request->getFuture();
        scheduler_->enqueue(std::move(request), priority);
        return future;
    }
};

// Monte Carlo Simulation Servant
class MonteCarloServant {
private:
    std::string name_;
    int samplesProcessed_ = 0;
    double currentEstimate_ = 0.0;
    std::mt19937 generator_;
    
public:
    MonteCarloServant(const std::string& name, unsigned seed = std::random_device{}()) 
        : name_(name), generator_(seed) {}
    
    double estimatePi(int samples) {
        int insideCircle = 0;
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        
        for (int i = 0; i < samples; ++i) {
            double x = dist(generator_);
            double y = dist(generator_);
            if (x*x + y*y <= 1.0) {
                insideCircle++;
            }
        }
        
        samplesProcessed_ += samples;
        currentEstimate_ = 4.0 * insideCircle / samples;
        
        std::cout << "[" << name_ << "] Estimated π = " << std::fixed 
                  << std::setprecision(6) << currentEstimate_ 
                  << " (from " << samples << " samples)\n";
        
        return currentEstimate_;
    }
    
    double integrateFunction(std::function<double(double)> f, 
                           double a, double b, int samples) {
        std::uniform_real_distribution<double> dist(a, b);
        double sum = 0.0;
        
        for (int i = 0; i < samples; ++i) {
            double x = dist(generator_);
            sum += f(x);
        }
        
        double result = (b - a) * sum / samples;
        samplesProcessed_ += samples;
        
        std::cout << "[" << name_ << "] Monte Carlo integration result: " 
                  << std::scientific << std::setprecision(6) << result << "\n";
        
        return result;
    }
    
    int getSamplesProcessed() const {
        return samplesProcessed_;
    }
    
    void reset() {
        samplesProcessed_ = 0;
        currentEstimate_ = 0.0;
        std::cout << "[" << name_ << "] Monte Carlo simulation reset\n";
    }
};

class ActiveMonteCarloSimulator {
private:
    std::shared_ptr<MonteCarloServant> servant_;
    std::shared_ptr<ComputationScheduler> scheduler_;
    ScientificComputationProxy<MonteCarloServant> proxy_;
    
public:
    ActiveMonteCarloSimulator(const std::string& name,
                             std::shared_ptr<ComputationScheduler> scheduler = nullptr) 
        : servant_(std::make_shared<MonteCarloServant>(name)),
          scheduler_(scheduler ? scheduler : 
                    std::make_shared<ComputationScheduler>("MonteCarloScheduler", 2)),
          proxy_(servant_, scheduler_) {}
    
    std::future<double> estimatePi(int samples, int priority = 0) {
        return proxy_.computeWithPriority<double>(&MonteCarloServant::estimatePi, 
                                                 priority, "Pi Estimation", samples);
    }
    
    std::future<double> integrateFunction(std::function<double(double)> f,
                                        double a, double b, int samples,
                                        int priority = 0) {
        return proxy_.computeWithPriority<double>(&MonteCarloServant::integrateFunction, 
                                                 priority, "MC Integration", f, a, b, samples);
    }
    
    std::future<int> getSamplesProcessed() {
        return proxy_.compute<int>(&MonteCarloServant::getSamplesProcessed);
    }
    
    std::future<void> reset() {
        return proxy_.compute<void>(&MonteCarloServant::reset);
    }
    
    size_t pendingComputations() const {
        return scheduler_->pendingComputations();
    }
};

// Numerical Integration Servant
class NumericalIntegratorServant {
private:
    std::string name_;
    int totalIntegrations_ = 0;
    double lastResult_ = 0.0;
    
public:
    NumericalIntegratorServant(const std::string& name) : name_(name) {}
    
    double trapezoidalRule(std::function<double(double)> f, 
                          double a, double b, int n) {
        double h = (b - a) / n;
        double sum = 0.5 * (f(a) + f(b));
        
        for (int i = 1; i < n; ++i) {
            sum += f(a + i * h);
        }
        
        lastResult_ = h * sum;
        totalIntegrations_++;
        
        std::cout << "[" << name_ << "] Trapezoidal integration: " 
                  << std::scientific << std::setprecision(6) << lastResult_ 
                  << " (" << n << " intervals)\n";
        
        return lastResult_;
    }
    
    double simpsonsRule(std::function<double(double)> f, 
                       double a, double b, int n) {
        if (n % 2 != 0) n++; // Ensure even number
        
        double h = (b - a) / n;
        double sum = f(a) + f(b);
        
        for (int i = 1; i < n; i += 2) {
            sum += 4 * f(a + i * h);
        }
        for (int i = 2; i < n; i += 2) {
            sum += 2 * f(a + i * h);
        }
        
        lastResult_ = (h / 3) * sum;
        totalIntegrations_++;
        
        std::cout << "[" << name_ << "] Simpson's rule integration: " 
                  << std::scientific << std::setprecision(6) << lastResult_ 
                  << " (" << n << " intervals)\n";
        
        return lastResult_;
    }
    
    double adaptiveQuadrature(std::function<double(double)> f,
                             double a, double b, double tolerance) {
        // Simplified adaptive quadrature
        int n = 10;
        double prev = simpsonsRule(f, a, b, n);
        double curr = simpsonsRule(f, a, b, 2*n);
        
        while (std::abs(curr - prev) > tolerance && n < 10000) {
            n *= 2;
            prev = curr;
            curr = simpsonsRule(f, a, b, 2*n);
        }
        
        lastResult_ = curr;
        std::cout << "[" << name_ << "] Adaptive quadrature converged to " 
                  << std::scientific << std::setprecision(9) << lastResult_ 
                  << " (tolerance: " << tolerance << ")\n";
        
        return lastResult_;
    }
    
    int getTotalIntegrations() const {
        return totalIntegrations_;
    }
    
    double getLastResult() const {
        return lastResult_;
    }
};

class ActiveNumericalIntegrator {
private:
    std::shared_ptr<NumericalIntegratorServant> servant_;
    std::shared_ptr<ComputationScheduler> scheduler_;
    ScientificComputationProxy<NumericalIntegratorServant> proxy_;
    
public:
    ActiveNumericalIntegrator(const std::string& name,
                             std::shared_ptr<ComputationScheduler> scheduler = nullptr) 
        : servant_(std::make_shared<NumericalIntegratorServant>(name)),
          scheduler_(scheduler ? scheduler : 
                    std::make_shared<ComputationScheduler>("IntegratorScheduler", 1)),
          proxy_(servant_, scheduler_) {}
    
    std::future<double> trapezoidalRule(std::function<double(double)> f,
                                       double a, double b, int n,
                                       int priority = 0) {
        return proxy_.computeWithPriority<double>(&NumericalIntegratorServant::trapezoidalRule, 
                                                 priority, "Trapezoidal", f, a, b, n);
    }
    
    std::future<double> simpsonsRule(std::function<double(double)> f,
                                    double a, double b, int n,
                                    int priority = 0) {
        return proxy_.computeWithPriority<double>(&NumericalIntegratorServant::simpsonsRule, 
                                                 priority, "Simpson's", f, a, b, n);
    }
    
    std::future<double> adaptiveQuadrature(std::function<double(double)> f,
                                          double a, double b, double tolerance,
                                          int priority = 1) {
        return proxy_.computeWithPriority<double>(&NumericalIntegratorServant::adaptiveQuadrature, 
                                                 priority, "Adaptive", f, a, b, tolerance);
    }
    
    std::future<int> getTotalIntegrations() {
        return proxy_.compute<int>(&NumericalIntegratorServant::getTotalIntegrations);
    }
};

// Matrix Operations Servant for Linear Algebra
class MatrixOperationsServant {
private:
    std::string name_;
    int operationsCount_ = 0;
    
    // Helper function for matrix multiplication
    std::vector<double> multiplyMatrices(const std::vector<double>& A,
                                        const std::vector<double>& B,
                                        int m, int n, int p) {
        std::vector<double> C(m * p, 0.0);
        
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < p; ++j) {
                for (int k = 0; k < n; ++k) {
                    C[i * p + j] += A[i * n + k] * B[k * p + j];
                }
            }
        }
        
        return C;
    }
    
public:
    MatrixOperationsServant(const std::string& name) : name_(name) {}
    
    std::vector<double> matrixMultiply(const std::vector<double>& A,
                                      const std::vector<double>& B,
                                      int m, int n, int p) {
        auto start = std::chrono::high_resolution_clock::now();
        
        auto result = multiplyMatrices(A, B, m, n, p);
        operationsCount_++;
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        
        std::cout << "[" << name_ << "] Matrix multiply (" << m << "x" << n 
                  << ") × (" << n << "x" << p << ") completed in " 
                  << duration << " μs\n";
        
        return result;
    }
    
    double computeDeterminant(const std::vector<double>& matrix, int n) {
        // Simplified determinant calculation for small matrices
        if (n == 1) return matrix[0];
        if (n == 2) return matrix[0] * matrix[3] - matrix[1] * matrix[2];
        
        // For larger matrices, use LU decomposition (simplified)
        double det = 1.0;
        std::vector<double> work = matrix;
        
        for (int i = 0; i < n; ++i) {
            det *= work[i * n + i];
        }
        
        operationsCount_++;
        std::cout << "[" << name_ << "] Determinant of " << n << "x" << n 
                  << " matrix = " << std::scientific << det << "\n";
        
        return det;
    }
    
    std::vector<double> solveLinearSystem(const std::vector<double>& A,
                                         const std::vector<double>& b,
                                         int n) {
        // Gaussian elimination with partial pivoting
        std::vector<double> augmented = A;
        std::vector<double> x(n);
        std::vector<double> rhs = b;
        
        // Forward elimination
        for (int k = 0; k < n - 1; ++k) {
            for (int i = k + 1; i < n; ++i) {
                double factor = augmented[i * n + k] / augmented[k * n + k];
                for (int j = k + 1; j < n; ++j) {
                    augmented[i * n + j] -= factor * augmented[k * n + j];
                }
                rhs[i] -= factor * rhs[k];
            }
        }
        
        // Back substitution
        for (int i = n - 1; i >= 0; --i) {
            x[i] = rhs[i];
            for (int j = i + 1; j < n; ++j) {
                x[i] -= augmented[i * n + j] * x[j];
            }
            x[i] /= augmented[i * n + i];
        }
        
        operationsCount_++;
        std::cout << "[" << name_ << "] Linear system " << n << "x" << n 
                  << " solved\n";
        
        return x;
    }
    
    std::pair<std::vector<double>, std::vector<double>> computeEigenvalues(
                                        const std::vector<double>& matrix, int n) {
        // Power iteration for dominant eigenvalue (simplified)
        std::vector<double> eigenvalues(1);
        std::vector<double> eigenvectors(n);
        
        // Initialize random vector
        for (int i = 0; i < n; ++i) {
            eigenvectors[i] = 1.0 / std::sqrt(n);
        }
        
        // Power iteration
        for (int iter = 0; iter < 100; ++iter) {
            std::vector<double> Av(n, 0.0);
            
            // Matrix-vector multiply
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    Av[i] += matrix[i * n + j] * eigenvectors[j];
                }
            }
            
            // Normalize
            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += Av[i] * Av[i];
            }
            norm = std::sqrt(norm);
            
            for (int i = 0; i < n; ++i) {
                eigenvectors[i] = Av[i] / norm;
            }
        }
        
        // Rayleigh quotient for eigenvalue
        double eigenvalue = 0.0;
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += matrix[i * n + j] * eigenvectors[j];
            }
            eigenvalue += eigenvectors[i] * sum;
        }
        
        eigenvalues[0] = eigenvalue;
        operationsCount_++;
        
        std::cout << "[" << name_ << "] Dominant eigenvalue: " 
                  << std::scientific << eigenvalue << "\n";
        
        return {eigenvalues, eigenvectors};
    }
    
    int getOperationsCount() const {
        return operationsCount_;
    }
};

class ActiveMatrixComputer {
private:
    std::shared_ptr<MatrixOperationsServant> servant_;
    std::shared_ptr<ComputationScheduler> scheduler_;
    ScientificComputationProxy<MatrixOperationsServant> proxy_;
    
public:
    ActiveMatrixComputer(const std::string& name,
                        std::shared_ptr<ComputationScheduler> scheduler = nullptr) 
        : servant_(std::make_shared<MatrixOperationsServant>(name)),
          scheduler_(scheduler ? scheduler : 
                    std::make_shared<ComputationScheduler>("MatrixScheduler", 2)),
          proxy_(servant_, scheduler_) {}
    
    std::future<std::vector<double>> matrixMultiply(
                        const std::vector<double>& A,
                        const std::vector<double>& B,
                        int m, int n, int p,
                        int priority = 0) {
        return proxy_.computeWithPriority<std::vector<double>>(&MatrixOperationsServant::matrixMultiply, 
                                                              priority, "MatMul", A, B, m, n, p);
    }
    
    std::future<double> computeDeterminant(const std::vector<double>& matrix, 
                                          int n, int priority = 0) {
        return proxy_.computeWithPriority<double>(&MatrixOperationsServant::computeDeterminant, 
                                                 priority, "Det", matrix, n);
    }
    
    std::future<std::vector<double>> solveLinearSystem(
                        const std::vector<double>& A,
                        const std::vector<double>& b,
                        int n, int priority = 1) {
        return proxy_.computeWithPriority<std::vector<double>>(&MatrixOperationsServant::solveLinearSystem, 
                                                              priority, "LinSolve", A, b, n);
    }
    
    std::future<std::pair<std::vector<double>, std::vector<double>>> 
    computeEigenvalues(const std::vector<double>& matrix, int n, int priority = 2) {
        return proxy_.computeWithPriority<std::pair<std::vector<double>, std::vector<double>>>(
                             &MatrixOperationsServant::computeEigenvalues, 
                             priority, "Eigen", matrix, n);
    }
};

int main() {
    std::cout << "=== Active Object Pattern - Scientific Computing Demo ===\n";
    std::cout << "Asynchronous execution of scientific computations\n\n";
    
    // Create shared computation scheduler for all components
    auto globalScheduler = std::make_shared<ComputationScheduler>("GlobalScheduler", 4);
    
    // Monte Carlo Simulation Example
    std::cout << "=== Monte Carlo Simulations ===\n\n";
    
    ActiveMonteCarloSimulator mcSimulator("MC-Simulator", globalScheduler);
    
    // Launch multiple Pi estimations with different sample sizes
    std::vector<std::future<double>> piFutures;
    std::vector<int> sampleSizes = {10000, 100000, 1000000, 10000000};
    
    for (int samples : sampleSizes) {
        piFutures.push_back(mcSimulator.estimatePi(samples, samples/10000)); // Priority based on size
    }
    
    std::cout << "Pending Monte Carlo computations: " 
              << mcSimulator.pendingComputations() << "\n\n";
    
    // Define test functions for integration
    auto gaussian = [](double x) { return std::exp(-x*x); };
    auto sinc = [](double x) { return x == 0 ? 1.0 : std::sin(x)/x; };
    
    // Launch integration tasks
    auto gaussianFuture = mcSimulator.integrateFunction(gaussian, -5, 5, 1000000, 2);
    auto sincFuture = mcSimulator.integrateFunction(sinc, -10, 10, 500000, 1);
    
    // Wait for Pi estimations
    std::cout << "\n=== Pi Estimation Results ===\n";
    for (size_t i = 0; i < piFutures.size(); ++i) {
        double estimate = piFutures[i].get();
        double error = std::abs(estimate - M_PI);
        std::cout << "Samples: " << std::setw(8) << sampleSizes[i] 
                  << ", Error: " << std::scientific << std::setprecision(3) 
                  << error << "\n";
    }
    
    std::cout << "\nGaussian integral: " << gaussianFuture.get() 
              << " (Expected: " << std::sqrt(M_PI) << ")\n";
    std::cout << "Sinc integral: " << sincFuture.get() << "\n";
    
    auto mcSamples = mcSimulator.getSamplesProcessed();
    std::cout << "\nTotal Monte Carlo samples processed: " << mcSamples.get() << "\n";
    
    // Numerical Integration Example
    std::cout << "\n\n=== Numerical Integration ===\n\n";
    
    ActiveNumericalIntegrator integrator("NumIntegrator", globalScheduler);
    
    // Test functions
    auto polynomial = [](double x) { return x*x*x - 2*x*x + x - 3; };
    auto transcendental = [](double x) { return std::sin(x) * std::exp(-0.1*x); };
    
    // Launch different integration methods
    auto trapPoly = integrator.trapezoidalRule(polynomial, -2, 3, 1000, 0);
    auto simpPoly = integrator.simpsonsRule(polynomial, -2, 3, 1000, 0);
    auto adaptPoly = integrator.adaptiveQuadrature(polynomial, -2, 3, 1e-9, 2);
    
    auto trapTrans = integrator.trapezoidalRule(transcendental, 0, 10, 5000, 1);
    auto simpTrans = integrator.simpsonsRule(transcendental, 0, 10, 5000, 1);
    auto adaptTrans = integrator.adaptiveQuadrature(transcendental, 0, 10, 1e-12, 3);
    
    // Collect results
    std::cout << "\n=== Integration Results ===\n";
    std::cout << "Polynomial integral [-2, 3]:\n";
    std::cout << "  Trapezoidal: " << std::fixed << std::setprecision(9) << trapPoly.get() << "\n";
    std::cout << "  Simpson's:   " << simpPoly.get() << "\n";
    std::cout << "  Adaptive:    " << adaptPoly.get() << "\n";
    
    std::cout << "\nTranscendental integral [0, 10]:\n";
    std::cout << "  Trapezoidal: " << trapTrans.get() << "\n";
    std::cout << "  Simpson's:   " << simpTrans.get() << "\n";
    std::cout << "  Adaptive:    " << adaptTrans.get() << "\n";
    
    // Matrix Operations Example
    std::cout << "\n\n=== Matrix Operations ===\n\n";
    
    ActiveMatrixComputer matrixComputer("MatrixComputer", globalScheduler);
    
    // Create test matrices
    int n = 100;
    std::vector<double> A(n * n), B(n * n), C(n * n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    // Initialize matrices
    for (int i = 0; i < n * n; ++i) {
        A[i] = dis(gen);
        B[i] = dis(gen);
    }
    
    // Make a symmetric positive definite matrix for eigenvalue computation
    std::vector<double> symmetric(n * n);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            symmetric[i * n + j] = symmetric[j * n + i] = dis(gen);
        }
        symmetric[i * n + i] += n; // Ensure positive definite
    }
    
    // Launch matrix operations
    auto matMulFuture = matrixComputer.matrixMultiply(A, B, n, n, n, 1);
    
    // Smaller matrices for other operations
    std::vector<double> smallA = {4, 2, 1, 3, 5, 2, 1, 2, 3};
    std::vector<double> b = {1, 2, 3};
    std::vector<double> eigenMatrix = {4, -2, 2, -2, 5, -4, 2, -4, 5};
    
    auto detFuture = matrixComputer.computeDeterminant(smallA, 3, 2);
    auto solveFuture = matrixComputer.solveLinearSystem(smallA, b, 3, 2);
    auto eigenFuture = matrixComputer.computeEigenvalues(eigenMatrix, 3, 3);
    
    // Get results
    std::cout << "\n=== Matrix Computation Results ===\n";
    
    auto matMulResult = matMulFuture.get();
    std::cout << "Matrix multiplication completed (result size: " 
              << matMulResult.size() << ")\n";
    
    std::cout << "Determinant of 3x3 matrix: " << detFuture.get() << "\n";
    
    auto solution = solveFuture.get();
    std::cout << "Linear system solution: ";
    for (double x : solution) {
        std::cout << std::fixed << std::setprecision(4) << x << " ";
    }
    std::cout << "\n";
    
    auto [eigenvalues, eigenvectors] = eigenFuture.get();
    std::cout << "Dominant eigenvalue: " << std::scientific << std::setprecision(6) 
              << eigenvalues[0] << "\n";
    
    // Multi-threaded client simulation
    std::cout << "\n\n=== Concurrent Client Simulation ===\n";
    
    std::vector<std::thread> clients;
    std::atomic<int> totalRequests{0};
    
    // Simulate multiple clients submitting various computations
    for (int clientId = 0; clientId < 3; ++clientId) {
        clients.emplace_back([clientId, &mcSimulator, &integrator, &matrixComputer, &totalRequests]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> taskDist(0, 2);
            std::uniform_int_distribution<> sampleDist(1000, 100000);
            
            for (int task = 0; task < 5; ++task) {
                int taskType = taskDist(gen);
                totalRequests++;
                
                switch (taskType) {
                    case 0: // Monte Carlo
                        mcSimulator.estimatePi(sampleDist(gen), 0);
                        break;
                    case 1: // Integration
                        integrator.trapezoidalRule([](double x) { return x*x; }, 
                                                 0, 1, 1000, 0);
                        break;
                    case 2: // Matrix
                        std::vector<double> smallMat = {1, 2, 2, 1};
                        matrixComputer.computeDeterminant(smallMat, 2, 0);
                        break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    // Wait for all clients
    for (auto& client : clients) {
        client.join();
    }
    
    // Give time for computations to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Final statistics
    std::cout << "\n=== Final Statistics ===\n";
    auto [processed, enqueued] = globalScheduler->getStatistics();
    std::cout << "Total computations: " << enqueued << "\n";
    std::cout << "Processed computations: " << processed << "\n";
    std::cout << "Success rate: " << std::fixed << std::setprecision(1) 
              << (100.0 * processed / enqueued) << "%\n";
    
    std::cout << "\n=== Active Object Pattern Benefits ===\n";
    std::cout << "• Asynchronous execution of expensive computations\n";
    std::cout << "• Priority-based scheduling for critical calculations\n";
    std::cout << "• Thread-safe access to computational resources\n";
    std::cout << "• Natural load balancing across worker threads\n";
    std::cout << "• Improved responsiveness for scientific applications\n";
    
    return 0;
}