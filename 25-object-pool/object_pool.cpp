// Object Pool Pattern - High-Performance Scientific Computing Resource Management
// Efficient reuse of expensive computational resources in HPC environments
#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>
#include <cstring>
#include <complex>
#include <numeric>
#include <algorithm>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <iomanip>

// GPU Compute Context for Scientific Computing
class GPUComputeContext {
private:
    static int contextCounter_;
    int id_;
    int deviceId_;
    bool allocated_ = false;
    size_t memorySize_;
    void* deviceMemory_ = nullptr;
    
public:
    GPUComputeContext(int deviceId, size_t memSize) 
        : id_(++contextCounter_), deviceId_(deviceId), memorySize_(memSize) {
        std::cout << "Creating GPU compute context #" << id_ 
                  << " on device " << deviceId_ 
                  << " with " << (memorySize_ / (1024*1024)) << " MB\n";
        // Simulate expensive GPU initialization
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        allocated_ = true;
    }
    
    ~GPUComputeContext() {
        std::cout << "Destroying GPU compute context #" << id_ << "\n";
        if (deviceMemory_) {
            // Simulate GPU memory cleanup
            deviceMemory_ = nullptr;
        }
    }
    
    void runKernel(const std::string& kernelName, int gridSize, int blockSize) {
        std::cout << "[GPU Context #" << id_ << "] Launching kernel: " 
                  << kernelName << " with grid(" << gridSize << ") block(" 
                  << blockSize << ")\n";
        // Simulate kernel execution
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    void matrixMultiply(int n) {
        std::cout << "[GPU Context #" << id_ << "] Matrix multiply " 
                  << n << "x" << n << "\n";
        int gridSize = (n + 15) / 16;
        runKernel("matmul_kernel", gridSize, 256);
    }
    
    void reset() {
        std::cout << "[GPU Context #" << id_ << "] Clearing GPU memory\n";
        // Clear any cached data
        deviceMemory_ = nullptr;
    }
    
    int getId() const { return id_; }
    int getDeviceId() const { return deviceId_; }
    size_t getMemorySize() const { return memorySize_; }
};

int GPUComputeContext::contextCounter_ = 0;

// Thread-safe Object Pool
template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> available_;
    std::vector<T*> inUse_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    
    size_t maxSize_;
    size_t currentSize_ = 0;
    std::function<std::unique_ptr<T>()> factory_;
    std::function<void(T*)> reset_;
    
public:
    ObjectPool(size_t maxSize, 
               std::function<std::unique_ptr<T>()> factory,
               std::function<void(T*)> reset = [](T* obj) { obj->reset(); })
        : maxSize_(maxSize), factory_(factory), reset_(reset) {}
    
    ~ObjectPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        // Clean up in-use objects
        for (auto* obj : inUse_) {
            delete obj;
        }
    }
    
    std::unique_ptr<T, std::function<void(T*)>> acquire() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait if pool is empty and we've reached max size
        while (available_.empty() && currentSize_ >= maxSize_) {
            std::cout << "Pool exhausted, waiting for available object...\n";
            condition_.wait(lock);
        }
        
        // Create new object if pool is empty
        if (available_.empty()) {
            currentSize_++;
            auto obj = factory_();
            T* rawPtr = obj.release();
            inUse_.push_back(rawPtr);
            
            // Return with custom deleter that returns to pool
            return std::unique_ptr<T, std::function<void(T*)>>(
                rawPtr,
                [this](T* ptr) { this->release(ptr); }
            );
        }
        
        // Get object from pool
        auto obj = std::move(available_.front());
        available_.pop();
        T* rawPtr = obj.release();
        inUse_.push_back(rawPtr);
        
        return std::unique_ptr<T, std::function<void(T*)>>(
            rawPtr,
            [this](T* ptr) { this->release(ptr); }
        );
    }
    
    void release(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Remove from in-use list
        auto it = std::find(inUse_.begin(), inUse_.end(), obj);
        if (it != inUse_.end()) {
            inUse_.erase(it);
        }
        
        // Reset object state
        reset_(obj);
        
        // Return to pool
        available_.push(std::unique_ptr<T>(obj));
        condition_.notify_one();
        
        std::cout << "Object returned to pool. Available: " 
                  << available_.size() << "/" << currentSize_ << "\n";
    }
    
    size_t availableCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return available_.size();
    }
    
    size_t inUseCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return inUse_.size();
    }
    
    size_t totalCount() const {
        return currentSize_;
    }
};

