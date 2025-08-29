// Builder Pattern - Molecular Dynamics Simulation Builder
// Constructs complex molecular simulations with various force fields and parameters
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Product class - Complete molecular dynamics simulation
class MolecularSimulation {
private:
    std::string forceField;
    std::string integrator;
    double timeStep;
    double temperature;
    double pressure;
    std::vector<std::string> molecules;
    std::vector<std::string> constraints;
    int particleCount;
    std::string ensemble;
    
public:
    void setForceField(const std::string& ff) { forceField = ff; }
    void setIntegrator(const std::string& integ) { integrator = integ; }
    void setTimeStep(double dt) { timeStep = dt; }
    void setTemperature(double T) { temperature = T; }
    void setPressure(double P) { pressure = P; }
    void setEnsemble(const std::string& ens) { ensemble = ens; }
    void setParticleCount(int count) { particleCount = count; }
    void addMolecule(const std::string& molecule) { 
        molecules.push_back(molecule); 
    }
    void addConstraint(const std::string& constraint) {
        constraints.push_back(constraint);
    }
    
    void displayConfiguration() {
        std::cout << "=== Molecular Dynamics Simulation Configuration ===\n";
        std::cout << "Force Field: " << forceField << "\n";
        std::cout << "Integrator: " << integrator << "\n";
        std::cout << "Time Step: " << timeStep << " fs\n";
        std::cout << "Temperature: " << temperature << " K\n";
        std::cout << "Pressure: " << pressure << " atm\n";
        std::cout << "Ensemble: " << ensemble << "\n";
        std::cout << "Total Particles: " << particleCount << "\n";
        std::cout << "Molecular Species: ";
        for (const auto& mol : molecules) {
            std::cout << mol << " ";
        }
        std::cout << "\nConstraints: ";
        for (const auto& con : constraints) {
            std::cout << con << " ";
        }
        std::cout << "\n\n";
    }
};

// Abstract builder for molecular simulations
class SimulationBuilder {
protected:
    std::unique_ptr<MolecularSimulation> simulation;
    
public:
    void createNewSimulation() {
        simulation = std::make_unique<MolecularSimulation>();
    }
    
    std::unique_ptr<MolecularSimulation> getSimulation() {
        return std::move(simulation);
    }
    
    virtual ~SimulationBuilder() = default;
    virtual void buildForceField() = 0;
    virtual void buildIntegrator() = 0;
    virtual void buildThermodynamics() = 0;
    virtual void buildMolecularSystem() = 0;
    virtual void buildConstraints() = 0;
};

// Concrete builder for protein folding simulations
class ProteinFoldingBuilder : public SimulationBuilder {
public:
    void buildForceField() override {
        simulation->setForceField("AMBER ff14SB");
    }
    
    void buildIntegrator() override {
        simulation->setIntegrator("Langevin Dynamics");
        simulation->setTimeStep(2.0);  // 2 femtoseconds
    }
    
    void buildThermodynamics() override {
        simulation->setTemperature(310.15);  // Body temperature
        simulation->setPressure(1.0);        // 1 atm
        simulation->setEnsemble("NPT");      // Constant pressure and temperature
    }
    
    void buildMolecularSystem() override {
        simulation->addMolecule("Protein (1UBQ)");
        simulation->addMolecule("TIP3P Water (10000)");
        simulation->addMolecule("Na+ ions (20)");
        simulation->addMolecule("Cl- ions (20)");
        simulation->setParticleCount(31416);
    }
    
    void buildConstraints() override {
        simulation->addConstraint("SHAKE (H-bonds)");
        simulation->addConstraint("Periodic Boundary Conditions");
        simulation->addConstraint("Long-range Electrostatics (PME)");
    }
};

// Concrete builder for fluid dynamics simulations
class FluidDynamicsBuilder : public SimulationBuilder {
public:
    void buildForceField() override {
        simulation->setForceField("Lennard-Jones 12-6");
    }
    
    void buildIntegrator() override {
        simulation->setIntegrator("Velocity Verlet");
        simulation->setTimeStep(0.005);  // 5 attoseconds for dense fluids
    }
    
    void buildThermodynamics() override {
        simulation->setTemperature(298.15);  // Room temperature
        simulation->setPressure(100.0);      // 100 atm (high pressure)
        simulation->setEnsemble("NVE");      // Microcanonical ensemble
    }
    
    void buildMolecularSystem() override {
        simulation->addMolecule("Argon atoms (100000)");
        simulation->setParticleCount(100000);
    }
    
    void buildConstraints() override {
        simulation->addConstraint("Neighbor Lists (Verlet)");
        simulation->addConstraint("Cutoff 2.5σ");
        simulation->addConstraint("Tail Corrections");
    }
};

// Director for orchestrating simulation construction
class SimulationDirector {
private:
    SimulationBuilder* builder;
    
public:
    void setBuilder(SimulationBuilder* b) {
        builder = b;
    }
    
    std::unique_ptr<MolecularSimulation> constructSimulation() {
        builder->createNewSimulation();
        builder->buildForceField();
        builder->buildIntegrator();
        builder->buildThermodynamics();
        builder->buildMolecularSystem();
        builder->buildConstraints();
        return builder->getSimulation();
    }
};

int main() {
    std::cout << "=== Molecular Dynamics Simulation Builder Demo ===\n\n";
    
    SimulationDirector director;
    
    // Build protein folding simulation
    {
        std::cout << "Constructing Protein Folding Simulation...\n\n";
        ProteinFoldingBuilder proteinBuilder;
        director.setBuilder(&proteinBuilder);
        auto proteinSim = director.constructSimulation();
        proteinSim->displayConfiguration();
        
        std::cout << "Initializing simulation...\n";
        std::cout << "Loading crystal structure from PDB...\n";
        std::cout << "Solvating protein in water box...\n";
        std::cout << "Adding counterions for neutralization...\n";
        std::cout << "Energy minimization in progress...\n\n";
    }
    
    // Build fluid dynamics simulation
    {
        std::cout << "Constructing Fluid Dynamics Simulation...\n\n";
        FluidDynamicsBuilder fluidBuilder;
        director.setBuilder(&fluidBuilder);
        auto fluidSim = director.constructSimulation();
        fluidSim->displayConfiguration();
        
        std::cout << "Initializing simulation...\n";
        std::cout << "Creating FCC lattice of Argon atoms...\n";
        std::cout << "Assigning Maxwell-Boltzmann velocities...\n";
        std::cout << "Equilibrating system for 100 ps...\n";
        std::cout << "Ready for production run...\n\n";
    }
    
    return 0;
}