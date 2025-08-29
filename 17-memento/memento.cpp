// Memento Pattern - Scientific Simulation State Management
// Provides checkpoint/restart capability for long-running scientific simulations
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stack>
#include <cmath>
#include <random>
#include <fstream>
#include <chrono>

// Forward declaration
class MolecularDynamicsSimulation;

// Simulation state snapshot for checkpoint/restart
struct ParticleState {
    std::vector<double> positions;  // x, y, z coordinates
    std::vector<double> velocities; // vx, vy, vz components
    std::vector<double> forces;     // fx, fy, fz components
    double potentialEnergy;
    double kineticEnergy;
    double temperature;
    double pressure;
    
    ParticleState(size_t numParticles) 
        : positions(3 * numParticles), velocities(3 * numParticles), 
          forces(3 * numParticles), potentialEnergy(0.0), kineticEnergy(0.0),
          temperature(0.0), pressure(0.0) {}
};

// Memento class - stores complete simulation state
class SimulationMemento {
private:
    ParticleState particleState_;
    double currentTime_;
    double timeStep_;
    int stepNumber_;
    double boxLength_;
    int numParticles_;
    std::string timestamp_;
    std::string checkpointReason_;
    size_t memoryUsage_;
    
    // Only MolecularDynamicsSimulation can create mementos
    friend class MolecularDynamicsSimulation;
    
    SimulationMemento(const ParticleState& state, double time, double dt, 
                     int step, double boxLen, int numPart, const std::string& reason)
        : particleState_(state), currentTime_(time), timeStep_(dt), 
          stepNumber_(step), boxLength_(boxLen), numParticles_(numPart),
          checkpointReason_(reason) {
        // Generate timestamp
        auto now = std::time(nullptr);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
        timestamp_ = ss.str();
        
        // Estimate memory usage
        memoryUsage_ = sizeof(ParticleState) + particleState_.positions.capacity() * sizeof(double) * 3;
    }
    
public:
    std::string getDescription() const {
        std::stringstream ss;
        ss << "Checkpoint: " << checkpointReason_ << " at " << timestamp_
           << "\n  Simulation time: " << std::scientific << currentTime_ << " s"
           << "\n  Step: " << stepNumber_ << ", Particles: " << numParticles_
           << "\n  Energy: " << std::fixed << std::setprecision(3) 
           << (particleState_.kineticEnergy + particleState_.potentialEnergy) << " J"
           << "\n  Memory: " << (memoryUsage_ / 1024.0) << " KB";
        return ss.str();
    }
    
    double getSimulationTime() const { return currentTime_; }
    int getStepNumber() const { return stepNumber_; }
    const std::string& getReason() const { return checkpointReason_; }
};

// Originator - Molecular Dynamics Simulation that creates and restores from mementos
class MolecularDynamicsSimulation {
private:
    ParticleState currentState_;
    double currentTime_;
    double timeStep_;
    int stepNumber_;
    double boxLength_;
    int numParticles_;
    double temperature_;
    double cutoffRadius_;
    std::mt19937 rng_;
    
public:
    MolecularDynamicsSimulation(int numParticles, double boxLength, double temp)
        : currentState_(numParticles), currentTime_(0.0), timeStep_(1e-15),
          stepNumber_(0), boxLength_(boxLength), numParticles_(numParticles),
          temperature_(temp), cutoffRadius_(2.5), rng_(42) {
        initializeSystem();
    }
    
    void initializeSystem() {
        std::uniform_real_distribution<double> positionDist(0.0, boxLength_);
        std::normal_distribution<double> velocityDist(0.0, std::sqrt(temperature_));
        
        // Initialize positions randomly
        for (int i = 0; i < numParticles_; ++i) {
            currentState_.positions[3*i] = positionDist(rng_);
            currentState_.positions[3*i+1] = positionDist(rng_);
            currentState_.positions[3*i+2] = positionDist(rng_);
            
            currentState_.velocities[3*i] = velocityDist(rng_);
            currentState_.velocities[3*i+1] = velocityDist(rng_);
            currentState_.velocities[3*i+2] = velocityDist(rng_);
        }
        
        calculateForces();
        calculateThermodynamics();
        
        std::cout << "Initialized MD system with " << numParticles_ 
                  << " particles in " << boxLength_ << "³ box\n";
    }
    