// FFT Processor for Signal Processing
class FFTProcessor {
private:
    static int processorCounter_;
    int id_;
    size_t fftSize_;
    bool busy_ = false;
    std::vector<std::complex<double>> workspace_;
    std::thread thread_;
    
public:
    FFTProcessor(size_t fftSize) 
        : id_(++processorCounter_), fftSize_(fftSize), workspace_(fftSize) {
        std::cout << "Creating FFT processor #" << id_ 
                  << " for size " << fftSize_ << "\n";
        // Pre-compute twiddle factors
        initializeTwiddleFactors();
    }
    
    ~FFTProcessor() {
        if (thread_.joinable()) {
            thread_.join();
        }
        std::cout << "Destroying FFT processor #" << id_ << "\n";
    }
    
    void processSignal(const std::vector<double>& signal, 
                      std::function<void(std::vector<std::complex<double>>)> callback) {
        if (thread_.joinable()) {
            thread_.join();
        }
        
        busy_ = true;
        thread_ = std::thread([this, signal, callback]() {
            std::cout << "[FFT Processor #" << id_ << "] Processing signal of length " 
                      << signal.size() << "\n";
            
            // Convert to complex
            for (size_t i = 0; i < std::min(signal.size(), fftSize_); ++i) {
                workspace_[i] = std::complex<double>(signal[i], 0.0);
            }
            
            // Simulate FFT computation
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            performFFT();
            
            callback(workspace_);
            std::cout << "[FFT Processor #" << id_ << "] Signal processed\n";
            busy_ = false;
        });
    }
    
    void reset() {
        if (thread_.joinable()) {
            thread_.join();
        }
        std::fill(workspace_.begin(), workspace_.end(), std::complex<double>(0, 0));
        busy_ = false;
    }
    
    bool isBusy() const { return busy_; }
    int getId() const { return id_; }
    size_t getFFTSize() const { return fftSize_; }
    
private:
    void initializeTwiddleFactors() {
        // Pre-compute twiddle factors for FFT
        std::cout << "[FFT Processor #" << id_ 
                  << "] Initializing twiddle factors\n";
    }
    
    void performFFT() {
        // Simplified FFT (normally would use Cooley-Tukey or similar)
        for (size_t i = 0; i < fftSize_; ++i) {
            workspace_[i] *= std::exp(std::complex<double>(0, -2 * M_PI * i / fftSize_));
        }
    }
};

int FFTProcessor::processorCounter_ = 0;

// Reusable Matrix Decomposition Objects
class MatrixDecomposition {
private:
    static int decompositionCounter_;
    int id_;
    size_t size_;
    std::vector<double> L_;  // Lower triangular
    std::vector<double> U_;  // Upper triangular
    std::vector<int> P_;     // Permutation
    bool decomposed_ = false;
    std::string method_ = "LU";
    
public:
    MatrixDecomposition(size_t matrixSize) 
        : id_(++decompositionCounter_), size_(matrixSize),
          L_(size_ * size_), U_(size_ * size_), P_(size_) {
        std::cout << "Creating matrix decomposition object #" << id_ 
                  << " for " << size_ << "x" << size_ << " matrices\n";
    }
    
    void decompose(const std::vector<double>& matrix, const std::string& method) {
        method_ = method;
        decomposed_ = true;
        
        std::cout << "[Decomposition #" << id_ << "] Performing " << method 
                  << " decomposition on " << size_ << "x" << size_ << " matrix\n";
        
        if (method == "LU") {
            performLUDecomposition(matrix);
        } else if (method == "Cholesky") {
            performCholeskyDecomposition(matrix);
        } else if (method == "QR") {
            performQRDecomposition(matrix);
        }
    }
    
    std::vector<double> solve(const std::vector<double>& b) {
        if (!decomposed_) {
            throw std::runtime_error("Matrix not decomposed");
        }
        
        std::cout << "[Decomposition #" << id_ << "] Solving linear system\n";
        
        // Forward substitution Ly = Pb
        std::vector<double> y(size_);
        forwardSubstitution(y, b);
        
        // Back substitution Ux = y
        std::vector<double> x(size_);
        backSubstitution(x, y);
        
        return x;
    }
    
    void reset() {
        std::fill(L_.begin(), L_.end(), 0.0);
        std::fill(U_.begin(), U_.end(), 0.0);
        std::fill(P_.begin(), P_.end(), 0);
        decomposed_ = false;
        method_ = "LU";
    }
    
