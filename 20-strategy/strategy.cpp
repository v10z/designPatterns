// Strategy Pattern - Scientific Algorithm Selection and Optimization
// Enables dynamic selection of numerical algorithms based on problem characteristics and performance requirements
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <chrono>
#include <random>
#include <complex>
#include <functional>

// Strategy interface for numerical integration methods
class IntegrationStrategy {
public:
    virtual ~IntegrationStrategy() = default;
    virtual double integrate(std::function<double(double)> f, double a, double b, int n = 1000) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getComplexity() const = 0;
};

// Concrete numerical integration strategies
class TrapezoidalRule : public IntegrationStrategy {
public:
    double integrate(std::function<double(double)> f, double a, double b, int n) override {
        double h = (b - a) / n;
        double sum = 0.5 * (f(a) + f(b));
        
        for (int i = 1; i < n; ++i) {
            sum += f(a + i * h);
        }
        
        return sum * h;
    }
    
    std::string getName() const override { return "Trapezoidal Rule"; }
    std::string getComplexity() const override { return "O(n), Error: O(h²)"; }
};

class SimpsonsRule : public IntegrationStrategy {
public:
    double integrate(std::function<double(double)> f, double a, double b, int n) override {
        if (n % 2 == 1) n++; // Ensure even number of intervals
        
        double h = (b - a) / n;
        double sum = f(a) + f(b);
        
        for (int i = 1; i < n; i += 2) {
            sum += 4 * f(a + i * h);
        }
        
        for (int i = 2; i < n; i += 2) {
            sum += 2 * f(a + i * h);
        }
        
        return sum * h / 3.0;
    }
    
    std::string getName() const override { return "Simpson's 1/3 Rule"; }
    std::string getComplexity() const override { return "O(n), Error: O(h⁴)"; }
};

class GaussianQuadrature : public IntegrationStrategy {
private:
    // Gauss-Legendre quadrature weights and points for n=5
    const std::vector<double> weights_ = {0.5688888889, 0.4786286705, 0.4786286705, 0.2369268851, 0.2369268851};
    const std::vector<double> points_ = {0.0, -0.5384693101, 0.5384693101, -0.9061798459, 0.9061798459};
    
public:
    double integrate(std::function<double(double)> f, double a, double b, int n) override {
        // Transform from [a,b] to [-1,1]
        double sum = 0.0;
        for (size_t i = 0; i < weights_.size(); ++i) {
            double x = 0.5 * ((b - a) * points_[i] + a + b);
            sum += weights_[i] * f(x);
        }
        return 0.5 * (b - a) * sum;
    }
    
    std::string getName() const override { return "Gaussian Quadrature"; }
    std::string getComplexity() const override { return "O(1), Error: exponential convergence"; }
};

class AdaptiveQuadrature : public IntegrationStrategy {
private:
    double adaptiveIntegral(std::function<double(double)> f, double a, double b, 
                           double tol, double fa, double fb, double fc) {
        double h = b - a;
        double c = (a + b) / 2.0;
        double fd = f((a + c) / 2.0);
        double fe = f((c + b) / 2.0);
        
        double S1 = h * (fa + 4*fc + fb) / 6.0;  // Simpson's rule for [a,b]
        double S2 = h * (fa + 4*fd + 2*fc + 4*fe + fb) / 12.0;  // Simpson's rule for [a,c] and [c,b]
        
        if (std::abs(S2 - S1) <= 15 * tol) {
            return S2 + (S2 - S1) / 15.0;
        } else {
            return adaptiveIntegral(f, a, c, tol/2, fa, fc, fd) + 
                   adaptiveIntegral(f, c, b, tol/2, fc, fb, fe);
        }
    }
    
public:
    double integrate(std::function<double(double)> f, double a, double b, int n) override {
        double tol = 1e-8;
        double fa = f(a);
        double fb = f(b);
        double fc = f((a + b) / 2.0);
        return adaptiveIntegral(f, a, b, tol, fa, fb, fc);
    }
    
    std::string getName() const override { return "Adaptive Quadrature"; }
    std::string getComplexity() const override { return "O(log(1/ε)), Error: adaptive tolerance"; }
};