    void calculateForces() {
        // Reset forces
        std::fill(currentState_.forces.begin(), currentState_.forces.end(), 0.0);
        currentState_.potentialEnergy = 0.0;
        
        // Simplified Lennard-Jones potential calculation
        for (int i = 0; i < numParticles_; ++i) {
            for (int j = i + 1; j < numParticles_; ++j) {
                double dx = currentState_.positions[3*i] - currentState_.positions[3*j];
                double dy = currentState_.positions[3*i+1] - currentState_.positions[3*j+1];
                double dz = currentState_.positions[3*i+2] - currentState_.positions[3*j+2];
                
                // Apply periodic boundary conditions
                dx -= boxLength_ * std::round(dx / boxLength_);
                dy -= boxLength_ * std::round(dy / boxLength_);
                dz -= boxLength_ * std::round(dz / boxLength_);
                
                double r2 = dx*dx + dy*dy + dz*dz;
                
                if (r2 < cutoffRadius_ * cutoffRadius_) {
                    double r2inv = 1.0 / r2;
                    double r6inv = r2inv * r2inv * r2inv;
                    double r12inv = r6inv * r6inv;
                    
                    double force_magnitude = 24.0 * (2.0 * r12inv - r6inv) * r2inv;
                    currentState_.potentialEnergy += 4.0 * (r12inv - r6inv);
                    
                    double fx = force_magnitude * dx;
                    double fy = force_magnitude * dy;
                    double fz = force_magnitude * dz;
                    
                    currentState_.forces[3*i] += fx;
                    currentState_.forces[3*i+1] += fy;
                    currentState_.forces[3*i+2] += fz;
                    currentState_.forces[3*j] -= fx;
                    currentState_.forces[3*j+1] -= fy;
                    currentState_.forces[3*j+2] -= fz;
                }
            }
        }
    }
    
    void calculateThermodynamics() {
        currentState_.kineticEnergy = 0.0;
        for (int i = 0; i < numParticles_; ++i) {
            double vx = currentState_.velocities[3*i];
            double vy = currentState_.velocities[3*i+1];
            double vz = currentState_.velocities[3*i+2];
            currentState_.kineticEnergy += 0.5 * (vx*vx + vy*vy + vz*vz);
        }
        
        currentState_.temperature = (2.0 * currentState_.kineticEnergy) / (3.0 * numParticles_);
        currentState_.pressure = (2.0 * currentState_.kineticEnergy) / (3.0 * boxLength_ * boxLength_ * boxLength_);
    }
    
    void integrateTimeStep() {
        // Velocity Verlet integration
        for (int i = 0; i < numParticles_; ++i) {
            // Update positions
            currentState_.positions[3*i] += currentState_.velocities[3*i] * timeStep_ + 
                                           0.5 * currentState_.forces[3*i] * timeStep_ * timeStep_;
            currentState_.positions[3*i+1] += currentState_.velocities[3*i+1] * timeStep_ + 
                                             0.5 * currentState_.forces[3*i+1] * timeStep_ * timeStep_;
            currentState_.positions[3*i+2] += currentState_.velocities[3*i+2] * timeStep_ + 
                                             0.5 * currentState_.forces[3*i+2] * timeStep_ * timeStep_;
            
            // Apply periodic boundary conditions
            currentState_.positions[3*i] = fmod(currentState_.positions[3*i] + boxLength_, boxLength_);
            currentState_.positions[3*i+1] = fmod(currentState_.positions[3*i+1] + boxLength_, boxLength_);
            currentState_.positions[3*i+2] = fmod(currentState_.positions[3*i+2] + boxLength_, boxLength_);
        }
        
        // Store old forces
        std::vector<double> oldForces = currentState_.forces;
        
        // Recalculate forces
        calculateForces();
        
        // Update velocities
        for (int i = 0; i < numParticles_; ++i) {
            currentState_.velocities[3*i] += 0.5 * (oldForces[3*i] + currentState_.forces[3*i]) * timeStep_;
            currentState_.velocities[3*i+1] += 0.5 * (oldForces[3*i+1] + currentState_.forces[3*i+1]) * timeStep_;
            currentState_.velocities[3*i+2] += 0.5 * (oldForces[3*i+2] + currentState_.forces[3*i+2]) * timeStep_;
        }
        
        currentTime_ += timeStep_;
        stepNumber_++;
        calculateThermodynamics();
    }
    
