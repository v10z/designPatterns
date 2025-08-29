// Read-Write Lock Pattern - Scientific Computing Data Access
// Efficient concurrent access to shared scientific datasets and computational results
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <map>
#include <string>
#include <random>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <complex>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Basic Read-Write Lock Implementation for Scientific Data Access
class ReadWriteLock {
private:
    mutable std::mutex mutex_;
    std::condition_variable read_cv_;
    std::condition_variable write_cv_;
    
    int readers_active_ = 0;       // Number of active scientific data readers
    int writers_waiting_ = 0;      // Number of waiting data writers
    bool writer_active_ = false;   // Is a writer currently modifying data?
    
public:
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait while a computation is updating data or updates are pending (write preference)
        read_cv_.wait(lock, [this] {
            return !writer_active_ && writers_waiting_ == 0;
        });
        
        readers_active_++;
    }
    
    void unlock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        readers_active_--;
        
        // If no more readers, wake up a writer
        if (readers_active_ == 0) {
            write_cv_.notify_one();
        }
    }
    
    void lock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        writers_waiting_++;
        
        // Wait while readers are active or another writer is active
        write_cv_.wait(lock, [this] {
            return readers_active_ == 0 && !writer_active_;
        });
        
        writers_waiting_--;
        writer_active_ = true;
    }
    
    void unlock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        writer_active_ = false;
        
        // Wake up all readers (multiple readers can proceed)
        read_cv_.notify_all();
        // Also wake up one writer in case no readers are waiting
        write_cv_.notify_one();
    }
    
    // RAII Guards
    class ReadGuard {
    private:
        ReadWriteLock& lock_;
    public:
        explicit ReadGuard(ReadWriteLock& lock) : lock_(lock) {
            lock_.lock_read();
        }
        ~ReadGuard() {
            lock_.unlock_read();
        }
        ReadGuard(const ReadGuard&) = delete;
        ReadGuard& operator=(const ReadGuard&) = delete;
    };
    
    class WriteGuard {
    private:
        ReadWriteLock& lock_;
    public:
        explicit WriteGuard(ReadWriteLock& lock) : lock_(lock) {
            lock_.lock_write();
        }
        ~WriteGuard() {
            lock_.unlock_write();
        }
        WriteGuard(const WriteGuard&) = delete;
        WriteGuard& operator=(const WriteGuard&) = delete;
    };
};

// Thread-safe computational results cache with read-write lock
template<typename Key, typename Value>
class ComputationalResultsCache {
private:
    mutable ReadWriteLock lock_;
    std::map<Key, Value> cache_;
    size_t max_size_;
    
    // Cache performance statistics
    mutable std::atomic<size_t> cache_hits_{0};
    mutable std::atomic<size_t> cache_misses_{0};
    
public:
    explicit ComputationalResultsCache(size_t max_size = 1000) : max_size_(max_size) {}
    
    bool get(const Key& key, Value& value) const {
        ReadWriteLock::ReadGuard guard(lock_);
        
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            value = it->second;
            cache_hits_++;
            return true;
        }
        
        cache_misses_++;
        return false;
    }
    
    void put(const Key& key, const Value& value) {
        ReadWriteLock::WriteGuard guard(lock_);
        
        // Evict oldest computation result if at capacity
        if (cache_.size() >= max_size_ && cache_.find(key) == cache_.end()) {
            cache_.erase(cache_.begin());
        }
        
        cache_[key] = value;
    }
    
    void remove(const Key& key) {
        ReadWriteLock::WriteGuard guard(lock_);
        cache_.erase(key);
    }
    
    size_t size() const {
        ReadWriteLock::ReadGuard guard(lock_);
        return cache_.size();
    }
    
    void clear() {
        ReadWriteLock::WriteGuard guard(lock_);
        cache_.clear();
    }
    
    void get_stats(size_t& hits, size_t& misses) const {
        hits = cache_hits_.load();
        misses = cache_misses_.load();
    }
    
    double hit_rate() const {
        size_t h = cache_hits_.load();
        size_t m = cache_misses_.load();
        size_t total = h + m;
        return total > 0 ? static_cast<double>(h) / total : 0.0;
    }
};

// Scientific data array with concurrent access using std::shared_mutex (C++17)
template<typename T>
class ScientificDataArray {
private:
    mutable std::shared_mutex data_mutex_;
    std::vector<T> scientific_data_;
    std::string dataset_name_;
    
public:
    explicit ScientificDataArray(const std::string& name = "Dataset") 
        : dataset_name_(name) {}
    