// Context - Numerical Integrator
class NumericalIntegrator {
private:
    std::unique_ptr<IntegrationStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<IntegrationStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    double performIntegration(std::function<double(double)> f, double a, double b, int n = 1000) {
        if (strategy_) {
            std::cout << "Integrating using: " << strategy_->getName() 
                      << " (" << strategy_->getComplexity() << ")\n";
            
            auto start = std::chrono::high_resolution_clock::now();
            double result = strategy_->integrate(f, a, b, n);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "Result: " << std::fixed << std::setprecision(8) << result 
                      << " (computed in " << duration.count() << " μs)\n";
            
            return result;
        } else {
            std::cout << "No integration strategy set!\n";
            return 0.0;
        }
    }
};

// Optimization Strategy for Scientific Computing
class OptimizationStrategy {
public:
    virtual ~OptimizationStrategy() = default;
    virtual double optimize(std::function<double(double)> f, double x0, double tolerance = 1e-6, int maxIter = 1000) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getConvergenceRate() const = 0;
};

class GradientDescent : public OptimizationStrategy {
private:
    double learningRate_;
    
public:
    GradientDescent(double learningRate = 0.01) : learningRate_(learningRate) {}
    
    double optimize(std::function<double(double)> f, double x0, double tolerance, int maxIter) override {
        double x = x0;
        double h = 1e-8; // Step size for numerical differentiation
        
        for (int i = 0; i < maxIter; ++i) {
            // Numerical gradient
            double gradient = (f(x + h) - f(x - h)) / (2 * h);
            double x_new = x - learningRate_ * gradient;
            
            if (std::abs(x_new - x) < tolerance) {
                std::cout << "  Converged after " << i + 1 << " iterations\n";
                return x_new;
            }
            x = x_new;
        }
        
        std::cout << "  Reached maximum iterations (" << maxIter << ")\n";
        return x;
    }
    
    std::string getName() const override { return "Gradient Descent"; }
    std::string getConvergenceRate() const override { return "Linear convergence"; }
};

class NewtonRaphson : public OptimizationStrategy {
public:
    double optimize(std::function<double(double)> f, double x0, double tolerance, int maxIter) override {
        double x = x0;
        double h = 1e-8;
        
        for (int i = 0; i < maxIter; ++i) {
            // Numerical first and second derivatives
            double f_prime = (f(x + h) - f(x - h)) / (2 * h);
            double f_double_prime = (f(x + h) - 2*f(x) + f(x - h)) / (h * h);
            
            if (std::abs(f_double_prime) < 1e-12) {
                std::cout << "  Second derivative too small, switching to gradient descent\n";
                break;
            }
            
            double x_new = x - f_prime / f_double_prime;
            
            if (std::abs(x_new - x) < tolerance) {
                std::cout << "  Converged after " << i + 1 << " iterations\n";
                return x_new;
            }
            x = x_new;
        }
        
        std::cout << "  Newton-Raphson failed, reached maximum iterations\n";
        return x;
    }
    
    std::string getName() const override { return "Newton-Raphson"; }
    std::string getConvergenceRate() const override { return "Quadratic convergence"; }
};

class BrentMethod : public OptimizationStrategy {
public:
    double optimize(std::function<double(double)> f, double x0, double tolerance, int maxIter) override {
        // Brent's method for finding minimum (requires bracketing)
        double a = x0 - 1.0, b = x0 + 1.0;
        
        // Ensure we have a proper bracket
        while (f(a) < f(x0)) a -= 1.0;
        while (f(b) < f(x0)) b += 1.0;
        
        double c = (a + b) / 2.0;
        double d = 0, e = 0;
        double goldenRatio = (3.0 - std::sqrt(5.0)) / 2.0;
        
        for (int i = 0; i < maxIter; ++i) {
            double m = (a + b) / 2.0;
            double tol1 = tolerance * std::abs(c) + tolerance;
            double tol2 = 2.0 * tol1;
            
            if (std::abs(c - m) <= tol2 - 0.5 * (b - a)) {
                std::cout << "  Converged after " << i + 1 << " iterations\n";
                return c;
            }
            
            // Parabolic interpolation step
            double p = 0, q = 0, r = 0;
            if (std::abs(e) > tol1) {
                r = (c - b) * (f(c) - f(a));
                q = (c - a) * (f(c) - f(b));
                p = (c - a) * q - (c - b) * r;
                q = 2.0 * (q - r);
                if (q > 0) p = -p;
                q = std::abs(q);
                r = e;
                e = d;
            }
            
            // Golden section step
            if (std::abs(p) < std::abs(0.5 * q * r) && p > q * (a - c) && p < q * (b - c)) {
                d = p / q;
            } else {
                e = (c >= m) ? a - c : b - c;
                d = goldenRatio * e;
            }
            
            double u = c + ((std::abs(d) >= tol1) ? d : (d > 0 ? tol1 : -tol1));
            double fu = f(u);
            
            if (fu <= f(c)) {
                if (u >= c) a = c; else b = c;
                c = u;
            } else {
                if (u < c) a = u; else b = u;
            }
        }
        
        std::cout << "  Reached maximum iterations (" << maxIter << ")\n";
        return c;
    }
    
