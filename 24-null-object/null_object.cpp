// Null Object Pattern - Scientific Computing Safety and Default Behaviors
// Provides safe default implementations for scientific computation interfaces
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <limits>
#include <functional>

// Abstract Convergence Monitor for iterative solvers
class ConvergenceMonitor {
public:
    virtual ~ConvergenceMonitor() = default;
    virtual void recordIteration(int iteration, double residual, double error) = 0;
    virtual void recordConvergence(int totalIterations, double finalError) = 0;
    virtual void recordDivergence(const std::string& reason) = 0;
    virtual bool shouldStop(double residual, int iteration) = 0;
    virtual bool isNull() const { return false; }
};

// Concrete Convergence Monitor
class DetailedConvergenceMonitor : public ConvergenceMonitor {
private:
    double tolerance_;
    int maxIterations_;
    std::string prefix_;
    
public:
    DetailedConvergenceMonitor(double tolerance = 1e-10, int maxIter = 1000, 
                              const std::string& prefix = "[SOLVER]") 
        : tolerance_(tolerance), maxIterations_(maxIter), prefix_(prefix) {}
    
    void recordIteration(int iteration, double residual, double error) override {
        std::cout << prefix_ << " Iteration " << iteration 
                  << ": residual = " << std::scientific << std::setprecision(6) 
                  << residual << ", error = " << error << "\n";
    }
    
    void recordConvergence(int totalIterations, double finalError) override {
        std::cout << prefix_ << " CONVERGED after " << totalIterations 
                  << " iterations, final error = " << std::scientific 
                  << finalError << "\n";
    }
    
    void recordDivergence(const std::string& reason) override {
        std::cout << prefix_ << " DIVERGED: " << reason << "\n";
    }
    
    bool shouldStop(double residual, int iteration) override {
        return residual < tolerance_ || iteration >= maxIterations_;
    }
};

// Summary Monitor - less verbose
class SummaryConvergenceMonitor : public ConvergenceMonitor {
private:
    double tolerance_;
    int maxIterations_;
    int startIteration_;
    std::chrono::steady_clock::time_point startTime_;
    
public:
    SummaryConvergenceMonitor(double tolerance = 1e-10, int maxIter = 1000)
        : tolerance_(tolerance), maxIterations_(maxIter), startIteration_(0) {}
    
    void recordIteration(int iteration, double residual, double error) override {
        if (iteration == 1) {
            startIteration_ = iteration;
            startTime_ = std::chrono::steady_clock::now();
            std::cout << "[SOLVER] Starting iterative solution...\n";
        }
    }
    
    void recordConvergence(int totalIterations, double finalError) override {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (endTime - startTime_).count();
        std::cout << "[SOLVER] Converged: " << totalIterations 
                  << " iterations in " << duration << " ms\n";
    }
    
    void recordDivergence(const std::string& reason) override {
        std::cout << "[SOLVER] Failed to converge: " << reason << "\n";
    }
    
    bool shouldStop(double residual, int iteration) override {
        return residual < tolerance_ || iteration >= maxIterations_;
    }
};

// Null Object Monitor - silent operation
class NullConvergenceMonitor : public ConvergenceMonitor {
public:
    void recordIteration(int iteration, double residual, double error) override {
        // Do nothing - silent operation
    }
    
    void recordConvergence(int totalIterations, double finalError) override {
        // Do nothing
    }
    
    void recordDivergence(const std::string& reason) override {
        // Do nothing
    }
    
    bool shouldStop(double residual, int iteration) override {
        // Default convergence criteria
        return residual < 1e-10 || iteration >= 10000;
    }
    
    bool isNull() const override { return true; }
};

// Iterative Solver that uses convergence monitor
class IterativeSolver {
private:
    std::shared_ptr<ConvergenceMonitor> monitor_;
    
public:
    IterativeSolver(std::shared_ptr<ConvergenceMonitor> monitor = 
                    std::make_shared<NullConvergenceMonitor>())
        : monitor_(monitor) {}
    
    void setMonitor(std::shared_ptr<ConvergenceMonitor> monitor) {
        monitor_ = monitor ? monitor : std::make_shared<NullConvergenceMonitor>();
    }
    
    void solveLinearSystem(int size = 100) {
        // Simulate iterative solution of Ax = b
        double residual = 1.0;
        double error = 1.0;
        int iteration = 0;
        
        while (!monitor_->shouldStop(residual, iteration)) {
            iteration++;
            
            // Simulate convergence
            residual *= 0.1;
            error *= 0.15;
            
            monitor_->recordIteration(iteration, residual, error);
            
            // Simulate possible divergence
            if (iteration > 5 && residual > 0.1) {
                monitor_->recordDivergence("Residual not decreasing");
                return;
            }
        }
        
        monitor_->recordConvergence(iteration, error);
    }
    