    void runSteps(int numSteps) {
        std::cout << "Running " << numSteps << " MD steps...\n";
        for (int i = 0; i < numSteps; ++i) {
            integrateTimeStep();
            if ((i + 1) % (numSteps / 5) == 0) {
                std::cout << "  Step " << stepNumber_ << ": T=" << std::fixed 
                          << std::setprecision(2) << currentState_.temperature 
                          << " K, E=" << std::setprecision(4) 
                          << (currentState_.kineticEnergy + currentState_.potentialEnergy) << " J\n";
            }
        }
    }
    
    void displayStatus() const {
        std::cout << "\n=== MD Simulation Status ===\n";
        std::cout << "Time: " << std::scientific << currentTime_ << " s\n";
        std::cout << "Step: " << stepNumber_ << "\n";
        std::cout << "Particles: " << numParticles_ << "\n";
        std::cout << "Box length: " << std::fixed << boxLength_ << " units\n";
        std::cout << "Temperature: " << std::setprecision(2) << currentState_.temperature << " K\n";
        std::cout << "Pressure: " << std::scientific << currentState_.pressure << " Pa\n";
        std::cout << "Kinetic Energy: " << currentState_.kineticEnergy << " J\n";
        std::cout << "Potential Energy: " << currentState_.potentialEnergy << " J\n";
        std::cout << "Total Energy: " << (currentState_.kineticEnergy + currentState_.potentialEnergy) << " J\n";
        std::cout << "============================\n\n";
    }
    
    // Create memento
    std::unique_ptr<SimulationMemento> saveCheckpoint(const std::string& reason) const {
        std::cout << "Creating simulation checkpoint: " << reason << "...\n";
        return std::unique_ptr<SimulationMemento>(
            new SimulationMemento(currentState_, currentTime_, timeStep_, 
                                stepNumber_, boxLength_, numParticles_, reason)
        );
    }
    
    // Restore from memento
    void restoreCheckpoint(const SimulationMemento& checkpoint) {
        currentState_ = checkpoint.particleState_;
        currentTime_ = checkpoint.currentTime_;
        timeStep_ = checkpoint.timeStep_;
        stepNumber_ = checkpoint.stepNumber_;
        boxLength_ = checkpoint.boxLength_;
        numParticles_ = checkpoint.numParticles_;
        std::cout << "Restored simulation from checkpoint:\n" << checkpoint.getDescription() << "\n";
    }
    
    // Getters
    double getCurrentTime() const { return currentTime_; }
    int getStepNumber() const { return stepNumber_; }
    double getTotalEnergy() const { return currentState_.kineticEnergy + currentState_.potentialEnergy; }
    double getTemperature() const { return currentState_.temperature; }
};

// Caretaker - manages simulation checkpoints and rollback
class SimulationCheckpointManager {
private:
    std::vector<std::unique_ptr<SimulationMemento>> checkpoints_;
    size_t currentIndex_ = 0;
    size_t maxCheckpoints_ = 10;
    
public:
    SimulationCheckpointManager(size_t maxCheckpoints = 10) : maxCheckpoints_(maxCheckpoints) {}
    
    void createCheckpoint(MolecularDynamicsSimulation& simulation, const std::string& reason) {
        // Remove any checkpoints after current index (for branching recovery)
        if (currentIndex_ < checkpoints_.size()) {
            checkpoints_.erase(checkpoints_.begin() + currentIndex_, checkpoints_.end());
        }
        
        // Add new checkpoint
        checkpoints_.push_back(simulation.saveCheckpoint(reason));
        currentIndex_ = checkpoints_.size();
        
        // Limit number of checkpoints to prevent excessive memory usage
        if (checkpoints_.size() > maxCheckpoints_) {
            checkpoints_.erase(checkpoints_.begin());
            currentIndex_--;
        }
        
        std::cout << "Checkpoint #" << checkpoints_.size() << " created\n\n";
    }
    
    void rollback(MolecularDynamicsSimulation& simulation) {
        if (currentIndex_ > 1) {
            currentIndex_--;
            simulation.restoreCheckpoint(*checkpoints_[currentIndex_ - 1]);
            std::cout << "Rolled back to checkpoint #" << currentIndex_ << "\n\n";
        } else {
            std::cout << "No earlier checkpoint available for rollback!\n";
        }
    }
    
