// State Pattern - Scientific Computational State Management
// Manages complex state transitions in scientific simulations and computational algorithms
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>

// Forward declarations
class IterativeSolver;

// Computational state interface for iterative solvers
class SolverState {
public:
    virtual ~SolverState() = default;
    
    virtual void initialize(IterativeSolver* solver) = 0;
    virtual void iterate(IterativeSolver* solver) = 0;
    virtual void checkConvergence(IterativeSolver* solver) = 0;
    virtual void terminate(IterativeSolver* solver) = 0;
    
    virtual std::string getStateName() const = 0;
    virtual double getConvergenceRate() const = 0;
};

// Context - Iterative Solver for Linear Systems and Nonlinear Equations
class IterativeSolver {
private:
    std::unique_ptr<SolverState> currentState_;
    std::vector<double> solution_;
    std::vector<double> residual_;
    double tolerance_;
    int maxIterations_;
    int currentIteration_;
    double currentResidual_;
    std::string problemType_;
    std::mt19937 rng_;
    
public:
    IterativeSolver(std::unique_ptr<SolverState> initialState, int systemSize, 
                   double tol = 1e-8, int maxIter = 1000, const std::string& type = "Linear System") 
        : currentState_(std::move(initialState)), solution_(systemSize, 0.0), 
          residual_(systemSize, 0.0), tolerance_(tol), maxIterations_(maxIter),
          currentIteration_(0), currentResidual_(1.0), problemType_(type), rng_(42) {
        std::cout << "[SOLVER] Initialized " << problemType_ << " solver\n";
        std::cout << "  System size: " << systemSize << "\n";
        std::cout << "  Tolerance: " << std::scientific << tolerance_ << "\n";
        std::cout << "  Max iterations: " << maxIterations_ << "\n\n";
    }
    
    void setState(std::unique_ptr<SolverState> state) {
        std::cout << "[STATE] " << currentState_->getStateName() 
                  << " -> " << state->getStateName() << "\n";
        currentState_ = std::move(state);
    }
    
    void initializeSolver() {
        currentState_->initialize(this);
    }
    
    void performIteration() {
        currentState_->iterate(this);
    }
    
    void checkConvergenceStatus() {
        currentState_->checkConvergence(this);
    }
    
    void terminateSolver() {
        currentState_->terminate(this);
    }
    
    // Solver operations
    void resetSolution() {
        std::fill(solution_.begin(), solution_.end(), 0.0);
        currentIteration_ = 0;
        currentResidual_ = 1.0;
        std::cout << "[SOLVER] Solution vector reset\n";
    }
    
    void updateSolution() {
        // Simulate iterative update (e.g., Jacobi, Gauss-Seidel, Newton-Raphson)
        std::uniform_real_distribution<double> updateDist(-0.1, 0.1);
        for (auto& x : solution_) {
            x += updateDist(rng_);
        }
        currentIteration_++;
        
        // Simulate residual calculation
        calculateResidual();
    }
    
    void calculateResidual() {
        // Simplified residual calculation
        std::uniform_real_distribution<double> residDist(0.0, 1.0);
        double norm = 0.0;
        for (auto& r : residual_) {
            r = residDist(rng_) * std::pow(0.85, currentIteration_); // Simulate convergence
            norm += r * r;
        }
        currentResidual_ = std::sqrt(norm / residual_.size());
    }
    
    void applyRelaxation(double omega = 0.8) {
        std::cout << "[SOLVER] Applying relaxation factor: " << omega << "\n";
        for (auto& x : solution_) {
            x *= omega;
        }
    }
    
    void displayStatus() const {
        std::cout << "[STATUS] Iteration " << currentIteration_ 
                  << ", Residual: " << std::scientific << std::setprecision(3)
                  << currentResidual_ << ", State: " << currentState_->getStateName() << "\n";
    }
    
    // Getters for state transitions
    bool hasConverged() const { return currentResidual_ < tolerance_; }
    bool hasReachedMaxIterations() const { return currentIteration_ >= maxIterations_; }
    bool isStagnant() const { return currentState_->getConvergenceRate() < 1e-6; }
    bool needsRelaxation() const { return currentResidual_ > 1.0; }
    
    double getCurrentResidual() const { return currentResidual_; }
    int getCurrentIteration() const { return currentIteration_; }
    double getTolerance() const { return tolerance_; }
    int getMaxIterations() const { return maxIterations_; }
    std::string getProblemType() const { return problemType_; }
    const std::vector<double>& getSolution() const { return solution_; }
    SolverState* getState() const { return currentState_.get(); }
};

