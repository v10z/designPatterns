// Prototype Pattern - Quantum System State Cloning
// Clones quantum states and configurations for parameter sweeps and ensemble simulations
#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <complex>

// Abstract prototype for quantum systems
class QuantumSystem {
public:
    virtual ~QuantumSystem() = default;
    virtual std::unique_ptr<QuantumSystem> clone() const = 0;
    virtual void simulate() const = 0;
    virtual void setMagneticField(double B) {
        magneticField_ = B;
    }
    virtual void setTemperature(double T) {
        temperature_ = T;
    }
    virtual double getEnergy() const = 0;
    
protected:
    double magneticField_ = 0.0;  // Tesla
    double temperature_ = 300.0;  // Kelvin
    std::vector<std::complex<double>> wavefunction_;
};

// Concrete prototype - Spin chain system
class SpinChain : public QuantumSystem {
private:
    int numSpins_;
    double couplingStrength_;  // J in Heisenberg model
    std::string boundaryCondition_;
    
public:
    SpinChain(int spins, double J, const std::string& bc) 
        : numSpins_(spins), couplingStrength_(J), boundaryCondition_(bc) {
        // Initialize quantum state
        wavefunction_.resize(1 << numSpins_, std::complex<double>(0.0, 0.0));
        wavefunction_[0] = std::complex<double>(1.0, 0.0);  // Ground state
    }
    
    std::unique_ptr<QuantumSystem> clone() const override {
        return std::make_unique<SpinChain>(*this);
    }
    
    void simulate() const override {
        std::cout << "Simulating Heisenberg Spin Chain:\n";
        std::cout << "  Spins: " << numSpins_ << "\n";
        std::cout << "  Coupling J: " << couplingStrength_ << " meV\n";
        std::cout << "  Boundary: " << boundaryCondition_ << "\n";
        std::cout << "  Magnetic Field: " << magneticField_ << " T\n";
        std::cout << "  Temperature: " << temperature_ << " K\n";
        std::cout << "  Ground State Energy: " << getEnergy() << " meV\n\n";
    }
    
    double getEnergy() const override {
        // Simplified energy calculation
        return -couplingStrength_ * (numSpins_ - 1) - magneticField_ * numSpins_ * 0.5;
    }
};

// Concrete prototype - Quantum dot system
class QuantumDot : public QuantumSystem {
private:
    double confinementEnergy_;
    int numElectrons_;
    std::string material_;
    
public:
    QuantumDot(double E0, int electrons, const std::string& mat)
        : confinementEnergy_(E0), numElectrons_(electrons), material_(mat) {
        // Initialize many-body wavefunction
        int hilbertSize = 1 << (2 * numElectrons_);  // Spin up/down states
        wavefunction_.resize(hilbertSize, std::complex<double>(0.0, 0.0));
        wavefunction_[0] = std::complex<double>(1.0, 0.0);
    }
    
    std::unique_ptr<QuantumSystem> clone() const override {
        return std::make_unique<QuantumDot>(*this);
    }
    
    void simulate() const override {
        std::cout << "Simulating Quantum Dot System:\n";
        std::cout << "  Material: " << material_ << "\n";
        std::cout << "  Electrons: " << numElectrons_ << "\n";
        std::cout << "  Confinement E0: " << confinementEnergy_ << " meV\n";
        std::cout << "  Magnetic Field: " << magneticField_ << " T\n";
        std::cout << "  Temperature: " << temperature_ << " K\n";
        std::cout << "  Total Energy: " << getEnergy() << " meV\n\n";
    }
    
    double getEnergy() const override {
        // Simplified Fock-Darwin spectrum
        double cyclotronFreq = 1.16 * magneticField_;  // meV/T for GaAs
        return numElectrons_ * (confinementEnergy_ + 0.5 * cyclotronFreq);
    }
};

// Quantum system prototype registry for parameter sweeps
class QuantumSystemRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<QuantumSystem>> prototypes_;
    
public:
    void registerPrototype(const std::string& key, 
                          std::unique_ptr<QuantumSystem> prototype) {
        prototypes_[key] = std::move(prototype);
    }
    
    std::unique_ptr<QuantumSystem> create(const std::string& key) {
        auto it = prototypes_.find(key);
        if (it != prototypes_.end()) {
            return it->second->clone();
        }
        return nullptr;
    }
    
    // Create ensemble for Monte Carlo simulations
    std::vector<std::unique_ptr<QuantumSystem>> createEnsemble(
            const std::string& key, int count) {
        std::vector<std::unique_ptr<QuantumSystem>> ensemble;
        for (int i = 0; i < count; ++i) {
            auto system = create(key);
            if (system) {
                ensemble.push_back(std::move(system));
            }
        }
        return ensemble;
    }
};

int main() {
    std::cout << "=== Quantum System Prototype Registry Demo ===\n\n";
    
    // Create registry and register quantum system prototypes
    QuantumSystemRegistry registry;
    
    // Register different spin chain configurations
    registry.registerPrototype("antiferromagnetic-chain",
        std::make_unique<SpinChain>(10, -1.0, "periodic"));
    registry.registerPrototype("ferromagnetic-chain",
        std::make_unique<SpinChain>(10, 1.0, "open"));
    
    // Register quantum dot systems
    registry.registerPrototype("gaas-quantum-dot",
        std::make_unique<QuantumDot>(5.0, 2, "GaAs"));
    registry.registerPrototype("inas-quantum-dot",
        std::make_unique<QuantumDot>(3.0, 4, "InAs"));
    
    // Parameter sweep: Magnetic field study
    std::cout << "=== Magnetic Field Parameter Sweep ===\n\n";
    auto spinSystem = registry.create("antiferromagnetic-chain");
    for (double B = 0.0; B <= 10.0; B += 5.0) {
        auto system = spinSystem->clone();
        system->setMagneticField(B);
        system->simulate();
    }
    
    // Temperature ensemble for statistical mechanics
    std::cout << "=== Temperature Ensemble Generation ===\n\n";
    auto ensemble = registry.createEnsemble("gaas-quantum-dot", 3);
    double temps[] = {4.2, 77.0, 300.0};  // LHe, LN2, Room temp
    for (size_t i = 0; i < ensemble.size(); ++i) {
        ensemble[i]->setTemperature(temps[i]);
        ensemble[i]->setMagneticField(2.0);
        std::cout << "Ensemble member " << i+1 << ":\n";
        ensemble[i]->simulate();
    }
    
    // Create copies for parallel computation
    std::cout << "=== Parallel Computation Setup ===\n\n";
    auto dotPrototype = registry.create("inas-quantum-dot");
    std::cout << "Creating 4 copies for parallel Monte Carlo...\n";
    for (int i = 0; i < 4; ++i) {
        auto copy = dotPrototype->clone();
        std::cout << "Thread " << i << ": Initialized quantum dot\n";
    }
    std::cout << "\nReady for distributed quantum Monte Carlo simulation\n";
    
    return 0;
}