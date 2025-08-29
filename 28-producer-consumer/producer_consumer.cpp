// Producer-Consumer Pattern - Scientific Computing Pipeline
// Parallel processing of simulation data, numerical computations, and analysis tasks
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>
#include <atomic>
#include <memory>
#include <functional>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <complex>
#include <fstream>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Basic Producer-Consumer with blocking queue
template<typename T>
class BlockingQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    size_t max_size_;
    
public:
    explicit BlockingQueue(size_t max_size = SIZE_MAX) 
        : max_size_(max_size) {}
    
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
        
        queue_.push(std::move(item));
        lock.unlock();
        not_empty_.notify_one();
    }
    
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return !queue_.empty(); });
        
        T item = std::move(queue_.front());
        queue_.pop();
        lock.unlock();
        not_full_.notify_one();
        
        return item;
    }
    
    bool try_push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= max_size_) {
            return false;
        }
        queue_.push(std::move(item));
        not_empty_.notify_one();
        return true;
    }
    
    bool try_pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return true;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
};

// Simulation timestep data for scientific computation queue
struct SimulationTimestep {
    int timestep;
    std::vector<double> particlePositions;
    std::vector<double> particleVelocities;
    double totalEnergy;
    double temperature;
    double pressure;
    int computationalComplexity;
    
    SimulationTimestep(int ts, const std::vector<double>& pos, const std::vector<double>& vel,
                      double energy, double temp, double press, int complexity)
        : timestep(ts), particlePositions(pos), particleVelocities(vel),
          totalEnergy(energy), temperature(temp), pressure(press),
          computationalComplexity(complexity) {}
};

// Producer base class
template<typename T>
class Producer {
protected:
    std::shared_ptr<BlockingQueue<T>> queue_;
    std::atomic<bool> running_{true};
    std::thread thread_;
    
    virtual T produce() = 0;
    virtual std::chrono::milliseconds getDelay() = 0;
    
    void run() {
        while (running_) {
            T item = produce();
            queue_->push(std::move(item));
            std::this_thread::sleep_for(getDelay());
        }
    }
    
public:
    explicit Producer(std::shared_ptr<BlockingQueue<T>> queue)
        : queue_(queue) {}
    
    virtual ~Producer() {
        stop();
    }
    
    void start() {
        thread_ = std::thread(&Producer::run, this);
    }
    
    void stop() {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }
};

// Consumer base class
template<typename T>
class Consumer {
protected:
    std::shared_ptr<BlockingQueue<T>> queue_;
    std::atomic<bool> running_{true};
    std::thread thread_;
    
    virtual void consume(T item) = 0;
    
    void run() {
        while (running_) {
            try {
                T item = queue_->pop();
                consume(std::move(item));
            } catch (const std::exception& e) {
                std::cerr << "Consumer error: " << e.what() << "\n";
            }
        }
    }
    
public:
    explicit Consumer(std::shared_ptr<BlockingQueue<T>> queue)
        : queue_(queue) {}
    
    virtual ~Consumer() {
        stop();
    }
    
    void start() {
        thread_ = std::thread(&Consumer::run, this);
    }
    
    void stop() {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }
};

// Concrete producers - Molecular Dynamics Simulation
class MDSimulationProducer : public Producer<SimulationTimestep> {
private:
    static std::atomic<int> timestep_counter_;
    int simulator_id_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> energy_dist_{-1000.0, -500.0};
    std::uniform_real_distribution<double> temp_dist_{273.0, 373.0};
    std::uniform_real_distribution<double> pressure_dist_{1.0, 10.0};
    std::uniform_int_distribution<int> complexity_dist_{100, 1000};
    std::uniform_int_distribution<int> delay_dist_{50, 200};
    int num_particles_;
    
    SimulationTimestep produce() override {
        int ts = timestep_counter_++;
        
        // Generate particle positions and velocities
        std::vector<double> positions, velocities;
        for (int i = 0; i < num_particles_ * 3; ++i) {
            positions.push_back(rng_() / static_cast<double>(rng_.max()) * 10.0);
            velocities.push_back((rng_() / static_cast<double>(rng_.max()) - 0.5) * 2.0);
        }
        
        double energy = energy_dist_(rng_);
        double temp = temp_dist_(rng_);
        double pressure = pressure_dist_(rng_);
        int complexity = complexity_dist_(rng_);
        
        std::cout << "[MDSimulator-" << simulator_id_ << "] Generated timestep " << ts 
                  << " (particles: " << num_particles_ 
                  << ", E: " << std::scientific << std::setprecision(3) << energy << " eV)\n";
        
        return SimulationTimestep(ts, positions, velocities, energy, temp, pressure, complexity);
    }
    
