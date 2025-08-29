// Observer Pattern - Scientific Simulation Monitoring and Analysis
// Enables real-time monitoring and analysis of scientific simulations
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <cmath>
#include <fstream>
#include <chrono>
#include <random>

// Forward declarations
class SimulationObserver;

// Simulation data structure
struct SimulationData {
    double currentTime;
    int stepNumber;
    double totalEnergy;
    double kineticEnergy;
    double potentialEnergy;
    double temperature;
    double pressure;
    double volume;
    std::vector<double> forces;
    std::vector<double> positions;
    double convergenceResidual;
    std::string status;
    
    SimulationData() : currentTime(0.0), stepNumber(0), totalEnergy(0.0),
                      kineticEnergy(0.0), potentialEnergy(0.0), temperature(0.0),
                      pressure(0.0), volume(0.0), convergenceResidual(0.0),
                      status("Initializing") {}
};

// Subject interface - Observable Simulation
class SimulationSubject {
public:
    virtual ~SimulationSubject() = default;
    virtual void attachObserver(std::shared_ptr<SimulationObserver> observer) = 0;
    virtual void detachObserver(std::shared_ptr<SimulationObserver> observer) = 0;
    virtual void notifyObservers() = 0;
    virtual const SimulationData& getSimulationData() const = 0;
};

// Observer interface - Simulation Monitor
class SimulationObserver {
public:
    virtual ~SimulationObserver() = default;
    virtual void onSimulationUpdate(const SimulationData& data) = 0;
    virtual std::string getObserverName() const = 0;
    virtual bool isActive() const { return true; }
};

// Concrete Subject - Molecular Dynamics Simulation
class MolecularDynamicsSimulation : public SimulationSubject {
private:
    std::vector<std::shared_ptr<SimulationObserver>> observers_;
    SimulationData currentData_;
    std::mt19937 rng_;
    int numParticles_;
    double timeStep_;
    double boxLength_;
    
public:
    MolecularDynamicsSimulation(int numParticles, double timeStep, double boxLength)
        : rng_(42), numParticles_(numParticles), timeStep_(timeStep), boxLength_(boxLength) {
        initializeSimulation();
    }
    
    void attachObserver(std::shared_ptr<SimulationObserver> observer) override {
        observers_.push_back(observer);
        std::cout << "[SIM] Attached monitor: " << observer->getObserverName() << "\n";
    }
    
    void detachObserver(std::shared_ptr<SimulationObserver> observer) override {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end()) {
            observers_.erase(it);
            std::cout << "[SIM] Detached monitor: " << observer->getObserverName() << "\n";
        }
    }
    
    void notifyObservers() override {
        for (auto& observer : observers_) {
            if (observer->isActive()) {
                observer->onSimulationUpdate(currentData_);
            }
        }
    }
    
    const SimulationData& getSimulationData() const override {
        return currentData_;
    }
    
    void initializeSimulation() {
        currentData_.status = "Initializing";
        currentData_.volume = boxLength_ * boxLength_ * boxLength_;
        currentData_.positions.resize(3 * numParticles_);
        currentData_.forces.resize(3 * numParticles_);
        
        // Initialize random positions
        std::uniform_real_distribution<double> posDist(0.0, boxLength_);
        for (int i = 0; i < numParticles_; ++i) {
            currentData_.positions[3*i] = posDist(rng_);
            currentData_.positions[3*i+1] = posDist(rng_);
            currentData_.positions[3*i+2] = posDist(rng_);
        }
        
        calculateEnergies();
        currentData_.status = "Ready";
        std::cout << "[SIM] Initialized MD simulation with " << numParticles_ << " particles\n";
        notifyObservers();
    }
    
    void runTimeStep() {
        currentData_.stepNumber++;
        currentData_.currentTime += timeStep_;
        currentData_.status = "Running";
        
        // Simulate physics calculations
        calculateForces();
        integratePositions();
        calculateEnergies();
        calculateThermodynamics();
        
        // Simulate convergence checking
        currentData_.convergenceResidual = 1e-6 * (1.0 + 0.1 * std::sin(currentData_.stepNumber * 0.1));
        
        notifyObservers();
    }
    
    void runSteps(int numSteps) {
        std::cout << "\n[SIM] Running " << numSteps << " MD steps...\n";
        for (int i = 0; i < numSteps; ++i) {
            runTimeStep();
            
            // Simulate occasional issues
            if (currentData_.stepNumber % 47 == 0) {
                currentData_.status = "Warning: Energy drift detected";
                notifyObservers();
                currentData_.status = "Running";
            }
        }
        currentData_.status = "Completed";
        notifyObservers();
    }
    