    void rollforward(MolecularDynamicsSimulation& simulation) {
        if (currentIndex_ < checkpoints_.size()) {
            currentIndex_++;
            simulation.restoreCheckpoint(*checkpoints_[currentIndex_ - 1]);
            std::cout << "Rolled forward to checkpoint #" << currentIndex_ << "\n\n";
        } else {
            std::cout << "No later checkpoint available!\n";
        }
    }
    
    void restoreToCheckpoint(MolecularDynamicsSimulation& simulation, size_t index) {
        if (index > 0 && index <= checkpoints_.size()) {
            currentIndex_ = index;
            simulation.restoreCheckpoint(*checkpoints_[index - 1]);
            std::cout << "Restored to checkpoint #" << index << "\n\n";
        } else {
            std::cout << "Invalid checkpoint index: " << index << "\n";
        }
    }
    
    void showCheckpointHistory() const {
        std::cout << "\n=== Checkpoint History ===\n";
        for (size_t i = 0; i < checkpoints_.size(); ++i) {
            std::cout << (i == currentIndex_ - 1 ? "-> " : "   ");
            std::cout << "Checkpoint #" << (i + 1) << ":\n";
            std::cout << "   " << checkpoints_[i]->getDescription() << "\n\n";
        }
        std::cout << "Current position: " << currentIndex_ 
                  << "/" << checkpoints_.size() << "\n";
        std::cout << "Memory usage: ~" << (checkpoints_.size() * 50) << " KB\n\n";
    }
    
    void exportCheckpoint(size_t index, const std::string& filename) const {
        if (index > 0 && index <= checkpoints_.size()) {
            std::cout << "Exporting checkpoint #" << index << " to " << filename << "\n";
            // In real implementation, would serialize to file
            std::cout << "Checkpoint exported successfully\n";
        } else {
            std::cout << "Invalid checkpoint index for export: " << index << "\n";
        }
    }
};

// Climate simulation with adaptive time stepping and checkpoint recovery
class ClimateSimulation {
private:
    struct ClimateState {
        std::vector<double> temperatureField;
        std::vector<double> humidityField;
        std::vector<double> pressureField;
        std::vector<double> windVelocityU;
        std::vector<double> windVelocityV;
        double globalMeanTemp;
        double co2Concentration;
        double iceVolume;
        double seaLevel;
        
        ClimateState(size_t gridSize) 
            : temperatureField(gridSize), humidityField(gridSize),
              pressureField(gridSize), windVelocityU(gridSize), windVelocityV(gridSize),
              globalMeanTemp(288.15), co2Concentration(400.0), 
              iceVolume(1e18), seaLevel(0.0) {}
    };
    
    // Memento as inner class for climate snapshots
    class ClimateMemento {
    private:
        ClimateState state_;
        double simulationYear_;
        double timeStep_;
        int yearNumber_;
        std::string scenario_;
        double co2EmissionRate_;
        std::string timestamp_;
        
        friend class ClimateSimulation;
    
    public:
        ClimateMemento(const ClimateState& state, double year, double dt, 
                      int yearNum, const std::string& scenario, double co2Rate)
            : state_(state), simulationYear_(year), timeStep_(dt), 
              yearNumber_(yearNum), scenario_(scenario), co2EmissionRate_(co2Rate) {
            auto now = std::time(nullptr);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
            timestamp_ = ss.str();
        }
        
        std::string getDescription() const {
            std::stringstream ss;
            ss << "Climate checkpoint at " << timestamp_
               << "\n  Simulation year: " << std::fixed << simulationYear_
               << "\n  Scenario: " << scenario_
               << "\n  Global temp: " << std::setprecision(2) << state_.globalMeanTemp << " K"
               << "\n  CO₂: " << std::setprecision(1) << state_.co2Concentration << " ppm"
               << "\n  Sea level: " << state_.seaLevel << " m";
            return ss.str();
        }
        
        double getSimulationYear() const { return simulationYear_; }
        const std::string& getScenario() const { return scenario_; }
    };
    
    ClimateState currentState_;
    double simulationYear_;
    double timeStep_;
    int yearNumber_;
    std::string scenario_;
    double co2EmissionRate_;
    size_t gridSize_;
    std::mt19937 rng_;
    
public:
    ClimateSimulation(size_t gridSize, const std::string& scenario = "RCP4.5")
        : currentState_(gridSize), simulationYear_(2020.0), timeStep_(0.1),
          yearNumber_(0), scenario_(scenario), co2EmissionRate_(2.0),
          gridSize_(gridSize), rng_(123) {
        initializeClimate();
    }
    
