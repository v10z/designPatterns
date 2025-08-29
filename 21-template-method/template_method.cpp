// Template Method Pattern - Scientific Computational Workflows
// Defines the skeleton of scientific algorithms while allowing subclasses to customize specific steps
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>
#include <random>
#include <complex>
#include <functional>

// Template Method for Scientific Simulation Workflows
class SimulationWorkflow {
public:
    // Template method - defines the scientific simulation algorithm
    void runSimulation(const std::string& configFile) {
        loadConfiguration(configFile);
        initializeSystem();
        setupInitialConditions();
        
        if (validatePhysics()) {  // Hook method
            performTimeIntegration();
            
            if (shouldPerformAnalysis()) {  // Hook method
                analyzeResults();
            }
            
            saveResults();
        }
        
        finalizeSimulation();
    }
    
    virtual ~SimulationWorkflow() = default;
    
protected:
    // Abstract methods that must be implemented by specific simulations
    virtual void initializeSystem() = 0;
    virtual void setupInitialConditions() = 0;
    virtual void performTimeIntegration() = 0;
    virtual void analyzeResults() = 0;
    
    // Hook methods - subclasses can override for specialized behavior
    virtual bool validatePhysics() {
        std::cout << "Performing standard physics validation...\n";
        return true;
    }
    
    virtual bool shouldPerformAnalysis() {
        return true;
    }
    
private:
    // Concrete methods - common to all simulations
    void loadConfiguration(const std::string& configFile) {
        std::cout << "Loading simulation configuration: " << configFile << "\n";
    }
    
    void saveResults() {
        std::cout << "Saving simulation results to output files\n";
    }
    
    void finalizeSimulation() {
        std::cout << "Cleaning up resources and finalizing simulation\n";
    }
};

// Concrete implementations - Specific scientific simulations
class MolecularDynamicsSimulation : public SimulationWorkflow {
private:
    int numParticles_ = 1000;
    double timeStep_ = 0.001;
    int totalSteps_ = 10000;
    
protected:
    void initializeSystem() override {
        std::cout << "Initializing Molecular Dynamics system...\n";
        std::cout << "  Number of particles: " << numParticles_ << "\n";
        std::cout << "  Time step: " << timeStep_ << " fs\n";
        std::cout << "  Total steps: " << totalSteps_ << "\n";
    }
    
    void setupInitialConditions() override {
        std::cout << "Setting up MD initial conditions...\n";
        std::cout << "  Placing particles in FCC lattice\n";
        std::cout << "  Assigning Maxwell-Boltzmann velocities (T=300K)\n";
        std::cout << "  Calculating initial forces\n";
    }
    
    void performTimeIntegration() override {
        std::cout << "Performing MD time integration...\n";
        std::cout << "  Using Velocity-Verlet integrator\n";
        
        for (int step = 1; step <= 5; ++step) {  // Simplified for demo
            std::cout << "  Step " << step << ": Energy = " 
                      << (-1.5e3 + step * 0.1) << " kJ/mol\n";
        }
        std::cout << "  ... continuing for " << totalSteps_ << " total steps\n";
    }
    
    void analyzeResults() override {
        std::cout << "Analyzing MD trajectory...\n";
        std::cout << "  Computing radial distribution function\n";
        std::cout << "  Calculating mean square displacement\n";
        std::cout << "  Analyzing velocity autocorrelation\n";
        std::cout << "  Thermodynamic averages: <T>=299.8K, <P>=1.02atm\n";
    }
};

class MonteCarloSimulation : public SimulationWorkflow {
private:
    int numSamples_ = 1000000;
    double convergenceTol_ = 1e-6;
    
protected:
    void initializeSystem() override {
        std::cout << "Initializing Monte Carlo simulation...\n";
        std::cout << "  Target samples: " << numSamples_ << "\n";
        std::cout << "  Convergence tolerance: " << convergenceTol_ << "\n";
        std::cout << "  Random seed: 42\n";
    }
    
    void setupInitialConditions() override {
        std::cout << "Setting up MC initial conditions...\n";
        std::cout << "  Initializing random number generator\n";
        std::cout << "  Setting up sampling domain\n";
        std::cout << "  Defining probability distributions\n";
    }
    