private:
    void calculateForces() {
        // Simplified force calculation
        std::normal_distribution<double> forceDist(0.0, 1e-18);
        for (size_t i = 0; i < currentData_.forces.size(); ++i) {
            currentData_.forces[i] = forceDist(rng_);
        }
    }
    
    void integratePositions() {
        // Simple integration
        for (int i = 0; i < numParticles_; ++i) {
            currentData_.positions[3*i] += 0.1 * timeStep_ * currentData_.forces[3*i];
            currentData_.positions[3*i+1] += 0.1 * timeStep_ * currentData_.forces[3*i+1];
            currentData_.positions[3*i+2] += 0.1 * timeStep_ * currentData_.forces[3*i+2];
            
            // Apply periodic boundary conditions
            currentData_.positions[3*i] = fmod(currentData_.positions[3*i] + boxLength_, boxLength_);
            currentData_.positions[3*i+1] = fmod(currentData_.positions[3*i+1] + boxLength_, boxLength_);
            currentData_.positions[3*i+2] = fmod(currentData_.positions[3*i+2] + boxLength_, boxLength_);
        }
    }
    
    void calculateEnergies() {
        // Simplified energy calculation
        currentData_.kineticEnergy = 1.5 * numParticles_ * 1.38e-23 * 300.0; // kT
        currentData_.potentialEnergy = -2.0 * currentData_.kineticEnergy * (1.0 + 0.05 * std::sin(currentData_.stepNumber * 0.01));
        currentData_.totalEnergy = currentData_.kineticEnergy + currentData_.potentialEnergy;
    }
    
    void calculateThermodynamics() {
        // Calculate thermodynamic properties
        currentData_.temperature = (2.0 * currentData_.kineticEnergy) / (3.0 * numParticles_ * 1.38e-23);
        currentData_.pressure = (2.0 * currentData_.kineticEnergy) / (3.0 * currentData_.volume);
    }
};