    void showMonitorStatus() {
        std::cout << "Convergence monitoring is " 
                  << (monitor_->isNull() ? "disabled" : "enabled") << "\n";
    }
};

// Boundary Condition Handler for PDEs
class BoundaryCondition {
public:
    virtual ~BoundaryCondition() = default;
    virtual std::string getType() const = 0;
    virtual double getValue(double x, double y, double t) const = 0;
    virtual double getDerivative(double x, double y, double t) const = 0;
    virtual bool isNull() const { return false; }
    virtual void applyToGrid(std::vector<std::vector<double>>& grid, double t) = 0;
};

// Dirichlet boundary condition (fixed value)
class DirichletBoundary : public BoundaryCondition {
private:
    std::string name_;
    double value_;
    
public:
    DirichletBoundary(const std::string& name, double value) 
        : name_(name), value_(value) {}
    
    std::string getType() const override { return "Dirichlet (" + name_ + ")"; }
    
    double getValue(double x, double y, double t) const override {
        // Can be made position/time dependent
        return value_;
    }
    
    double getDerivative(double x, double y, double t) const override {
        return 0.0;  // Dirichlet doesn't specify derivative
    }
    
    void applyToGrid(std::vector<std::vector<double>>& grid, double t) override {
        int rows = grid.size();
        int cols = grid[0].size();
        
        // Apply to boundaries
        for (int i = 0; i < rows; ++i) {
            grid[i][0] = value_;         // Left boundary
            grid[i][cols-1] = value_;    // Right boundary
        }
        for (int j = 0; j < cols; ++j) {
            grid[0][j] = value_;         // Top boundary
            grid[rows-1][j] = value_;    // Bottom boundary
        }
    }
};

// Neumann boundary condition (fixed derivative)
class NeumannBoundary : public BoundaryCondition {
private:
    std::string name_;
    double flux_;
    double dx_;
    
public:
    NeumannBoundary(const std::string& name, double flux, double dx = 0.01) 
        : name_(name), flux_(flux), dx_(dx) {}
    
    std::string getType() const override { return "Neumann (" + name_ + ")"; }
    
    double getValue(double x, double y, double t) const override {
        // Neumann specifies derivative, not value
        return std::numeric_limits<double>::quiet_NaN();
    }
    
    double getDerivative(double x, double y, double t) const override {
        return flux_;
    }
    
    void applyToGrid(std::vector<std::vector<double>>& grid, double t) override {
        int rows = grid.size();
        int cols = grid[0].size();
        
        // Apply zero-flux boundary (∂u/∂n = flux)
        for (int i = 1; i < rows-1; ++i) {
            grid[i][0] = grid[i][1] - flux_ * dx_;         // Left
            grid[i][cols-1] = grid[i][cols-2] + flux_ * dx_; // Right
        }
        for (int j = 1; j < cols-1; ++j) {
            grid[0][j] = grid[1][j] - flux_ * dx_;         // Top
            grid[rows-1][j] = grid[rows-2][j] + flux_ * dx_; // Bottom
        }
    }
};

// Null boundary condition - no boundary effects
class NullBoundary : public BoundaryCondition {
public:
    std::string getType() const override { return "None (Open Boundary)"; }
    double getValue(double x, double y, double t) const override { return 0.0; }
    double getDerivative(double x, double y, double t) const override { return 0.0; }
    bool isNull() const override { return true; }
    
    void applyToGrid(std::vector<std::vector<double>>& grid, double t) override {
        // Do nothing - leave boundary values unchanged
        // This simulates an open/transparent boundary
    }
};

// PDE Solver that uses boundary conditions
class PDESolver {
private:
    std::shared_ptr<BoundaryCondition> boundary_;
    int gridSize_;
    double dx_;
    std::vector<std::vector<double>> grid_;
    
public:
    PDESolver(int gridSize = 50, 
              std::shared_ptr<BoundaryCondition> boundary = 
              std::make_shared<NullBoundary>())
        : boundary_(boundary), gridSize_(gridSize), dx_(1.0 / gridSize) {
        grid_.resize(gridSize, std::vector<double>(gridSize, 0.0));
    }
    