    // Multiple analysis threads can read data simultaneously
    T get(size_t index) const {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        if (index >= scientific_data_.size()) {
            throw std::out_of_range("Scientific data index out of range");
        }
        return scientific_data_[index];
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        return scientific_data_.size();
    }
    
    bool contains(const T& value) const {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        return std::find(scientific_data_.begin(), scientific_data_.end(), value) != scientific_data_.end();
    }
    
    // Only one computation can append results at a time
    void push_back(const T& value) {
        std::unique_lock<std::shared_mutex> lock(data_mutex_);
        scientific_data_.push_back(value);
    }
    
    void update(size_t index, const T& value) {
        std::unique_lock<std::shared_mutex> lock(data_mutex_);
        if (index >= scientific_data_.size()) {
            throw std::out_of_range("Scientific data index out of range");
        }
        scientific_data_[index] = value;
    }
    
    void clear() {
        std::unique_lock<std::shared_mutex> lock(data_mutex_);
        scientific_data_.clear();
    }
    
    // Bulk read operation for analysis
    std::vector<T> get_all() const {
        std::shared_lock<std::shared_mutex> lock(data_mutex_);
        return scientific_data_;  // Return copy for safe analysis
    }
};

// Simulation state with version tracking for scientific computations
class SimulationState {
private:
    mutable std::shared_mutex state_mutex_;
    std::vector<double> state_vector_;
    std::vector<double> gradients_;
    double total_energy_ = 0.0;
    double temperature_ = 298.15;  // Kelvin
    int iteration_ = 0;
    std::vector<std::pair<int, double>> convergence_history_;
    
public:
    SimulationState() : state_vector_(100, 0.0), gradients_(100, 0.0) {}
    
    // Read operations for analysis
    std::vector<double> read_state() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        std::cout << "[Analyzer-" << std::this_thread::get_id() << "] "
                  << "Reading simulation state (iteration " << iteration_ << ")\n";
        return state_vector_;
    }
    
    int get_iteration() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return iteration_;
    }
    
    std::vector<std::pair<int, double>> get_convergence_history() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return convergence_history_;
    }
    
    // Write operations for simulation updates
    void update_state(const std::vector<double>& new_state, double energy) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        std::cout << "[Simulator-" << std::this_thread::get_id() << "] "
                  << "Updating simulation state\n";
        
        // Store convergence history
        convergence_history_.push_back({iteration_, total_energy_});
        
        // Update state
        state_vector_ = new_state;
        total_energy_ = energy;
        iteration_++;
        
        // Calculate gradients (simplified)
        for (size_t i = 0; i < gradients_.size(); ++i) {
            gradients_[i] = (i > 0) ? state_vector_[i] - state_vector_[i-1] : 0.0;
        }
    }
    
    void update_temperature(double new_temperature) {
        std::unique_lock<std::shared_mutex> lock(state_mutex_);
        std::cout << "[ThermalController-" << std::this_thread::get_id() << "] "
                  << "Updating temperature\n";
        
        temperature_ = new_temperature;
        
        // Temperature affects the simulation dynamics
        double scaling_factor = std::sqrt(temperature_ / 298.15);
        for (auto& val : state_vector_) {
            val *= scaling_factor;
        }
    }
    
    // Read operations for monitoring
    double get_energy() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return total_energy_;
    }
    
    double get_temperature() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return temperature_;
    }
    
    std::vector<double> get_gradients() const {
        std::shared_lock<std::shared_mutex> lock(state_mutex_);
        return gradients_;
    }
};

// Upgradeable lock for scientific data processing pipelines
class UpgradeableLock {
private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    
    int readers_ = 0;
    bool writer_ = false;
    bool upgrader_ = false;
    std::thread::id upgrade_thread_id_;
    
public:
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !writer_ && !upgrader_; });
        readers_++;
    }
    
    void unlock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        readers_--;
        if (readers_ == 0) {
            cv_.notify_all();
        }
    }
    
    void lock_upgradeable() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !writer_ && !upgrader_; });
        upgrader_ = true;
        upgrade_thread_id_ = std::this_thread::get_id();
        readers_++;
    }
    
    void unlock_upgradeable() {
        std::unique_lock<std::mutex> lock(mutex_);
        upgrader_ = false;
        readers_--;
        if (readers_ == 0) {
            cv_.notify_all();
        }
    }
    
    void upgrade_to_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (std::this_thread::get_id() != upgrade_thread_id_) {
            throw std::runtime_error("Only upgrade lock holder can upgrade");
        }
        
        readers_--;  // Remove self from readers
        cv_.wait(lock, [this] { return readers_ == 0; });
        writer_ = true;
    }
    
    void downgrade_to_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        writer_ = false;
        readers_++;
        cv_.notify_all();
    }
    
    void unlock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        writer_ = false;
        cv_.notify_all();
    }
};