    bool isDecomposed() const { return decomposed_; }
    int getId() const { return id_; }
    size_t getSize() const { return size_; }
    
private:
    void performLUDecomposition(const std::vector<double>& matrix) {
        // Simplified LU decomposition
        std::copy(matrix.begin(), matrix.end(), U_.begin());
        for (size_t i = 0; i < size_; ++i) {
            L_[i * size_ + i] = 1.0;
            P_[i] = i;
        }
    }
    
    void performCholeskyDecomposition(const std::vector<double>& matrix) {
        // Simplified Cholesky for positive definite matrices
        std::cout << "  Using Cholesky for symmetric positive definite matrix\n";
    }
    
    void performQRDecomposition(const std::vector<double>& matrix) {
        // Simplified QR decomposition
        std::cout << "  Using QR decomposition via Gram-Schmidt\n";
    }
    
    void forwardSubstitution(std::vector<double>& y, const std::vector<double>& b) {
        for (size_t i = 0; i < size_; ++i) {
            y[i] = b[P_[i]];
            for (size_t j = 0; j < i; ++j) {
                y[i] -= L_[i * size_ + j] * y[j];
            }
        }
    }
    
    void backSubstitution(std::vector<double>& x, const std::vector<double>& y) {
        for (int i = size_ - 1; i >= 0; --i) {
            x[i] = y[i];
            for (size_t j = i + 1; j < size_; ++j) {
                x[i] -= U_[i * size_ + j] * x[j];
            }
            x[i] /= U_[i * size_ + i];
        }
    }
};

int MatrixDecomposition::decompositionCounter_ = 0;

// Scientific Data Buffer for Simulation Results
class SimulationBuffer {
private:
    static int bufferCounter_;
    int id_;
    std::vector<double> data_;
    size_t size_;
    size_t timesteps_ = 0;
    std::string dataType_;
    
public:
    SimulationBuffer(size_t size) 
        : id_(++bufferCounter_), size_(size), data_(size) {
        std::cout << "Allocating simulation buffer #" << id_ 
                  << " for " << size << " doubles (" 
                  << (size * sizeof(double) / 1024) << " KB)\n";
    }
    
    ~SimulationBuffer() {
        std::cout << "Deallocating simulation buffer #" << id_ << "\n";
    }
    
    void storeTimestep(const std::vector<double>& state, const std::string& type) {
        dataType_ = type;
        size_t toStore = std::min(state.size(), size_ - timesteps_);
        
        std::copy(state.begin(), state.begin() + toStore, 
                  data_.begin() + timesteps_);
        timesteps_ += toStore;
        
        std::cout << "[Buffer #" << id_ << "] Stored " << type 
                  << " timestep. Used: " << timesteps_ << "/" << size_ << "\n";
    }
    
    void computeStatistics() {
        if (timesteps_ == 0) return;
        
        double sum = 0.0, min = data_[0], max = data_[0];
        for (size_t i = 0; i < timesteps_; ++i) {
            sum += data_[i];
            min = std::min(min, data_[i]);
            max = std::max(max, data_[i]);
        }
        
        std::cout << "[Buffer #" << id_ << "] Statistics for " << dataType_ << ":\n";
        std::cout << "  Min: " << min << ", Max: " << max 
                  << ", Avg: " << (sum / timesteps_) << "\n";
    }
    
    void reset() {
        timesteps_ = 0;
        std::fill(data_.begin(), data_.end(), 0.0);
        dataType_.clear();
    }
    
    size_t getSize() const { return size_; }
    size_t getTimesteps() const { return timesteps_; }
    int getId() const { return id_; }
};

int SimulationBuffer::bufferCounter_ = 0;

// Solver Pool Manager for Different Problem Sizes
class SolverPoolManager {
private:
    std::unordered_map<size_t, std::unique_ptr<ObjectPool<MatrixDecomposition>>> decompositionPools_;
    std::unordered_map<size_t, std::unique_ptr<ObjectPool<FFTProcessor>>> fftPools_;
    
public:
    std::unique_ptr<MatrixDecomposition, std::function<void(MatrixDecomposition*)>> 
    getDecomposition(size_t matrixSize) {
        // Round up to standard sizes
        size_t poolSize = 64;
        while (poolSize < matrixSize && poolSize < 4096) poolSize *= 2;
        
        // Create pool if it doesn't exist
        if (decompositionPools_.find(poolSize) == decompositionPools_.end()) {
            decompositionPools_[poolSize] = std::make_unique<ObjectPool<MatrixDecomposition>>(
                4,  // max 4 decompositions per size
                [poolSize]() { return std::make_unique<MatrixDecomposition>(poolSize); }
            );
        }
        
        std::cout << "Requesting decomposition for " << matrixSize 
                  << "x" << matrixSize << " matrix (using pool size " 
                  << poolSize << ")\n";
        return decompositionPools_[poolSize]->acquire();
    }
    