    void performTimeIntegration() override {
        std::cout << "Performing Monte Carlo sampling...\n";
        std::cout << "  Using Metropolis-Hastings algorithm\n";
        
        for (int block = 1; block <= 5; ++block) {
            double estimate = 3.14159 + (0.001 * block);  // Simulated π estimation
            std::cout << "  Block " << block << ": Current estimate = " 
                      << std::fixed << std::setprecision(6) << estimate 
                      << ", Error = " << std::scientific << (0.01 / block) << "\n";
        }
        std::cout << "  ... continuing sampling until convergence\n";
    }
    
    void analyzeResults() override {
        std::cout << "Analyzing MC results...\n";
        std::cout << "  Computing statistical uncertainties\n";
        std::cout << "  Checking equilibration and autocorrelation\n";
        std::cout << "  Final estimate: π = 3.141592 ± 0.000005\n";
        std::cout << "  Acceptance ratio: 0.457\n";
    }
};

class QuantumChemistryCalculation : public SimulationWorkflow {
private:
    std::string molecule_ = "H2O";
    std::string basisSet_ = "cc-pVDZ";
    bool hasConvIssues_ = false;
    
protected:
    void initializeSystem() override {
        std::cout << "Initializing Quantum Chemistry calculation...\n";
        std::cout << "  Molecule: " << molecule_ << "\n";
        std::cout << "  Basis set: " << basisSet_ << "\n";
        std::cout << "  Method: Hartree-Fock + MP2\n";
    }
    
    void setupInitialConditions() override {
        std::cout << "Setting up QC initial conditions...\n";
        std::cout << "  Reading molecular geometry\n";
        std::cout << "  Generating basis functions\n";
        std::cout << "  Computing overlap and kinetic energy integrals\n";
        std::cout << "  Initial SCF guess: Core Hamiltonian\n";
    }
    
    void performTimeIntegration() override {
        std::cout << "Performing SCF iterations...\n";
        
        for (int cycle = 1; cycle <= 8; ++cycle) {
            double energy = -76.02 - 0.001 * cycle;
            double deltE = 0.1 / cycle;
            std::cout << "  SCF cycle " << cycle << ": E = " 
                      << std::fixed << std::setprecision(6) << energy 
                      << " Hartree, ΔE = " << std::scientific << deltE << "\n";
                      
            if (cycle == 6 && molecule_ == "unstable_radical") {
                hasConvIssues_ = true;
                std::cout << "  WARNING: Convergence difficulties detected!\n";
            }
        }
        
        if (!hasConvIssues_) {
            std::cout << "  SCF converged!\n";
            std::cout << "  Computing MP2 correlation energy...\n";
        }
    }
    
    void analyzeResults() override {
        std::cout << "Analyzing QC results...\n";
        if (!hasConvIssues_) {
            std::cout << "  Final energies:\n";
            std::cout << "    HF energy: -76.026745 Hartree\n";
            std::cout << "    MP2 correlation: -0.234567 Hartree\n";
            std::cout << "    Total energy: -76.261312 Hartree\n";
            std::cout << "  Analyzing molecular orbitals and electron density\n";
        } else {
            std::cout << "  Analysis limited due to convergence issues\n";
        }
    }
    
    bool validatePhysics() override {
        if (hasConvIssues_) {
            std::cout << "Physics validation failed: SCF convergence issues\n";
            return false;
        }
        std::cout << "Quantum chemistry physics validation passed\n";
        return true;
    }
};

// Template Method for Iterative Scientific Solvers
class IterativeSolver {
public:
    // Template method for iterative solution process
    void solve() {
        initializeSolver();
        
        while (canContinue()) {
            performIteration();
            
            if (hasConverged()) {
                applyConvergenceAcceleration();
                break;
            }
            
            updateParameters();
        }
        
        finalizeSolution();
    }
    
    virtual ~IterativeSolver() = default;
    
protected:
    // Required methods that define the specific solver
    virtual void performIteration() = 0;
    virtual bool hasConverged() = 0;
    
    // Hook methods with default implementation
    virtual bool canContinue() {
        return getCurrentIteration() < getMaxIterations();
    }
    
    virtual void applyConvergenceAcceleration() {
        // Default: no acceleration
    }
    
    virtual void initializeSolver() {
        std::cout << "\n=== " << getSolverName() << " ===\n";
        std::cout << "Initializing solver parameters...\n";
    }
    
    virtual void updateParameters() {
        incrementIteration();
    }
    
    virtual void finalizeSolution() {
        std::cout << "Solution converged after " << getCurrentIteration() 
                  << " iterations\n";
    }
    
