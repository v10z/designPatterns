// Monitor Object Pattern - Thread-Safe Scientific Computing Resources
// Synchronized access to shared computational resources and data structures
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>
#include <chrono>
#include <random>
#include <iomanip>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <memory>
#include <functional>
#include <unordered_map>
#include <atomic>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Scientific data structure for simulation results
struct SimulationResult {
    int timestep;
    double energy;
    double temperature;
    double pressure;
    std::vector<double> state;
    
    SimulationResult(int t, double e, double temp, double p, const std::vector<double>& s)
        : timestep(t), energy(e), temperature(temp), pressure(p), state(s) {}
};

// Monitor Object - Thread-safe Simulation Data Buffer
class SimulationDataBuffer {
private:
    std::queue<SimulationResult> buffer_;
    const size_t capacity_;
    size_t totalProduced_ = 0;
    size_t totalConsumed_ = 0;
    
    // Monitor synchronization primitives
    mutable std::mutex monitor_lock_;
    std::condition_variable data_available_;
    std::condition_variable space_available_;
    
public:
    explicit SimulationDataBuffer(size_t capacity) : capacity_(capacity) {}
    
    // Synchronized method - store simulation result
    void storeResult(const SimulationResult& result) {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        // Wait while buffer is full
        space_available_.wait(lock, [this] { return buffer_.size() < capacity_; });
        
        buffer_.push(result);
        totalProduced_++;
        
        std::cout << "[Simulator-" << std::this_thread::get_id() << "] "
                  << "Stored timestep " << result.timestep 
                  << " (E=" << std::scientific << std::setprecision(3) << result.energy
                  << ", buffer size: " << buffer_.size() << ")\n";
        
        // Signal that data is available
        data_available_.notify_one();
    }
    
    // Synchronized method - retrieve simulation result
    SimulationResult retrieveResult() {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        // Wait while buffer is empty
        data_available_.wait(lock, [this] { return !buffer_.empty(); });
        
        SimulationResult result = buffer_.front();
        buffer_.pop();
        totalConsumed_++;
        
        std::cout << "[Analyzer-" << std::this_thread::get_id() << "] "
                  << "Retrieved timestep " << result.timestep
                  << " (buffer size: " << buffer_.size() << ")\n";
        
        // Signal that space is available
        space_available_.notify_one();
        
        return result;
    }
    
    // Synchronized method - check if empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(monitor_lock_);
        return buffer_.empty();
    }
    
    // Synchronized method - get statistics
    std::pair<size_t, size_t> getStatistics() const {
        std::lock_guard<std::mutex> lock(monitor_lock_);
        return {totalProduced_, totalConsumed_};
    }
};

// Monitor Object - Thread-safe Computational Resource Manager
class ComputationalResourceManager {
private:
    double availableMemoryGB_;
    const double totalMemoryGB_;
    int availableGPUs_;
    const int totalGPUs_;
    std::string resourceName_;
    std::queue<std::pair<std::string, double>> allocationLog_;
    
    // Monitor synchronization
    mutable std::mutex monitor_lock_;
    std::condition_variable resources_available_;
    
public:
    ComputationalResourceManager(const std::string& name, double totalMemoryGB, int totalGPUs)
        : resourceName_(name), 
          availableMemoryGB_(totalMemoryGB),
          totalMemoryGB_(totalMemoryGB),
          availableGPUs_(totalGPUs),
          totalGPUs_(totalGPUs) {}
    
    // Synchronized method - allocate resources with wait
    void allocateResources(double memoryGB, int gpus, const std::string& taskName) {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        // Wait until sufficient resources available
        resources_available_.wait(lock, [this, memoryGB, gpus] {
            return availableMemoryGB_ >= memoryGB && availableGPUs_ >= gpus;
        });
        
        availableMemoryGB_ -= memoryGB;
        availableGPUs_ -= gpus;
        allocationLog_.push({taskName, memoryGB});
        
        std::cout << "[" << resourceName_ << "] Allocated to " << taskName << ": " 
                  << std::fixed << std::setprecision(2) << memoryGB << " GB, "
                  << gpus << " GPU(s) | Available: " << availableMemoryGB_ 
                  << " GB, " << availableGPUs_ << " GPU(s)\n";
    }
    