    void setBoundaryCondition(std::shared_ptr<BoundaryCondition> boundary) {
        boundary_ = boundary ? boundary : std::make_shared<NullBoundary>();
    }
    
    void initializeGrid(const std::string& pattern = "gaussian") {
        std::cout << "Initializing " << gridSize_ << "x" << gridSize_ 
                  << " grid with " << pattern << " pattern\n";
        
        double centerX = gridSize_ / 2.0;
        double centerY = gridSize_ / 2.0;
        double sigma = gridSize_ / 6.0;
        
        for (int i = 0; i < gridSize_; ++i) {
            for (int j = 0; j < gridSize_; ++j) {
                if (pattern == "gaussian") {
                    double r2 = pow(i - centerX, 2) + pow(j - centerY, 2);
                    grid_[i][j] = exp(-r2 / (2 * sigma * sigma));
                } else if (pattern == "step") {
                    grid_[i][j] = (i < gridSize_/2) ? 1.0 : 0.0;
                }
            }
        }
    }
    
    void solveHeatEquation(double dt = 0.001, int steps = 100) {
        std::cout << "\nSolving heat equation with " 
                  << boundary_->getType() << " boundary conditions\n";
        
        double alpha = 0.1;  // Thermal diffusivity
        double stability = alpha * dt / (dx_ * dx_);
        
        if (stability > 0.5) {
            std::cout << "Warning: Stability parameter = " << stability 
                      << " (should be < 0.5)\n";
        }
        
        // Show initial state
        std::cout << "Initial state - Center value: " 
                  << std::fixed << std::setprecision(4) 
                  << grid_[gridSize_/2][gridSize_/2] << "\n";
        
        // Time evolution
        for (int step = 0; step < steps; ++step) {
            // Apply boundary conditions
            boundary_->applyToGrid(grid_, step * dt);
            
            // Update interior points (explicit scheme)
            std::vector<std::vector<double>> newGrid = grid_;
            
            for (int i = 1; i < gridSize_-1; ++i) {
                for (int j = 1; j < gridSize_-1; ++j) {
                    double laplacian = (grid_[i+1][j] + grid_[i-1][j] + 
                                       grid_[i][j+1] + grid_[i][j-1] - 
                                       4 * grid_[i][j]) / (dx_ * dx_);
                    newGrid[i][j] = grid_[i][j] + alpha * dt * laplacian;
                }
            }
            
            grid_ = newGrid;
        }
        
        // Show final state
        std::cout << "Final state - Center value: " 
                  << grid_[gridSize_/2][gridSize_/2] << "\n";
        
        // Calculate total heat (integral)
        double totalHeat = 0.0;
        for (int i = 0; i < gridSize_; ++i) {
            for (int j = 0; j < gridSize_; ++j) {
                totalHeat += grid_[i][j] * dx_ * dx_;
            }
        }
        
        std::cout << "Total heat in domain: " << totalHeat << "\n";
        
        if (!boundary_->isNull()) {
            std::cout << "Boundary effects: Active (" 
                      << boundary_->getType() << ")\n";
        } else {
            std::cout << "Boundary effects: None (open boundary)\n";
        }
    }
};

// Numerical Integrator with Null Object
class NumericalIntegrator {
public:
    virtual ~NumericalIntegrator() = default;
    virtual double integrate(std::function<double(double)> f, 
                           double a, double b, int n) = 0;
    virtual std::string getName() const = 0;
    virtual double getErrorEstimate() const = 0;
    virtual bool isNull() const { return false; }
};

// Trapezoidal Rule Integrator
class TrapezoidalIntegrator : public NumericalIntegrator {
private:
    mutable double lastError_;
    
public:
    double integrate(std::function<double(double)> f, 
                    double a, double b, int n) override {
        std::cout << "Performing trapezoidal integration with " << n << " intervals...\n";
        
        double h = (b - a) / n;
        double sum = 0.5 * (f(a) + f(b));
        
        for (int i = 1; i < n; ++i) {
            sum += f(a + i * h);
        }
        
        double result = h * sum;
        
        // Estimate error using Richardson extrapolation
        double coarse = integrate_internal(f, a, b, n/2);
        lastError_ = std::abs(result - coarse) / 3.0;
        
        return result;
    }
    
    std::string getName() const override { return "Trapezoidal Rule"; }
    
    double getErrorEstimate() const override { return lastError_; }
    
private:
    double integrate_internal(std::function<double(double)> f, 
                            double a, double b, int n) {
        double h = (b - a) / n;
        double sum = 0.5 * (f(a) + f(b));
        for (int i = 1; i < n; ++i) {
            sum += f(a + i * h);
        }
        return h * sum;
    }
};

