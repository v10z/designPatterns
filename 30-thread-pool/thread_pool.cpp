// Thread Pool Pattern for Scientific Computing - Parallel Computation Management
// Efficient execution of numerical simulations, data analysis, and HPC workloads
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>
#include <iomanip>
#include <complex>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Scientific Computation Thread Pool for parallel numerical tasks
class ScientificThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> busy_threads_{0};
    
    void worker_thread(size_t worker_id) {
        std::cout << "[ComputeWorker-" << worker_id << "] Started on CPU core\n";
        
        while (true) {
            std::function<void()> computation_task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { 
                    return stop_ || !tasks_.empty(); 
                });
                
                if (stop_ && tasks_.empty()) {
                    std::cout << "[ComputeWorker-" << worker_id << "] Shutting down\n";
                    return;
                }
                
                computation_task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            busy_threads_++;
            auto start = std::chrono::high_resolution_clock::now();
            computation_task();
            auto end = std::chrono::high_resolution_clock::now();
            busy_threads_--;
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            total_compute_time_ += duration.count();
            tasks_completed_++;
        }
    }
    
    std::atomic<size_t> tasks_completed_{0};
    std::atomic<long long> total_compute_time_{0};
    
public:
    explicit ScientificThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back(&ScientificThreadPool::worker_thread, this, i);
        }
        std::cout << "Scientific computation pool initialized with " << num_threads 
                  << " worker threads (" << std::thread::hardware_concurrency() 
                  << " hardware cores detected)\n";
    }
    
    ~ScientificThreadPool() {
        shutdown();
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result_t<F, Args...>> {
        using return_type = typename std::invoke_result_t<F, Args...>;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> res = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return res;
    }
    
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "\nScientific thread pool statistics:\n";
        std::cout << "  Tasks completed: " << tasks_completed_ << "\n";
        if (tasks_completed_ > 0) {
            std::cout << "  Average computation time: " 
                      << (total_compute_time_ / tasks_completed_) << " μs\n";
        }
        std::cout << "  Thread pool shut down\n";
    }
    
    size_t size() const { return workers_.size(); }
    size_t queued() const { 
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return tasks_.size(); 
    }
    size_t busy() const { return busy_threads_.load(); }
    size_t idle() const { return size() - busy(); }
};

// Priority-based computation pool for HPC workloads
class HPCPriorityPool {
private:
    struct ComputationTask {
        int priority;  // 0-10: 10 = critical path, 5 = normal, 1 = background
        std::function<void()> computation;
        std::string task_name;
        std::chrono::steady_clock::time_point submission_time;
        
        bool operator<(const ComputationTask& other) const {
            return priority < other.priority;  // Higher priority first
        }
    };
    
    std::vector<std::thread> compute_workers_;
    std::priority_queue<ComputationTask> computation_queue_;
    
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    
    // Statistics
    std::atomic<size_t> tasks_completed_{0};
    std::atomic<size_t> total_wait_time_ms_{0};
    
    void hpc_worker_thread(int worker_id) {
        std::cout << "[HPCWorker-" << worker_id << "] Started for high-performance computing\n";
        
        // Set thread affinity for NUMA optimization (platform-specific)
        // In real HPC, would bind to specific CPU cores
        
        while (true) {
            ComputationTask task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { 
                    return stop_ || !computation_queue_.empty(); 
                });
                
                if (stop_ && computation_queue_.empty()) {
                    break;
                }
                
                task = computation_queue_.top();
                computation_queue_.pop();
            }
            
            auto wait_time = std::chrono::steady_clock::now() - task.submission_time;
            auto wait_ms = std::chrono::duration_cast<std::chrono::milliseconds>(wait_time).count();
            
            std::cout << "[HPCWorker-" << worker_id << "] Executing " << task.task_name 
                      << " (priority: " << task.priority << ", waited: " << wait_ms << "ms)\n";
            
            auto start = std::chrono::steady_clock::now();
            task.computation();
            auto end = std::chrono::steady_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            total_wait_time_ms_ += duration.count();
            tasks_completed_++;
        }
        
        std::cout << "[HPCWorker-" << worker_id << "] Terminated\n";
    }
    