    std::chrono::milliseconds getDelay() override {
        return std::chrono::milliseconds(delay_dist_(rng_));
    }
    
public:
    MDSimulationProducer(std::shared_ptr<BlockingQueue<SimulationTimestep>> queue, 
                        int id, int num_particles = 100)
        : Producer(queue), simulator_id_(id), num_particles_(num_particles),
          rng_(std::random_device{}()) {}
};

std::atomic<int> MDSimulationProducer::timestep_counter_{0};

// Concrete consumers - Scientific Data Analyzer
class SimulationAnalyzer : public Consumer<SimulationTimestep> {
private:
    int analyzer_id_;
    int timesteps_analyzed_ = 0;
    double total_energy_ = 0.0;
    double min_energy_ = std::numeric_limits<double>::max();
    double max_energy_ = std::numeric_limits<double>::lowest();
    
    void consume(SimulationTimestep timestep) override {
        std::cout << "[Analyzer-" << analyzer_id_ << "] Analyzing timestep " 
                  << timestep.timestep << "\n";
        
        // Calculate kinetic energy
        double kinetic_energy = 0.0;
        for (size_t i = 0; i < timestep.particleVelocities.size(); ++i) {
            kinetic_energy += 0.5 * timestep.particleVelocities[i] * 
                             timestep.particleVelocities[i];
        }
        
        // Calculate center of mass
        double com_x = 0.0, com_y = 0.0, com_z = 0.0;
        int num_particles = timestep.particlePositions.size() / 3;
        for (int i = 0; i < num_particles; ++i) {
            com_x += timestep.particlePositions[i * 3];
            com_y += timestep.particlePositions[i * 3 + 1];
            com_z += timestep.particlePositions[i * 3 + 2];
        }
        com_x /= num_particles;
        com_y /= num_particles;
        com_z /= num_particles;
        
        // Simulate analysis computation
        std::this_thread::sleep_for(
            std::chrono::microseconds(timestep.computationalComplexity)
        );
        
        // Update statistics
        timesteps_analyzed_++;
        total_energy_ += timestep.totalEnergy;
        min_energy_ = std::min(min_energy_, timestep.totalEnergy);
        max_energy_ = std::max(max_energy_, timestep.totalEnergy);
        
        std::cout << "[Analyzer-" << analyzer_id_ << "] Timestep " << timestep.timestep 
                  << " - KE: " << std::scientific << std::setprecision(3) << kinetic_energy
                  << ", COM: (" << std::fixed << std::setprecision(2) 
                  << com_x << ", " << com_y << ", " << com_z << ")\n";
    }
    
public:
    SimulationAnalyzer(std::shared_ptr<BlockingQueue<SimulationTimestep>> queue, int id)
        : Consumer(queue), analyzer_id_(id) {}
    
    int getTimestepsAnalyzed() const { return timesteps_analyzed_; }
    double getAverageEnergy() const { 
        return timesteps_analyzed_ > 0 ? total_energy_ / timesteps_analyzed_ : 0.0;
    }
    double getMinEnergy() const { return min_energy_; }
    double getMaxEnergy() const { return max_energy_; }
};

// Multiple producers, single consumer - Scientific Event Logger
class ScientificEvent {
public:
    enum EventType { CONVERGENCE, DIVERGENCE, CHECKPOINT, WARNING, ERROR };
    
    EventType type;
    std::string source;
    std::string description;
    double metric_value;
    std::chrono::system_clock::time_point timestamp;
    std::thread::id computation_thread;
    
    ScientificEvent(EventType t, const std::string& src, const std::string& desc, double metric)
        : type(t), source(src), description(desc), metric_value(metric),
          timestamp(std::chrono::system_clock::now()),
          computation_thread(std::this_thread::get_id()) {}
};

