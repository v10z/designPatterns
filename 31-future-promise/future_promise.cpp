// Future-Promise Pattern for Scientific Computing - Asynchronous Computation Results
// Handling async numerical simulations, parallel analysis, and deferred calculations
#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include <numeric>
#include <exception>
#include <functional>
#include <memory>
#include <queue>
#include <random>
#include <cmath>
#include <complex>
#include <algorithm>
#include <iomanip>
#include <array>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Scientific computation result structure
struct SimulationResult {
    double energy;
    double temperature;
    std::vector<double> state;
    int iterations;
    double residual;
};

// Basic Future-Promise for eigenvalue computation
void eigenvalue_computation_example() {
    std::cout << "=== Eigenvalue Computation with Future-Promise ===\n";
    
    // Promise for expensive eigenvalue calculation
    std::promise<double> eigenvalue_promise;
    std::future<double> eigenvalue_future = eigenvalue_promise.get_future();
    
    // Matrix size
    const int n = 100;
    
    // Launch async eigenvalue computation
    std::thread compute_thread([n](std::promise<double> p) {
        std::cout << "[Compute Thread] Starting eigenvalue computation for " 
                  << n << "x" << n << " matrix...\n";
        
        // Simulate power iteration method
        std::vector<double> v(n, 1.0/std::sqrt(n));
        double eigenvalue = 0.0;
        
        for (int iter = 0; iter < 50; ++iter) {
            // Matrix-vector multiplication (simplified)
            std::vector<double> Av(n);
            for (int i = 0; i < n; ++i) {
                Av[i] = 0.0;
                for (int j = 0; j < n; ++j) {
                    // Tridiagonal matrix
                    if (i == j) Av[i] += 2.0 * v[j];
                    else if (abs(i-j) == 1) Av[i] += -1.0 * v[j];
                }
            }
            
            // Compute eigenvalue estimate
            eigenvalue = std::inner_product(v.begin(), v.end(), Av.begin(), 0.0);
            
            // Normalize
            double norm = std::sqrt(std::inner_product(Av.begin(), Av.end(), Av.begin(), 0.0));
            for (auto& val : Av) val /= norm;
            v = Av;
        }
        
        std::cout << "[Compute Thread] Eigenvalue converged\n";
        p.set_value(eigenvalue);
    }, std::move(eigenvalue_promise));
    
    // Main thread can prepare other computations
    std::cout << "[Main Thread] Preparing mesh for visualization...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Get the eigenvalue result (blocks if not ready)
    std::cout << "[Main Thread] Waiting for eigenvalue...\n";
    double lambda = eigenvalue_future.get();
    std::cout << "[Main Thread] Dominant eigenvalue: " 
              << std::scientific << std::setprecision(6) << lambda << "\n";
    
    compute_thread.join();
}

// Monte Carlo integration with std::async
void monte_carlo_integration_example() {
    std::cout << "\n=== Monte Carlo Integration with std::async ===\n";
    
    // Function to integrate: exp(-x²) * sin(2πy) over [0,1]²
    auto integrand = [](double x, double y) {
        return std::exp(-x*x) * std::sin(2*M_PI*y);
    };
    
    const int total_samples = 10000000;
    const int num_threads = 4;
    const int samples_per_thread = total_samples / num_threads;
    
    // Launch async Monte Carlo computations
    std::vector<std::future<double>> futures;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_threads; ++i) {
        futures.push_back(std::async(std::launch::async, 
            [i, samples_per_thread, integrand]() {
                std::cout << "[Thread " << i << "] Starting Monte Carlo sampling...\n";
                
                std::random_device rd;
                std::mt19937 gen(rd() + i);
                std::uniform_real_distribution<> dis(0.0, 1.0);
                
                double sum = 0.0;
                for (int j = 0; j < samples_per_thread; ++j) {
                    double x = dis(gen);
                    double y = dis(gen);
                    sum += integrand(x, y);
                }
                
                return sum / samples_per_thread;
            }));
    }
    
    // Collect results
    double total_sum = 0.0;
    for (auto& future : futures) {
        total_sum += future.get();
    }
    
    double integral = total_sum / num_threads;
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\nMonte Carlo Integration Results:\n";
    std::cout << "  Integral value: " << std::scientific << std::setprecision(6) 
              << integral << "\n";
    std::cout << "  Total samples: " << total_samples << "\n";
    std::cout << "  Computation time: " << duration.count() << "ms\n";
    std::cout << "  Samples/second: " << (total_samples * 1000.0 / duration.count()) << "\n";
}