public:
    explicit HPCPriorityPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            compute_workers_.emplace_back(&HPCPriorityPool::hpc_worker_thread, this, i);
        }
        std::cout << "HPC priority pool initialized with " << num_threads << " compute workers\n";
    }
    
    ~HPCPriorityPool() {
        shutdown();
    }
    
    void submit_computation(std::function<void()> computation, const std::string& name, int priority = 5) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (!stop_) {
                computation_queue_.push({
                    priority, 
                    std::move(computation), 
                    name,
                    std::chrono::steady_clock::now()
                });
            }
        }
        condition_.notify_one();
    }
    
    void shutdown() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        
        condition_.notify_all();
        
        for (std::thread& worker : compute_workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        std::cout << "\nHPC Priority Pool Statistics:\n";
        std::cout << "  Computations completed: " << tasks_completed_ << "\n";
        if (tasks_completed_ > 0) {
            std::cout << "  Average computation time: " 
                      << (total_wait_time_ms_ / tasks_completed_) << "ms\n";
            std::cout << "  Total compute time: " << total_wait_time_ms_ << "ms\n";
        }
    }
};

// Work-stealing pool for dynamic load balancing in scientific simulations
class SimulationWorkStealingPool {
private:
    struct SimulationWorker {
        std::thread compute_thread;
        std::queue<std::function<void()>> simulation_queue;
        std::mutex queue_mutex;
        std::condition_variable cv;
        std::atomic<bool> has_simulations{false};
        std::atomic<size_t> simulations_completed{0};
        std::atomic<double> total_flops{0.0};  // Floating-point operations
    };
    
    std::vector<std::unique_ptr<SimulationWorker>> simulation_workers_;
    std::atomic<size_t> next_worker_{0};
    std::atomic<bool> stop_{false};
    std::string pool_name_;
    
    void simulation_worker_thread(SimulationWorker* worker, size_t worker_id) {
        std::cout << "[SimWorker-" << worker_id << "] Started for " << pool_name_ << "\n";
        
        while (!stop_) {
            std::function<void()> simulation_task;
            
            // Try to get simulation from local queue
            {
                std::unique_lock<std::mutex> lock(worker->queue_mutex);
                
                if (!worker->simulation_queue.empty()) {
                    simulation_task = std::move(worker->simulation_queue.front());
                    worker->simulation_queue.pop();
                    
                    if (worker->simulation_queue.empty()) {
                        worker->has_simulations = false;
                    }
                } else {
                    // Try to steal simulation work from other workers
                    for (size_t i = 0; i < simulation_workers_.size(); ++i) {
                        if (i == worker_id) continue;
                        
                        std::unique_lock<std::mutex> other_lock(
                            simulation_workers_[i]->queue_mutex, std::try_to_lock);
                        
                        if (other_lock.owns_lock() && !simulation_workers_[i]->simulation_queue.empty()) {
                            simulation_task = std::move(simulation_workers_[i]->simulation_queue.front());
                            simulation_workers_[i]->simulation_queue.pop();
                            
                            if (simulation_workers_[i]->simulation_queue.empty()) {
                                simulation_workers_[i]->has_simulations = false;
                            }
                            
                            std::cout << "[SimWorker-" << worker_id 
                                      << "] Stole simulation task from worker " << i 
                                      << " (load balancing)\n";
                            break;
                        }
                    }
                    
                    if (!simulation_task) {
                        worker->cv.wait_for(lock, std::chrono::milliseconds(10));
                        continue;
                    }
                }
            }
            
            if (simulation_task) {
                auto start = std::chrono::high_resolution_clock::now();
                simulation_task();
                auto end = std::chrono::high_resolution_clock::now();
                
                worker->simulations_completed++;
                // Estimate FLOPS (simplified)
                auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                worker->total_flops.store(worker->total_flops.load() + duration_us * 1000.0);  // Rough estimate
            }
        }
        
        std::cout << "[SimWorker-" << worker_id << "] Completed " 
                  << worker->simulations_completed << " simulations\n";
    }
    
public:
    explicit SimulationWorkStealingPool(size_t num_threads, const std::string& name = "Simulation")
        : pool_name_(name) {
        for (size_t i = 0; i < num_threads; ++i) {
            simulation_workers_.push_back(std::make_unique<SimulationWorker>());
            simulation_workers_.back()->compute_thread = std::thread(
                &SimulationWorkStealingPool::simulation_worker_thread, 
                this, simulation_workers_.back().get(), i
            );
        }
        std::cout << "Work-stealing pool '" << name << "' initialized with " 
                  << num_threads << " simulation workers\n";
    }
    