    std::string getName() const override { return "Brent's Method"; }
    std::string getConvergenceRate() const override { return "Superlinear convergence"; }
};

// Scientific Function Optimizer
class ScientificOptimizer {
private:
    std::unique_ptr<OptimizationStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<OptimizationStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    double findMinimum(std::function<double(double)> f, double x0, const std::string& functionName = "f(x)") {
        if (strategy_) {
            std::cout << "Optimizing " << functionName << " using: " << strategy_->getName() 
                      << " (" << strategy_->getConvergenceRate() << ")\n";
            
            auto start = std::chrono::high_resolution_clock::now();
            double result = strategy_->optimize(f, x0);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "  Minimum at x = " << std::fixed << std::setprecision(6) << result 
                      << ", f(x) = " << f(result) << "\n";
            std::cout << "  Optimization time: " << duration.count() << " μs\n";
            
            return result;
        } else {
            std::cout << "No optimization strategy set!\n";
            return x0;
        }
    }
};

// Linear Algebra Solver Strategy for Scientific Computing
class LinearSolverStrategy {
public:
    virtual ~LinearSolverStrategy() = default;
    virtual std::vector<double> solve(const std::vector<std::vector<double>>& A, const std::vector<double>& b) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getComplexity() const = 0;
    virtual bool isIterative() const = 0;
};

class GaussianElimination : public LinearSolverStrategy {
public:
    std::vector<double> solve(const std::vector<std::vector<double>>& A, const std::vector<double>& b) override {
        int n = A.size();
        std::vector<std::vector<double>> augmented(n, std::vector<double>(n + 1));
        
        // Create augmented matrix
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                augmented[i][j] = A[i][j];
            }
            augmented[i][n] = b[i];
        }
        
        // Forward elimination
        for (int i = 0; i < n; ++i) {
            // Find pivot
            int maxRow = i;
            for (int k = i + 1; k < n; ++k) {
                if (std::abs(augmented[k][i]) > std::abs(augmented[maxRow][i])) {
                    maxRow = k;
                }
            }
            std::swap(augmented[maxRow], augmented[i]);
            
            // Make all rows below this one 0 in current column
            for (int k = i + 1; k < n; ++k) {
                double factor = augmented[k][i] / augmented[i][i];
                for (int j = i; j <= n; ++j) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
        
        // Back substitution
        std::vector<double> x(n);
        for (int i = n - 1; i >= 0; --i) {
            x[i] = augmented[i][n];
            for (int j = i + 1; j < n; ++j) {
                x[i] -= augmented[i][j] * x[j];
            }
            x[i] /= augmented[i][i];
        }
        
        return x;
    }
    
    std::string getName() const override { return "Gaussian Elimination with Partial Pivoting"; }
    std::string getComplexity() const override { return "O(n³)"; }
    bool isIterative() const override { return false; }
};