// Simpson's Rule Integrator
class SimpsonsIntegrator : public NumericalIntegrator {
private:
    mutable double lastError_;
    
public:
    double integrate(std::function<double(double)> f, 
                    double a, double b, int n) override {
        std::cout << "Performing Simpson's rule integration...\n";
        
        if (n % 2 != 0) n++; // Ensure even number of intervals
        
        double h = (b - a) / n;
        double sum = f(a) + f(b);
        
        for (int i = 1; i < n; i += 2) {
            sum += 4 * f(a + i * h);
        }
        for (int i = 2; i < n; i += 2) {
            sum += 2 * f(a + i * h);
        }
        
        double result = (h / 3) * sum;
        lastError_ = pow(h, 4) * (b - a) / 180.0; // Theoretical error bound
        
        return result;
    }
    
    std::string getName() const override { return "Simpson's Rule"; }
    
    double getErrorEstimate() const override { return lastError_; }
};

// Null Integrator - returns zero
class NullIntegrator : public NumericalIntegrator {
public:
    double integrate(std::function<double(double)> f, 
                    double a, double b, int n) override {
        // Return zero - safe default for missing integrator
        return 0.0;
    }
    
    std::string getName() const override { return "No Integration"; }
    double getErrorEstimate() const override { return 0.0; }
    bool isNull() const override { return true; }
};

// Scientific Function Analyzer
class FunctionAnalyzer {
private:
    std::shared_ptr<NumericalIntegrator> integrator_;
    
public:
    FunctionAnalyzer(std::shared_ptr<NumericalIntegrator> integrator = 
                     std::make_shared<NullIntegrator>())
        : integrator_(integrator) {}
    
    void setIntegrator(std::shared_ptr<NumericalIntegrator> integrator) {
        integrator_ = integrator ? integrator : std::make_shared<NullIntegrator>();
    }
    
    void analyzeFunction(std::function<double(double)> f, 
                        const std::string& name,
                        double a, double b) {
        std::cout << "\nAnalyzing function: " << name 
                  << " on [" << a << ", " << b << "]\n";
        
        std::cout << "Integration method: " << integrator_->getName() << "\n";
        
        double integral = integrator_->integrate(f, a, b, 1000);
        
        std::cout << "Integral value: " << std::fixed << std::setprecision(8) 
                  << integral << "\n";
        
        if (!integrator_->isNull()) {
            std::cout << "Error estimate: " << std::scientific 
                      << integrator_->getErrorEstimate() << "\n";
        } else {
            std::cout << "(Integration disabled - returning zero)\n";
        }
    }
};

// Preconditioner hierarchy with Null Object
class Preconditioner {
public:
    virtual ~Preconditioner() = default;
    virtual std::string getName() const = 0;
    virtual std::string getType() const = 0;
    virtual void apply(std::vector<double>& x, const std::vector<double>& b) = 0;
    virtual double getConditionNumber() const = 0;
    virtual bool isNull() const { return false; }
    virtual void analyze(int level = 0) const = 0;
};

// Null Preconditioner - identity operation
class NullPreconditioner : public Preconditioner {
public:
    std::string getName() const override { return "No Preconditioning"; }
    std::string getType() const override { return "Identity"; }
    
    void apply(std::vector<double>& x, const std::vector<double>& b) override {
        // Identity preconditioner: x = b
        x = b;
    }
    
    double getConditionNumber() const override { return 1.0; }
    bool isNull() const override { return true; }
    
    void analyze(int level) const override {
        // Do nothing
    }
};

// Jacobi Preconditioner
class JacobiPreconditioner : public Preconditioner {
private:
    std::string name_;
    std::vector<double> diagonal_;
    std::shared_ptr<Preconditioner> subPreconditioner_;
    
    static std::shared_ptr<Preconditioner> nullPreconditioner_;
    
public:
    JacobiPreconditioner(const std::string& name, 
                        const std::vector<double>& diag,
                        std::shared_ptr<Preconditioner> subPrec = nullptr)
        : name_(name), diagonal_(diag) {
        subPreconditioner_ = subPrec ? subPrec : getNullPreconditioner();
    }
    
    static std::shared_ptr<Preconditioner> getNullPreconditioner() {
        if (!nullPreconditioner_) {
            nullPreconditioner_ = std::make_shared<NullPreconditioner>();
        }
        return nullPreconditioner_;
    }
    