    ~SimulationWorkStealingPool() {
        stop_ = true;
        
        for (auto& worker : simulation_workers_) {
            worker->cv.notify_all();
            if (worker->compute_thread.joinable()) {
                worker->compute_thread.join();
            }
        }
        
        // Print statistics
        std::cout << "\nWork-Stealing Pool '" << pool_name_ << "' Statistics:\n";
        size_t total_simulations = 0;
        double total_gflops = 0.0;
        for (size_t i = 0; i < simulation_workers_.size(); ++i) {
            auto sims = simulation_workers_[i]->simulations_completed.load();
            auto flops = simulation_workers_[i]->total_flops.load();
            total_simulations += sims;
            total_gflops += flops / 1e9;
            std::cout << "  Worker " << i << ": " << sims << " simulations, "
                      << std::fixed << std::setprecision(2) << (flops / 1e9) << " GFLOPS\n";
        }
        std::cout << "  Total: " << total_simulations << " simulations, " 
                  << total_gflops << " GFLOPS\n";
    }
    
    void submit_simulation(std::function<void()> simulation) {
        size_t worker_id = next_worker_++ % simulation_workers_.size();
        
        {
            std::lock_guard<std::mutex> lock(simulation_workers_[worker_id]->queue_mutex);
            simulation_workers_[worker_id]->simulation_queue.push(std::move(simulation));
            simulation_workers_[worker_id]->has_simulations = true;
        }
        
        simulation_workers_[worker_id]->cv.notify_one();
    }
};

// Scheduled computation pool for time-dependent scientific simulations
class TimeSteppingComputationPool {
private:
    struct TimestepComputation {
        std::chrono::steady_clock::time_point execution_time;
        std::function<void()> computation;
        double simulation_time;  // Simulation time (not wall time)
        std::string computation_type;
        
        bool operator>(const TimestepComputation& other) const {
            return execution_time > other.execution_time;
        }
    };
    
    std::priority_queue<TimestepComputation, std::vector<TimestepComputation>, 
                       std::greater<TimestepComputation>> timestep_queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread scheduler_thread_;
    std::atomic<bool> stop_{false};
    ScientificThreadPool computation_executor_;
    double current_simulation_time_{0.0};
    
    void timestep_scheduler_loop() {
        std::cout << "[TimeStepScheduler] Started for time-dependent computations\n";
        
        while (!stop_) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            if (timestep_queue_.empty()) {
                cv_.wait_for(lock, std::chrono::milliseconds(100));
                continue;
            }
            
            auto now = std::chrono::steady_clock::now();
            auto next_computation = timestep_queue_.top();
            
            if (next_computation.execution_time <= now) {
                timestep_queue_.pop();
                current_simulation_time_ = next_computation.simulation_time;
                
                std::cout << "[TimeStepScheduler] Executing " << next_computation.computation_type
                          << " at t=" << std::fixed << std::setprecision(3) 
                          << next_computation.simulation_time << "s\n";
                
                lock.unlock();
                
                computation_executor_.enqueue(next_computation.computation);
            } else {
                cv_.wait_until(lock, next_computation.execution_time);
            }
        }
    }
    
public:
    explicit TimeSteppingComputationPool(size_t num_threads = 4)
        : computation_executor_(num_threads) {
        scheduler_thread_ = std::thread(&TimeSteppingComputationPool::timestep_scheduler_loop, this);
        std::cout << "Time-stepping computation pool initialized with " 
                  << num_threads << " compute threads\n";
    }
    
    ~TimeSteppingComputationPool() {
        stop_ = true;
        cv_.notify_all();
        
        if (scheduler_thread_.joinable()) {
            scheduler_thread_.join();
        }
        
        std::cout << "Time-stepping pool shut down at simulation time t=" 
                  << current_simulation_time_ << "s\n";
    }
    
    void schedule_timestep(std::function<void()> computation, 
                          double simulation_time,
                          std::chrono::milliseconds real_time_delay,
                          const std::string& type = "Timestep") {
        auto execution_time = std::chrono::steady_clock::now() + real_time_delay;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            timestep_queue_.push({
                execution_time, 
                std::move(computation), 
                simulation_time,
                type
            });
        }
        
        cv_.notify_one();
    }
    
    void schedule_periodic_analysis(std::function<void()> analysis, 
                                   double timestep,
                                   std::chrono::milliseconds initial_delay,
                                   std::chrono::milliseconds period) {
        static std::atomic<double> sim_time{0.0};
        
        schedule_timestep([this, analysis, timestep, period]() {
            analysis();
            sim_time.store(sim_time.load() + timestep);
            schedule_periodic_analysis(analysis, timestep, period, period);
        }, sim_time.load(), initial_delay, "Periodic Analysis");
    }
};