// Concrete Solver States
class InitializationState : public SolverState {
public:
    void initialize(IterativeSolver* solver) override;
    void iterate(IterativeSolver* solver) override;
    void checkConvergence(IterativeSolver* solver) override;
    void terminate(IterativeSolver* solver) override;
    std::string getStateName() const override { return "INITIALIZING"; }
    double getConvergenceRate() const override { return 0.0; }
};

class IteratingState : public SolverState {
public:
    void initialize(IterativeSolver* solver) override;
    void iterate(IterativeSolver* solver) override;
    void checkConvergence(IterativeSolver* solver) override;
    void terminate(IterativeSolver* solver) override;
    std::string getStateName() const override { return "ITERATING"; }
    double getConvergenceRate() const override { return 0.15; } // Standard convergence
};

class StagnantState : public SolverState {
public:
    void initialize(IterativeSolver* solver) override;
    void iterate(IterativeSolver* solver) override;
    void checkConvergence(IterativeSolver* solver) override;
    void terminate(IterativeSolver* solver) override;
    std::string getStateName() const override { return "STAGNANT"; }
    double getConvergenceRate() const override { return 1e-8; } // Very slow progress
};

class RelaxationState : public SolverState {
public:
    void initialize(IterativeSolver* solver) override;
    void iterate(IterativeSolver* solver) override;
    void checkConvergence(IterativeSolver* solver) override;
    void terminate(IterativeSolver* solver) override;
    std::string getStateName() const override { return "RELAXATION"; }
    double getConvergenceRate() const override { return 0.25; } // Enhanced convergence
};

class ConvergedState : public SolverState {
public:
    void initialize(IterativeSolver* solver) override;
    void iterate(IterativeSolver* solver) override;
    void checkConvergence(IterativeSolver* solver) override;
    void terminate(IterativeSolver* solver) override;
    std::string getStateName() const override { return "CONVERGED"; }
    double getConvergenceRate() const override { return 1.0; } // Complete
};

class FailedState : public SolverState {
public:
    void initialize(IterativeSolver* solver) override;
    void iterate(IterativeSolver* solver) override;
    void checkConvergence(IterativeSolver* solver) override;
    void terminate(IterativeSolver* solver) override;
    std::string getStateName() const override { return "FAILED"; }
    double getConvergenceRate() const override { return 0.0; } // No progress
};

// Solver State Implementations
void InitializationState::initialize(IterativeSolver* solver) {
    std::cout << "[INIT] Setting up initial solution vector and parameters\n";
    solver->resetSolution();
    solver->setState(std::make_unique<IteratingState>());
}

void InitializationState::iterate(IterativeSolver* solver) {
    std::cout << "[INIT] Cannot iterate before initialization\n";
}

void InitializationState::checkConvergence(IterativeSolver* solver) {
    std::cout << "[INIT] Convergence check skipped during initialization\n";
}

void InitializationState::terminate(IterativeSolver* solver) {
    std::cout << "[INIT] Terminating during initialization\n";
    solver->setState(std::make_unique<FailedState>());
}

void IteratingState::initialize(IterativeSolver* solver) {
    std::cout << "[ITER] Already initialized and iterating\n";
}

void IteratingState::iterate(IterativeSolver* solver) {
    solver->updateSolution();
    solver->displayStatus();
    
    // Check for divergence or stagnation
    if (solver->needsRelaxation()) {
        std::cout << "[ITER] Residual too large, switching to relaxation\n";
        solver->setState(std::make_unique<RelaxationState>());
    } else if (solver->isStagnant()) {
        std::cout << "[ITER] Convergence stagnation detected\n";
        solver->setState(std::make_unique<StagnantState>());
    }
    
    solver->getState()->checkConvergence(solver);
}

void IteratingState::checkConvergence(IterativeSolver* solver) {
    if (solver->hasConverged()) {
        std::cout << "[ITER] ✓ Convergence achieved!\n";
        solver->setState(std::make_unique<ConvergedState>());
    } else if (solver->hasReachedMaxIterations()) {
        std::cout << "[ITER] ✗ Maximum iterations reached without convergence\n";
        solver->setState(std::make_unique<FailedState>());
    }
}

void IteratingState::terminate(IterativeSolver* solver) {
    std::cout << "[ITER] Terminating iterative process\n";
    solver->setState(std::make_unique<FailedState>());
}