class LUDecomposition : public LinearSolverStrategy {
public:
    std::vector<double> solve(const std::vector<std::vector<double>>& A, const std::vector<double>& b) override {
        int n = A.size();
        std::vector<std::vector<double>> L(n, std::vector<double>(n, 0.0));
        std::vector<std::vector<double>> U(n, std::vector<double>(n, 0.0));
        
        // LU decomposition
        for (int i = 0; i < n; ++i) {
            // Upper triangular
            for (int k = i; k < n; ++k) {
                double sum = 0;
                for (int j = 0; j < i; ++j) {
                    sum += L[i][j] * U[j][k];
                }
                U[i][k] = A[i][k] - sum;
            }
            
            // Lower triangular
            for (int k = i; k < n; ++k) {
                if (i == k) {
                    L[i][i] = 1;
                } else {
                    double sum = 0;
                    for (int j = 0; j < i; ++j) {
                        sum += L[k][j] * U[j][i];
                    }
                    L[k][i] = (A[k][i] - sum) / U[i][i];
                }
            }
        }
        
        // Forward substitution (Ly = b)
        std::vector<double> y(n);
        for (int i = 0; i < n; ++i) {
            y[i] = b[i];
            for (int j = 0; j < i; ++j) {
                y[i] -= L[i][j] * y[j];
            }
        }
        
        // Back substitution (Ux = y)
        std::vector<double> x(n);
        for (int i = n - 1; i >= 0; --i) {
            x[i] = y[i];
            for (int j = i + 1; j < n; ++j) {
                x[i] -= U[i][j] * x[j];
            }
            x[i] /= U[i][i];
        }
        
        return x;
    }
    
    std::string getName() const override { return "LU Decomposition"; }
    std::string getComplexity() const override { return "O(n³), O(n²) for multiple RHS"; }
    bool isIterative() const override { return false; }
};

class JacobiIterative : public LinearSolverStrategy {
public:
    std::vector<double> solve(const std::vector<std::vector<double>>& A, const std::vector<double>& b) override {
        int n = A.size();
        std::vector<double> x(n, 0.0);
        std::vector<double> x_new(n);
        double tolerance = 1e-10;
        int maxIter = 1000;
        
        for (int iter = 0; iter < maxIter; ++iter) {
            for (int i = 0; i < n; ++i) {
                double sum = 0.0;
                for (int j = 0; j < n; ++j) {
                    if (i != j) {
                        sum += A[i][j] * x[j];
                    }
                }
                x_new[i] = (b[i] - sum) / A[i][i];
            }
            
            // Check convergence
            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += (x_new[i] - x[i]) * (x_new[i] - x[i]);
            }
            norm = std::sqrt(norm);
            
            x = x_new;
            
            if (norm < tolerance) {
                std::cout << "  Converged after " << iter + 1 << " iterations\n";
                break;
            }
        }
        
        return x;
    }
    
    std::string getName() const override { return "Jacobi Iterative Method"; }
    std::string getComplexity() const override { return "O(n² × k), k = iterations"; }
    bool isIterative() const override { return true; }
};

class GaussSeidelIterative : public LinearSolverStrategy {
public:
    std::vector<double> solve(const std::vector<std::vector<double>>& A, const std::vector<double>& b) override {
        int n = A.size();
        std::vector<double> x(n, 0.0);
        double tolerance = 1e-10;
        int maxIter = 1000;
        
        for (int iter = 0; iter < maxIter; ++iter) {
            std::vector<double> x_old = x;
            
            for (int i = 0; i < n; ++i) {
                double sum = 0.0;
                for (int j = 0; j < n; ++j) {
                    if (i != j) {
                        sum += A[i][j] * x[j];
                    }
                }
                x[i] = (b[i] - sum) / A[i][i];
            }
            
            // Check convergence
            double norm = 0.0;
            for (int i = 0; i < n; ++i) {
                norm += (x[i] - x_old[i]) * (x[i] - x_old[i]);
            }
            norm = std::sqrt(norm);
            
            if (norm < tolerance) {
                std::cout << "  Converged after " << iter + 1 << " iterations\n";
                break;
            }
        }
        
        return x;
    }
    
    std::string getName() const override { return "Gauss-Seidel Iterative Method"; }
    std::string getComplexity() const override { return "O(n² × k), faster convergence than Jacobi"; }
    bool isIterative() const override { return true; }
};

// Scientific Linear System Solver
class LinearSystemSolver {
private:
    std::unique_ptr<LinearSolverStrategy> strategy_;
    
public:
    void setStrategy(std::unique_ptr<LinearSolverStrategy> strategy) {
        strategy_ = std::move(strategy);
    }
    