// Numerical solver with exception handling
void numerical_solver_exception_example() {
    std::cout << "\n=== Numerical Solver Exception Handling ===\n";
    
    // Promise for Newton-Raphson solver result
    std::promise<double> solver_promise;
    std::future<double> solver_future = solver_promise.get_future();
    
    std::thread solver_thread([](std::promise<double> p) {
        try {
            std::cout << "[Solver] Starting Newton-Raphson iteration...\n";
            
            // Solve: x³ - 2x - 5 = 0
            auto f = [](double x) { return x*x*x - 2*x - 5; };
            auto df = [](double x) { return 3*x*x - 2; };
            
            double x = 2.0;  // Initial guess
            double tolerance = 1e-10;
            int max_iterations = 100;
            
            for (int iter = 0; iter < max_iterations; ++iter) {
                double fx = f(x);
                double dfx = df(x);
                
                if (std::abs(dfx) < 1e-14) {
                    throw std::runtime_error("Derivative too small - singular Jacobian");
                }
                
                double x_new = x - fx/dfx;
                
                if (std::abs(x_new - x) < tolerance) {
                    std::cout << "[Solver] Converged in " << iter << " iterations\n";
                    p.set_value(x_new);
                    return;
                }
                
                x = x_new;
            }
            
            throw std::runtime_error("Failed to converge within max iterations");
            
        } catch (...) {
            p.set_exception(std::current_exception());
        }
    }, std::move(solver_promise));
    
    // Try to get solver result
    try {
        double root = solver_future.get();
        std::cout << "[Main] Found root: x = " << std::setprecision(10) << root << "\n";
        
        // Verify
        auto f = [](double x) { return x*x*x - 2*x - 5; };
        std::cout << "[Main] Verification: f(x) = " << std::scientific << f(root) << "\n";
    } catch (const std::exception& e) {
        std::cout << "[Main] Solver failed: " << e.what() << "\n";
    }
    
    solver_thread.join();
}

// Shared future for simulation parameters
void shared_simulation_parameters_example() {
    std::cout << "\n=== Shared Simulation Parameters Example ===\n";
    
    struct SimulationParams {
        double timestep;
        double temperature;
        int num_particles;
        std::vector<double> force_constants;
    };
    
    std::promise<SimulationParams> param_promise;
    std::shared_future<SimulationParams> shared_params = param_promise.get_future().share();
    
    // Multiple analysis threads waiting for parameters
    std::vector<std::thread> analyzers;
    
    // Energy analyzer
    analyzers.emplace_back([shared_params]() {
        std::cout << "[Energy Analyzer] Waiting for parameters...\n";
        SimulationParams params = shared_params.get();
        
        double kinetic_energy = 0.5 * params.num_particles * 3.0 * 8.314 * params.temperature;
        std::cout << "[Energy Analyzer] Kinetic energy: " 
                  << std::scientific << std::setprecision(3) << kinetic_energy << " J\n";
    });
    
    // Stability analyzer
    analyzers.emplace_back([shared_params]() {
        std::cout << "[Stability Analyzer] Waiting for parameters...\n";
        SimulationParams params = shared_params.get();
        
        double max_timestep = 0.5 / (*std::max_element(
            params.force_constants.begin(), params.force_constants.end()));
        std::cout << "[Stability Analyzer] Max stable timestep: " 
                  << max_timestep << "s (current: " << params.timestep << "s)\n";
    });
    
    // Performance estimator
    analyzers.emplace_back([shared_params]() {
        std::cout << "[Performance Estimator] Waiting for parameters...\n";
        SimulationParams params = shared_params.get();
        
        double flops_per_step = params.num_particles * params.num_particles * 20.0;
        std::cout << "[Performance Estimator] FLOPS per timestep: " 
                  << std::scientific << std::setprecision(3) << flops_per_step << "\n";
    });
    
    // Load and set parameters
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "[Main] Loading simulation parameters...\n";
    
    SimulationParams params{
        0.001,    // 1 fs timestep
        298.15,   // Room temperature
        1000,     // Number of particles
        {100.0, 150.0, 200.0}  // Force constants
    };
    
    param_promise.set_value(params);
    
    for (auto& t : analyzers) {
        t.join();
    }
}