class ComputationEventLogger : public Consumer<ScientificEvent> {
private:
    std::ofstream log_file_;
    
    void consume(ScientificEvent event) override {
        auto time = std::chrono::system_clock::to_time_t(event.timestamp);
        
        std::cout << "[" << std::put_time(std::localtime(&time), "%H:%M:%S") << "] ";
        
        switch (event.type) {
            case ScientificEvent::CONVERGENCE: 
                std::cout << "[CONVERGED] "; 
                break;
            case ScientificEvent::DIVERGENCE:  
                std::cout << "[DIVERGED] "; 
                break;
            case ScientificEvent::CHECKPOINT:  
                std::cout << "[CHECKPOINT] "; 
                break;
            case ScientificEvent::WARNING:     
                std::cout << "[WARNING] "; 
                break;
            case ScientificEvent::ERROR:       
                std::cout << "[ERROR] "; 
                break;
        }
        
        std::cout << "[" << event.source << "] " 
                  << event.description
                  << " (metric: " << std::scientific << std::setprecision(6) 
                  << event.metric_value << ")\n";
    }
    
public:
    explicit ComputationEventLogger(std::shared_ptr<BlockingQueue<ScientificEvent>> queue)
        : Consumer(queue) {}
};

// Pipeline pattern - Producer -> Processor -> Consumer
template<typename In, typename Out>
class Processor : public Consumer<In>, public Producer<Out> {
protected:
    virtual Out process(In input) = 0;
    
    void consume(In item) override {
        Out result = process(std::move(item));
        Producer<Out>::queue_->push(std::move(result));
    }
    
public:
    Processor(std::shared_ptr<BlockingQueue<In>> in_queue,
             std::shared_ptr<BlockingQueue<Out>> out_queue)
        : Consumer<In>(in_queue), Producer<Out>(out_queue) {}
    
    void start() {
        Consumer<In>::start();
    }
    
    void stop() {
        Consumer<In>::stop();
    }
    
    // Override Producer methods to prevent double threading
    Out produce() override { return Out{}; }
    std::chrono::milliseconds getDelay() override { return std::chrono::milliseconds(0); }
};

// Scientific data processing pipeline
struct SpectralData {
    int sample_id;
    std::vector<std::complex<double>> frequency_domain;
    double sampling_rate;
};

struct SpectralFeatures {
    int sample_id;
    double dominant_frequency;
    double total_power;
    double spectral_centroid;
    std::vector<double> peak_frequencies;
};

class SpectralAnalysisProcessor : public Processor<SpectralData, SpectralFeatures> {
protected:
    SpectralFeatures process(SpectralData input) override {
        SpectralFeatures features;
        features.sample_id = input.sample_id;
        
        if (!input.frequency_domain.empty()) {
            // Calculate power spectrum
            std::vector<double> power_spectrum;
            for (const auto& freq : input.frequency_domain) {
                power_spectrum.push_back(std::norm(freq));
            }
            
            // Find dominant frequency
            auto max_it = std::max_element(power_spectrum.begin(), power_spectrum.end());
            int max_idx = std::distance(power_spectrum.begin(), max_it);
            features.dominant_frequency = max_idx * input.sampling_rate / 
                                        input.frequency_domain.size();
            
            // Calculate total power
            features.total_power = std::accumulate(power_spectrum.begin(), 
                                                  power_spectrum.end(), 0.0);
            
            // Calculate spectral centroid
            double weighted_sum = 0.0;
            for (size_t i = 0; i < power_spectrum.size(); ++i) {
                weighted_sum += i * power_spectrum[i];
            }
            features.spectral_centroid = (weighted_sum / features.total_power) * 
                                       input.sampling_rate / power_spectrum.size();
            
            // Find peak frequencies (local maxima)
            for (size_t i = 1; i < power_spectrum.size() - 1; ++i) {
                if (power_spectrum[i] > power_spectrum[i-1] && 
                    power_spectrum[i] > power_spectrum[i+1] &&
                    power_spectrum[i] > 0.1 * (*max_it)) {
                    features.peak_frequencies.push_back(
                        i * input.sampling_rate / input.frequency_domain.size()
                    );
                }
            }
        }
        
        std::cout << "[SpectralProcessor] Analyzed sample #" << features.sample_id 
                  << " - Dominant freq: " << std::fixed << std::setprecision(2) 
                  << features.dominant_frequency << " Hz"
                  << ", Power: " << std::scientific << std::setprecision(3) 
                  << features.total_power << "\n";
        
        return features;
    }
    
public:
    using Processor::Processor;
};