    std::string getName() const override { return name_; }
    std::string getType() const override { return "Jacobi (Diagonal)"; }
    
    void apply(std::vector<double>& x, const std::vector<double>& b) override {
        // Apply Jacobi preconditioning: x = D^(-1) * b
        for (size_t i = 0; i < x.size() && i < diagonal_.size(); ++i) {
            x[i] = b[i] / diagonal_[i];
        }
    }
    
    double getConditionNumber() const override {
        // Estimate condition number improvement
        double maxDiag = *std::max_element(diagonal_.begin(), diagonal_.end());
        double minDiag = *std::min_element(diagonal_.begin(), diagonal_.end());
        return maxDiag / minDiag;
    }
    
    std::shared_ptr<Preconditioner> getSubPreconditioner() const { 
        return subPreconditioner_; 
    }
    
    void analyze(int level = 0) const override {
        std::string indent(level * 2, ' ');
        std::cout << indent << name_ << " (" << getType() << ")";
        std::cout << " - Condition number: " << std::scientific 
                  << std::setprecision(2) << getConditionNumber();
        
        if (!subPreconditioner_->isNull()) {
            std::cout << " with sub-preconditioner: " 
                      << subPreconditioner_->getName();
        }
        std::cout << "\n";
    }
};

// ILU Preconditioner
class ILUPreconditioner : public Preconditioner {
private:
    std::string name_;
    int fillLevel_;
    
public:
    ILUPreconditioner(const std::string& name, int fillLevel = 0)
        : name_(name), fillLevel_(fillLevel) {}
    
    std::string getName() const override { return name_; }
    std::string getType() const override { 
        return "ILU(" + std::to_string(fillLevel_) + ")"; 
    }
    
    void apply(std::vector<double>& x, const std::vector<double>& b) override {
        // Simplified ILU application
        std::cout << "Applying " << getType() << " preconditioning\n";
        x = b; // Placeholder - would involve triangular solves
    }
    
    double getConditionNumber() const override {
        // ILU typically provides good conditioning
        return 10.0 / (fillLevel_ + 1);
    }
    
    void analyze(int level = 0) const override {
        std::string indent(level * 2, ' ');
        std::cout << indent << name_ << " (" << getType() << ")";
        std::cout << " - Fill level: " << fillLevel_;
        std::cout << ", Estimated condition: " << getConditionNumber() << "\n";
    }
};

std::shared_ptr<Preconditioner> JacobiPreconditioner::nullPreconditioner_ = nullptr;