// Example scenarios for scientific computing
void scientific_cache_example() {
    std::cout << "=== Scientific Computation Cache Example ===\n";
    ComputationalResultsCache<std::pair<int, int>, double> eigen_cache(5);  // Cache eigenvalues
    
    // Simulate multiple computational threads
    std::vector<std::thread> threads;
    
    // Computation threads storing results
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([&eigen_cache, i]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dist(-10.0, 10.0);
            
            for (int j = 0; j < 5; ++j) {
                // Matrix dimensions as key
                std::pair<int, int> matrix_key{i * 10 + j, i * 10 + j};
                
                // Simulate eigenvalue computation
                double eigenvalue = dist(gen) * std::exp(-j * 0.1);
                
                eigen_cache.put(matrix_key, eigenvalue);
                std::cout << "[EigenSolver-" << i << "] Cached eigenvalue for matrix " 
                         << matrix_key.first << "x" << matrix_key.second
                         << " -> λ = " << std::scientific << std::setprecision(3) 
                         << eigenvalue << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
    
    // Analysis threads reading cached results
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&eigen_cache, i]() {
            std::mt19937 rng(i);
            std::uniform_int_distribution<int> dist(0, 19);
            
            for (int j = 0; j < 8; ++j) {
                std::pair<int, int> matrix_key{dist(rng), dist(rng)};
                double eigenvalue;
                
                if (eigen_cache.get(matrix_key, eigenvalue)) {
                    std::cout << "[Analyzer-" << i << "] Cache hit: matrix " 
                             << matrix_key.first << "x" << matrix_key.second
                             << " -> λ = " << std::scientific << std::setprecision(3)
                             << eigenvalue << "\n";
                } else {
                    std::cout << "[Analyzer-" << i << "] Cache miss: matrix " 
                             << matrix_key.first << "x" << matrix_key.second << "\n";
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    size_t hits, misses;
    eigen_cache.get_stats(hits, misses);
    std::cout << "\nEigenvalue Cache Statistics:\n";
    std::cout << "Hits: " << hits << ", Misses: " << misses << "\n";
    std::cout << "Hit Rate: " << std::fixed << std::setprecision(1) 
              << (eigen_cache.hit_rate() * 100) << "%\n";
    std::cout << "Cache Size: " << eigen_cache.size() << "\n";
}

void simulation_state_example() {
    std::cout << "\n\n=== Simulation State Example ===\n";
    SimulationState sim_state;
    
    // Initialize with random state
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(-1.0, 1.0);
    
    std::vector<double> initial_state(100);
    for (auto& val : initial_state) {
        val = dist(gen);
    }
    sim_state.update_state(initial_state, -523.45);
    
    std::vector<std::thread> threads;
    
    // Multiple analysis threads
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&sim_state, i]() {
            for (int j = 0; j < 2; ++j) {
                std::vector<double> state = sim_state.read_state();
                
                // Compute statistical properties
                double mean = std::accumulate(state.begin(), state.end(), 0.0) / state.size();
                double variance = 0.0;
                for (double val : state) {
                    variance += (val - mean) * (val - mean);
                }
                variance /= state.size();
                
                std::cout << "[Analyzer-" << i << "] State statistics: mean = " 
                         << std::fixed << std::setprecision(4) << mean 
                         << ", variance = " << variance << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    // Simulation update threads
    threads.emplace_back([&sim_state, &gen, &dist]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        
        // Gradient descent update
        auto current_state = sim_state.read_state();
        auto gradients = sim_state.get_gradients();
        
        for (size_t i = 0; i < current_state.size(); ++i) {
            current_state[i] -= 0.01 * gradients[i];  // Learning rate = 0.01
        }
        
        sim_state.update_state(current_state, -515.23);
    });
    
    // Temperature controller thread
    threads.emplace_back([&sim_state]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        sim_state.update_temperature(373.15);  // Heat to 100°C
    });
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nFinal simulation state:\n";
    std::cout << "Energy: " << std::scientific << std::setprecision(3) 
              << sim_state.get_energy() << " eV\n";
    std::cout << "Temperature: " << std::fixed << std::setprecision(2) 
              << sim_state.get_temperature() << " K\n";
    std::cout << "Iteration: " << sim_state.get_iteration() << "\n";
    
    auto history = sim_state.get_convergence_history();
    std::cout << "Convergence history: ";
    for (const auto& [iter, energy] : history) {
        std::cout << "(" << iter << ", " << std::scientific << std::setprecision(2) 
                  << energy << ") ";
    }
    std::cout << "\n";
}

void performance_comparison() {
    std::cout << "\n\n=== Performance Comparison: Scientific Data Access ===\n";
    
    const int num_datapoints = 1000;
    const int num_analyzers = 5;  // Multiple threads analyzing data
    const int num_updaters = 1;   // One thread updating simulation
    const int operations_per_thread = 1000;
    
    // Regular mutex version for scientific data
    class MutexScientificData {
        mutable std::mutex mutex_;
        std::vector<double> data_;
    public:
        MutexScientificData(int size) : data_(size, 0.0) {
            // Initialize with sine wave
            for (int i = 0; i < size; ++i) {
                data_[i] = std::sin(2.0 * M_PI * i / size);
            }
        }
        
        double get(size_t index) const {
            std::lock_guard<std::mutex> lock(mutex_);
            return data_[index];
        }
        
        void set(size_t index, double value) {
            std::lock_guard<std::mutex> lock(mutex_);
            data_[index] = value;
        }
    };
    
    // Test with regular mutex
    {
        MutexScientificData data(num_datapoints);
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        
        // Data analysis threads (readers)
        for (int i = 0; i < num_analyzers; ++i) {
            threads.emplace_back([&data, num_datapoints, operations_per_thread]() {
                std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<int> dist(0, num_datapoints - 1);
                double sum = 0.0;
                
                for (int j = 0; j < operations_per_thread; ++j) {
                    sum += data.get(dist(rng));
                }
                volatile double result = sum;  // Prevent optimization
                (void)result;
            });
        }
        
        // Simulation update thread (writer)
        threads.emplace_back([&data, num_datapoints, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                data.set(j % num_datapoints, std::cos(2.0 * M_PI * j / num_datapoints));
            }
        });
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Regular Mutex: " << duration.count() << "ms\n";
    }
    
    // Test with read-write lock
    {
        ScientificDataArray<double> data_array("WaveformData");
        // Initialize with sine wave
        for (int i = 0; i < num_datapoints; ++i) {
            data_array.push_back(std::sin(2.0 * M_PI * i / num_datapoints));
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> threads;
        
        // Data analysis threads (readers)
        for (int i = 0; i < num_analyzers; ++i) {
            threads.emplace_back([&data_array, num_datapoints, operations_per_thread]() {
                std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<int> dist(0, num_datapoints - 1);
                double sum = 0.0;
                
                for (int j = 0; j < operations_per_thread; ++j) {
                    sum += data_array.get(dist(rng));
                }
                volatile double result = sum;  // Prevent optimization
                (void)result;
            });
        }
        
        // Simulation update thread (writer)
        threads.emplace_back([&data_array, num_datapoints, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                data_array.update(j % num_datapoints, std::cos(2.0 * M_PI * j / num_datapoints));
            }
        });
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Read-Write Lock: " << duration.count() << "ms\n";
        std::cout << "\nRead-Write lock provides better performance for "
                  << "scientific analysis workloads with many readers!\n";
    }
}

int main() {
    std::cout << "=== Read-Write Lock Pattern - Scientific Computing Demo ===\n\n";
    std::cout << "Efficient concurrent access to scientific data and computational results\n\n";
    
    scientific_cache_example();
    simulation_state_example();
    performance_comparison();
    
    std::cout << "\n=== Key Benefits for Scientific Computing ===\n";
    std::cout << "• Multiple threads can analyze data simultaneously\n";
    std::cout << "• Single writer ensures data consistency during updates\n";
    std::cout << "• Optimal for read-heavy scientific analysis workloads\n";
    std::cout << "• Reduced contention compared to exclusive locking\n";
    std::cout << "• Scales well with number of analysis threads\n";
    
    return 0;
}