// Scientific computing helper functions
double compute_matrix_eigenvalue(int size) {
    // Simulate expensive eigenvalue computation
    std::this_thread::sleep_for(std::chrono::milliseconds(50 + size));
    return -2.4 + 0.1 * size + (std::rand() % 100) / 100.0;
}

std::vector<double> monte_carlo_integration(int samples) {
    // Monte Carlo integration of a complex function
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    double sum = 0.0;
    double sum_sq = 0.0;
    
    for (int i = 0; i < samples; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        double f = std::exp(-x*x) * std::sin(2*M_PI*y);  // Complex integrand
        sum += f;
        sum_sq += f * f;
    }
    
    double mean = sum / samples;
    double variance = (sum_sq / samples) - (mean * mean);
    double std_error = std::sqrt(variance / samples);
    
    return {mean, std_error};
}

// Example: Parallel eigenvalue computation
void parallel_eigenvalue_example() {
    std::cout << "=== Parallel Eigenvalue Computation ===\n";
    ScientificThreadPool computation_pool(4);
    
    std::vector<std::future<double>> eigenvalue_futures;
    std::vector<int> matrix_sizes = {10, 20, 30, 40, 50, 60, 70, 80};
    
    // Submit eigenvalue computations
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int size : matrix_sizes) {
        eigenvalue_futures.emplace_back(
            computation_pool.enqueue([size] {
                std::cout << "[Thread-" << std::this_thread::get_id() 
                         << "] Computing eigenvalue for " << size << "x" << size << " matrix\n";
                return compute_matrix_eigenvalue(size);
            })
        );
    }
    
    // Collect results
    std::cout << "\nEigenvalues computed:\n";
    for (size_t i = 0; i < eigenvalue_futures.size(); ++i) {
        double eigenvalue = eigenvalue_futures[i].get();
        std::cout << "  Matrix " << matrix_sizes[i] << "x" << matrix_sizes[i] 
                  << ": λ = " << std::fixed << std::setprecision(4) << eigenvalue << "\n";
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\nComputation statistics:\n";
    std::cout << "  Total time: " << duration.count() << "ms\n";
    std::cout << "  Speedup vs serial: ~" 
              << std::fixed << std::setprecision(1) 
              << (360.0 / duration.count()) << "x\n";
    std::cout << "  Queued computations: " << computation_pool.queued() << "\n";
    std::cout << "  Active threads: " << computation_pool.busy() << "\n";
}