    std::vector<double> solveSystem(const std::vector<std::vector<double>>& A, 
                                   const std::vector<double>& b, 
                                   const std::string& systemName = "Ax = b") {
        if (strategy_) {
            std::cout << "Solving " << systemName << " using: " << strategy_->getName() 
                      << " (" << strategy_->getComplexity() << ")\n";
            
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<double> solution = strategy_->solve(A, b);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << "  Solution computed in " << duration.count() << " μs\n";
            
            // Verify solution quality
            double residualNorm = 0.0;
            for (size_t i = 0; i < A.size(); ++i) {
                double sum = 0.0;
                for (size_t j = 0; j < A[i].size(); ++j) {
                    sum += A[i][j] * solution[j];
                }
                double residual = sum - b[i];
                residualNorm += residual * residual;
            }
            residualNorm = std::sqrt(residualNorm);
            std::cout << "  Residual norm: " << std::scientific << residualNorm << "\n";
            
            return solution;
        } else {
            std::cout << "No linear solver strategy set!\n";
            return std::vector<double>();
        }
    }
};

// Helper function to print vector with scientific formatting
template<typename T>
void printVector(const std::vector<T>& vec, const std::string& label) {
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::fixed << std::setprecision(6) << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    std::cout << "=== Scientific Algorithm Selection and Optimization ===\n\n";
    
    // Numerical Integration Strategy Example
    std::cout << "=== Numerical Integration Strategies ===\n";
    
    NumericalIntegrator integrator;
    
    // Define test functions
    auto polynomialFunc = [](double x) { return x*x*x - 2*x*x + x - 1; }; // ∫(x³-2x²+x-1)dx from 0 to 2
    auto trigFunc = [](double x) { return std::sin(x) * std::cos(x); };     // ∫sin(x)cos(x)dx from 0 to π/2
    auto exponentialFunc = [](double x) { return std::exp(-x*x); };         // ∫e^(-x²)dx from 0 to 1 (Gaussian)
    
    double exactPolynomial = (16.0/4.0 - 16.0/3.0 + 4.0/2.0 - 2.0) - (0.0 - 0.0 + 0.0 - 0.0); // Exact = -2/3
    
    std::cout << "\n--- Integrating f(x) = x³ - 2x² + x - 1 from 0 to 2 ---\n";
    std::cout << "Analytical result: " << std::fixed << std::setprecision(8) << exactPolynomial << "\n\n";
    
    // Trapezoidal Rule
    integrator.setStrategy(std::make_unique<TrapezoidalRule>());
    double result1 = integrator.performIntegration(polynomialFunc, 0.0, 2.0, 1000);
    std::cout << "Error: " << std::abs(result1 - exactPolynomial) << "\n\n";
    
    // Simpson's Rule
    integrator.setStrategy(std::make_unique<SimpsonsRule>());
    double result2 = integrator.performIntegration(polynomialFunc, 0.0, 2.0, 1000);
    std::cout << "Error: " << std::abs(result2 - exactPolynomial) << "\n\n";
    
    // Gaussian Quadrature
    integrator.setStrategy(std::make_unique<GaussianQuadrature>());
    double result3 = integrator.performIntegration(polynomialFunc, 0.0, 2.0, 1000);
    std::cout << "Error: " << std::abs(result3 - exactPolynomial) << "\n\n";
    
    // Adaptive Quadrature
    integrator.setStrategy(std::make_unique<AdaptiveQuadrature>());
    double result4 = integrator.performIntegration(polynomialFunc, 0.0, 2.0, 1000);
    std::cout << "Error: " << std::abs(result4 - exactPolynomial) << "\n\n";
    
    std::cout << "--- Integrating f(x) = e^(-x²) from 0 to 1 (Gaussian integral) ---\n";
    std::cout << "Expected ≈ 0.746824 (erf(1)/2 × √π/2)\n\n";
    
    integrator.setStrategy(std::make_unique<GaussianQuadrature>());
    integrator.performIntegration(exponentialFunc, 0.0, 1.0, 1000);
    
    // Optimization Strategy Example
    std::cout << "\n\n=== Function Optimization Strategies ===\n";
    
    ScientificOptimizer optimizer;
    
    // Define test functions for optimization
    auto quadraticFunc = [](double x) { return (x - 2.5) * (x - 2.5) + 1.0; }; // Minimum at x = 2.5, f(x) = 1.0
    auto quarticFunc = [](double x) { return x*x*x*x - 4*x*x*x + 6*x*x - 4*x + 5; }; // Multiple local minima
    auto rosenbrockFunc = [](double x) { return 100*(x*x - x)*(x*x - x) + (1-x)*(1-x); }; // Rosenbrock-like 1D
    
    std::cout << "\n--- Optimizing f(x) = (x-2.5)² + 1 ---\n";
    std::cout << "Analytical minimum: x = 2.5, f(x) = 1.0\n\n";
    
    // Gradient Descent
    optimizer.setStrategy(std::make_unique<GradientDescent>(0.1));
    optimizer.findMinimum(quadraticFunc, 0.0, "Quadratic function");
    std::cout << "\n";
    
    // Newton-Raphson
    optimizer.setStrategy(std::make_unique<NewtonRaphson>());
    optimizer.findMinimum(quadraticFunc, 0.0, "Quadratic function");
    std::cout << "\n";
    
    // Brent's Method
    optimizer.setStrategy(std::make_unique<BrentMethod>());
    optimizer.findMinimum(quadraticFunc, 0.0, "Quadratic function");
    std::cout << "\n";
    
    std::cout << "--- Optimizing challenging function f(x) = x⁴ - 4x³ + 6x² - 4x + 5 ---\n";
    
    optimizer.setStrategy(std::make_unique<BrentMethod>());
    optimizer.findMinimum(quarticFunc, 1.5, "Quartic polynomial");
    
    // Linear Algebra Strategy Example
    std::cout << "\n\n=== Linear System Solver Strategies ===\n";
    
    LinearSystemSolver solver;
    
    // Create a test linear system: Hilbert matrix (ill-conditioned)
    int n = 4;
    std::vector<std::vector<double>> hilbertMatrix(n, std::vector<double>(n));
    std::vector<double> rhs(n);
    
    // Generate Hilbert matrix H[i][j] = 1/(i+j+1)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            hilbertMatrix[i][j] = 1.0 / (i + j + 1);
        }
        rhs[i] = 1.0; // Right-hand side
    }
    
    std::cout << "\n--- Solving 4×4 Hilbert system (ill-conditioned) ---\n";
    std::cout << "Matrix: H[i,j] = 1/(i+j+1), RHS = [1,1,1,1]ᵀ\n\n";
    
    // Gaussian Elimination
    solver.setStrategy(std::make_unique<GaussianElimination>());
    std::vector<double> solution1 = solver.solveSystem(hilbertMatrix, rhs, "Hilbert system");
    printVector(solution1, "Solution");
    std::cout << "\n";
    
    // LU Decomposition
    solver.setStrategy(std::make_unique<LUDecomposition>());
    std::vector<double> solution2 = solver.solveSystem(hilbertMatrix, rhs, "Hilbert system");
    printVector(solution2, "Solution");
    std::cout << "\n";
    
    // Create a diagonally dominant system for iterative methods
    std::vector<std::vector<double>> diagMatrix = {
        {10.0, 1.0, 2.0},
        {1.0, 8.0, -1.0},
        {2.0, -1.0, 12.0}
    };
    std::vector<double> diagRHS = {13.0, 8.0, 13.0}; // Solution should be [1, 1, 1]
    
    std::cout << "--- Solving 3×3 diagonally dominant system ---\n";
    std::cout << "Expected solution: [1, 1, 1]ᵀ\n\n";
    
    // Jacobi Method
    solver.setStrategy(std::make_unique<JacobiIterative>());
    std::vector<double> solution3 = solver.solveSystem(diagMatrix, diagRHS, "Diagonally dominant system");
    printVector(solution3, "Solution");
    std::cout << "\n";
    
    // Gauss-Seidel Method
    solver.setStrategy(std::make_unique<GaussSeidelIterative>());
    std::vector<double> solution4 = solver.solveSystem(diagMatrix, diagRHS, "Diagonally dominant system");
    printVector(solution4, "Solution");
    
    std::cout << "\n=== Strategy Pattern Summary ===\n";
    std::cout << "The Strategy pattern enables dynamic selection of scientific algorithms:\n";
    std::cout << "• Integration methods: Trading accuracy vs. computational cost\n";
    std::cout << "• Optimization algorithms: Different convergence properties\n";
    std::cout << "• Linear solvers: Direct vs. iterative methods based on matrix properties\n";
    std::cout << "\nThis pattern is essential for adaptive scientific computing where\n";
    std::cout << "algorithm choice depends on problem characteristics and requirements.\n";
    
    return 0;
}