void StagnantState::initialize(IterativeSolver* solver) {
    std::cout << "[STAG] Cannot reinitialize from stagnant state\n";
}

void StagnantState::iterate(IterativeSolver* solver) {
    std::cout << "[STAG] Attempting to break stagnation with modified algorithm\n";
    solver->applyRelaxation(0.5); // Aggressive relaxation
    solver->updateSolution();
    solver->displayStatus();
    
    // Give stagnant solver a few chances before failing
    static int stagnantAttempts = 0;
    stagnantAttempts++;
    
    if (stagnantAttempts > 3) {
        std::cout << "[STAG] Unable to break stagnation, switching to relaxation\n";
        solver->setState(std::make_unique<RelaxationState>());
        stagnantAttempts = 0;
    }
    
    solver->getState()->checkConvergence(solver);
}

void StagnantState::checkConvergence(IterativeSolver* solver) {
    if (solver->hasConverged()) {
        std::cout << "[STAG] ✓ Convergence achieved after stagnation recovery!\n";
        solver->setState(std::make_unique<ConvergedState>());
    } else if (solver->hasReachedMaxIterations()) {
        std::cout << "[STAG] ✗ Failed to recover from stagnation\n";
        solver->setState(std::make_unique<FailedState>());
    }
}

void StagnantState::terminate(IterativeSolver* solver) {
    std::cout << "[STAG] Terminating stagnant solver\n";
    solver->setState(std::make_unique<FailedState>());
}

void RelaxationState::initialize(IterativeSolver* solver) {
    std::cout << "[RELAX] Cannot reinitialize during relaxation\n";
}

void RelaxationState::iterate(IterativeSolver* solver) {
    std::cout << "[RELAX] Applying under-relaxation for stability\n";
    solver->applyRelaxation(0.8);
    solver->updateSolution();
    solver->displayStatus();
    
    // Check if relaxation helped
    if (solver->getCurrentResidual() < 0.5) {
        std::cout << "[RELAX] Relaxation successful, returning to normal iteration\n";
        solver->setState(std::make_unique<IteratingState>());
    }
    
    solver->getState()->checkConvergence(solver);
}

void RelaxationState::checkConvergence(IterativeSolver* solver) {
    if (solver->hasConverged()) {
        std::cout << "[RELAX] ✓ Convergence achieved with relaxation!\n";
        solver->setState(std::make_unique<ConvergedState>());
    } else if (solver->hasReachedMaxIterations()) {
        std::cout << "[RELAX] ✗ Relaxation could not achieve convergence\n";
        solver->setState(std::make_unique<FailedState>());
    }
}

void RelaxationState::terminate(IterativeSolver* solver) {
    std::cout << "[RELAX] Terminating relaxation process\n";
    solver->setState(std::make_unique<FailedState>());
}

void ConvergedState::initialize(IterativeSolver* solver) {
    std::cout << "[CONVERGED] Solution already converged\n";
}

void ConvergedState::iterate(IterativeSolver* solver) {
    std::cout << "[CONVERGED] No further iterations needed\n";
}

void ConvergedState::checkConvergence(IterativeSolver* solver) {
    std::cout << "[CONVERGED] Solution verified as converged\n";
}

void ConvergedState::terminate(IterativeSolver* solver) {
    std::cout << "[CONVERGED] Solver terminated successfully\n";
}

void FailedState::initialize(IterativeSolver* solver) {
    std::cout << "[FAILED] Cannot reinitialize failed solver\n";
}

void FailedState::iterate(IterativeSolver* solver) {
    std::cout << "[FAILED] Cannot iterate failed solver\n";
}

void FailedState::checkConvergence(IterativeSolver* solver) {
    std::cout << "[FAILED] Solver failed to converge\n";
}

void FailedState::terminate(IterativeSolver* solver) {
    std::cout << "[FAILED] Solver terminated due to failure\n";
}

// Scientific Monte Carlo Simulation State Management
class MonteCarloSimulation;

class SimulationState {
public:
    virtual ~SimulationState() = default;
    
    virtual void initialize(MonteCarloSimulation* sim) = 0;
    virtual void sample(MonteCarloSimulation* sim) = 0;
    virtual void estimateError(MonteCarloSimulation* sim) = 0;
    virtual void finalize(MonteCarloSimulation* sim) = 0;
    
    virtual std::string getStateName() const = 0;
};