    std::unique_ptr<FFTProcessor, std::function<void(FFTProcessor*)>> 
    getFFTProcessor(size_t fftSize) {
        // Round up to power of 2
        size_t poolSize = 1;
        while (poolSize < fftSize) poolSize <<= 1;
        
        if (fftPools_.find(poolSize) == fftPools_.end()) {
            fftPools_[poolSize] = std::make_unique<ObjectPool<FFTProcessor>>(
                3,  // max 3 FFT processors per size
                [poolSize]() { return std::make_unique<FFTProcessor>(poolSize); }
            );
        }
        
        std::cout << "Requesting FFT processor for size " << fftSize 
                  << " (using pool size " << poolSize << ")\n";
        return fftPools_[poolSize]->acquire();
    }
    
    void showStatistics() {
        std::cout << "\n=== Solver Pool Statistics ===\n";
        
        std::cout << "Matrix Decomposition Pools:\n";
        for (const auto& pair : decompositionPools_) {
            std::cout << "  Size " << pair.first << "x" << pair.first << ": "
                      << pair.second->inUseCount() << " in use, "
                      << pair.second->availableCount() << " available\n";
        }
        
        std::cout << "FFT Processor Pools:\n";
        for (const auto& pair : fftPools_) {
            std::cout << "  Size " << pair.first << ": "
                      << pair.second->inUseCount() << " in use, "
                      << pair.second->availableCount() << " available\n";
        }
    }
};