    // Synchronized method - try allocate without wait
    bool tryAllocateResources(double memoryGB, int gpus, const std::string& taskName) {
        std::lock_guard<std::mutex> lock(monitor_lock_);
        
        if (availableMemoryGB_ >= memoryGB && availableGPUs_ >= gpus) {
            availableMemoryGB_ -= memoryGB;
            availableGPUs_ -= gpus;
            allocationLog_.push({taskName, memoryGB});
            
            std::cout << "[" << resourceName_ << "] Allocated to " << taskName << ": " 
                      << std::fixed << std::setprecision(2) << memoryGB << " GB, "
                      << gpus << " GPU(s)\n";
            return true;
        }
        
        std::cout << "[" << resourceName_ << "] Insufficient resources for " 
                  << taskName << " (needs " << memoryGB << " GB, " << gpus << " GPU(s))\n";
        return false;
    }
    
    // Synchronized method - release resources
    void releaseResources(double memoryGB, int gpus, const std::string& taskName) {
        {
            std::lock_guard<std::mutex> lock(monitor_lock_);
            availableMemoryGB_ = std::min(availableMemoryGB_ + memoryGB, totalMemoryGB_);
            availableGPUs_ = std::min(availableGPUs_ + gpus, totalGPUs_);
            
            std::cout << "[" << resourceName_ << "] Released by " << taskName << ": " 
                      << std::fixed << std::setprecision(2) << memoryGB << " GB, "
                      << gpus << " GPU(s) | Available: " << availableMemoryGB_ 
                      << " GB, " << availableGPUs_ << " GPU(s)\n";
        }
        
        // Notify waiting threads
        resources_available_.notify_all();
    }
    
    // Synchronized method - get resource status
    std::pair<double, int> getAvailableResources() const {
        std::lock_guard<std::mutex> lock(monitor_lock_);
        return {availableMemoryGB_, availableGPUs_};
    }
    
    // Synchronized method - transfer resources between managers
    void transferResources(ComputationalResourceManager& to, double memoryGB, int gpus) {
        // Lock both managers in consistent order to avoid deadlock
        if (this < &to) {
            std::lock_guard<std::mutex> lock1(monitor_lock_);
            std::lock_guard<std::mutex> lock2(to.monitor_lock_);
            performTransfer(to, memoryGB, gpus);
        } else {
            std::lock_guard<std::mutex> lock1(to.monitor_lock_);
            std::lock_guard<std::mutex> lock2(monitor_lock_);
            performTransfer(to, memoryGB, gpus);
        }
    }
    
private:
    void performTransfer(ComputationalResourceManager& to, double memoryGB, int gpus) {
        if (availableMemoryGB_ >= memoryGB && availableGPUs_ >= gpus &&
            to.availableMemoryGB_ + memoryGB <= to.totalMemoryGB_ &&
            to.availableGPUs_ + gpus <= to.totalGPUs_) {
            
            availableMemoryGB_ -= memoryGB;
            availableGPUs_ -= gpus;
            to.availableMemoryGB_ += memoryGB;
            to.availableGPUs_ += gpus;
            
            std::cout << "Transfer: " << memoryGB << " GB, " << gpus << " GPU(s) from " 
                      << resourceName_ << " to " << to.resourceName_ << "\n";
        } else {
            std::cout << "Transfer failed: Insufficient or exceeding resources\n";
        }
    }
};

// Monitor Object - Scientific Data Grid Read/Write Synchronization
class ScientificDataGrid {
private:
    std::vector<std::vector<double>> grid_;
    int rows_;
    int cols_;
    int active_readers_ = 0;
    int waiting_readers_ = 0;
    int active_writers_ = 0;
    int waiting_writers_ = 0;
    double lastComputedNorm_ = 0.0;
    
