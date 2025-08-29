// Singleton Pattern - Computational Physics Simulation Manager
// Used for managing a global simulation environment in HPC applications
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>
#include <cmath>

class SimulationEnvironment {
private:
    static std::unique_ptr<SimulationEnvironment> instance;
    static std::mutex mutex;
    
    // Simulation parameters
    double timeStep = 1e-6;  // Time step in seconds
    double totalEnergy = 0.0;
    int particleCount = 0;
    std::vector<double> fieldStrengths;
    
    // Private constructor prevents direct instantiation
    SimulationEnvironment() {
        std::cout << "Initializing global simulation environment...\n";
        std::cout << "Setting up MPI communication channels...\n";
        std::cout << "Allocating GPU resources...\n";
        fieldStrengths.reserve(1000);
    }
    
public:
    // Delete copy/move operations
    SimulationEnvironment(const SimulationEnvironment&) = delete;
    SimulationEnvironment& operator=(const SimulationEnvironment&) = delete;
    
    // Thread-safe instance getter for HPC environments
    static SimulationEnvironment* getInstance() {
        std::lock_guard<std::mutex> lock(mutex);
        if (!instance) {
            instance.reset(new SimulationEnvironment());
        }
        return instance.get();
    }
    
    // Simulation control methods
    void setTimeStep(double dt) {
        timeStep = dt;
        std::cout << "Time step updated to: " << dt << " seconds\n";
    }
    
    void updateFieldStrength(double strength) {
        fieldStrengths.push_back(strength);
        totalEnergy += strength * strength;
        std::cout << "Field strength recorded: " << strength << " Tesla\n";
    }
    
    void runDiagnostics() {
        std::cout << "\n=== Simulation Diagnostics ===\n";
        std::cout << "Total energy: " << totalEnergy << " Joules\n";
        std::cout << "Particle count: " << particleCount << "\n";
        std::cout << "Field measurements: " << fieldStrengths.size() << "\n";
        std::cout << "Time step: " << timeStep << " seconds\n";
    }
    
    void incrementParticles(int count) {
        particleCount += count;
    }
};

// Static member initialization
std::unique_ptr<SimulationEnvironment> SimulationEnvironment::instance = nullptr;
std::mutex SimulationEnvironment::mutex;

int main() {
    std::cout << "=== Particle Physics Simulation Control ===\n\n";
    
    // First researcher initializes the simulation environment
    SimulationEnvironment* env1 = SimulationEnvironment::getInstance();
    env1->setTimeStep(1e-9);  // 1 nanosecond for particle collisions
    env1->incrementParticles(1000000);
    
    // Second researcher accesses the same environment
    SimulationEnvironment* env2 = SimulationEnvironment::getInstance();
    env2->updateFieldStrength(2.5);  // Tesla
    env2->updateFieldStrength(3.7);
    
    std::cout << "\nSame simulation environment? " << (env1 == env2 ? "Yes" : "No") << "\n";
    
    // Run diagnostics
    env1->runDiagnostics();
    
    return 0;
}