// Adapter Pattern - Numerical Library Interface Adapter
// Adapts different scientific computing libraries (BLAS, LAPACK, cuBLAS) to a unified interface
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Target interface - Unified linear algebra operations
class LinearAlgebraSolver {
public:
    virtual ~LinearAlgebraSolver() = default;
    virtual void solveLinearSystem(const std::vector<std::vector<double>>& A,
                                   const std::vector<double>& b,
                                   std::vector<double>& x) = 0;
    virtual void computeEigenvalues(const std::vector<std::vector<double>>& A,
                                   std::vector<double>& eigenvalues) = 0;
};

// Existing libraries with different interfaces

// LAPACK-style interface (Fortran-based)
class LAPACKInterface {
public:
    virtual ~LAPACKInterface() = default;
    // LAPACK uses column-major order and specific naming conventions
    virtual void dgesv(int n, int nrhs, double* a, int lda, int* ipiv,
                      double* b, int ldb, int* info) = 0;
    virtual void dsyev(char jobz, char uplo, int n, double* a, int lda,
                      double* w, double* work, int lwork, int* info) = 0;
};

// CUDA cuBLAS interface (GPU-based)
class CuBLASInterface {
public:
    virtual ~CuBLASInterface() = default;
    // cuBLAS uses device pointers and handles
    virtual void cublasDgesvBatched(void* handle, int n, double** aArray,
                                   int lda, int* pivotArray, double** bArray,
                                   int ldb, int* info, int batchSize) = 0;
    virtual void cublasDsyevd(void* handle, int jobz, int uplo, int n,
                             double* a, int lda, double* w) = 0;
};

// Concrete LAPACK implementation
class IntelMKLLAPACK : public LAPACKInterface {
public:
    void dgesv(int n, int nrhs, double* a, int lda, int* ipiv,
               double* b, int ldb, int* info) override {
        std::cout << "Intel MKL: Solving linear system Ax=b\n";
        std::cout << "  Matrix size: " << n << "x" << n << "\n";
        std::cout << "  Using optimized LU decomposition (dgesv)\n";
        std::cout << "  Utilizing AVX-512 instructions\n";
        *info = 0; // Success
    }
    
    void dsyev(char jobz, char uplo, int n, double* a, int lda,
               double* w, double* work, int lwork, int* info) override {
        std::cout << "Intel MKL: Computing eigenvalues\n";
        std::cout << "  Symmetric matrix size: " << n << "x" << n << "\n";
        std::cout << "  Using divide-and-conquer algorithm (dsyev)\n";
        *info = 0; // Success
    }
};

// Concrete cuBLAS implementation
class NVIDIACuBLAS : public CuBLASInterface {
public:
    void cublasDgesvBatched(void* handle, int n, double** aArray,
                           int lda, int* pivotArray, double** bArray,
                           int ldb, int* info, int batchSize) override {
        std::cout << "NVIDIA cuBLAS: Batch solving linear systems on GPU\n";
        std::cout << "  Batch size: " << batchSize << "\n";
        std::cout << "  Matrix size per system: " << n << "x" << n << "\n";
        std::cout << "  Using parallel LU decomposition on CUDA cores\n";
        std::cout << "  Memory transfer: Host -> Device\n";
        for (int i = 0; i < batchSize; ++i) info[i] = 0;
    }
    
    void cublasDsyevd(void* handle, int jobz, int uplo, int n,
                      double* a, int lda, double* w) override {
        std::cout << "NVIDIA cuBLAS: GPU eigenvalue computation\n";
        std::cout << "  Matrix size: " << n << "x" << n << "\n";
        std::cout << "  Using Jacobi algorithm on GPU\n";
        std::cout << "  Tensor cores acceleration enabled\n";
    }
};

// Adapter for LAPACK libraries
class LAPACKAdapter : public LinearAlgebraSolver {
private:
    std::unique_ptr<LAPACKInterface> lapack;
    
    // Convert from row-major to column-major
    std::vector<double> convertToColumnMajor(const std::vector<std::vector<double>>& A) {
        int n = A.size();
        std::vector<double> colMajor(n * n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                colMajor[j * n + i] = A[i][j];
            }
        }
        return colMajor;
    }
    