class MonteCarloSimulation {
private:
    std::unique_ptr<SimulationState> state_;
    std::vector<double> samples_;
    double currentEstimate_;
    double standardError_;
    size_t targetSamples_;
    size_t currentSamples_;
    std::mt19937 rng_;
    std::string integralType_;
    
public:
    MonteCarloSimulation(std::unique_ptr<SimulationState> initialState, 
                        size_t targetSamples, const std::string& type = "Multi-dimensional Integral")
        : state_(std::move(initialState)), currentEstimate_(0.0), standardError_(1.0),
          targetSamples_(targetSamples), currentSamples_(0), rng_(123), integralType_(type) {
        samples_.reserve(targetSamples);
    }
    
    void setState(std::unique_ptr<SimulationState> state) {
        std::cout << "[MC] " << state_->getStateName() << " (samples: " 
                  << currentSamples_ << "/" << targetSamples_ << ")\n";
        state_ = std::move(state);
    }
    
    void initializeSimulation() { state_->initialize(this); }
    void generateSample() { state_->sample(this); }
    void calculateError() { state_->estimateError(this); }
    void finalizeSampling() { state_->finalize(this); }
    
    void addSample(double value) {
        samples_.push_back(value);
        currentSamples_++;
        
        // Update running estimate
        double sum = 0.0;
        for (const auto& sample : samples_) {
            sum += sample;
        }
        currentEstimate_ = sum / currentSamples_;
    }
    
    void calculateStandardError() {
        if (currentSamples_ < 2) return;
        
        double variance = 0.0;
        for (const auto& sample : samples_) {
            variance += (sample - currentEstimate_) * (sample - currentEstimate_);
        }
        variance /= (currentSamples_ - 1);
        standardError_ = std::sqrt(variance / currentSamples_);
    }
    
    void displayProgress() const {
        std::cout << "[MC] Progress: " << currentSamples_ << "/" << targetSamples_ 
                  << " samples, Estimate: " << std::fixed << std::setprecision(6)
                  << currentEstimate_ << " ± " << standardError_ << "\n";
    }
    
    // Getters for state logic
    bool hasCompletedSampling() const { return currentSamples_ >= targetSamples_; }
    bool hasAccurateEstimate() const { return standardError_ < 0.001; }
    bool needsMoreSamples() const { return standardError_ > 0.01 && currentSamples_ < targetSamples_ * 2; }
    
    size_t getCurrentSamples() const { return currentSamples_; }
    size_t getTargetSamples() const { return targetSamples_; }
    double getCurrentEstimate() const { return currentEstimate_; }
    double getStandardError() const { return standardError_; }
    std::mt19937& getRNG() { return rng_; }
    const std::string& getIntegralType() const { return integralType_; }
    SimulationState* getState() const { return state_.get(); }
};

// Monte Carlo States
class MCInitializationState : public SimulationState {
public:
    void initialize(MonteCarloSimulation* sim) override;
    void sample(MonteCarloSimulation* sim) override;
    void estimateError(MonteCarloSimulation* sim) override;
    void finalize(MonteCarloSimulation* sim) override;
    std::string getStateName() const override { return "MC_INITIALIZING"; }
};

class MCSamplingState : public SimulationState {
public:
    void initialize(MonteCarloSimulation* sim) override;
    void sample(MonteCarloSimulation* sim) override;
    void estimateError(MonteCarloSimulation* sim) override;
    void finalize(MonteCarloSimulation* sim) override;
    std::string getStateName() const override { return "MC_SAMPLING"; }
};

class MCRefinementState : public SimulationState {
public:
    void initialize(MonteCarloSimulation* sim) override;
    void sample(MonteCarloSimulation* sim) override;
    void estimateError(MonteCarloSimulation* sim) override;
    void finalize(MonteCarloSimulation* sim) override;
    std::string getStateName() const override { return "MC_REFINEMENT"; }
};

class MCCompletedState : public SimulationState {
public:
    void initialize(MonteCarloSimulation* sim) override;
    void sample(MonteCarloSimulation* sim) override;
    void estimateError(MonteCarloSimulation* sim) override;
    void finalize(MonteCarloSimulation* sim) override;
    std::string getStateName() const override { return "MC_COMPLETED"; }
};

// Monte Carlo State Implementations
void MCInitializationState::initialize(MonteCarloSimulation* sim) {
    std::cout << "[MC-INIT] Setting up random number generator and sampling parameters\n";
    std::cout << "[MC-INIT] Target integral: " << sim->getIntegralType() << "\n";
    sim->setState(std::make_unique<MCSamplingState>());
}