// Examples
void molecularDynamicsExample() {
    std::cout << "=== Molecular Dynamics Simulation Pipeline ===\n";
    std::cout << "Multiple simulators producing timesteps, analyzers processing them\n\n";
    
    auto simulation_queue = std::make_shared<BlockingQueue<SimulationTimestep>>(10);
    
    // Create MD simulators (producers)
    std::vector<std::unique_ptr<MDSimulationProducer>> simulators;
    for (int i = 0; i < 2; ++i) {
        simulators.push_back(std::make_unique<MDSimulationProducer>(
            simulation_queue, i, 50 + i * 50)); // Different particle counts
        simulators.back()->start();
    }
    
    // Create analyzers (consumers)
    std::vector<std::unique_ptr<SimulationAnalyzer>> analyzers;
    for (int i = 0; i < 3; ++i) {
        analyzers.push_back(std::make_unique<SimulationAnalyzer>(simulation_queue, i));
        analyzers.back()->start();
    }
    
    // Run simulation for a while
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // Stop simulators first
    std::cout << "\nStopping simulators...\n";
    for (auto& simulator : simulators) {
        simulator->stop();
    }
    
    // Wait for queue to empty
    while (!simulation_queue->empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop analyzers
    std::cout << "Stopping analyzers...\n";
    for (auto& analyzer : analyzers) {
        analyzer->stop();
    }
    
    // Print analysis statistics
    std::cout << "\n=== Analysis Statistics ===\n";
    for (size_t i = 0; i < analyzers.size(); ++i) {
        std::cout << "Analyzer " << i << ":\n";
        std::cout << "  Timesteps analyzed: " << analyzers[i]->getTimestepsAnalyzed() << "\n";
        std::cout << "  Average energy: " << std::scientific << std::setprecision(3) 
                  << analyzers[i]->getAverageEnergy() << " eV\n";
        std::cout << "  Energy range: [" << analyzers[i]->getMinEnergy() 
                  << ", " << analyzers[i]->getMaxEnergy() << "] eV\n";
    }
}

void scientificEventLoggingExample() {
    std::cout << "\n\n=== Scientific Event Logging (Multiple Computations) ===\n";
    std::cout << "Multiple solvers producing convergence/divergence events\n\n";
    
    auto event_queue = std::make_shared<BlockingQueue<ScientificEvent>>();
    auto event_logger = std::make_unique<ComputationEventLogger>(event_queue);
    event_logger->start();
    
    // Multiple solver threads producing events
    std::vector<std::thread> solvers;
    
    // Conjugate Gradient solver
    solvers.emplace_back([event_queue]() {
        event_queue->push(ScientificEvent(ScientificEvent::CHECKPOINT, 
            "ConjugateGradient", "Starting CG solver for linear system", 0.0));
        
        double residual = 1.0;
        for (int iter = 0; iter < 5; ++iter) {
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            residual *= 0.1;
            
            if (residual < 1e-6) {
                event_queue->push(ScientificEvent(ScientificEvent::CONVERGENCE,
                    "ConjugateGradient", "Solution converged at iteration " + 
                    std::to_string(iter), residual));
                break;
            } else {
                event_queue->push(ScientificEvent(ScientificEvent::CHECKPOINT,
                    "ConjugateGradient", "Iteration " + std::to_string(iter) + 
                    " completed", residual));
            }
        }
    });
    
    // Newton-Raphson solver
    solvers.emplace_back([event_queue]() {
        event_queue->push(ScientificEvent(ScientificEvent::CHECKPOINT, 
            "NewtonRaphson", "Starting nonlinear solver", 0.0));
        
        double error = 10.0;
        for (int iter = 0; iter < 4; ++iter) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            error *= 0.3;
            
            if (error > 100.0) {
                event_queue->push(ScientificEvent(ScientificEvent::DIVERGENCE,
                    "NewtonRaphson", "Solution diverging!", error));
                break;
            } else if (error < 1e-8) {
                event_queue->push(ScientificEvent(ScientificEvent::CONVERGENCE,
                    "NewtonRaphson", "Converged to solution", error));
                break;
            }
        }
    });
    
    // FFT computation
    solvers.emplace_back([event_queue]() {
        event_queue->push(ScientificEvent(ScientificEvent::CHECKPOINT, 
            "FFT", "Computing Fast Fourier Transform", 0.0));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        event_queue->push(ScientificEvent(ScientificEvent::WARNING,
            "FFT", "Input size not power of 2, using slower algorithm", 1024.0));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        event_queue->push(ScientificEvent(ScientificEvent::CHECKPOINT,
            "FFT", "FFT computation completed", 2.345e-12));
    });
    
    // Wait for all solvers
    for (auto& solver : solvers) {
        solver.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    event_logger->stop();
}

void spectralAnalysisPipeline() {
    std::cout << "\n\n=== Spectral Analysis Pipeline ===\n";
    std::cout << "Signal acquisition -> FFT -> Feature extraction\n\n";
    
    auto spectral_queue = std::make_shared<BlockingQueue<SpectralData>>(10);
    auto features_queue = std::make_shared<BlockingQueue<SpectralFeatures>>(10);
    
    // Signal acquisition and FFT (producer)
    std::thread signal_generator([spectral_queue]() {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> freq_dist(10.0, 1000.0);
        std::uniform_real_distribution<double> phase_dist(0, 2 * M_PI);
        
        for (int sample = 0; sample < 5; ++sample) {
            SpectralData data;
            data.sample_id = sample;
            data.sampling_rate = 44100.0; // 44.1 kHz
            
            // Generate synthetic frequency spectrum
            int fft_size = 1024;
            for (int i = 0; i < fft_size; ++i) {
                double magnitude = 0.0;
                
                // Add some peaks at specific frequencies
                if (i == 100 || i == 250 || i == 400) {
                    magnitude = 10.0 + (rng() % 1000) / 100.0;
                } else {
                    magnitude = (rng() % 100) / 100.0; // Noise floor
                }
                
                double phase = phase_dist(rng);
                data.frequency_domain.push_back(
                    std::complex<double>(magnitude * std::cos(phase), 
                                       magnitude * std::sin(phase))
                );
            }
            
            std::cout << "[SignalAcquisition] Generated spectrum for sample #" << sample 
                      << " (FFT size: " << fft_size << ")\n";
            spectral_queue->push(std::move(data));
            
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });
    
    // Spectral processor
    auto processor = std::make_unique<SpectralAnalysisProcessor>(
        spectral_queue, features_queue);
    processor->start();
    
    // Feature consumer
    std::thread feature_analyzer([features_queue]() {
        for (int i = 0; i < 5; ++i) {
            SpectralFeatures features = features_queue->pop();
            
            std::cout << "[FeatureAnalyzer] Sample #" << features.sample_id << " analysis:\n";
            std::cout << "  - Dominant frequency: " << std::fixed << std::setprecision(1) 
                      << features.dominant_frequency << " Hz\n";
            std::cout << "  - Spectral centroid: " << features.spectral_centroid << " Hz\n";
            std::cout << "  - Total power: " << std::scientific << std::setprecision(3) 
                      << features.total_power << "\n";
            std::cout << "  - Peak frequencies: ";
            
            for (double freq : features.peak_frequencies) {
                std::cout << std::fixed << std::setprecision(1) << freq << " ";
            }
            std::cout << "Hz\n\n";
        }
    });
    
    signal_generator.join();
    feature_analyzer.join();
    processor->stop();
}

int main() {
    std::cout << "=== Producer-Consumer Pattern - Scientific Computing ===\n";
    std::cout << "Parallel processing pipelines for scientific data\n\n";
    
    molecularDynamicsExample();
    scientificEventLoggingExample();
    spectralAnalysisPipeline();
    
    std::cout << "\n=== Pattern Benefits in Scientific Computing ===\n";
    std::cout << "• Decouples data generation from analysis\n";
    std::cout << "• Enables parallel processing of simulation timesteps\n";
    std::cout << "• Natural load balancing between producers and consumers\n";
    std::cout << "• Buffering handles varying computation speeds\n";
    std::cout << "• Pipeline architecture for multi-stage processing\n";
    
    return 0;
}