// Concrete Observers - Simulation Monitors
class EnergyConservationMonitor : public SimulationObserver {
private:
    std::string name_ = "Energy Conservation Monitor";
    double initialEnergy_;
    bool initialized_ = false;
    double tolerance_ = 1e-12;
    
public:
    void onSimulationUpdate(const SimulationData& data) override {
        if (!initialized_ && data.stepNumber > 0) {
            initialEnergy_ = data.totalEnergy;
            initialized_ = true;
            std::cout << "[" << name_ << "] Initial energy: " 
                      << std::scientific << initialEnergy_ << " J\n";
            return;
        }
        
        if (initialized_) {
            double energyDrift = std::abs(data.totalEnergy - initialEnergy_);
            double relativeDrift = energyDrift / std::abs(initialEnergy_);
            
            if (relativeDrift > tolerance_) {
                std::cout << "[" << name_ << "] ⚠️  ENERGY DRIFT WARNING: "
                          << std::scientific << std::setprecision(3) 
                          << relativeDrift << " (step " << data.stepNumber << ")\n";
            }
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

class PerformanceMonitor : public SimulationObserver {
private:
    std::string name_ = "Performance Monitor";
    std::chrono::steady_clock::time_point lastUpdateTime_;
    int lastStep_ = 0;
    bool initialized_ = false;
    
public:
    void onSimulationUpdate(const SimulationData& data) override {
        auto currentTime = std::chrono::steady_clock::now();
        
        if (!initialized_) {
            lastUpdateTime_ = currentTime;
            lastStep_ = data.stepNumber;
            initialized_ = true;
            return;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime_);
        int stepsDone = data.stepNumber - lastStep_;
        
        if (elapsed.count() > 1000 && stepsDone > 0) { // Report every second
            double stepsPerSecond = (double)stepsDone / (elapsed.count() / 1000.0);
            
            std::cout << "[" << name_ << "] Performance: " 
                      << std::fixed << std::setprecision(1) << stepsPerSecond 
                      << " steps/sec (step " << data.stepNumber << ")\n";
            
            lastUpdateTime_ = currentTime;
            lastStep_ = data.stepNumber;
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

class ThermodynamicsLogger : public SimulationObserver {
private:
    std::string name_ = "Thermodynamics Logger";
    std::vector<double> temperatureHistory_;
    std::vector<double> pressureHistory_;
    int reportInterval_ = 10;
    
public:
    void onSimulationUpdate(const SimulationData& data) override {
        temperatureHistory_.push_back(data.temperature);
        pressureHistory_.push_back(data.pressure);
        
        if (data.stepNumber % reportInterval_ == 0 && data.stepNumber > 0) {
            double avgTemp = 0.0, avgPressure = 0.0;
            for (double temp : temperatureHistory_) avgTemp += temp;
            for (double press : pressureHistory_) avgPressure += press;
            
            avgTemp /= temperatureHistory_.size();
            avgPressure /= pressureHistory_.size();
            
            std::cout << "[" << name_ << "] Step " << data.stepNumber
                      << ": T=" << std::fixed << std::setprecision(2) << data.temperature << " K"
                      << ", P=" << std::scientific << data.pressure << " Pa"
                      << " (avg: T=" << std::fixed << avgTemp << " K)\n";
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

class DataRecorder : public SimulationObserver {
private:
    std::string name_ = "Data Recorder";
    std::string filename_;
    std::ofstream file_;
    bool fileOpen_ = false;
    
public:
    DataRecorder(const std::string& filename) : filename_(filename) {}
    
    ~DataRecorder() {
        if (fileOpen_) {
            file_.close();
        }
    }
    
    void onSimulationUpdate(const SimulationData& data) override {
        if (!fileOpen_) {
            file_.open(filename_);
            if (file_.is_open()) {
                fileOpen_ = true;
                file_ << "# Step Time(s) TotalEnergy(J) KineticEnergy(J) PotentialEnergy(J) Temperature(K) Pressure(Pa)\n";
                std::cout << "[" << name_ << "] Started recording to " << filename_ << "\n";
            }
        }
        
        if (fileOpen_ && data.stepNumber % 5 == 0) { // Record every 5 steps
            file_ << data.stepNumber << " "
                  << std::scientific << data.currentTime << " "
                  << data.totalEnergy << " "
                  << data.kineticEnergy << " "
                  << data.potentialEnergy << " "
                  << data.temperature << " "
                  << data.pressure << "\n";
            file_.flush();
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

// Climate Model Monitoring
struct ClimateData {
    double simulationYear;
    double globalMeanTemperature;
    double co2Concentration;
    double seaLevelRise;
    double arcticIceExtent;
    double precipitationAnomaly;
    std::string scenario;
    std::string status;
    
    ClimateData() : simulationYear(2020.0), globalMeanTemperature(288.15),
                   co2Concentration(400.0), seaLevelRise(0.0), 
                   arcticIceExtent(1.0), precipitationAnomaly(0.0),
                   scenario("RCP4.5"), status("Initializing") {}
};

class ClimateSimulation : public SimulationSubject {
private:
    std::vector<std::shared_ptr<SimulationObserver>> observers_;
    ClimateData currentData_;
    std::mt19937 rng_;
    double emissionRate_;
    
public:
    ClimateSimulation(const std::string& scenario, double emissionRate)
        : rng_(123), emissionRate_(emissionRate) {
        currentData_.scenario = scenario;
        currentData_.status = "Ready";
        std::cout << "[CLIMATE] Initialized " << scenario << " climate simulation\n";
    }
    
    void attachObserver(std::shared_ptr<SimulationObserver> observer) override {
        observers_.push_back(observer);
        std::cout << "[CLIMATE] Attached climate monitor: " << observer->getObserverName() << "\n";
    }
    
    void detachObserver(std::shared_ptr<SimulationObserver> observer) override {
        auto it = std::find(observers_.begin(), observers_.end(), observer);
        if (it != observers_.end()) {
            observers_.erase(it);
            std::cout << "[CLIMATE] Detached monitor: " << observer->getObserverName() << "\n";
        }
    }
    
    void notifyObservers() override {
        for (auto& observer : observers_) {
            if (observer->isActive()) {
                // Adapt SimulationData for climate data
                SimulationData adaptedData;
                adaptedData.currentTime = currentData_.simulationYear;
                adaptedData.temperature = currentData_.globalMeanTemperature;
                adaptedData.pressure = currentData_.co2Concentration;
                adaptedData.status = currentData_.status;
                observer->onSimulationUpdate(adaptedData);
            }
        }
        
        // Also notify climate-specific observers
        for (auto& observer : climateObservers_) {
            observer->onClimateUpdate(currentData_);
        }
    }
    
    const SimulationData& getSimulationData() const override {
        static SimulationData adaptedData;
        adaptedData.currentTime = currentData_.simulationYear;
        adaptedData.temperature = currentData_.globalMeanTemperature;
        adaptedData.pressure = currentData_.co2Concentration;
        adaptedData.status = currentData_.status;
        return adaptedData;
    }
    
    void simulateYears(int numYears) {
        std::cout << "\n[CLIMATE] Simulating " << numYears << " years...\n";
        currentData_.status = "Running";
        
        std::normal_distribution<double> tempNoise(0.0, 0.1);
        
        for (int year = 0; year < numYears; ++year) {
            currentData_.simulationYear += 1.0;
            currentData_.co2Concentration += emissionRate_;
            
            // Simple climate model
            double forcing = 5.35 * std::log(currentData_.co2Concentration / 280.0);
            double tempIncrease = 0.8 * forcing / 3.7; // Climate sensitivity ~3°C per doubling
            currentData_.globalMeanTemperature = 288.15 + tempIncrease + tempNoise(rng_);
            
            // Calculate impacts
            double tempAnomaly = currentData_.globalMeanTemperature - 288.15;
            currentData_.seaLevelRise = tempAnomaly * 0.2; // 20cm per degree
            currentData_.arcticIceExtent = std::max(0.1, 1.0 - tempAnomaly * 0.15);
            currentData_.precipitationAnomaly = tempAnomaly * 0.05;
            
            if ((year + 1) % (numYears / 3) == 0) {
                notifyObservers();
            }
            
            // Check for tipping points
            if (tempAnomaly > 4.0) {
                currentData_.status = "WARNING: Extreme warming scenario";
                notifyObservers();
                currentData_.status = "Running";
            }
        }
        
        currentData_.status = "Completed";
        notifyObservers();
    }
    
private:
    // Climate-specific observer interface
    class ClimateObserver {
    public:
        virtual ~ClimateObserver() = default;
        virtual void onClimateUpdate(const ClimateData& data) = 0;
        virtual std::string getName() const = 0;
    };
    
    std::vector<std::shared_ptr<ClimateObserver>> climateObservers_;
    
public:
    void attachClimateObserver(std::shared_ptr<ClimateObserver> observer) {
        climateObservers_.push_back(observer);
    }
    
    const ClimateData& getClimateData() const { return currentData_; }
};

class ClimateImpactMonitor : public SimulationObserver {
private:
    std::string name_ = "Climate Impact Monitor";
    double baselineTemp_ = 288.15;
    
public:
    void onSimulationUpdate(const SimulationData& data) override {
        double tempAnomaly = data.temperature - baselineTemp_;
        
        if (tempAnomaly > 1.5) {
            std::cout << "[" << name_ << "] ⚠️  Paris Agreement threshold exceeded: +"
                      << std::fixed << std::setprecision(2) << tempAnomaly << "°C\n";
        }
        
        if (tempAnomaly > 2.0) {
            std::cout << "[" << name_ << "] 🚨 CRITICAL: Dangerous warming level: +"
                      << tempAnomaly << "°C\n";
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

class EmissionTracker : public SimulationObserver {
private:
    std::string name_ = "CO2 Emission Tracker";
    double initialCO2_ = 400.0;
    
public:
    void onSimulationUpdate(const SimulationData& data) override {
        double co2Level = data.pressure; // Using pressure field for CO2
        double increase = co2Level - initialCO2_;
        
        if (co2Level > 450.0) {
            std::cout << "[" << name_ << "] ⚠️  CO2 level: " 
                      << std::fixed << std::setprecision(1) << co2Level 
                      << " ppm (+" << increase << " ppm)\n";
        }
        
        if (co2Level > 560.0) { // 2x pre-industrial
            std::cout << "[" << name_ << "] 🚨 CO2 DOUBLING: " << co2Level << " ppm!\n";
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

// Scientific Event System for HPC Monitoring
class SimulationEventType {
public:
    static const std::string SIMULATION_STARTED;
    static const std::string STEP_COMPLETED;
    static const std::string CONVERGENCE_ACHIEVED;
    static const std::string ERROR_DETECTED;
    static const std::string CHECKPOINT_CREATED;
    static const std::string SIMULATION_COMPLETED;
};

const std::string SimulationEventType::SIMULATION_STARTED = "simulation_started";
const std::string SimulationEventType::STEP_COMPLETED = "step_completed";
const std::string SimulationEventType::CONVERGENCE_ACHIEVED = "convergence_achieved";
const std::string SimulationEventType::ERROR_DETECTED = "error_detected";
const std::string SimulationEventType::CHECKPOINT_CREATED = "checkpoint_created";
const std::string SimulationEventType::SIMULATION_COMPLETED = "simulation_completed";

class SimulationEvent {
private:
    std::string type_;
    std::unordered_map<std::string, std::string> metadata_;
    std::chrono::system_clock::time_point timestamp_;
    
public:
    SimulationEvent(const std::string& type) : type_(type), timestamp_(std::chrono::system_clock::now()) {}
    
    void setMetadata(const std::string& key, const std::string& value) {
        metadata_[key] = value;
    }
    
    void setMetadata(const std::string& key, double value) {
        metadata_[key] = std::to_string(value);
    }
    
    std::string getType() const { return type_; }
    
    std::string getMetadata(const std::string& key) const {
        auto it = metadata_.find(key);
        return (it != metadata_.end()) ? it->second : "";
    }
    
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }
};

class SimulationEventObserver {
public:
    virtual ~SimulationEventObserver() = default;
    virtual void onSimulationEvent(const SimulationEvent& event) = 0;
    virtual std::string getObserverName() const = 0;
};

class HpcEventManager {
private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<SimulationEventObserver>>> eventListeners_;
    
public:
    void subscribe(const std::string& eventType, std::shared_ptr<SimulationEventObserver> observer) {
        eventListeners_[eventType].push_back(observer);
        std::cout << "[HPC-EVENTS] Subscribed " << observer->getObserverName() 
                  << " to " << eventType << "\n";
    }
    
    void unsubscribe(const std::string& eventType, std::shared_ptr<SimulationEventObserver> observer) {
        auto& list = eventListeners_[eventType];
        auto it = std::find(list.begin(), list.end(), observer);
        if (it != list.end()) {
            list.erase(it);
            std::cout << "[HPC-EVENTS] Unsubscribed " << observer->getObserverName() 
                      << " from " << eventType << "\n";
        }
    }
    
    void publishEvent(const SimulationEvent& event) {
        auto it = eventListeners_.find(event.getType());
        if (it != eventListeners_.end()) {
            for (auto& observer : it->second) {
                observer->onSimulationEvent(event);
            }
        }
    }
};

class HpcLogger : public SimulationEventObserver {
private:
    std::string name_ = "HPC System Logger";
    std::ofstream logFile_;
    
public:
    HpcLogger(const std::string& logFilename) {
        logFile_.open(logFilename, std::ios::app);
    }
    
    ~HpcLogger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }
    
    void onSimulationEvent(const SimulationEvent& event) override {
        auto now = std::time(nullptr);
        std::cout << "[" << name_ << "] "
                  << std::put_time(std::localtime(&now), "%H:%M:%S")
                  << " " << event.getType();
        
        if (!event.getMetadata("step").empty()) {
            std::cout << " step=" << event.getMetadata("step");
        }
        if (!event.getMetadata("energy").empty()) {
            std::cout << " energy=" << event.getMetadata("energy");
        }
        std::cout << "\n";
        
        // Also log to file
        if (logFile_.is_open()) {
            logFile_ << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S")
                     << " " << event.getType() << "\n";
            logFile_.flush();
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

class FailureDetector : public SimulationEventObserver {
private:
    std::string name_ = "HPC Failure Detector";
    int consecutiveErrors_ = 0;
    
public:
    void onSimulationEvent(const SimulationEvent& event) override {
        if (event.getType() == SimulationEventType::ERROR_DETECTED) {
            consecutiveErrors_++;
            std::cout << "[" << name_ << "] 🚨 ERROR #" << consecutiveErrors_ 
                      << ": " << event.getMetadata("message") << "\n";
            
            if (consecutiveErrors_ >= 3) {
                std::cout << "[" << name_ << "] 🚨 CRITICAL: Multiple failures detected! "
                          << "Recommending simulation abort.\n";
                
                // Trigger emergency checkpoint
                SimulationEvent checkpointEvent(SimulationEventType::CHECKPOINT_CREATED);
                checkpointEvent.setMetadata("reason", "emergency");
                checkpointEvent.setMetadata("priority", "critical");
                std::cout << "[" << name_ << "] Triggering emergency checkpoint...\n";
            }
        } else {
            consecutiveErrors_ = 0; // Reset on successful events
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

class ResourceMonitor : public SimulationEventObserver {
private:
    std::string name_ = "HPC Resource Monitor";
    
public:
    void onSimulationEvent(const SimulationEvent& event) override {
        if (event.getType() == SimulationEventType::STEP_COMPLETED) {
            // Simulate resource monitoring
            static int memoryUsage = 50;
            memoryUsage += (rand() % 10 - 5); // Random walk
            memoryUsage = std::max(10, std::min(95, memoryUsage));
            
            if (memoryUsage > 90) {
                std::cout << "[" << name_ << "] ⚠️  HIGH MEMORY WARNING: " 
                          << memoryUsage << "% used\n";
            }
            
            if (!event.getMetadata("step").empty()) {
                int step = std::stoi(event.getMetadata("step"));
                if (step % 20 == 0) {
                    std::cout << "[" << name_ << "] Resources: Memory " 
                              << memoryUsage << "%, CPU 85%\n";
                }
            }
        }
    }
    
    std::string getObserverName() const override { return name_; }
};

int main() {
    std::cout << "=== Scientific Simulation Monitoring System ===\n\n";
    
    // Create MD simulation
    auto mdSimulation = std::make_shared<MolecularDynamicsSimulation>(100, 1e-15, 10.0);
    
    // Create simulation monitors
    auto energyMonitor = std::make_shared<EnergyConservationMonitor>();
    auto perfMonitor = std::make_shared<PerformanceMonitor>();
    auto thermoLogger = std::make_shared<ThermodynamicsLogger>();
    auto dataRecorder = std::make_shared<DataRecorder>("md_simulation.dat");
    
    // Attach monitors
    mdSimulation->attachObserver(energyMonitor);
    mdSimulation->attachObserver(perfMonitor);
    mdSimulation->attachObserver(thermoLogger);
    mdSimulation->attachObserver(dataRecorder);
    
    // Run simulation
    std::cout << "\n=== Running Molecular Dynamics Simulation ===\n";
    mdSimulation->runSteps(50);
    
    // Detach performance monitor
    std::cout << "\n=== Detaching Performance Monitor ===\n";
    mdSimulation->detachObserver(perfMonitor);
    
    // Continue simulation
    mdSimulation->runSteps(30);
    
    // Climate Simulation Example
    std::cout << "\n\n=== Climate Simulation Monitoring ===\n";
    
    auto climateSimRCP45 = std::make_shared<ClimateSimulation>("RCP4.5", 2.0);
    auto climateSimRCP85 = std::make_shared<ClimateSimulation>("RCP8.5", 4.0);
    
    auto impactMonitor = std::make_shared<ClimateImpactMonitor>();
    auto emissionTracker = std::make_shared<EmissionTracker>();
    
    // Monitor both climate scenarios
    climateSimRCP45->attachObserver(impactMonitor);
    climateSimRCP45->attachObserver(emissionTracker);
    climateSimRCP85->attachObserver(impactMonitor);
    climateSimRCP85->attachObserver(emissionTracker);
    
    // Run climate scenarios
    std::cout << "\n--- RCP4.5 Moderate Warming Scenario ---\n";
    climateSimRCP45->simulateYears(50);
    
    std::cout << "\n--- RCP8.5 High Emissions Scenario ---\n";
    climateSimRCP85->simulateYears(50);
    
    // HPC Event System Example
    std::cout << "\n\n=== HPC Event Monitoring System ===\n";
    
    HpcEventManager eventManager;
    auto hpcLogger = std::make_shared<HpcLogger>("hpc_simulation.log");
    auto failureDetector = std::make_shared<FailureDetector>();
    auto resourceMonitor = std::make_shared<ResourceMonitor>();
    
    // Subscribe to simulation events
    eventManager.subscribe(SimulationEventType::SIMULATION_STARTED, hpcLogger);
    eventManager.subscribe(SimulationEventType::STEP_COMPLETED, hpcLogger);
    eventManager.subscribe(SimulationEventType::STEP_COMPLETED, resourceMonitor);
    eventManager.subscribe(SimulationEventType::ERROR_DETECTED, hpcLogger);
    eventManager.subscribe(SimulationEventType::ERROR_DETECTED, failureDetector);
    eventManager.subscribe(SimulationEventType::CONVERGENCE_ACHIEVED, hpcLogger);
    eventManager.subscribe(SimulationEventType::SIMULATION_COMPLETED, hpcLogger);
    
    // Simulate HPC events
    std::cout << "\n--- Simulating HPC Simulation Events ---\n";
    
    SimulationEvent startEvent(SimulationEventType::SIMULATION_STARTED);
    startEvent.setMetadata("nodes", "64");
    startEvent.setMetadata("cores", "2048");
    eventManager.publishEvent(startEvent);
    
    // Simulate some computation steps
    for (int step = 1; step <= 10; ++step) {
        SimulationEvent stepEvent(SimulationEventType::STEP_COMPLETED);
        stepEvent.setMetadata("step", std::to_string(step));
        stepEvent.setMetadata("energy", std::to_string(-1.5e-17 + step * 1e-19));
        eventManager.publishEvent(stepEvent);
    }
    
    // Simulate some errors
    SimulationEvent errorEvent1(SimulationEventType::ERROR_DETECTED);
    errorEvent1.setMetadata("message", "Node 23 communication timeout");
    eventManager.publishEvent(errorEvent1);
    
    SimulationEvent errorEvent2(SimulationEventType::ERROR_DETECTED);
    errorEvent2.setMetadata("message", "Memory allocation failed on node 45");
    eventManager.publishEvent(errorEvent2);
    
    SimulationEvent errorEvent3(SimulationEventType::ERROR_DETECTED);
    errorEvent3.setMetadata("message", "Numerical instability detected");
    eventManager.publishEvent(errorEvent3);
    
    // Convergence and completion
    SimulationEvent convergenceEvent(SimulationEventType::CONVERGENCE_ACHIEVED);
    convergenceEvent.setMetadata("residual", "1.2e-12");
    convergenceEvent.setMetadata("iterations", "156");
    eventManager.publishEvent(convergenceEvent);
    
    SimulationEvent completionEvent(SimulationEventType::SIMULATION_COMPLETED);
    completionEvent.setMetadata("total_steps", "1000000");
    completionEvent.setMetadata("walltime", "04:32:18");
    eventManager.publishEvent(completionEvent);
    
    std::cout << "\nObserver pattern enables comprehensive real-time monitoring\n";
    std::cout << "of scientific simulations with automated analysis and alerts!\n";
    
    return 0;
}