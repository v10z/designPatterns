// Factory Method Pattern - Numerical Solver Factory for Scientific Computing
// Creates different solvers for PDEs, ODEs, and linear systems
#include <iostream>
#include <memory>
#include <vector>
#include <cmath>
#include <string>

// Abstract numerical solver interface
class NumericalSolver {
public:
    virtual ~NumericalSolver() = default;
    virtual void solve(const std::vector<double>& initialConditions) = 0;
    virtual std::string getMethod() const = 0;
    virtual double getAccuracy() const = 0;
};

// Finite Element Method (FEM) Solver
class FEMSolver : public NumericalSolver {
private:
    int meshElements = 1000;
    double tolerance = 1e-6;
    
public:
    void solve(const std::vector<double>& initialConditions) override {
        std::cout << "Solving using Finite Element Method\n";
        std::cout << "Mesh elements: " << meshElements << "\n";
        std::cout << "Initial temperature field: ";
        for (size_t i = 0; i < std::min(size_t(3), initialConditions.size()); ++i) {
            std::cout << initialConditions[i] << " K ";
        }
        std::cout << "...\n";
        std::cout << "Assembling stiffness matrix...\n";
        std::cout << "Applying boundary conditions...\n";
        std::cout << "Solving linear system with tolerance " << tolerance << "\n";
    }
    
    std::string getMethod() const override {
        return "Galerkin Finite Element Method";
    }
    
    double getAccuracy() const override {
        return tolerance;
    }
};

// Runge-Kutta ODE Solver
class RungeKuttaSolver : public NumericalSolver {
private:
    int order = 4;
    double timeStep = 0.001;
    
public:
    void solve(const std::vector<double>& initialConditions) override {
        std::cout << "Solving using Runge-Kutta Method (RK" << order << ")\n";
        std::cout << "Time step: " << timeStep << " seconds\n";
        std::cout << "Initial state vector: ";
        for (size_t i = 0; i < std::min(size_t(3), initialConditions.size()); ++i) {
            std::cout << initialConditions[i] << " ";
        }
        std::cout << "...\n";
        std::cout << "Computing k1, k2, k3, k4 coefficients...\n";
        std::cout << "Advancing solution in time...\n";
    }
    
    std::string getMethod() const override {
        return "4th Order Runge-Kutta";
    }
    
    double getAccuracy() const override {
        return std::pow(timeStep, order);
    }
};

// Spectral Method Solver for Fluid Dynamics
class SpectralSolver : public NumericalSolver {
private:
    int fourierModes = 256;
    
public:
    void solve(const std::vector<double>& initialConditions) override {
        std::cout << "Solving using Spectral Method\n";
        std::cout << "Fourier modes: " << fourierModes << "\n";
        std::cout << "Initial vorticity distribution: ";
        for (size_t i = 0; i < std::min(size_t(3), initialConditions.size()); ++i) {
            std::cout << initialConditions[i] << " ";
        }
        std::cout << "...\n";
        std::cout << "Performing FFT...\n";
        std::cout << "Computing nonlinear terms in spectral space...\n";
        std::cout << "Time-stepping with exponential integrator...\n";
    }
    
    std::string getMethod() const override {
        return "Pseudo-spectral Method with FFT";
    }
    
    double getAccuracy() const override {
        return std::exp(-fourierModes / 10.0);  // Exponential convergence
    }
};

// Abstract simulation framework
class SimulationFramework {
protected:
    std::string problemType;
    std::vector<double> problemData;
    
public:
    virtual ~SimulationFramework() = default;
    
    // Factory method for creating appropriate solver
    virtual std::unique_ptr<NumericalSolver> createSolver() = 0;
    
    // Template method for running simulation
    void runSimulation() {
        std::cout << "\n=== " << problemType << " Simulation ===\n";
        
        // Create appropriate solver
        auto solver = createSolver();
        
        std::cout << "Selected solver: " << solver->getMethod() << "\n";
        std::cout << "Expected accuracy: " << solver->getAccuracy() << "\n\n";
        
        // Prepare initial conditions
        std::cout << "Preparing initial conditions...\n";
        
        // Solve the problem
        solver->solve(problemData);
        
        std::cout << "\nPost-processing results...\n";
        std::cout << "Simulation complete.\n";
    }
    
    void setProblemData(const std::vector<double>& data) {
        problemData = data;
    }
};

// Heat Transfer Simulation (uses FEM)
class HeatTransferSimulation : public SimulationFramework {
public:
    HeatTransferSimulation() {
        problemType = "Heat Transfer";
    }
    
    std::unique_ptr<NumericalSolver> createSolver() override {
        return std::make_unique<FEMSolver>();
    }
};

// Orbital Mechanics Simulation (uses Runge-Kutta)
class OrbitalMechanicsSimulation : public SimulationFramework {
public:
    OrbitalMechanicsSimulation() {
        problemType = "Orbital Mechanics";
    }
    
    std::unique_ptr<NumericalSolver> createSolver() override {
        return std::make_unique<RungeKuttaSolver>();
    }
};

// Turbulent Flow Simulation (uses Spectral Method)
class TurbulentFlowSimulation : public SimulationFramework {
public:
    TurbulentFlowSimulation() {
        problemType = "Turbulent Flow";
    }
    
    std::unique_ptr<NumericalSolver> createSolver() override {
        return std::make_unique<SpectralSolver>();
    }
};

int main() {
    std::cout << "=== Scientific Computing Solver Factory Demo ===\n";
    
    // Heat conduction in a reactor vessel
    {
        HeatTransferSimulation heatSim;
        heatSim.setProblemData({473.15, 373.15, 323.15, 298.15});  // Temperature in Kelvin
        heatSim.runSimulation();
    }
    
    // Three-body problem in celestial mechanics
    {
        OrbitalMechanicsSimulation orbitalSim;
        // Initial positions and velocities for three bodies
        orbitalSim.setProblemData({1.0, 0.0, 0.0, 0.5,  // Body 1: x, y, vx, vy
                                  -0.5, 0.866, -0.25, -0.433,  // Body 2
                                  -0.5, -0.866, -0.25, 0.433}); // Body 3
        orbitalSim.runSimulation();
    }
    
    // Navier-Stokes simulation for turbulent flow
    {
        TurbulentFlowSimulation flowSim;
        // Initial vorticity field samples
        flowSim.setProblemData({0.1, -0.2, 0.15, -0.1, 0.05});
        flowSim.runSimulation();
    }
    
    return 0;
}