    virtual std::string getSolverName() const = 0;
    virtual int getCurrentIteration() const = 0;
    virtual int getMaxIterations() const = 0;
    virtual void incrementIteration() = 0;
};

class ConjugateGradientSolver : public IterativeSolver {
private:
    int iteration_ = 0;
    int maxIterations_ = 1000;
    double residualNorm_ = 1.0;
    double tolerance_ = 1e-8;
    
protected:
    void performIteration() override {
        iteration_++;
        residualNorm_ *= 0.85;  // Simulated convergence
        std::cout << "  CG iteration " << iteration_ 
                  << ": ||r|| = " << std::scientific << residualNorm_ << "\n";
    }
    
    bool hasConverged() override {
        return residualNorm_ < tolerance_;
    }
    
    void applyConvergenceAcceleration() override {
        std::cout << "  Applying Jacobi preconditioning for acceleration\n";
    }
    
    std::string getSolverName() const override {
        return "Conjugate Gradient Solver";
    }
    
    int getCurrentIteration() const override { return iteration_; }
    int getMaxIterations() const override { return maxIterations_; }
    void incrementIteration() override { /* Already done in performIteration */ }
};

class NewtonRaphsonSolver : public IterativeSolver {
private:
    int iteration_ = 0;
    int maxIterations_ = 50;
    double functionValue_ = 10.0;
    double tolerance_ = 1e-10;
    bool needsLineSearch_ = false;
    
protected:
    void performIteration() override {
        iteration_++;
        
        if (needsLineSearch_) {
            std::cout << "  NR iteration " << iteration_ 
                      << ": Applying line search for stability\n";
            functionValue_ *= 0.6;  // More conservative step
        } else {
            functionValue_ *= 0.3;  // Fast quadratic convergence
        }
        
        std::cout << "  NR iteration " << iteration_ 
                  << ": |f(x)| = " << std::scientific << functionValue_ << "\n";
                  
        // Simulate occasional line search need
        needsLineSearch_ = (iteration_ % 3 == 0);
    }
    
    bool hasConverged() override {
        return std::abs(functionValue_) < tolerance_;
    }
    
    bool canContinue() override {
        if (std::abs(functionValue_) > 1e10) {
            std::cout << "  Newton-Raphson diverged! Terminating.\n";
            return false;
        }
        return IterativeSolver::canContinue();
    }
    
    std::string getSolverName() const override {
        return "Newton-Raphson Nonlinear Solver";
    }
    
    int getCurrentIteration() const override { return iteration_; }
    int getMaxIterations() const override { return maxIterations_; }
    void incrementIteration() override { /* Already done in performIteration */ }
};

class GaussSeidelSolver : public IterativeSolver {
private:
    int iteration_ = 0;
    int maxIterations_ = 200;
    double residualNorm_ = 5.0;
    double tolerance_ = 1e-6;
    bool useRelaxation_ = false;
    
protected:
    void initializeSolver() override {
        IterativeSolver::initializeSolver();
        std::cout << "  Matrix size: 500x500\n";
        std::cout << "  Relaxation parameter: " << (useRelaxation_ ? "1.2" : "1.0") << "\n";
    }
    
    void performIteration() override {
        iteration_++;
        residualNorm_ *= 0.92;  // Slower than CG but steady
        
        std::cout << "  GS iteration " << iteration_ 
                  << ": ||r|| = " << std::scientific << residualNorm_;
                  
        if (useRelaxation_) {
            std::cout << " (SOR accelerated)";
        }
        std::cout << "\n";
        
        // Enable relaxation after initial iterations
        if (iteration_ == 10) {
            useRelaxation_ = true;
            std::cout << "  Enabling SOR acceleration\n";
        }
    }
    
    bool hasConverged() override {
        return residualNorm_ < tolerance_;
    }
    
    std::string getSolverName() const override {
        return "Gauss-Seidel Iterative Solver";
    }
    
    int getCurrentIteration() const override { return iteration_; }
    int getMaxIterations() const override { return maxIterations_; }
    void incrementIteration() override { /* Already done in performIteration */ }
};

