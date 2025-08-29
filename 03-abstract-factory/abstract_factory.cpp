// Abstract Factory Pattern - Scientific Computing Backend Factory
// Creates families of related numerical computing components (solvers, integrators, mesh generators)
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Abstract products for scientific computing
class LinearSolver {
public:
    virtual ~LinearSolver() = default;
    virtual void solve(const std::vector<std::vector<double>>& matrix, 
                      const std::vector<double>& rhs) = 0;
    virtual std::string getMethod() const = 0;
};

class MeshGenerator {
public:
    virtual ~MeshGenerator() = default;
    virtual void generateMesh(double domainSize, int resolution) = 0;
    virtual std::string getMeshType() const = 0;
};

// Concrete products - CPU-based computation family
class DenseLinearSolver : public LinearSolver {
public:
    void solve(const std::vector<std::vector<double>>& matrix, 
               const std::vector<double>& rhs) override {
        std::cout << "Solving linear system using Dense LU decomposition (LAPACK)\n";
        std::cout << "Matrix size: " << matrix.size() << "x" << matrix.size() << "\n";
        std::cout << "Using optimized BLAS Level 3 operations\n";
        std::cout << "Factorizing matrix...\n";
        std::cout << "Forward/backward substitution...\n";
    }
    
    std::string getMethod() const override {
        return "Dense LU (LAPACK)";
    }
};

class StructuredMeshGenerator : public MeshGenerator {
public:
    void generateMesh(double domainSize, int resolution) override {
        std::cout << "Generating structured Cartesian mesh\n";
        std::cout << "Domain: [0, " << domainSize << "]^3\n";
        std::cout << "Grid points: " << resolution << "^3\n";
        std::cout << "Creating uniform spacing...\n";
        std::cout << "Setting up connectivity matrix...\n";
    }
    
    std::string getMeshType() const override {
        return "Structured Cartesian";
    }
};

// Concrete products - GPU-accelerated computation family
class SparseLinearSolver : public LinearSolver {
public:
    void solve(const std::vector<std::vector<double>>& matrix, 
               const std::vector<double>& rhs) override {
        std::cout << "Solving sparse linear system using GPU-accelerated CG (cuSPARSE)\n";
        std::cout << "Matrix size: " << matrix.size() << "x" << matrix.size() << "\n";
        std::cout << "Transferring data to GPU...\n";
        std::cout << "Preconditioning with incomplete Cholesky...\n";
        std::cout << "Iterating until convergence (tol=1e-10)...\n";
    }
    
    std::string getMethod() const override {
        return "GPU Sparse CG (cuSPARSE)";
    }
};

class UnstructuredMeshGenerator : public MeshGenerator {
public:
    void generateMesh(double domainSize, int resolution) override {
        std::cout << "Generating unstructured tetrahedral mesh\n";
        std::cout << "Domain: Complex geometry, max dimension " << domainSize << "\n";
        std::cout << "Target elements: ~" << resolution*resolution*resolution << "\n";
        std::cout << "Delaunay triangulation in progress...\n";
        std::cout << "Optimizing mesh quality (aspect ratio)...\n";
    }
    
    std::string getMeshType() const override {
        return "Unstructured Tetrahedral";
    }
};

// Abstract factory for scientific computing backends
class ComputationalBackendFactory {
public:
    virtual ~ComputationalBackendFactory() = default;
    virtual std::unique_ptr<LinearSolver> createLinearSolver() = 0;
    virtual std::unique_ptr<MeshGenerator> createMeshGenerator() = 0;
    virtual std::string getBackendName() const = 0;
};

// Concrete factories for different HPC backends
class CPUBackendFactory : public ComputationalBackendFactory {
public:
    std::unique_ptr<LinearSolver> createLinearSolver() override {
        return std::make_unique<DenseLinearSolver>();
    }
    
    std::unique_ptr<MeshGenerator> createMeshGenerator() override {
        return std::make_unique<StructuredMeshGenerator>();
    }
    
    std::string getBackendName() const override {
        return "CPU-Optimized Backend (Intel MKL)";
    }
};

class GPUBackendFactory : public ComputationalBackendFactory {
public:
    std::unique_ptr<LinearSolver> createLinearSolver() override {
        return std::make_unique<SparseLinearSolver>();
    }
    
    std::unique_ptr<MeshGenerator> createMeshGenerator() override {
        return std::make_unique<UnstructuredMeshGenerator>();
    }
    
    std::string getBackendName() const override {
        return "GPU-Accelerated Backend (CUDA)";
    }
};

// FEM Simulation Framework using abstract factory
class FEMSimulation {
private:
    std::unique_ptr<LinearSolver> solver;
    std::unique_ptr<MeshGenerator> meshGen;
    std::string simulationType;
    
public:
    FEMSimulation(ComputationalBackendFactory& factory, const std::string& type) 
        : simulationType(type) {
        solver = factory.createLinearSolver();
        meshGen = factory.createMeshGenerator();
        std::cout << "\nInitializing " << simulationType << " simulation\n";
        std::cout << "Backend: " << factory.getBackendName() << "\n\n";
    }
    
    void runSimulation() {
        std::cout << "=== Running " << simulationType << " ===\n";
        
        // Generate computational mesh
        std::cout << "Step 1: Mesh Generation\n";
        meshGen->generateMesh(1.0, 100);  // 1m domain, 100x100x100 resolution
        std::cout << "Mesh type: " << meshGen->getMeshType() << "\n\n";
        
        // Solve system of equations
        std::cout << "Step 2: Solving Linear System\n";
        // Dummy matrix representing discretized PDE
        std::vector<std::vector<double>> stiffnessMatrix(1000, std::vector<double>(1000, 0.0));
        std::vector<double> forceVector(1000, 1.0);
        solver->solve(stiffnessMatrix, forceVector);
        std::cout << "Solver: " << solver->getMethod() << "\n\n";
        
        std::cout << "Simulation completed successfully!\n";
    }
};

int main() {
    std::cout << "=== Scientific Computing Backend Factory Demo ===\n";
    
    // Heat conduction simulation on CPU
    {
        std::cout << "\n--- Heat Conduction Analysis ---\n";
        CPUBackendFactory cpuFactory;
        FEMSimulation heatSim(cpuFactory, "Heat Conduction in Reactor Vessel");
        heatSim.runSimulation();
    }
    
    std::cout << "\n" << std::string(50, '-') << "\n";
    
    // Fluid dynamics simulation on GPU
    {
        std::cout << "\n--- Fluid Dynamics Analysis ---\n";
        GPUBackendFactory gpuFactory;
        FEMSimulation fluidSim(gpuFactory, "Turbulent Flow in Combustion Chamber");
        fluidSim.runSimulation();
    }
    
    return 0;
}