void MCInitializationState::sample(MonteCarloSimulation* sim) {
    std::cout << "[MC-INIT] Cannot sample before initialization\n";
}

void MCInitializationState::estimateError(MonteCarloSimulation* sim) {
    std::cout << "[MC-INIT] No samples available for error estimation\n";
}

void MCInitializationState::finalize(MonteCarloSimulation* sim) {
    std::cout << "[MC-INIT] Finalizing initialization\n";
}

void MCSamplingState::initialize(MonteCarloSimulation* sim) {
    std::cout << "[MC-SAMPLE] Already initialized, continuing sampling\n";
}

void MCSamplingState::sample(MonteCarloSimulation* sim) {
    // Generate random sample using Monte Carlo method
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    // Simulate evaluating a multi-dimensional integral (e.g., π estimation)
    double x = dist(sim->getRNG());
    double y = dist(sim->getRNG());
    double sample = (x*x + y*y <= 1.0) ? 4.0 : 0.0; // π estimation
    
    sim->addSample(sample);
    
    if (sim->getCurrentSamples() % 500 == 0) {
        sim->displayProgress();
    }
    
    sim->getState()->estimateError(sim);
}

void MCSamplingState::estimateError(MonteCarloSimulation* sim) {
    sim->calculateStandardError();
    
    if (sim->hasCompletedSampling()) {
        std::cout << "[MC-SAMPLE] Target samples reached\n";
        if (sim->hasAccurateEstimate()) {
            sim->setState(std::make_unique<MCCompletedState>());
        } else if (sim->needsMoreSamples()) {
            std::cout << "[MC-SAMPLE] Accuracy insufficient, switching to refinement\n";
            sim->setState(std::make_unique<MCRefinementState>());
        } else {
            sim->setState(std::make_unique<MCCompletedState>());
        }
    }
}

void MCSamplingState::finalize(MonteCarloSimulation* sim) {
    std::cout << "[MC-SAMPLE] Finalizing sampling phase\n";
    sim->setState(std::make_unique<MCCompletedState>());
}

void MCRefinementState::initialize(MonteCarloSimulation* sim) {
    std::cout << "[MC-REFINE] Cannot reinitialize during refinement\n";
}

void MCRefinementState::sample(MonteCarloSimulation* sim) {
    std::cout << "[MC-REFINE] Adaptive sampling for improved accuracy\n";
    
    // Enhanced sampling with importance sampling or variance reduction
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::normal_distribution<double> importanceDist(0.5, 0.2); // Importance sampling near center
    
    double x = std::max(0.0, std::min(1.0, importanceDist(sim->getRNG())));
    double y = std::max(0.0, std::min(1.0, importanceDist(sim->getRNG())));
    double sample = (x*x + y*y <= 1.0) ? 4.0 : 0.0;
    
    sim->addSample(sample);
    
    if (sim->getCurrentSamples() % 200 == 0) {
        sim->displayProgress();
    }
    
    sim->getState()->estimateError(sim);
}

void MCRefinementState::estimateError(MonteCarloSimulation* sim) {
    sim->calculateStandardError();
    
    if (sim->hasAccurateEstimate()) {
        std::cout << "[MC-REFINE] ✓ Required accuracy achieved through refinement!\n";
        sim->setState(std::make_unique<MCCompletedState>());
    } else if (sim->getCurrentSamples() >= sim->getTargetSamples() * 3) {
        std::cout << "[MC-REFINE] ✗ Maximum refinement samples reached\n";
        sim->setState(std::make_unique<MCCompletedState>());
    }
}

void MCRefinementState::finalize(MonteCarloSimulation* sim) {
    std::cout << "[MC-REFINE] Finalizing refinement phase\n";
    sim->setState(std::make_unique<MCCompletedState>());
}

void MCCompletedState::initialize(MonteCarloSimulation* sim) {
    std::cout << "[MC-COMPLETE] Simulation already completed\n";
}

void MCCompletedState::sample(MonteCarloSimulation* sim) {
    std::cout << "[MC-COMPLETE] No additional sampling needed\n";
}

void MCCompletedState::estimateError(MonteCarloSimulation* sim) {
    std::cout << "[MC-COMPLETE] Final estimate: " << std::fixed << std::setprecision(6)
              << sim->getCurrentEstimate() << " ± " << sim->getStandardError() << "\n";
}