// Template Method for Scientific Benchmarking Framework
class PerformanceBenchmark {
public:
    // Template method for performance benchmarking
    void runBenchmark() {
        prepareEnvironment();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            warmupPhase();
            measurementPhase();
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            std::cout << "✓ " << getBenchmarkName() << " completed\n";
            std::cout << "  Total time: " << duration.count() << " μs\n";
            
            analyzePerformance(duration.count());
            
        } catch (const std::exception& e) {
            std::cout << "✗ " << getBenchmarkName() << " failed: " 
                      << e.what() << "\n";
        }
        
        cleanupEnvironment();
    }
    
    virtual ~PerformanceBenchmark() = default;
    
protected:
    // Abstract methods
    virtual void measurementPhase() = 0;
    virtual std::string getBenchmarkName() const = 0;
    
    // Hook methods with default implementations
    virtual void prepareEnvironment() {
        std::cout << "Preparing benchmark environment...\n";
    }
    
    virtual void warmupPhase() {
        std::cout << "  Performing warmup iterations...\n";
    }
    
    virtual void analyzePerformance(long microseconds) {
        std::cout << "  Performance analysis: " << microseconds << " μs\n";
    }
    
    virtual void cleanupEnvironment() {
        std::cout << "  Cleaning up benchmark resources\n";
    }
};

class MatrixMultiplicationBenchmark : public PerformanceBenchmark {
private:
    std::vector<std::vector<double>> matrixA_, matrixB_, result_;
    int size_ = 500;
    
protected:
    void prepareEnvironment() override {
        PerformanceBenchmark::prepareEnvironment();
        std::cout << "  Allocating " << size_ << "x" << size_ << " matrices\n";
        
        matrixA_.resize(size_, std::vector<double>(size_, 1.5));
        matrixB_.resize(size_, std::vector<double>(size_, 2.0));
        result_.resize(size_, std::vector<double>(size_, 0.0));
    }
    
    void warmupPhase() override {
        PerformanceBenchmark::warmupPhase();
        // Perform smaller warmup multiplication
        for (int i = 0; i < 10; ++i) {
            for (int j = 0; j < 10; ++j) {
                double sum = 0.0;
                for (int k = 0; k < 10; ++k) {
                    sum += matrixA_[i][k] * matrixB_[k][j];
                }
                result_[i][j] = sum;
            }
        }
    }
    
    void measurementPhase() override {
        std::cout << "  Performing matrix multiplication (" << size_ << "x" << size_ << ")\n";
        
        for (int i = 0; i < size_; ++i) {
            for (int j = 0; j < size_; ++j) {
                double sum = 0.0;
                for (int k = 0; k < size_; ++k) {
                    sum += matrixA_[i][k] * matrixB_[k][j];
                }
                result_[i][j] = sum;
            }
        }
    }
    
    void analyzePerformance(long microseconds) override {
        double operations = 2.0 * size_ * size_ * size_;  // 2n³ operations
        double gflops = (operations / 1e9) / (microseconds / 1e6);
        
        std::cout << "  Operations: " << std::scientific << operations << "\n";
        std::cout << "  Performance: " << std::fixed << std::setprecision(2) 
                  << gflops << " GFLOPS\n";
    }
    
    std::string getBenchmarkName() const override {
        return "Matrix Multiplication Benchmark";
    }
};

class FFTBenchmark : public PerformanceBenchmark {
private:
    std::vector<std::complex<double>> signal_, fftResult_;
    int signalSize_ = 65536;  // 2^16
    
protected:
    void prepareEnvironment() override {
        PerformanceBenchmark::prepareEnvironment();
        std::cout << "  Generating signal with " << signalSize_ << " samples\n";
        
        signal_.resize(signalSize_);
        fftResult_.resize(signalSize_);
        
        // Generate test signal
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> dist(0.0, 1.0);
        
        for (int i = 0; i < signalSize_; ++i) {
            signal_[i] = std::complex<double>(dist(gen), dist(gen));
        }
    }
    
    void measurementPhase() override {
        std::cout << "  Performing FFT computation\n";
        
        // Simplified FFT simulation (normally would use FFTW or similar)
        for (int i = 0; i < signalSize_; ++i) {
            std::complex<double> sum(0.0, 0.0);
            for (int k = 0; k < signalSize_; ++k) {
                double angle = -2.0 * M_PI * i * k / signalSize_;
                std::complex<double> twiddle(std::cos(angle), std::sin(angle));
                sum += signal_[k] * twiddle;
            }
            fftResult_[i] = sum;
        }
    }
    