// Define M_PI if not already defined (for MSVC)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    std::cout << "=== Scientific Computing with Null Object Pattern ===\n";
    std::cout << "Safe defaults for numerical algorithms and boundary conditions\n\n";
    
    // Iterative Solver with Convergence Monitoring
    std::cout << "=== Convergence Monitoring with Null Object Pattern ===\n\n";
    
    // Solver without monitor (uses NullConvergenceMonitor)
    std::cout << "Solver without monitoring:\n";
    IterativeSolver solver1;
    solver1.showMonitorStatus();
    solver1.solveLinearSystem();
    
    // Solver with detailed monitoring
    std::cout << "\nSolver with detailed monitoring:\n";
    IterativeSolver solver2(std::make_shared<DetailedConvergenceMonitor>(1e-12, 100));
    solver2.showMonitorStatus();
    solver2.solveLinearSystem();
    
    // Solver with summary monitoring
    std::cout << "\nSolver with summary monitoring:\n";
    IterativeSolver solver3(std::make_shared<SummaryConvergenceMonitor>());
    solver3.solveLinearSystem();
    
    // Switching monitors at runtime
    std::cout << "\nSwitching monitor at runtime:\n";
    solver1.setMonitor(std::make_shared<DetailedConvergenceMonitor>(1e-6, 50, "[ADAPTIVE]"));
    solver1.showMonitorStatus();
    solver1.solveLinearSystem();
    
    // PDE Solver with Boundary Conditions
    std::cout << "\n\n=== PDE Solver with Null Object Pattern ===\n";
    
    // Heat equation without boundary conditions (null boundary)
    std::cout << "\nHeat Equation - Open Boundary:\n";
    PDESolver pde1;
    pde1.initializeGrid("gaussian");
    pde1.solveHeatEquation(0.0001, 1000);
    
    // Heat equation with Dirichlet boundary
    std::cout << "\n\nHeat Equation - Dirichlet Boundary (T=0):\n";
    auto dirichlet = std::make_shared<DirichletBoundary>("Cold walls", 0.0);
    PDESolver pde2(50, dirichlet);
    pde2.initializeGrid("gaussian");
    pde2.solveHeatEquation(0.0001, 1000);
    
    // Heat equation with Neumann boundary
    std::cout << "\n\nHeat Equation - Neumann Boundary (insulated):\n";
    auto neumann = std::make_shared<NeumannBoundary>("Insulated", 0.0);
    PDESolver pde3(50, neumann);
    pde3.initializeGrid("step");
    pde3.solveHeatEquation(0.0001, 1000);
    
    // Function Analysis with Numerical Integration
    std::cout << "\n\n=== Function Analysis with Null Object Pattern ===\n";
    
    FunctionAnalyzer analyzer;
    
    // Define test functions
    auto f1 = [](double x) { return x * x; };  // x^2
    auto f2 = [](double x) { return std::sin(x); };  // sin(x)
    auto f3 = [](double x) { return std::exp(-x * x); };  // Gaussian
    
    // Analyze without integration (null integrator)
    std::cout << "\nAnalysis without integration:\n";
    analyzer.analyzeFunction(f1, "f(x) = x^2", 0, 1);
    
    // Analyze with trapezoidal integration
    std::cout << "\nAnalysis with Trapezoidal integration:\n";
    analyzer.setIntegrator(std::make_shared<TrapezoidalIntegrator>());
    analyzer.analyzeFunction(f1, "f(x) = x^2", 0, 1);
    std::cout << "Analytical result: " << 1.0/3.0 << "\n";
    
    // Analyze with Simpson's rule
    std::cout << "\nAnalysis with Simpson's rule:\n";
    analyzer.setIntegrator(std::make_shared<SimpsonsIntegrator>());
    analyzer.analyzeFunction(f2, "f(x) = sin(x)", 0, M_PI);
    std::cout << "Analytical result: " << 2.0 << "\n";
    
    // Gaussian integral
    analyzer.analyzeFunction(f3, "f(x) = exp(-x^2)", -5, 5);
    std::cout << "Analytical result: " << std::sqrt(M_PI) << "\n";
    
    // Preconditioner Hierarchy
    std::cout << "\n\n=== Preconditioner Hierarchy with Null Object Pattern ===\n";
    
    // Create diagonal for Jacobi preconditioner
    std::vector<double> diagonal = {4.0, 3.5, 3.0, 2.5, 2.0};
    
    auto mainPrecond = std::make_shared<JacobiPreconditioner>("Main System", diagonal);
    auto subPrecond = std::make_shared<JacobiPreconditioner>("Subsystem", 
                                                               std::vector<double>{2.0, 1.8, 1.6});
    auto iluPrecond = std::make_shared<ILUPreconditioner>("Block ILU", 2);
    
    // Preconditioner without sub-preconditioner
    auto simplePrecond = std::make_shared<JacobiPreconditioner>("Simple Diagonal", 
                                                                 std::vector<double>{1.0, 1.0, 1.0});
    
    std::cout << "\nPreconditioner Hierarchy:\n";
    mainPrecond->analyze();
    subPrecond->analyze(1);
    iluPrecond->analyze(1);
    simplePrecond->analyze();
    
    std::cout << "\nApplying preconditioners:\n";
    std::vector<double> b = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> x(5);
    
    std::cout << "Input vector b: ";
    for (double val : b) std::cout << val << " ";
    std::cout << "\n";
    
    mainPrecond->apply(x, b);
    std::cout << mainPrecond->getName() << " result: ";
    for (double val : x) std::cout << std::fixed << std::setprecision(2) << val << " ";
    std::cout << "\n";
    
    // Null preconditioner (identity)
    auto nullPrecond = JacobiPreconditioner::getNullPreconditioner();
    nullPrecond->apply(x, b);
    std::cout << nullPrecond->getName() << " result: ";
    for (double val : x) std::cout << val << " ";
    std::cout << "\n";
    
    std::cout << "\n=== Null Object Pattern Summary ===\n";
    std::cout << "The Null Object pattern provides safe defaults in scientific computing:\n";
    std::cout << "• Silent convergence monitoring for production runs\n";
    std::cout << "• Open boundary conditions for unbounded domains\n";
    std::cout << "• Identity preconditioning when no acceleration needed\n";
    std::cout << "• Zero integration for disabled numerical quadrature\n";
    std::cout << "\nThis pattern eliminates null checks and provides predictable behavior\n";
    std::cout << "for optional scientific computing components.\n";
    
    return 0;
}