void MCCompletedState::finalize(MonteCarloSimulation* sim) {
    std::cout << "[MC-COMPLETE] Monte Carlo simulation finalized successfully\n";
}

// Quantum Chemistry Computation State Management
class QuantumChemistryCalculation;

class ComputationState {
public:
    virtual ~ComputationState() = default;
    
    virtual void initializeBasis(QuantumChemistryCalculation* calc) = 0;
    virtual void performSCF(QuantumChemistryCalculation* calc) = 0;
    virtual void calculateCorrelation(QuantumChemistryCalculation* calc) = 0;
    virtual void finalizeResults(QuantumChemistryCalculation* calc) = 0;
    
    virtual std::string getStateName() const = 0;
};

class QuantumChemistryCalculation {
private:
    std::unique_ptr<ComputationState> state_;
    std::string moleculeName_;
    std::string basisSet_;
    double hfEnergy_;
    double correlationEnergy_;
    double totalEnergy_;
    int scfCycles_;
    bool converged_;
    
public:
    QuantumChemistryCalculation(const std::string& molecule, const std::string& basis = "6-31G*") 
        : moleculeName_(molecule), basisSet_(basis), hfEnergy_(0.0), 
          correlationEnergy_(0.0), totalEnergy_(0.0), scfCycles_(0), converged_(false) {}
    
    void setState(std::unique_ptr<ComputationState> state) {
        std::cout << "[QC] " << moleculeName_ << " (" << basisSet_ << ") - State: " 
                  << state->getStateName() << "\n";
        state_ = std::move(state);
    }
    
    void setupBasisSet() { state_->initializeBasis(this); }
    void runSCF() { state_->performSCF(this); }
    void addCorrelation() { state_->calculateCorrelation(this); }
    void finalize() { state_->finalizeResults(this); }
    
    void setBasisSet(const std::string& basis) { basisSet_ = basis; }
    void setHFEnergy(double energy) { hfEnergy_ = energy; }
    void setCorrelationEnergy(double energy) { correlationEnergy_ = energy; }
    void updateTotalEnergy() { totalEnergy_ = hfEnergy_ + correlationEnergy_; }
    void incrementSCFCycles() { scfCycles_++; }
    void setConverged(bool conv) { converged_ = conv; }
    
    std::string getMoleculeName() const { return moleculeName_; }
    std::string getBasisSet() const { return basisSet_; }
    double getHFEnergy() const { return hfEnergy_; }
    double getCorrelationEnergy() const { return correlationEnergy_; }
    double getTotalEnergy() const { return totalEnergy_; }
    int getSCFCycles() const { return scfCycles_; }
    bool isConverged() const { return converged_; }
    
    void displayResults() const {
        std::cout << "[QC-RESULTS] " << moleculeName_ << " (" << basisSet_ << "):\n";
        std::cout << "  HF Energy: " << std::fixed << std::setprecision(8) << hfEnergy_ << " Hartree\n";
        std::cout << "  Correlation: " << correlationEnergy_ << " Hartree\n";
        std::cout << "  Total Energy: " << totalEnergy_ << " Hartree\n";
        std::cout << "  SCF Cycles: " << scfCycles_ << "\n";
    }
};

class BasisInitializationState : public ComputationState {
public:
    void initializeBasis(QuantumChemistryCalculation* calc) override;
    void performSCF(QuantumChemistryCalculation* calc) override;
    void calculateCorrelation(QuantumChemistryCalculation* calc) override;
    void finalizeResults(QuantumChemistryCalculation* calc) override;
    std::string getStateName() const override { return "BASIS_SETUP"; }
};

class SCFState : public ComputationState {
public:
    void initializeBasis(QuantumChemistryCalculation* calc) override;
    void performSCF(QuantumChemistryCalculation* calc) override;
    void calculateCorrelation(QuantumChemistryCalculation* calc) override;
    void finalizeResults(QuantumChemistryCalculation* calc) override;
    std::string getStateName() const override { return "SCF_ITERATION"; }
};

class CorrelationState : public ComputationState {
public:
    void initializeBasis(QuantumChemistryCalculation* calc) override;
    void performSCF(QuantumChemistryCalculation* calc) override;
    void calculateCorrelation(QuantumChemistryCalculation* calc) override;
    void finalizeResults(QuantumChemistryCalculation* calc) override;
    std::string getStateName() const override { return "CORRELATION"; }
};