    mutable std::mutex monitor_lock_;
    std::condition_variable readers_can_proceed_;
    std::condition_variable writers_can_proceed_;
    
public:
    ScientificDataGrid(int rows, int cols) 
        : rows_(rows), cols_(cols), grid_(rows, std::vector<double>(cols, 0.0)) {}
    
    // Synchronized method - acquire read access for computation
    void beginRead() {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        waiting_readers_++;
        
        // Wait while writers are active or waiting (writer preference)
        readers_can_proceed_.wait(lock, [this] {
            return active_writers_ == 0 && waiting_writers_ == 0;
        });
        
        waiting_readers_--;
        active_readers_++;
        
        std::cout << "[DataReader-" << std::this_thread::get_id() << "] "
                  << "Acquired read access (active readers: " << active_readers_ << ")\n";
    }
    
    // Synchronized method - release read access
    void endRead() {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        active_readers_--;
        std::cout << "[DataReader-" << std::this_thread::get_id() << "] "
                  << "Released read access (active readers: " << active_readers_ << ")\n";
        
        // If no more readers, notify writers
        if (active_readers_ == 0) {
            writers_can_proceed_.notify_one();
        }
    }
    
    // Synchronized method - acquire write access for updates
    void beginWrite() {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        waiting_writers_++;
        
        // Wait while readers or writers are active
        writers_can_proceed_.wait(lock, [this] {
            return active_readers_ == 0 && active_writers_ == 0;
        });
        
        waiting_writers_--;
        active_writers_++;
        
        std::cout << "[DataWriter-" << std::this_thread::get_id() << "] "
                  << "Acquired write access\n";
    }
    
    // Synchronized method - release write access
    void endWrite() {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        active_writers_--;
        std::cout << "[DataWriter-" << std::this_thread::get_id() << "] "
                  << "Released write access\n";
        
        // Prefer writers if any are waiting
        if (waiting_writers_ > 0) {
            writers_can_proceed_.notify_one();
        } else {
            readers_can_proceed_.notify_all();
        }
    }
    
    // Non-synchronized computational methods (must call beginRead/endRead)
    double computeFrobeniusNorm() const {
        double sum = 0.0;
        for (const auto& row : grid_) {
            for (double val : row) {
                sum += val * val;
            }
        }
        return std::sqrt(sum);
    }
    
    double computeSpectralRadius() const {
        // Simplified power iteration for largest eigenvalue
        std::vector<double> v(cols_, 1.0);
        for (int iter = 0; iter < 10; ++iter) {
            std::vector<double> Av(rows_, 0.0);
            for (int i = 0; i < rows_; ++i) {
                for (int j = 0; j < cols_; ++j) {
                    Av[i] += grid_[i][j] * v[j];
                }
            }
            double norm = std::sqrt(std::inner_product(Av.begin(), Av.end(), Av.begin(), 0.0));
            for (double& val : Av) val /= norm;
            v = Av;
        }
        return lastComputedNorm_;
    }
    
    // Non-synchronized update methods (must call beginWrite/endWrite)
    void applyStencilOperation(std::function<double(int, int)> stencil) {
        for (int i = 1; i < rows_ - 1; ++i) {
            for (int j = 1; j < cols_ - 1; ++j) {
                grid_[i][j] = stencil(i, j);
            }
        }
    }
    
    void updateElement(int row, int col, double value) {
        if (row >= 0 && row < rows_ && col >= 0 && col < cols_) {
            grid_[row][col] = value;
        }
    }
    
    double getElement(int row, int col) const {
        if (row >= 0 && row < rows_ && col >= 0 && col < cols_) {
            return grid_[row][col];
        }
        return 0.0;
    }
    
    // RAII wrapper for read access
    class ReadAccess {
    private:
        ScientificDataGrid& grid_;
    public:
        explicit ReadAccess(ScientificDataGrid& grid) : grid_(grid) {
            grid_.beginRead();
        }
        ~ReadAccess() { grid_.endRead(); }
    };
    