// Scientific computation future with progress tracking
template<typename T>
class ScientificFuture {
private:
    mutable std::mutex mutex_;
    mutable std::condition_variable cv_;
    std::shared_ptr<T> result_;
    std::exception_ptr exception_;
    bool ready_ = false;
    double progress_ = 0.0;  // Progress percentage
    std::string status_message_;
    
public:
    T get() const {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return ready_; });
        
        if (exception_) {
            std::rethrow_exception(exception_);
        }
        
        return *result_;
    }
    
    bool is_ready() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return ready_;
    }
    
    void wait() const {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return ready_; });
    }
    
    template<typename Rep, typename Period>
    bool wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, timeout, [this] { return ready_; });
    }
    
    // Progress tracking for long computations
    double get_progress() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return progress_;
    }
    
    std::string get_status() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return status_message_;
    }
    
    // Used by ScientificPromise
    void set_value(const T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ready_) {
            throw std::runtime_error("Result already set");
        }
        result_ = std::make_shared<T>(value);
        progress_ = 100.0;
        ready_ = true;
        cv_.notify_all();
    }
    
    void set_exception(std::exception_ptr e) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ready_) {
            throw std::runtime_error("Result already set");
        }
        exception_ = e;
        ready_ = true;
        cv_.notify_all();
    }
    
    void update_progress(double progress, const std::string& status = "") {
        std::lock_guard<std::mutex> lock(mutex_);
        progress_ = progress;
        if (!status.empty()) {
            status_message_ = status;
        }
    }
};

// Scientific computation pipeline with chaining
class ComputationPipeline {
private:
    using TransformFunc = std::function<std::vector<double>(const std::vector<double>&)>;
    std::vector<std::pair<std::string, TransformFunc>> stages_;
    
public:
    ComputationPipeline& add_stage(const std::string& name, TransformFunc transform) {
        stages_.push_back({name, transform});
        return *this;
    }
    
    std::future<std::vector<double>> execute(const std::vector<double>& initial_data) {
        return std::async(std::launch::async, [this, initial_data]() {
            std::vector<double> data = initial_data;
            
            for (const auto& [stage_name, transform] : stages_) {
                std::cout << "[Pipeline] Executing stage: " << stage_name 
                         << " (input size: " << data.size() << ")\n";
                
                auto start = std::chrono::high_resolution_clock::now();
                data = transform(data);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "[Pipeline] Stage completed in " << duration.count() << "ms\n";
            }
            
            return data;
        });
    }
};

// Parallel matrix-vector multiplication
std::future<std::vector<double>> parallel_matrix_vector_multiply(
    const std::vector<std::vector<double>>& matrix,
    const std::vector<double>& vector) {
    
    return std::async(std::launch::async, [matrix, vector]() {
        const size_t rows = matrix.size();
        const size_t cols = vector.size();
        
        if (rows == 0 || matrix[0].size() != cols) {
            throw std::invalid_argument("Matrix-vector dimension mismatch");
        }
        
        const size_t hardware_threads = std::thread::hardware_concurrency();
        const size_t num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, rows);
        const size_t rows_per_thread = rows / num_threads;
        
        std::cout << "[Matrix-Vector] Using " << num_threads << " threads for " 
                  << rows << "x" << cols << " multiplication\n";
        
        std::vector<std::future<std::vector<double>>> futures;
        
        for (size_t t = 0; t < num_threads; ++t) {
            size_t start_row = t * rows_per_thread;
            size_t end_row = (t == num_threads - 1) ? rows : (t + 1) * rows_per_thread;
            
            futures.push_back(std::async(std::launch::async,
                [&matrix, &vector, start_row, end_row]() {
                    std::vector<double> partial_result;
                    
                    for (size_t i = start_row; i < end_row; ++i) {
                        double sum = 0.0;
                        for (size_t j = 0; j < vector.size(); ++j) {
                            sum += matrix[i][j] * vector[j];
                        }
                        partial_result.push_back(sum);
                    }
                    
                    return partial_result;
                }));
        }
        
        // Combine results
        std::vector<double> result;
        result.reserve(rows);
        
        for (auto& future : futures) {
            auto partial = future.get();
            result.insert(result.end(), partial.begin(), partial.end());
        }
        
        return result;
    });
}

// Scientific computation queue with result promises
class ScientificComputationQueue {
private:
    struct Computation {
        std::function<double()> compute_func;
        std::promise<double> result_promise;
        std::string name;
        int priority;
    };
    
    // Priority queue for computations
    struct CompareComputation {
        bool operator()(const Computation& a, const Computation& b) {
            return a.priority < b.priority;
        }
    };
    
    std::priority_queue<Computation, std::vector<Computation>, CompareComputation> computations_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> completed_computations_{0};
    
    void worker_thread(int id) {
        while (!stop_) {
            Computation comp;
            
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return stop_ || !computations_.empty(); });
                
                if (stop_ && computations_.empty()) return;
                