class CompletedState : public ComputationState {
public:
    void initializeBasis(QuantumChemistryCalculation* calc) override;
    void performSCF(QuantumChemistryCalculation* calc) override;
    void calculateCorrelation(QuantumChemistryCalculation* calc) override;
    void finalizeResults(QuantumChemistryCalculation* calc) override;
    std::string getStateName() const override { return "COMPLETED"; }
};

// Quantum Chemistry State Implementations
void BasisInitializationState::initializeBasis(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-BASIS] Setting up " << calc->getBasisSet() 
              << " basis set for " << calc->getMoleculeName() << "\n";
    std::cout << "[QC-BASIS] Generating atomic orbitals and overlap integrals\n";
    calc->setState(std::make_unique<SCFState>());
}

void BasisInitializationState::performSCF(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-BASIS] Cannot perform SCF before basis set initialization\n";
}

void BasisInitializationState::calculateCorrelation(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-BASIS] Cannot calculate correlation before SCF convergence\n";
}

void BasisInitializationState::finalizeResults(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-BASIS] Cannot finalize without completing calculation\n";
}

void SCFState::initializeBasis(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-SCF] Basis set already initialized\n";
}

void SCFState::performSCF(QuantumChemistryCalculation* calc) {
    calc->incrementSCFCycles();
    std::cout << "[QC-SCF] SCF iteration " << calc->getSCFCycles() 
              << " - solving Hartree-Fock equations\n";
    
    // Simulate SCF convergence
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> energyDist(-150.0, -50.0);
    
    double energy = energyDist(gen) * std::pow(0.99, calc->getSCFCycles());
    calc->setHFEnergy(energy);
    
    if (calc->getSCFCycles() >= 8) {
        std::cout << "[QC-SCF] ✓ SCF converged after " << calc->getSCFCycles() << " cycles\n";
        calc->setConverged(true);
        calc->setState(std::make_unique<CorrelationState>());
    } else if (calc->getSCFCycles() > 50) {
        std::cout << "[QC-SCF] ✗ SCF failed to converge\n";
        calc->setState(std::make_unique<CompletedState>());
    }
}

void SCFState::calculateCorrelation(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-SCF] SCF must converge before correlation calculation\n";
}

void SCFState::finalizeResults(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-SCF] SCF incomplete, finalizing current results\n";
    calc->setState(std::make_unique<CompletedState>());
}

void CorrelationState::initializeBasis(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-CORR] Basis already initialized\n";
}

void CorrelationState::performSCF(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-CORR] SCF already converged\n";
}

void CorrelationState::calculateCorrelation(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-CORR] Calculating electron correlation using MP2/CCSD methods\n";
    std::cout << "[QC-CORR] Computing 2-electron integrals and excitation amplitudes\n";
    
    // Simulate correlation energy calculation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> corrDist(-5.0, -0.1);
    
    double corrEnergy = corrDist(gen);
    calc->setCorrelationEnergy(corrEnergy);
    calc->updateTotalEnergy();
    
    std::cout << "[QC-CORR] ✓ Correlation energy: " << std::fixed << std::setprecision(6)
              << corrEnergy << " Hartree\n";
    calc->setState(std::make_unique<CompletedState>());
}

void CorrelationState::finalizeResults(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-CORR] Finalizing correlation calculation\n";
    calc->setState(std::make_unique<CompletedState>());
}

void CompletedState::initializeBasis(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-COMPLETE] Calculation already completed\n";
}

void CompletedState::performSCF(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-COMPLETE] SCF already completed\n";
}

void CompletedState::calculateCorrelation(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-COMPLETE] Correlation already calculated\n";
}

void CompletedState::finalizeResults(QuantumChemistryCalculation* calc) {
    std::cout << "[QC-COMPLETE] Finalizing quantum chemistry calculation\n";
    calc->displayResults();
    std::cout << "[QC-COMPLETE] Calculation completed successfully!\n";
}