    // RAII wrapper for write access
    class WriteAccess {
    private:
        ScientificDataGrid& grid_;
    public:
        explicit WriteAccess(ScientificDataGrid& grid) : grid_(grid) {
            grid_.beginWrite();
        }
        ~WriteAccess() { grid_.endWrite(); }
    };
};

// Monitor Object - Parallel Computation License Manager
class ComputationLicenseManager {
private:
    int availableLicenses_;
    const int totalLicenses_;
    std::unordered_map<std::thread::id, std::pair<std::string, int>> activeLicenses_;
    std::chrono::steady_clock::time_point startTime_;
    
    mutable std::mutex monitor_lock_;
    std::condition_variable licenses_available_;
    
public:
    explicit ComputationLicenseManager(int totalLicenses)
        : availableLicenses_(totalLicenses), 
          totalLicenses_(totalLicenses),
          startTime_(std::chrono::steady_clock::now()) {}
    
    // Synchronized method - acquire computation licenses
    void acquireLicenses(int numLicenses, const std::string& computationType) {
        std::unique_lock<std::mutex> lock(monitor_lock_);
        
        auto threadId = std::this_thread::get_id();
        std::cout << "[" << computationType << "-" << threadId << "] "
                  << "Requesting " << numLicenses << " license(s)...\n";
        
        licenses_available_.wait(lock, [this, numLicenses] { 
            return availableLicenses_ >= numLicenses; 
        });
        
        availableLicenses_ -= numLicenses;
        activeLicenses_[threadId] = {computationType, numLicenses};
        
        std::cout << "[" << computationType << "-" << threadId << "] "
                  << "Acquired " << numLicenses << " license(s) "
                  << "(available: " << availableLicenses_ << "/" 
                  << totalLicenses_ << ")\n";
    }
    
    // Synchronized method - release computation licenses
    void releaseLicenses() {
        std::thread::id threadId = std::this_thread::get_id();
        int numLicenses = 0;
        std::string computationType;
        
        {
            std::lock_guard<std::mutex> lock(monitor_lock_);
            
            auto it = activeLicenses_.find(threadId);
            if (it != activeLicenses_.end()) {
                computationType = it->second.first;
                numLicenses = it->second.second;
                availableLicenses_ += numLicenses;
                activeLicenses_.erase(it);
                
                std::cout << "[" << computationType << "-" << threadId << "] "
                          << "Released " << numLicenses << " license(s) "
                          << "(available: " << availableLicenses_ << "/" 
                          << totalLicenses_ << ")\n";
            }
        }
        
        if (numLicenses > 0) {
            if (numLicenses == 1) {
                licenses_available_.notify_one();
            } else {
                licenses_available_.notify_all();
            }
        }
    }
    
    // Synchronized method - try acquire without waiting
    bool tryAcquireLicenses(int numLicenses, const std::string& computationType) {
        std::lock_guard<std::mutex> lock(monitor_lock_);
        
        if (availableLicenses_ >= numLicenses) {
            availableLicenses_ -= numLicenses;
            activeLicenses_[std::this_thread::get_id()] = {computationType, numLicenses};
            
            std::cout << "[" << computationType << "-" << std::this_thread::get_id() << "] "
                      << "Try-acquired " << numLicenses << " license(s)\n";
            return true;
        }
        return false;
    }
    
    // Synchronized method - get usage statistics
    void printUsageStatistics() const {
        std::lock_guard<std::mutex> lock(monitor_lock_);
        
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime_).count();
        
        std::cout << "\n=== License Usage Statistics ===";
        std::cout << "\nTotal licenses: " << totalLicenses_;
        std::cout << "\nAvailable: " << availableLicenses_;
        std::cout << "\nIn use: " << (totalLicenses_ - availableLicenses_);
        std::cout << "\nUptime: " << uptime << " seconds";
        