                comp = std::move(const_cast<Computation&>(computations_.top()));
                computations_.pop();
            }
            
            std::cout << "[Worker " << id << "] Starting computation: " << comp.name << "\n";
            
            try {
                auto start = std::chrono::high_resolution_clock::now();
                double result = comp.compute_func();
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                std::cout << "[Worker " << id << "] Completed " << comp.name 
                         << " in " << duration.count() << "ms\n";
                
                comp.result_promise.set_value(result);
                completed_computations_++;
            } catch (...) {
                comp.result_promise.set_exception(std::current_exception());
            }
        }
    }
    
public:
    explicit ScientificComputationQueue(size_t num_workers = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < num_workers; ++i) {
            workers_.emplace_back(&ScientificComputationQueue::worker_thread, this, i);
        }
        std::cout << "[ComputationQueue] Started with " << num_workers << " workers\n";
    }
    
    ~ScientificComputationQueue() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        
        for (auto& worker : workers_) {
            worker.join();
        }
        
        std::cout << "[ComputationQueue] Completed " << completed_computations_ 
                  << " computations\n";
    }
    
    std::future<double> submit_computation(
        const std::string& name,
        std::function<double()> compute_func,
        int priority = 5) {
        
        Computation comp;
        comp.name = name;
        comp.compute_func = std::move(compute_func);
        comp.priority = priority;
        auto future = comp.result_promise.get_future();
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            computations_.push(std::move(comp));
        }
        
        cv_.notify_one();
        return future;
    }
};

// Combine multiple simulation results
template<typename... Futures>
auto combine_simulation_results(Futures... futures) {
    return std::async(std::launch::async, [](auto... futs) {
        std::cout << "[Combiner] Waiting for all simulation results...\n";
        auto results = std::make_tuple(futs.get()...);
        std::cout << "[Combiner] All results collected\n";
        return results;
    }, std::move(futures)...);
}

// First converged solver wins
std::future<std::pair<std::string, double>> first_converged_solver(
    std::vector<std::pair<std::string, std::future<double>>>& solver_futures) {
    
    auto promise = std::make_shared<std::promise<std::pair<std::string, double>>>();
    auto result_future = promise->get_future();
    auto first_set = std::make_shared<std::atomic<bool>>(false);
    
    for (auto& [solver_name, future] : solver_futures) {
        std::thread([promise, first_set, solver_name](std::future<double> f) {
            try {
                double value = f.get();
                bool expected = false;
                if (first_set->compare_exchange_strong(expected, true)) {
                    promise->set_value({solver_name, value});
                    std::cout << "[FirstConverged] " << solver_name << " wins!\n";
                }
            } catch (...) {
                // Solver failed to converge
            }
        }, std::move(future)).detach();
    }
    
    return result_future;
}

// Examples
void chaining_example() {
    std::cout << "\n=== Task Chaining Example ===\n";
    
    TaskChain chain;
    auto future = chain
        .then([](int x) { return x * 2; })
        .then([](int x) { return x + 10; })
        .then([](int x) { return x / 3; })
        .execute(15);
    
    std::cout << "Final result: " << future.get() << "\n";
}

void parallel_computation_example() {
    std::cout << "\n=== Parallel Computation Example ===\n";
    
    std::vector<int> numbers(1000000);
    std::iota(numbers.begin(), numbers.end(), 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    long long sum = parallel_accumulate(numbers.begin(), numbers.end(), 0LL);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sum of 1 to " << numbers.size() << " = " << sum << "\n";
    std::cout << "Time taken: " << duration.count() << "ms\n";
}

void task_queue_example() {
    std::cout << "\n=== Promise Task Queue Example ===\n";
    
    PromiseTaskQueue queue;
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 5; ++i) {
        futures.push_back(queue.submit([i]() {
            std::cout << "Task " << i << " executing on thread " 
                      << std::this_thread::get_id() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }));
    }
    
    // Wait for all tasks
    for (auto& future : futures) {
        future.wait();
    }
    
    std::cout << "All tasks completed\n";
}

void combinator_example() {
    std::cout << "\n=== Future Combinator Example ===\n";
    
    // when_all example
    auto f1 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 10;
    });
    
    auto f2 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return 20;
    });
    
    auto f3 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        return 30;
    });
    
    auto all_future = when_all(std::move(f1), std::move(f2), std::move(f3));
    auto [r1, r2, r3] = all_future.get();
    
    std::cout << "when_all results: " << r1 << ", " << r2 << ", " << r3 << "\n";
    std::cout << "Sum: " << (r1 + r2 + r3) << "\n";
}

int main() {
    std::cout << "=== Future-Promise Pattern Demo ===\n\n";
    
    basic_example();
    async_example();
    exception_example();
    shared_future_example();
    chaining_example();
    parallel_computation_example();
    task_queue_example();
    combinator_example();
    
    return 0;
}