int main() {
    std::cout << "=== Scientific Computational State Management ===\n\n";
    
    // Iterative Solver Example - Conjugate Gradient for Linear Systems
    std::cout << "=== Iterative Linear System Solver ===\n";
    
    IterativeSolver cgSolver(std::make_unique<InitializationState>(), 1000, 1e-8, 200, "Conjugate Gradient");
    
    cgSolver.initializeSolver();
    cgSolver.displayStatus();
    
    // Run solver iterations
    std::cout << "\n--- Running Solver Iterations ---\n";
    for (int i = 0; i < 25; ++i) {
        cgSolver.performIteration();
        if (cgSolver.hasConverged() || cgSolver.hasReachedMaxIterations()) {
            break;
        }
        if (i % 5 == 4) std::cout << "\n";
    }
    
    cgSolver.terminateSolver();
    
    // Nonlinear Solver Example - Newton-Raphson
    std::cout << "\n\n=== Newton-Raphson Nonlinear Solver ===\n";
    
    IterativeSolver newtonSolver(std::make_unique<InitializationState>(), 500, 1e-10, 100, "Newton-Raphson");
    
    newtonSolver.initializeSolver();
    
    // Run iterations with potential stagnation
    std::cout << "\n--- Solving Nonlinear Equations ---\n";
    for (int i = 0; i < 15; ++i) {
        newtonSolver.performIteration();
        if (newtonSolver.hasConverged() || newtonSolver.hasReachedMaxIterations()) {
            break;
        }
    }
    
    // Monte Carlo Integration Example
    std::cout << "\n\n=== Monte Carlo Integration - π Estimation ===\n";
    
    MonteCarloSimulation mcPi(std::make_unique<MCInitializationState>(), 2000, "π estimation (quarter circle)");
    
    mcPi.initializeSimulation();
    
    // Run Monte Carlo sampling
    std::cout << "\n--- Monte Carlo Sampling ---\n";
    for (size_t i = 0; i < 2500; ++i) {
        mcPi.generateSample();
        if (mcPi.hasCompletedSampling() && mcPi.hasAccurateEstimate()) {
            break;
        }
    }
    
    mcPi.calculateError();
    mcPi.finalizeSampling();
    
    // High-dimensional integral
    std::cout << "\n\n=== Monte Carlo High-Dimensional Integration ===\n";
    
    MonteCarloSimulation mcMultiD(std::make_unique<MCInitializationState>(), 1000, "6D Gaussian integral");
    
    mcMultiD.initializeSimulation();
    
    // Simulate challenging high-dimensional case
    for (size_t i = 0; i < 1500; ++i) {
        mcMultiD.generateSample();
        if (mcMultiD.hasCompletedSampling()) {
            if (!mcMultiD.hasAccurateEstimate() && mcMultiD.needsMoreSamples()) {
                continue; // Will switch to refinement automatically
            }
            break;
        }
    }
    
    mcMultiD.finalizeSampling();
    
    // Quantum Chemistry Calculation Example
    std::cout << "\n\n=== Quantum Chemistry Calculation ===\n";
    
    QuantumChemistryCalculation h2oCalc("H2O", "cc-pVDZ");
    h2oCalc.setState(std::make_unique<BasisInitializationState>());
    
    // Initialize basis set
    h2oCalc.setupBasisSet();
    
    // Run SCF iterations
    std::cout << "\n--- Self-Consistent Field Calculation ---\n";
    for (int i = 0; i < 12; ++i) {
        h2oCalc.runSCF();
        if (h2oCalc.isConverged()) {
            break;
        }
    }
    
    // Add electron correlation
    std::cout << "\n--- Post-HF Correlation Treatment ---\n";
    h2oCalc.addCorrelation();
    
    // Finalize results
    h2oCalc.finalize();
    
    // Larger molecule example
    std::cout << "\n\n=== Complex Molecule Calculation ===\n";
    
    QuantumChemistryCalculation caffeineCalc("Caffeine", "6-31G*");
    caffeineCalc.setState(std::make_unique<BasisInitializationState>());
    
    caffeineCalc.setupBasisSet();
    
    // SCF for larger system
    std::cout << "\n--- SCF for Organic Molecule ---\n";
    for (int i = 0; i < 15; ++i) {
        caffeineCalc.runSCF();
        if (caffeineCalc.isConverged()) {
            break;
        }
    }
    
    if (caffeineCalc.isConverged()) {
        caffeineCalc.addCorrelation();
    }
    
    caffeineCalc.finalize();
    
    std::cout << "\n=== State Pattern Summary ===\n";
    std::cout << "The State pattern enables sophisticated management of computational states\n";
    std::cout << "in scientific algorithms, providing clean transitions between:\n";
    std::cout << "• Iterative solver states (initialization → iteration → convergence/failure)\n";
    std::cout << "• Monte Carlo sampling phases (sampling → refinement → completion)\n";
    std::cout << "• Quantum chemistry computation stages (basis setup → SCF → correlation)\n";
    std::cout << "\nThis pattern is essential for complex scientific simulations with\n";
    std::cout << "multiple computational phases and state-dependent behavior.\n";
    
    return 0;
}