        if (!activeLicenses_.empty()) {
            std::cout << "\n\nActive licenses by computation type:\n";
            std::unordered_map<std::string, int> typeCount;
            for (const auto& [tid, info] : activeLicenses_) {
                typeCount[info.first] += info.second;
            }
            for (const auto& [type, count] : typeCount) {
                std::cout << "  " << type << ": " << count << " license(s)\n";
            }
        }
        std::cout << "\n";
    }
    
    // RAII wrapper for automatic license management
    class LicenseGuard {
    private:
        ComputationLicenseManager& manager_;
        bool acquired_;
        
    public:
        LicenseGuard(ComputationLicenseManager& manager, int numLicenses, 
                     const std::string& computationType)
            : manager_(manager), acquired_(true) {
            manager_.acquireLicenses(numLicenses, computationType);
        }
        
        ~LicenseGuard() {
            if (acquired_) {
                manager_.releaseLicenses();
            }
        }
        
        // Prevent copying
        LicenseGuard(const LicenseGuard&) = delete;
        LicenseGuard& operator=(const LicenseGuard&) = delete;
        
        // Allow moving
        LicenseGuard(LicenseGuard&& other) noexcept 
            : manager_(other.manager_), acquired_(other.acquired_) {
            other.acquired_ = false;
        }
    };
};

// Example usage functions
void simulationPipelineExample() {
    std::cout << "=== Simulation Pipeline Example ===\n";
    std::cout << "Multiple simulators producing results, analyzers consuming them\n\n";
    
    SimulationDataBuffer dataBuffer(10); // Buffer for 10 simulation results
    std::atomic<bool> simulationComplete{false};
    
    // Simulator threads - producing simulation data
    std::vector<std::thread> simulators;
    for (int simId = 0; simId < 2; ++simId) {
        simulators.emplace_back([&dataBuffer, simId, &simulationComplete]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> energyDist(-100.0, -50.0);
            std::uniform_real_distribution<> tempDist(273.0, 373.0);
            std::uniform_real_distribution<> pressureDist(1.0, 10.0);
            
            for (int timestep = 0; timestep < 5; ++timestep) {
                // Simulate computation time
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                // Generate simulation data
                std::vector<double> state(10);
                for (auto& s : state) {
                    s = energyDist(gen) / 10.0;
                }
                
                SimulationResult result(
                    simId * 100 + timestep,
                    energyDist(gen),
                    tempDist(gen),
                    pressureDist(gen),
                    state
                );
                
                dataBuffer.storeResult(result);
            }
        });
    }
    
    // Analyzer threads - consuming and processing simulation data
    std::vector<std::thread> analyzers;
    for (int analyzerId = 0; analyzerId < 3; ++analyzerId) {
        analyzers.emplace_back([&dataBuffer, analyzerId, &simulationComplete]() {
            double totalEnergy = 0.0;
            int resultsAnalyzed = 0;
            
            for (int i = 0; i < 3; ++i) {
                SimulationResult result = dataBuffer.retrieveResult();
                
                // Analyze the result
                double avgStateValue = std::accumulate(result.state.begin(), 
                                                      result.state.end(), 0.0) / result.state.size();
                totalEnergy += result.energy;
                resultsAnalyzed++;
                
                std::cout << "[Analyzer-" << analyzerId << "] Analyzed timestep " 
                          << result.timestep << ": avg state = " 
                          << std::fixed << std::setprecision(3) << avgStateValue 
                          << ", T = " << result.temperature << " K\n";
                
                // Simulate analysis time
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
            
            std::cout << "[Analyzer-" << analyzerId << "] Average energy: " 
                      << (totalEnergy / resultsAnalyzed) << " eV\n";
        });
    }
    
    // Wait for completion
    for (auto& s : simulators) s.join();
    simulationComplete = true;
    
    for (auto& a : analyzers) a.join();
    
    // Process remaining results
    while (!dataBuffer.empty()) {
        SimulationResult result = dataBuffer.retrieveResult();
        std::cout << "Final cleanup: timestep " << result.timestep << "\n";
    }
    
    auto [produced, consumed] = dataBuffer.getStatistics();
    std::cout << "\nStatistics: Produced " << produced << ", Consumed " << consumed << "\n";
}