int main() {
    std::cout << "=== High-Performance Scientific Computing Resource Pools ===\n\n";
    
    // GPU Compute Context Pool
    std::cout << "=== GPU Compute Context Pool ===\n\n";
    
    ObjectPool<GPUComputeContext> gpuPool(
        2,  // max 2 GPU contexts (limited GPU resources)
        []() { return std::make_unique<GPUComputeContext>(0, 4ULL * 1024 * 1024 * 1024); }
    );
    
    // Simulate multiple scientific computations
    std::vector<std::thread> gpuTasks;
    
    for (int i = 0; i < 4; ++i) {
        gpuTasks.emplace_back([&gpuPool, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(i * 50));
            
            std::cout << "\nComputation " << i << " requesting GPU context...\n";
            auto gpu = gpuPool.acquire();
            
            // Perform matrix operations
            gpu->matrixMultiply(1024 + i * 256);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            gpu->runKernel("fft_radix2", 512, 128);
            
            std::cout << "Computation " << i << " releasing GPU context\n";
            // GPU context automatically returned to pool
        });
    }
    
    for (auto& task : gpuTasks) {
        task.join();
    }
    
    std::cout << "\nFinal GPU pool state: " << gpuPool.availableCount() 
              << " available contexts\n";
    
    // FFT Processor Pool Example
    std::cout << "\n\n=== FFT Processor Pool ===\n\n";
    
    ObjectPool<FFTProcessor> fftPool(
        2,  // max 2 FFT processors
        []() { return std::make_unique<FFTProcessor>(1024); }
    );
    
    // Submit signal processing tasks
    std::vector<std::thread> signalTasks;
    
    for (int i = 0; i < 4; ++i) {
        signalTasks.emplace_back([&fftPool, i]() {
            std::cout << "Signal " << i << " requesting FFT processor...\n";
            auto fft = fftPool.acquire();
            
            // Generate test signal
            std::vector<double> signal(1024);
            for (size_t j = 0; j < signal.size(); ++j) {
                signal[j] = std::sin(2 * M_PI * (10 + i * 5) * j / 1024.0) + 
                           0.5 * std::sin(2 * M_PI * 50 * j / 1024.0);
            }
            
            fft->processSignal(signal, [i](const std::vector<std::complex<double>>& spectrum) {
                std::cout << "  Signal " << i << " spectrum computed. ";
                // Find dominant frequency
                double maxMag = 0;
                size_t maxIdx = 0;
                for (size_t k = 0; k < spectrum.size() / 2; ++k) {
                    double mag = std::abs(spectrum[k]);
                    if (mag > maxMag) {
                        maxMag = mag;
                        maxIdx = k;
                    }
                }
                std::cout << "Peak at bin " << maxIdx << "\n";
            });
            
            // Wait for processing to complete
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    
    for (auto& task : signalTasks) {
        task.join();
    }
    
    // Matrix Decomposition Pool Example
    std::cout << "\n\n=== Matrix Decomposition Pool ===\n\n";
    
    ObjectPool<MatrixDecomposition> decompositionPool(
        3,  // max 3 decomposition objects
        []() { return std::make_unique<MatrixDecomposition>(100); }
    );
    
    std::cout << "Solving multiple linear systems...\n";
    
    // Create multiple linear systems to solve
    std::vector<std::unique_ptr<MatrixDecomposition, 
                               std::function<void(MatrixDecomposition*)>>> solvers;
    
    for (int i = 0; i < 5; ++i) {
        std::cout << "\nSystem " << i << " requesting decomposition object...\n";
        auto decomp = decompositionPool.acquire();
        
        // Create test matrix (diagonally dominant for stability)
        std::vector<double> matrix(100 * 100);
        for (int j = 0; j < 100; ++j) {
            matrix[j * 100 + j] = 10.0 + i;  // Diagonal
            if (j > 0) matrix[j * 100 + j - 1] = -1.0;  // Sub-diagonal
            if (j < 99) matrix[j * 100 + j + 1] = -1.0;  // Super-diagonal
        }
        
        // Decompose and solve
        decomp->decompose(matrix, i % 2 == 0 ? "LU" : "Cholesky");
        
        std::vector<double> b(100, 1.0);  // RHS vector
        auto solution = decomp->solve(b);
        
        std::cout << "System " << i << " solved. Solution norm: " 
                  << std::sqrt(std::inner_product(solution.begin(), 
                                                 solution.end(), 
                                                 solution.begin(), 0.0)) << "\n";
        
        if (i < 3) {
            solvers.push_back(std::move(decomp));
        }
        // Others will be automatically returned
    }
    
    std::cout << "\nDecompositions in use: " << decompositionPool.inUseCount() << "\n";
    std::cout << "Pool available: " << decompositionPool.availableCount() << "\n";
    
    // Simulation Buffer Pool Example
    std::cout << "\n\n=== Simulation Buffer Pool ===\n\n";
    
    ObjectPool<SimulationBuffer> bufferPool(
        4,  // max 4 buffers
        []() { return std::make_unique<SimulationBuffer>(10000); }
    );
    
    // Simulate different simulation outputs
    auto tempBuffer = bufferPool.acquire();
    auto pressureBuffer = bufferPool.acquire();
    auto velocityBuffer = bufferPool.acquire();
    
    // Store simulation timesteps
    for (int t = 0; t < 5; ++t) {
        std::vector<double> tempData(100);
        std::vector<double> pressureData(100);
        std::vector<double> velocityData(100);
        
        // Generate synthetic data
        for (int i = 0; i < 100; ++i) {
            tempData[i] = 300.0 + 10.0 * std::sin(2 * M_PI * i / 100.0) * (t + 1);
            pressureData[i] = 101325.0 + 1000.0 * std::cos(2 * M_PI * i / 100.0) * t;
            velocityData[i] = 5.0 * std::exp(-0.1 * t) * std::sin(4 * M_PI * i / 100.0);
        }
        
        tempBuffer->storeTimestep(tempData, "Temperature");
        pressureBuffer->storeTimestep(pressureData, "Pressure");
        velocityBuffer->storeTimestep(velocityData, "Velocity");
    }
    
    // Compute statistics
    std::cout << "\nSimulation statistics:\n";
    tempBuffer->computeStatistics();
    pressureBuffer->computeStatistics();
    velocityBuffer->computeStatistics();
    
    // Release one buffer
    tempBuffer.reset();
    
    // Get another buffer for different data
    auto densityBuffer = bufferPool.acquire();  // Should get recycled buffer
    std::vector<double> densityData(100, 1.225);  // Standard air density
    densityBuffer->storeTimestep(densityData, "Density");
    
    // Combined Solver Pool Manager
    std::cout << "\n\n=== Solver Pool Manager ===\n\n";
    
    SolverPoolManager solverManager;
    
    // Request various solver resources
    auto decomp1 = solverManager.getDecomposition(50);
    auto decomp2 = solverManager.getDecomposition(200);
    auto fft1 = solverManager.getFFTProcessor(512);
    auto fft2 = solverManager.getFFTProcessor(2048);
    
    solverManager.showStatistics();
    
    std::cout << "\n=== Object Pool Pattern Summary ===\n";
    std::cout << "Resource pooling in scientific computing provides:\n";
    std::cout << "• Reduced allocation overhead for expensive resources\n";
    std::cout << "• Better GPU/accelerator utilization\n";
    std::cout << "• Predictable memory usage patterns\n";
    std::cout << "• Improved cache locality for frequently reused objects\n";
    
    return 0;
}