    void initializeClimate() {
        std::normal_distribution<double> tempDist(288.15, 20.0);
        std::uniform_real_distribution<double> humidityDist(0.3, 0.9);
        std::normal_distribution<double> pressureDist(101325.0, 5000.0);
        
        for (size_t i = 0; i < gridSize_; ++i) {
            currentState_.temperatureField[i] = tempDist(rng_);
            currentState_.humidityField[i] = humidityDist(rng_);
            currentState_.pressureField[i] = pressureDist(rng_);
            currentState_.windVelocityU[i] = 0.0;
            currentState_.windVelocityV[i] = 0.0;
        }
        
        updateGlobalParameters();
        std::cout << "Initialized climate simulation with " << gridSize_ 
                  << " grid points (scenario: " << scenario_ << ")\n";
    }
    
    void updateGlobalParameters() {
        // Calculate global mean temperature
        double sum = 0.0;
        for (const auto& temp : currentState_.temperatureField) {
            sum += temp;
        }
        currentState_.globalMeanTemp = sum / gridSize_;
        
        // Simple climate feedback models
        double tempAnomaly = currentState_.globalMeanTemp - 288.15;
        currentState_.seaLevel = tempAnomaly * 0.2; // 20cm per degree
        currentState_.iceVolume = 1e18 * (1.0 - 0.1 * std::max(0.0, tempAnomaly));
    }
    
    void simulateYears(int numYears) {
        std::cout << "Simulating " << numYears << " years of climate evolution...\n";
        
        for (int year = 0; year < numYears; ++year) {
            // Update CO2 concentration
            currentState_.co2Concentration += co2EmissionRate_;
            
            // Simple radiative forcing
            double forcing = 5.35 * std::log(currentState_.co2Concentration / 280.0);
            
            // Update temperature field with greenhouse effect
            std::normal_distribution<double> tempNoise(0.0, 0.5);
            for (size_t i = 0; i < gridSize_; ++i) {
                currentState_.temperatureField[i] += 0.1 * forcing + tempNoise(rng_);
                
                // Simple humidity feedback
                if (currentState_.temperatureField[i] > 290.0) {
                    currentState_.humidityField[i] = std::min(1.0, currentState_.humidityField[i] + 0.01);
                }
            }
            
            simulationYear_ += 1.0;
            yearNumber_++;
            updateGlobalParameters();
            
            if ((year + 1) % (numYears / 5) == 0) {
                std::cout << "  Year " << static_cast<int>(simulationYear_) 
                          << ": T=" << std::fixed << std::setprecision(2) 
                          << currentState_.globalMeanTemp << " K, CO₂=" 
                          << std::setprecision(1) << currentState_.co2Concentration << " ppm\n";
            }
        }
    }
    
    void showClimateStatus() const {
        std::cout << "\n=== Climate Simulation Status ===\n";
        std::cout << "Simulation year: " << std::fixed << simulationYear_ << "\n";
        std::cout << "Scenario: " << scenario_ << "\n";
        std::cout << "Global mean temperature: " << std::setprecision(2) 
                  << currentState_.globalMeanTemp << " K (" 
                  << (currentState_.globalMeanTemp - 288.15) << " K anomaly)\n";
        std::cout << "CO₂ concentration: " << std::setprecision(1) 
                  << currentState_.co2Concentration << " ppm\n";
        std::cout << "Sea level rise: " << std::setprecision(2) 
                  << currentState_.seaLevel << " m\n";
        std::cout << "Ice volume: " << std::scientific 
                  << currentState_.iceVolume << " m³\n";
        std::cout << "Grid resolution: " << gridSize_ << " points\n";
        std::cout << "================================\n\n";
    }
    
    // Memento methods
    using Memento = ClimateMemento;
    
    std::unique_ptr<Memento> saveClimateCheckpoint(const std::string& reason) {
        std::cout << "Saving climate checkpoint: " << reason << "\n";
        return std::make_unique<Memento>(currentState_, simulationYear_, timeStep_, 
                                        yearNumber_, scenario_, co2EmissionRate_);
    }
    
    void restoreClimateCheckpoint(const Memento& checkpoint) {
        currentState_ = checkpoint.state_;
        simulationYear_ = checkpoint.simulationYear_;
        timeStep_ = checkpoint.timeStep_;
        yearNumber_ = checkpoint.yearNumber_;
        scenario_ = checkpoint.scenario_;
        co2EmissionRate_ = checkpoint.co2EmissionRate_;
        std::cout << "Restored climate from checkpoint:\n" << checkpoint.getDescription() << "\n\n";
    }
    