public:
    LAPACKAdapter() : lapack(std::make_unique<IntelMKLLAPACK>()) {}
    
    void solveLinearSystem(const std::vector<std::vector<double>>& A,
                          const std::vector<double>& b,
                          std::vector<double>& x) override {
        int n = A.size();
        auto aColMajor = convertToColumnMajor(A);
        x = b; // LAPACK overwrites b with solution
        std::vector<int> ipiv(n);
        int info;
        
        std::cout << "\nAdapting unified interface to LAPACK format...\n";
        lapack->dgesv(n, 1, aColMajor.data(), n, ipiv.data(),
                      x.data(), n, &info);
        std::cout << "Solution computed successfully\n\n";
    }
    
    void computeEigenvalues(const std::vector<std::vector<double>>& A,
                           std::vector<double>& eigenvalues) override {
        int n = A.size();
        auto aColMajor = convertToColumnMajor(A);
        eigenvalues.resize(n);
        int lwork = 3 * n;
        std::vector<double> work(lwork);
        int info;
        
        std::cout << "\nAdapting unified interface to LAPACK format...\n";
        lapack->dsyev('N', 'U', n, aColMajor.data(), n,
                      eigenvalues.data(), work.data(), lwork, &info);
        std::cout << "Eigenvalues computed successfully\n\n";
    }
};

// Adapter for GPU libraries
class CuBLASAdapter : public LinearAlgebraSolver {
private:
    std::unique_ptr<CuBLASInterface> cublas;
    void* handle; // CUDA handle
    
public:
    CuBLASAdapter() : cublas(std::make_unique<NVIDIACuBLAS>()) {
        // In real implementation, would initialize CUDA handle
        handle = nullptr;
    }
    
    void solveLinearSystem(const std::vector<std::vector<double>>& A,
                          const std::vector<double>& b,
                          std::vector<double>& x) override {
        std::cout << "\nAdapting unified interface to cuBLAS GPU format...\n";
        std::cout << "Allocating GPU memory...\n";
        std::cout << "Transferring data to GPU...\n";
        
        // Simplified - in reality would handle device memory
        int n = A.size();
        x = b;
        
        // Simulate batch processing on GPU
        double* aArray[] = {nullptr}; // Device pointers
        double* bArray[] = {nullptr};
        std::vector<int> info(1);
        std::vector<int> ipiv(n);
        
        cublas->cublasDgesvBatched(handle, n, aArray, n, ipiv.data(),
                                   bArray, n, info.data(), 1);
        
        std::cout << "Transferring results back to CPU...\n";
        std::cout << "GPU computation completed\n\n";
    }
    
    void computeEigenvalues(const std::vector<std::vector<double>>& A,
                           std::vector<double>& eigenvalues) override {
        std::cout << "\nAdapting unified interface to cuBLAS GPU format...\n";
        int n = A.size();
        eigenvalues.resize(n);
        
        cublas->cublasDsyevd(handle, 1, 1, n, nullptr, n, eigenvalues.data());
        std::cout << "GPU eigenvalue computation completed\n\n";
    }
};

// Scientific computation client using unified interface
class ScientificComputation {
private:
    std::unique_ptr<LinearAlgebraSolver> solver;
    
public:
    void setBackend(const std::string& backend) {
        if (backend == "CPU") {
            solver = std::make_unique<LAPACKAdapter>();
            std::cout << "Selected CPU backend (Intel MKL)\n";
        } else if (backend == "GPU") {
            solver = std::make_unique<CuBLASAdapter>();
            std::cout << "Selected GPU backend (NVIDIA cuBLAS)\n";
        }
    }
    
    void runFluidDynamicsSimulation() {
        std::cout << "\n=== Fluid Dynamics Pressure Solver ===\n";
        
        // Discretized pressure Poisson equation
        std::vector<std::vector<double>> A = {
            { 4, -1,  0, -1},
            {-1,  4, -1,  0},
            { 0, -1,  4, -1},
            {-1,  0, -1,  4}
        };
        std::vector<double> b = {1.0, 2.0, 3.0, 4.0}; // Boundary conditions
        std::vector<double> x;
        
        solver->solveLinearSystem(A, b, x);
    }
    
    void runQuantumSimulation() {
        std::cout << "\n=== Quantum System Energy Levels ===\n";
        
        // Hamiltonian matrix
        std::vector<std::vector<double>> H = {
            {2.0, -1.0,  0.0},
            {-1.0, 2.0, -1.0},
            {0.0, -1.0,  2.0}
        };
        std::vector<double> energyLevels;
        
        solver->computeEigenvalues(H, energyLevels);
    }
};

int main() {
    std::cout << "=== Scientific Computing Library Adapter Demo ===\n\n";
    
    ScientificComputation computation;
    
    // Run simulations on CPU
    std::cout << "--- CPU-based Computations ---\n";
    computation.setBackend("CPU");
    computation.runFluidDynamicsSimulation();
    computation.runQuantumSimulation();
    
    // Run simulations on GPU
    std::cout << "\n--- GPU-accelerated Computations ---\n";
    computation.setBackend("GPU");
    computation.runFluidDynamicsSimulation();
    computation.runQuantumSimulation();
    
    std::cout << "\nAdapter pattern allows switching between CPU and GPU\n";
    std::cout << "backends without changing the scientific code!\n";
    
    return 0;
}