void computationalResourceExample() {
    std::cout << "\n\n=== Computational Resource Management Example ===\n";
    std::cout << "Managing GPU and memory resources for parallel computations\n\n";
    
    ComputationalResourceManager cluster1("Cluster-1", 256.0, 8); // 256 GB RAM, 8 GPUs
    ComputationalResourceManager cluster2("Cluster-2", 128.0, 4); // 128 GB RAM, 4 GPUs
    
    std::vector<std::thread> computations;
    
    // Deep Learning training task
    computations.emplace_back([&cluster1]() {
        cluster1.allocateResources(64.0, 4, "DeepLearning-Training");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        cluster1.allocateResources(32.0, 2, "DeepLearning-Validation"); // Will wait
    });
    
    // Molecular Dynamics simulation
    computations.emplace_back([&cluster1]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!cluster1.tryAllocateResources(128.0, 4, "MolecularDynamics")) {
            std::cout << "MD simulation waiting for resources...\n";
            cluster1.allocateResources(128.0, 4, "MolecularDynamics");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        cluster1.releaseResources(128.0, 4, "MolecularDynamics");
    });
    
    // CFD computation
    computations.emplace_back([&cluster1]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        cluster1.allocateResources(48.0, 2, "CFD-Simulation");
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        cluster1.releaseResources(48.0, 2, "CFD-Simulation");
    });
    
    // Resource transfer between clusters
    computations.emplace_back([&cluster1, &cluster2]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        cluster1.transferResources(cluster2, 32.0, 2);
    });
    
    // Monitor thread
    computations.emplace_back([&cluster1, &cluster2]() {
        for (int i = 0; i < 3; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            auto [mem1, gpu1] = cluster1.getAvailableResources();
            auto [mem2, gpu2] = cluster2.getAvailableResources();
            std::cout << "\n[Monitor] Resource status:";
            std::cout << "\n  Cluster-1: " << mem1 << " GB, " << gpu1 << " GPUs";
            std::cout << "\n  Cluster-2: " << mem2 << " GB, " << gpu2 << " GPUs\n\n";
        }
    });
    
    // Wait for all computations
    for (auto& t : computations) t.join();
    
    // Final cleanup
    cluster1.releaseResources(64.0, 4, "DeepLearning-Training");
    cluster1.releaseResources(32.0, 2, "DeepLearning-Validation");
    
    std::cout << "\nFinal resource status:\n";
    auto [finalMem1, finalGpu1] = cluster1.getAvailableResources();
    auto [finalMem2, finalGpu2] = cluster2.getAvailableResources();
    std::cout << "Cluster-1: " << finalMem1 << " GB, " << finalGpu1 << " GPUs\n";
    std::cout << "Cluster-2: " << finalMem2 << " GB, " << finalGpu2 << " GPUs\n";
}