    void analyzePerformance(long microseconds) override {
        double operations = signalSize_ * std::log2(signalSize_) * 5;  // Approx N log N
        double mflops = (operations / 1e6) / (microseconds / 1e6);
        
        std::cout << "  Signal size: " << signalSize_ << " samples\n";
        std::cout << "  Performance: " << std::fixed << std::setprecision(2) 
                  << mflops << " MFLOPS\n";
    }
    
    std::string getBenchmarkName() const override {
        return "FFT Performance Benchmark";
    }
};

class MonteCarloBenchmark : public PerformanceBenchmark {
private:
    double piEstimate_ = 0.0;
    int numSamples_ = 10000000;
    
protected:
    void measurementPhase() override {
        std::cout << "  Computing π using Monte Carlo (" << numSamples_ << " samples)\n";
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(-1.0, 1.0);
        
        int insideCircle = 0;
        for (int i = 0; i < numSamples_; ++i) {
            double x = dist(gen);
            double y = dist(gen);
            if (x*x + y*y <= 1.0) {
                insideCircle++;
            }
        }
        
        piEstimate_ = 4.0 * insideCircle / numSamples_;
    }
    
    void analyzePerformance(long microseconds) override {
        double samplesPerSecond = (numSamples_ / 1e6) / (microseconds / 1e6);
        double error = std::abs(piEstimate_ - M_PI);
        
        std::cout << "  π estimate: " << std::fixed << std::setprecision(6) << piEstimate_ << "\n";
        std::cout << "  Error: " << std::scientific << error << "\n";
        std::cout << "  Sampling rate: " << std::fixed << std::setprecision(2) 
                  << samplesPerSecond << " MSamples/sec\n";
    }
    
    std::string getBenchmarkName() const override {
        return "Monte Carlo π Estimation Benchmark";
    }
};

// Template Method for Scientific Data Processing Pipeline
class DataProcessingPipeline {
public:
    bool processDataset(const std::string& datasetName) {
        if (!validateInputData(datasetName)) {
            std::cout << "Data validation failed for: " << datasetName << "\n";
            return false;
        }
        
        loadDataToMemory();
        
        if (requiresPreprocessing()) {
            if (!performPreprocessing()) {
                std::cout << "Preprocessing failed!\n";
                return false;
            }
        }
        
        analyzeData();
        generateResults();
        return true;
    }
    
    virtual ~DataProcessingPipeline() = default;
    
protected:
    // Abstract methods - must be implemented by specific data processors
    virtual bool validateInputData(const std::string& datasetName) = 0;
    virtual void analyzeData() = 0;
    virtual void generateResults() = 0;
    
    // Hook methods - can be overridden for specialized behavior
    virtual bool requiresPreprocessing() {
        return false;
    }
    
    virtual bool performPreprocessing() {
        std::cout << "  Applying standard preprocessing...\n";
        return true;
    }
    
private:
    void loadDataToMemory() {
        std::cout << "Loading dataset into memory...\n";
    }
};

class GenomicsDataProcessor : public DataProcessingPipeline {
protected:
    bool validateInputData(const std::string& datasetName) override {
        std::cout << "Validating genomics dataset: " << datasetName << "\n";
        std::cout << "  Checking FASTA/FASTQ format integrity\n";
        std::cout << "  Verifying sequence quality scores\n";
        std::cout << "  Validating metadata completeness\n";
        return true;
    }
    
    void analyzeData() override {
        std::cout << "Performing genomics analysis...\n";
        std::cout << "  Sequence alignment and mapping\n";
        std::cout << "  Variant calling and annotation\n";
        std::cout << "  Gene expression quantification\n";
        std::cout << "  Pathway enrichment analysis\n";
    }
    
    void generateResults() override {
        std::cout << "Generating genomics results...\n";
        std::cout << "  VCF file with identified variants\n";
        std::cout << "  Expression matrix for differential analysis\n";
        std::cout << "  Quality control reports\n";
        std::cout << "  Functional annotation summaries\n";
    }
    
    bool requiresPreprocessing() override {
        return true;
    }
    
    bool performPreprocessing() override {
        std::cout << "  Quality trimming and adapter removal\n";
        std::cout << "  Read deduplication and filtering\n";
        std::cout << "  Contamination screening\n";
        return true;
    }
};

class ClimateDataProcessor : public DataProcessingPipeline {
protected:
    bool validateInputData(const std::string& datasetName) override {
        std::cout << "Validating climate dataset: " << datasetName << "\n";
        std::cout << "  Checking NetCDF format and metadata\n";
        std::cout << "  Verifying temporal and spatial coverage\n";
        std::cout << "  Validating measurement units and scales\n";
        return true;
    }
    