    void setScenario(const std::string& newScenario, double newEmissionRate) {
        scenario_ = newScenario;
        co2EmissionRate_ = newEmissionRate;
        std::cout << "Updated scenario to " << scenario_ 
                  << " (emission rate: " << co2EmissionRate_ << " ppm/year)\n";
    }
    
    // Getters
    double getSimulationYear() const { return simulationYear_; }
    double getGlobalTemperature() const { return currentState_.globalMeanTemp; }
    double getCO2Concentration() const { return currentState_.co2Concentration; }
};

// Climate scenario manager with checkpoint branching
class ClimateScenarioManager {
private:
    std::stack<std::unique_ptr<ClimateSimulation::Memento>> recentCheckpoints_;
    std::vector<std::unique_ptr<ClimateSimulation::Memento>> scenarioCheckpoints_;
    std::vector<std::string> scenarioNames_;
    
public:
    void createScenarioCheckpoint(ClimateSimulation& climate, const std::string& scenarioName) {
        scenarioCheckpoints_.push_back(climate.saveClimateCheckpoint("Scenario: " + scenarioName));
        scenarioNames_.push_back(scenarioName);
        std::cout << "Created scenario checkpoint: " << scenarioName << "\n";
    }
    
    void loadScenario(ClimateSimulation& climate, size_t index) {
        if (index < scenarioCheckpoints_.size()) {
            climate.restoreClimateCheckpoint(*scenarioCheckpoints_[index]);
            std::cout << "Loaded scenario: " << scenarioNames_[index] << "\n";
        } else {
            std::cout << "Invalid scenario index: " << index << "\n";
        }
    }
    
    void autoCheckpoint(ClimateSimulation& climate, const std::string& reason) {
        recentCheckpoints_.push(climate.saveClimateCheckpoint("Auto: " + reason));
        
        // Keep only last 5 auto-checkpoints to manage memory
        std::stack<std::unique_ptr<ClimateSimulation::Memento>> temp;
        int count = 0;
        while (!recentCheckpoints_.empty() && count < 5) {
            temp.push(std::move(const_cast<std::unique_ptr<ClimateSimulation::Memento>&>(recentCheckpoints_.top())));
            recentCheckpoints_.pop();
            count++;
        }
        
        // Clear remaining old checkpoints
        while (!recentCheckpoints_.empty()) {
            recentCheckpoints_.pop();
        }
        
        // Restore the kept checkpoints
        while (!temp.empty()) {
            recentCheckpoints_.push(std::move(temp.top()));
            temp.pop();
        }
    }
    
    void rollbackToRecent(ClimateSimulation& climate) {
        if (!recentCheckpoints_.empty()) {
            climate.restoreClimateCheckpoint(*recentCheckpoints_.top());
            recentCheckpoints_.pop();
        } else {
            std::cout << "No recent checkpoints available!\n";
        }
    }
    
    void compareScenarios() const {
        std::cout << "\n=== Climate Scenario Comparison ===\n";
        for (size_t i = 0; i < scenarioCheckpoints_.size(); ++i) {
            std::cout << "[" << i << "] " << scenarioNames_[i] << ":\n";
            std::cout << "   " << scenarioCheckpoints_[i]->getDescription() << "\n\n";
        }
        std::cout << "Recent auto-checkpoints: " << recentCheckpoints_.size() << "\n\n";
    }
    
    void exportScenario(size_t index, const std::string& filename) const {
        if (index < scenarioCheckpoints_.size()) {
            std::cout << "Exporting scenario '" << scenarioNames_[index] 
                      << "' to " << filename << "\n";
            // In real implementation, would serialize checkpoint to file
            std::cout << "Scenario exported for publication/sharing\n";
        } else {
            std::cout << "Invalid scenario index for export: " << index << "\n";
        }
    }
};