void scientificDataGridExample() {
    std::cout << "\n\n=== Scientific Data Grid Example ===\n";
    std::cout << "Multiple readers computing metrics, writers updating grid\n\n";
    
    ScientificDataGrid grid(100, 100); // 100x100 scientific data grid
    std::vector<std::thread> threads;
    
    // Initialize grid with some data
    {
        ScientificDataGrid::WriteAccess writer(grid);
        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                grid.updateElement(i, j, std::sin(i * 0.1) * std::cos(j * 0.1));
            }
        }
    }
    
    // Reader threads - computing various metrics
    for (int readerId = 0; readerId < 3; ++readerId) {
        threads.emplace_back([&grid, readerId]() {
            ScientificDataGrid::ReadAccess reader(grid);
            
            if (readerId == 0) {
                double norm = grid.computeFrobeniusNorm();
                std::cout << "Reader " << readerId << " computed Frobenius norm: " 
                          << std::scientific << std::setprecision(6) << norm << "\n";
            } else if (readerId == 1) {
                double spectral = grid.computeSpectralRadius();
                std::cout << "Reader " << readerId << " computed spectral radius: " 
                          << spectral << "\n";
            } else {
                double sum = 0.0;
                for (int i = 0; i < 100; ++i) {
                    sum += grid.getElement(i, i); // Trace
                }
                std::cout << "Reader " << readerId << " computed trace: " << sum << "\n";
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Writer thread - applying stencil operation
    threads.emplace_back([&grid]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        ScientificDataGrid::WriteAccess writer(grid);
        std::cout << "Writer applying 5-point stencil operation...\n";
        
        grid.applyStencilOperation([&grid](int i, int j) {
            // 5-point stencil (Laplacian)
            return 0.25 * (grid.getElement(i-1, j) + grid.getElement(i+1, j) +
                          grid.getElement(i, j-1) + grid.getElement(i, j+1));
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    });
    
    // More reader threads after write
    for (int readerId = 3; readerId < 5; ++readerId) {
        threads.emplace_back([&grid, readerId]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            
            ScientificDataGrid::ReadAccess reader(grid);
            double centerValue = grid.getElement(50, 50);
            std::cout << "Reader " << readerId << " read center value: " 
                      << std::fixed << std::setprecision(6) << centerValue << "\n";
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });
    }
    
    for (auto& t : threads) t.join();
    
    std::cout << "\nNote: Writers have priority - readers wait when writers are pending\n";
}

void computationLicenseExample() {
    std::cout << "\n\n=== Computation License Management Example ===\n";
    std::cout << "Managing limited licenses for expensive scientific computations\n\n";
    
    ComputationLicenseManager licenseManager(8); // 8 total computation licenses
    std::vector<std::thread> computations;
    
    // Various computation types requiring different numbers of licenses
    struct ComputationTask {
        std::string name;
        int licenses;
        int duration_ms;
    };
    
    std::vector<ComputationTask> tasks = {
        {"QuantumChemistry", 3, 300},
        {"ProteinFolding", 2, 250},
        {"WeatherSimulation", 4, 400},
        {"FluidDynamics", 2, 200},
        {"NeuralNetTraining", 3, 350}
    };
    
    // Launch computation threads
    for (const auto& task : tasks) {
        computations.emplace_back([&licenseManager, task]() {
            // Try to acquire without waiting first
            if (!licenseManager.tryAcquireLicenses(task.licenses, task.name)) {
                std::cout << "[" << task.name << "] Waiting in queue...\n";
                
                // Use RAII guard for automatic release
                ComputationLicenseManager::LicenseGuard guard(
                    licenseManager, task.licenses, task.name
                );
                
                std::cout << "[" << task.name << "] Starting computation...\n";
                
                // Simulate intensive computation
                std::this_thread::sleep_for(std::chrono::milliseconds(task.duration_ms));
                
                std::cout << "[" << task.name << "] Computation completed!\n";
            } else {
                std::cout << "[" << task.name << "] Starting computation immediately...\n";
                
                // Simulate computation
                std::this_thread::sleep_for(std::chrono::milliseconds(task.duration_ms));
                
                std::cout << "[" << task.name << "] Computation completed!\n";
                licenseManager.releaseLicenses();
            }
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Monitor thread showing license usage
    std::thread monitor([&licenseManager]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        licenseManager.printUsageStatistics();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        licenseManager.printUsageStatistics();
    });
    
    // Wait for all computations
    for (auto& t : computations) t.join();
    monitor.join();
    
    // Final statistics
    licenseManager.printUsageStatistics();
}

int main() {
    std::cout << "=== Monitor Object Pattern - Scientific Computing Demo ===\n";
    std::cout << "Thread-safe synchronization for computational resources\n\n";
    
    simulationPipelineExample();
    computationalResourceExample();
    scientificDataGridExample();
    computationLicenseExample();
    
    std::cout << "\n=== Summary ===\n";
    std::cout << "Monitor Object Pattern provides:\n";
    std::cout << "• Thread-safe access to shared computational resources\n";
    std::cout << "• Automatic synchronization without explicit locking\n";
    std::cout << "• Condition-based waiting for resource availability\n";
    std::cout << "• Prevention of race conditions in scientific computations\n";
    std::cout << "• Efficient resource utilization in parallel computing\n";
    
    return 0;
}