    void analyzeData() override {
        std::cout << "Performing climate analysis...\n";
        std::cout << "  Temporal trend analysis and seasonality\n";
        std::cout << "  Spatial correlation and teleconnections\n";
        std::cout << "  Extreme event identification\n";
        std::cout << "  Climate model validation\n";
    }
    
    void generateResults() override {
        std::cout << "Generating climate results...\n";
        std::cout << "  Time series plots and trend maps\n";
        std::cout << "  Statistical summary tables\n";
        std::cout << "  Anomaly detection reports\n";
        std::cout << "  Climate projection visualizations\n";
    }
    
    bool requiresPreprocessing() override {
        return true;
    }
    
    bool performPreprocessing() override {
        std::cout << "  Grid interpolation and resampling\n";
        std::cout << "  Missing data imputation\n";
        std::cout << "  Unit conversion and normalization\n";
        return true;
    }
};

int main() {
    std::cout << "=== Scientific Computational Workflows Template Method ===\n\n";
    
    // Scientific Simulation Workflows
    std::cout << "=== Scientific Simulation Workflows ===\n";
    
    auto mdSimulation = std::make_unique<MolecularDynamicsSimulation>();
    auto mcSimulation = std::make_unique<MonteCarloSimulation>();
    auto qcCalculation = std::make_unique<QuantumChemistryCalculation>();
    
    std::cout << "\n--- Molecular Dynamics Simulation ---\n";
    mdSimulation->runSimulation("md_config.yaml");
    
    std::cout << "\n--- Monte Carlo Simulation ---\n";
    mcSimulation->runSimulation("mc_config.yaml");
    
    std::cout << "\n--- Quantum Chemistry Calculation ---\n";
    qcCalculation->runSimulation("qc_config.yaml");
    
    // Iterative Scientific Solvers
    std::cout << "\n\n=== Iterative Scientific Solvers ===\n";
    
    auto cgSolver = std::make_unique<ConjugateGradientSolver>();
    auto nrSolver = std::make_unique<NewtonRaphsonSolver>();
    auto gsSolver = std::make_unique<GaussSeidelSolver>();
    
    std::cout << "\n--- Linear System Solvers ---\n";
    cgSolver->solve();
    
    std::cout << "\n--- Nonlinear System Solver ---\n";
    nrSolver->solve();
    
    std::cout << "\n--- Alternative Linear Solver ---\n";
    gsSolver->solve();
    
    // Scientific Performance Benchmarking
    std::cout << "\n\n=== Scientific Performance Benchmarks ===\n\n";
    
    std::vector<std::unique_ptr<PerformanceBenchmark>> benchmarks;
    benchmarks.push_back(std::make_unique<MatrixMultiplicationBenchmark>());
    benchmarks.push_back(std::make_unique<FFTBenchmark>());
    benchmarks.push_back(std::make_unique<MonteCarloBenchmark>());
    
    std::cout << "Running benchmark suite...\n";
    std::cout << "========================\n";
    
    for (auto& benchmark : benchmarks) {
        benchmark->runBenchmark();
        std::cout << "\n";
    }
    
    // Scientific Data Processing Pipelines
    std::cout << "\n=== Scientific Data Processing Pipelines ===\n\n";
    
    GenomicsDataProcessor genomicsProcessor;
    ClimateDataProcessor climateProcessor;
    
    std::cout << "--- Genomics Data Processing ---\n";
    genomicsProcessor.processDataset("human_genome_v38.fastq");
    
    std::cout << "\n--- Climate Data Processing ---\n";
    climateProcessor.processDataset("global_temperature_1880_2023.nc");
    
    std::cout << "\n=== Template Method Summary ===\n";
    std::cout << "The Template Method pattern provides a powerful framework for:\n";
    std::cout << "• Defining scientific computational workflows with customizable steps\n";
    std::cout << "• Creating iterative solvers with different convergence strategies\n";
    std::cout << "• Building performance benchmarking frameworks\n";
    std::cout << "• Implementing data processing pipelines for various scientific domains\n";
    std::cout << "\nThis pattern ensures consistent algorithm structure while allowing\n";
    std::cout << "specialized implementations for different scientific applications.\n";
    
    return 0;
}