int main() {
    std::cout << "=== Molecular Dynamics Simulation with Checkpoint Recovery ===\n\n";
    
    MolecularDynamicsSimulation mdSim(100, 10.0, 300.0);
    SimulationCheckpointManager checkpointMgr(8);
    
    // Initial checkpoint
    checkpointMgr.createCheckpoint(mdSim, "Initial configuration");
    mdSim.displayStatus();
    
    // Run equilibration
    std::cout << "=== Equilibration Phase ===\n";
    mdSim.runSteps(1000);
    mdSim.displayStatus();
    checkpointMgr.createCheckpoint(mdSim, "After equilibration");
    
    // Production run
    std::cout << "=== Production Run ===\n";
    mdSim.runSteps(2000);
    mdSim.displayStatus();
    checkpointMgr.createCheckpoint(mdSim, "Mid-production");
    
    // Continue simulation
    mdSim.runSteps(1500);
    mdSim.displayStatus();
    checkpointMgr.createCheckpoint(mdSim, "Near completion");
    
    // Show checkpoint history
    checkpointMgr.showCheckpointHistory();
    
    // Simulate a problem requiring rollback
    std::cout << "=== Simulating Numerical Instability (rollback needed) ===\n";
    std::cout << "Detected energy drift! Rolling back to previous stable state...\n";
    checkpointMgr.rollback(mdSim);
    mdSim.displayStatus();
    
    // Roll back further
    std::cout << "\n=== Rolling Back Further ===\n";
    checkpointMgr.rollback(mdSim);
    mdSim.displayStatus();
    
    // Roll forward
    std::cout << "\n=== Rolling Forward ===\n";
    checkpointMgr.rollforward(mdSim);
    mdSim.displayStatus();
    
    // Continue with different parameters
    std::cout << "\n=== Continuing with Corrected Parameters ===\n";
    mdSim.runSteps(1000);
    checkpointMgr.createCheckpoint(mdSim, "Stable continuation");
    
    // Export checkpoint for sharing
    std::cout << "\n=== Exporting Checkpoint for Collaboration ===\n";
    checkpointMgr.exportCheckpoint(3, "stable_md_config.chk");
    
    checkpointMgr.showCheckpointHistory();
    
    // Climate Simulation Example
    std::cout << "\n\n=== Climate Simulation with Scenario Branching ===\n";
    
    ClimateSimulation climate(1000, "RCP4.5");
    ClimateScenarioManager scenarioMgr;
    
    climate.showClimateStatus();
    
    // Save baseline scenario
    scenarioMgr.createScenarioCheckpoint(climate, "RCP4.5 Baseline");
    
    // Run moderate warming scenario
    std::cout << "=== Running RCP4.5 Scenario ===\n";
    climate.simulateYears(20);
    climate.showClimateStatus();
    scenarioMgr.autoCheckpoint(climate, "20-year mark");
    
    climate.simulateYears(30);
    climate.showClimateStatus();
    scenarioMgr.createScenarioCheckpoint(climate, "RCP4.5 Mid-century");
    
    // Branch to high emissions scenario
    std::cout << "\n=== Branching to RCP8.5 High Emissions ===\n";
    scenarioMgr.loadScenario(climate, 0);  // Load baseline
    climate.setScenario("RCP8.5", 4.0);   // Higher emission rate
    
    climate.simulateYears(50);
    climate.showClimateStatus();
    scenarioMgr.createScenarioCheckpoint(climate, "RCP8.5 Mid-century");
    
    // Branch to mitigation scenario
    std::cout << "\n=== Branching to RCP2.6 Mitigation ===\n";
    scenarioMgr.loadScenario(climate, 0);  // Load baseline again
    climate.setScenario("RCP2.6", 0.5);   // Aggressive mitigation
    
    climate.simulateYears(50);
    climate.showClimateStatus();
    scenarioMgr.createScenarioCheckpoint(climate, "RCP2.6 Mitigation");
    
    // Compare all scenarios
    scenarioMgr.compareScenarios();
    
    // Test rollback to recent checkpoint
    std::cout << "\n=== Testing Auto-Checkpoint Rollback ===\n";
    scenarioMgr.rollbackToRecent(climate);
    climate.showClimateStatus();
    
    // Export scenario for IPCC report
    std::cout << "\n=== Exporting Scenarios for Publication ===\n";
    scenarioMgr.exportScenario(1, "rcp45_midcentury.dat");
    scenarioMgr.exportScenario(2, "rcp85_midcentury.dat");
    scenarioMgr.exportScenario(3, "rcp26_mitigation.dat");
    
    std::cout << "\nMemento pattern enables robust checkpoint/restart capability\n";
    std::cout << "for long-running scientific simulations with scenario exploration!\n";
    
    return 0;
}