void hpc_priority_computation_example() {
    std::cout << "\n\n=== HPC Priority-Based Computation ===\n";
    HPCPriorityPool hpc_pool(2);
    
    // Critical path computations (high priority)
    hpc_pool.submit_computation([]() {
        auto result = monte_carlo_integration(100000);
        std::cout << "  Critical: Large-scale Monte Carlo result = " 
                  << std::scientific << std::setprecision(4) << result[0] 
                  << " ± " << result[1] << "\n";
    }, "Critical Monte Carlo", 10);
    
    // Background analysis (low priority)
    hpc_pool.submit_computation([]() {
        std::cout << "  Background: Computing statistics...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        std::cout << "  Background: Statistics complete\n";
    }, "Background Statistics", 1);
    
    // Normal priority FFT
    hpc_pool.submit_computation([]() {
        std::cout << "  Normal: Computing FFT of signal...\n";
        // Simulate FFT computation
        int n = 1024;
        std::vector<std::complex<double>> signal(n);
        for (int i = 0; i < n; ++i) {
            signal[i] = std::complex<double>(std::cos(2*M_PI*i/n), std::sin(2*M_PI*i/n));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        std::cout << "  Normal: FFT complete (" << n << " points)\n";
    }, "FFT Analysis", 5);
    
    // Critical numerical solver
    hpc_pool.submit_computation([]() {
        std::cout << "  Critical: Solving linear system...\n";
        // Simulate solving Ax = b
        int n = 500;
        double residual = 1.0;
        for (int iter = 0; iter < 10; ++iter) {
            residual *= 0.1;
        }
        std::cout << "  Critical: Linear solver converged (residual: " 
                  << std::scientific << residual << ")\n";
    }, "Linear Solver", 10);
    
    // Low priority visualization prep
    hpc_pool.submit_computation([]() {
        std::cout << "  Background: Preparing visualization data...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "  Background: Visualization data ready\n";
    }, "Visualization Prep", 1);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
}

void simulation_work_stealing_example() {
    std::cout << "\n\n=== Work-Stealing Simulation Pool ===\n";
    SimulationWorkStealingPool md_pool(3, "Molecular Dynamics");
    
    // Submit MD simulation timesteps with varying complexity
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> complexity_dist(20, 100);
    
    for (int timestep = 0; timestep < 12; ++timestep) {
        int complexity = complexity_dist(gen);
        
        md_pool.submit_simulation([timestep, complexity]() {
            std::cout << "  [MD] Timestep " << timestep 
                     << " (complexity: " << complexity << ") executed by " 
                     << std::this_thread::get_id() << "\n";
            
            // Simulate particle force calculations
            double total_force = 0.0;
            for (int i = 0; i < complexity * 100; ++i) {
                total_force += std::exp(-i * 0.001) * std::cos(i * 0.1);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(complexity));
            
            if (complexity > 80) {
                std::cout << "    -> Heavy computation detected, may trigger work stealing\n";
            }
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
}

void time_stepping_simulation_example() {
    std::cout << "\n\n=== Time-Stepping Scientific Simulation ===\n";
    TimeSteppingComputationPool timestep_pool(2);
    
    auto wall_start = std::chrono::steady_clock::now();
    double dt = 0.01;  // Simulation timestep (seconds)
    
    // Schedule initial condition computation
    timestep_pool.schedule_timestep([wall_start]() {
        auto elapsed = std::chrono::steady_clock::now() - wall_start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        std::cout << "  [Wall: " << ms.count() << "ms] Computing initial conditions...\n";
        
        // Initialize temperature field
        int nx = 100, ny = 100;
        std::vector<double> temperature(nx * ny, 293.15);  // Room temperature
        std::cout << "    Initial temperature field: " << nx << "x" << ny 
                  << " grid at 293.15K\n";
    }, 0.0, std::chrono::milliseconds(50), "Initial Conditions");
    
    // Schedule heat equation solver at t=0.01s
    timestep_pool.schedule_timestep([wall_start, dt]() {
        auto elapsed = std::chrono::steady_clock::now() - wall_start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        std::cout << "  [Wall: " << ms.count() << "ms] Solving heat equation at t=" 
                  << dt << "s...\n";
        
        // Simulate finite difference heat equation
        double max_temp_change = 0.5;  // K
        std::cout << "    Max temperature change: " << max_temp_change << "K\n";
    }, dt, std::chrono::milliseconds(150), "Heat Equation Solver");
    
    // Schedule boundary condition update at t=0.02s  
    timestep_pool.schedule_timestep([wall_start]() {
        auto elapsed = std::chrono::steady_clock::now() - wall_start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        std::cout << "  [Wall: " << ms.count() << "ms] Updating boundary conditions...\n";
        std::cout << "    Applied heat flux: 1000 W/m²\n";
    }, 0.02, std::chrono::milliseconds(200), "Boundary Update");
    
    // Periodic analysis every 0.01s simulation time
    std::atomic<int> analysis_count{0};
    timestep_pool.schedule_periodic_analysis([&analysis_count, wall_start]() {
        auto elapsed = std::chrono::steady_clock::now() - wall_start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
        int count = ++analysis_count;
        
        std::cout << "  [Wall: " << ms.count() << "ms] Periodic analysis #" << count << "\n";
        
        // Compute statistics
        double avg_temp = 293.15 + count * 0.1;
        double max_temp = 293.15 + count * 0.5;
        std::cout << "    Average temperature: " << std::fixed << std::setprecision(2) 
                  << avg_temp << "K\n";
        std::cout << "    Maximum temperature: " << max_temp << "K\n";
    }, dt, std::chrono::milliseconds(100), std::chrono::milliseconds(150));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
}

int main() {
    std::cout << "=== Thread Pool Pattern - Scientific Computing Demo ===\n";
    std::cout << "Parallel execution of numerical simulations and HPC workloads\n\n";
    
    std::cout << "Hardware: " << std::thread::hardware_concurrency() << " CPU cores available\n\n";
    
    parallel_eigenvalue_example();
    hpc_priority_computation_example();
    simulation_work_stealing_example();
    time_stepping_simulation_example();
    
    std::cout << "\n=== Key Benefits for Scientific Computing ===\n";
    std::cout << "• Parallel execution of independent computations\n";
    std::cout << "• Priority scheduling for critical path calculations\n";
    std::cout << "• Work stealing for dynamic load balancing\n";
    std::cout << "• Time-stepping for numerical simulations\n";
    std::cout << "• Efficient utilization of multi-core processors\n";
    
    return 0;
}