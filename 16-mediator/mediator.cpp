// Mediator Pattern - Multi-Physics Simulation Coordination
// Coordinates data exchange and synchronization between coupled physics solvers
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <chrono>

// Forward declarations
class SimulationCoordinator;
class PhysicsSolver;

// Physics data structure for inter-solver communication
struct PhysicsData {
    std::string dataType;
    std::vector<double> values;
    double timestamp;
    std::string sourceRegion;
    std::string units;
    
    PhysicsData(const std::string& type, const std::vector<double>& vals, 
                double t, const std::string& region, const std::string& unit)
        : dataType(type), values(vals), timestamp(t), sourceRegion(region), units(unit) {}
};

// Abstract colleague - Physics Solver
class PhysicsSolver {
protected:
    std::string solverName_;
    std::string physicsType_;
    std::shared_ptr<SimulationCoordinator> coordinator_;
    double currentTime_;
    double timeStep_;
    bool isActive_;
    std::vector<PhysicsData> receivedData_;
    
public:
    PhysicsSolver(const std::string& name, const std::string& physics) 
        : solverName_(name), physicsType_(physics), currentTime_(0.0), 
          timeStep_(1e-6), isActive_(true) {}
    virtual ~PhysicsSolver() = default;
    
    void setCoordinator(std::shared_ptr<SimulationCoordinator> coordinator) {
        coordinator_ = coordinator;
    }
    
    const std::string& getName() const { return solverName_; }
    const std::string& getPhysicsType() const { return physicsType_; }
    double getCurrentTime() const { return currentTime_; }
    double getTimeStep() const { return timeStep_; }
    bool isActive() const { return isActive_; }
    
    virtual void solveTimeStep() = 0;
    virtual void sendCouplingData(const std::string& dataType, const std::vector<double>& data) = 0;
    virtual void receiveCouplingData(const PhysicsData& data) = 0;
    virtual void synchronizeTime(double globalTime) = 0;
    virtual void displayStatus() const = 0;
};

// Mediator interface - Simulation Coordinator
class SimulationCoordinator {
public:
    virtual ~SimulationCoordinator() = default;
    virtual void addSolver(std::shared_ptr<PhysicsSolver> solver) = 0;
    virtual void exchangeCouplingData(const std::string& dataType, 
                                     const std::vector<double>& data,
                                     PhysicsSolver* fromSolver) = 0;
    virtual void synchronizeAllSolvers() = 0;
    virtual void requestDataExchange(const std::string& dataType,
                                   PhysicsSolver* requester,
                                   const std::string& targetSolver) = 0;
    virtual void coordinateTimeStep() = 0;
};

// Concrete colleague - Fluid Dynamics Solver
class FluidDynamicsSolver : public PhysicsSolver {
private:
    std::vector<double> velocityField_;
    std::vector<double> pressureField_;
    std::vector<double> temperatureField_;
    double reynoldsNumber_;
    double viscosity_;
    
public:
    FluidDynamicsSolver(const std::string& name) 
        : PhysicsSolver(name, "Computational Fluid Dynamics"),
          reynoldsNumber_(1000.0), viscosity_(1e-6) {
        // Initialize fields
        int gridSize = 100;
        velocityField_.resize(gridSize, 10.0);
        pressureField_.resize(gridSize, 101325.0);
        temperatureField_.resize(gridSize, 300.0);
        timeStep_ = 1e-5;
    }
    
    void solveTimeStep() override {
        std::cout << "[CFD] " << solverName_ << " solving Navier-Stokes equations...\n";
        
        // Simulate CFD computation
        for (size_t i = 0; i < velocityField_.size(); ++i) {
            velocityField_[i] += 0.1 * std::sin(currentTime_ * 1000) * (rand() / (double)RAND_MAX - 0.5);
            pressureField_[i] += 100 * std::cos(currentTime_ * 500) * (rand() / (double)RAND_MAX - 0.5);
            temperatureField_[i] += 0.5 * std::sin(currentTime_ * 200) * (rand() / (double)RAND_MAX - 0.5);
        }
        
        currentTime_ += timeStep_;
        
        // Send coupling data to other solvers
        sendCouplingData("temperature", temperatureField_);
        sendCouplingData("pressure", pressureField_);
        
        std::cout << "  Re = " << std::fixed << std::setprecision(1) << reynoldsNumber_
                  << ", max velocity = " << std::setprecision(3) 
                  << *std::max_element(velocityField_.begin(), velocityField_.end()) << " m/s\n";
    }
    
    void sendCouplingData(const std::string& dataType, const std::vector<double>& data) override {
        if (coordinator_) {
            coordinator_->exchangeCouplingData(dataType, data, this);
        }
    }
    
    void receiveCouplingData(const PhysicsData& data) override {
        receivedData_.push_back(data);
        
        if (data.dataType == "stress") {
            std::cout << "  [CFD] Received structural stress data: "
                      << "avg = " << std::accumulate(data.values.begin(), data.values.end(), 0.0) / data.values.size()
                      << " Pa\n";
            // Apply stress boundary conditions
        } else if (data.dataType == "temperature") {
            std::cout << "  [CFD] Received thermal data for fluid properties update\n";
            // Update fluid properties based on temperature
        }
    }
    
    void synchronizeTime(double globalTime) override {
        currentTime_ = globalTime;
    }
    
    void displayStatus() const override {
        std::cout << "[CFD Status] " << solverName_ << ":\n";
        std::cout << "  Physics: " << physicsType_ << "\n";
        std::cout << "  Time: " << std::scientific << currentTime_ << " s\n";
        std::cout << "  Time step: " << timeStep_ << " s\n";
        std::cout << "  Reynolds number: " << std::fixed << reynoldsNumber_ << "\n";
        std::cout << "  Grid points: " << velocityField_.size() << "\n";
        std::cout << "  Coupling data received: " << receivedData_.size() << " datasets\n";
    }
};

// Concrete colleague - Structural Mechanics Solver
class StructuralMechanicsSolver : public PhysicsSolver {
private:
    std::vector<double> displacementField_;
    std::vector<double> stressField_;
    std::vector<double> strainField_;
    double youngsModulus_;
    double poissonRatio_;
    
public:
    StructuralMechanicsSolver(const std::string& name)
        : PhysicsSolver(name, "Structural Mechanics"),
          youngsModulus_(200e9), poissonRatio_(0.3) {
        // Initialize fields
        int nodeCount = 500;
        displacementField_.resize(nodeCount, 0.0);
        stressField_.resize(nodeCount, 0.0);
        strainField_.resize(nodeCount, 0.0);
        timeStep_ = 1e-4;
    }
    
    void solveTimeStep() override {
        std::cout << "[FEM] " << solverName_ << " solving structural equations...\n";
        
        // Simulate FEM computation
        for (size_t i = 0; i < displacementField_.size(); ++i) {
            double thermalStrain = 0.0;
            
            // Apply thermal loads if received from thermal solver
            for (const auto& data : receivedData_) {
                if (data.dataType == "temperature" && !data.values.empty()) {
                    double avgTemp = std::accumulate(data.values.begin(), data.values.end(), 0.0) / data.values.size();
                    thermalStrain = 1.2e-5 * (avgTemp - 293.15); // Thermal expansion
                }
            }
            
            strainField_[i] = thermalStrain + 1e-6 * std::sin(currentTime_ * 100) * (rand() / (double)RAND_MAX - 0.5);
            stressField_[i] = youngsModulus_ * strainField_[i];
            displacementField_[i] += strainField_[i] * 0.1; // Integration
        }
        
        currentTime_ += timeStep_;
        
        // Send coupling data
        sendCouplingData("stress", stressField_);
        sendCouplingData("displacement", displacementField_);
        
        double maxStress = *std::max_element(stressField_.begin(), stressField_.end());
        std::cout << "  E = " << std::scientific << youngsModulus_ << " Pa"
                  << ", max stress = " << std::setprecision(3) << maxStress << " Pa\n";
    }
    
    void sendCouplingData(const std::string& dataType, const std::vector<double>& data) override {
        if (coordinator_) {
            coordinator_->exchangeCouplingData(dataType, data, this);
        }
    }
    
    void receiveCouplingData(const PhysicsData& data) override {
        receivedData_.push_back(data);
        
        if (data.dataType == "pressure") {
            std::cout << "  [FEM] Received fluid pressure for load application\n";
            // Apply pressure loads from CFD
        } else if (data.dataType == "temperature") {
            std::cout << "  [FEM] Received temperature field for thermal stress analysis\n";
            // Clear old thermal data and use new
            receivedData_.erase(
                std::remove_if(receivedData_.begin(), receivedData_.end(),
                    [](const PhysicsData& d) { return d.dataType == "temperature" && d.timestamp < data.timestamp; }),
                receivedData_.end());
        }
    }
    
    void synchronizeTime(double globalTime) override {
        currentTime_ = globalTime;
    }
    
    void displayStatus() const override {
        std::cout << "[FEM Status] " << solverName_ << ":\n";
        std::cout << "  Physics: " << physicsType_ << "\n";
        std::cout << "  Time: " << std::scientific << currentTime_ << " s\n";
        std::cout << "  Time step: " << timeStep_ << " s\n";
        std::cout << "  Young's modulus: " << youngsModulus_ << " Pa\n";
        std::cout << "  Poisson's ratio: " << std::fixed << poissonRatio_ << "\n";
        std::cout << "  Nodes: " << displacementField_.size() << "\n";
        std::cout << "  Coupling data received: " << receivedData_.size() << " datasets\n";
    }
};

// Heat Transfer Solver
class HeatTransferSolver : public PhysicsSolver {
private:
    std::vector<double> temperatureField_;
    std::vector<double> heatFluxField_;
    double thermalConductivity_;
    double specificHeat_;
    double density_;
    
public:
    HeatTransferSolver(const std::string& name)
        : PhysicsSolver(name, "Heat Transfer"),
          thermalConductivity_(50.0), specificHeat_(500.0), density_(7800.0) {
        int nodeCount = 200;
        temperatureField_.resize(nodeCount, 293.15); // Room temperature
        heatFluxField_.resize(nodeCount, 0.0);
        timeStep_ = 1e-3;
    }
    
    void solveTimeStep() override {
        std::cout << "[HEAT] " << solverName_ << " solving heat equation...\n";
        
        // Simulate heat transfer computation
        for (size_t i = 0; i < temperatureField_.size(); ++i) {
            double heatGeneration = 1000.0 * std::sin(currentTime_ * 0.1); // Heat source
            
            // Apply heat flux from structural deformation
            for (const auto& data : receivedData_) {
                if (data.dataType == "stress" && !data.values.empty()) {
                    double avgStress = std::accumulate(data.values.begin(), data.values.end(), 0.0) / data.values.size();
                    heatGeneration += 0.01 * std::abs(avgStress); // Plastic dissipation
                }
            }
            
            heatFluxField_[i] = -thermalConductivity_ * (rand() / (double)RAND_MAX - 0.5) * 0.1;
            temperatureField_[i] += (heatGeneration * timeStep_) / (density_ * specificHeat_);
        }
        
        currentTime_ += timeStep_;
        
        // Send coupling data
        sendCouplingData("temperature", temperatureField_);
        sendCouplingData("heat_flux", heatFluxField_);
        
        double avgTemp = std::accumulate(temperatureField_.begin(), temperatureField_.end(), 0.0) / temperatureField_.size();
        std::cout << "  k = " << thermalConductivity_ << " W/m·K"
                  << ", avg temp = " << std::fixed << std::setprecision(2) << avgTemp << " K\n";
    }
    
    void sendCouplingData(const std::string& dataType, const std::vector<double>& data) override {
        if (coordinator_) {
            coordinator_->exchangeCouplingData(dataType, data, this);
        }
    }
    
    void receiveCouplingData(const PhysicsData& data) override {
        receivedData_.push_back(data);
        
        if (data.dataType == "stress") {
            std::cout << "  [HEAT] Received stress data for dissipative heating calculation\n";
        } else if (data.dataType == "velocity") {
            std::cout << "  [HEAT] Received velocity field for convective heat transfer\n";
        }
    }
    
    void synchronizeTime(double globalTime) override {
        currentTime_ = globalTime;
    }
    
    void displayStatus() const override {
        std::cout << "[HEAT Status] " << solverName_ << ":\n";
        std::cout << "  Physics: " << physicsType_ << "\n";
        std::cout << "  Time: " << std::scientific << currentTime_ << " s\n";
        std::cout << "  Time step: " << timeStep_ << " s\n";
        std::cout << "  Thermal conductivity: " << std::fixed << thermalConductivity_ << " W/m·K\n";
        std::cout << "  Density: " << density_ << " kg/m³\n";
        std::cout << "  Nodes: " << temperatureField_.size() << "\n";
        std::cout << "  Coupling data received: " << receivedData_.size() << " datasets\n";
    }
};

// Concrete mediator - Multi-Physics Simulation Coordinator
class MultiPhysicsCoordinator : public SimulationCoordinator, 
                               public std::enable_shared_from_this<MultiPhysicsCoordinator> {
private:
    std::vector<std::shared_ptr<PhysicsSolver>> solvers_;
    std::vector<std::string> couplingLog_;
    double globalTime_;
    double globalTimeStep_;
    int couplingIteration_;
    
    std::shared_ptr<PhysicsSolver> findSolver(const std::string& name) {
        auto it = std::find_if(solvers_.begin(), solvers_.end(),
            [&name](const std::shared_ptr<PhysicsSolver>& solver) {
                return solver->getName() == name;
            });
        return (it != solvers_.end()) ? *it : nullptr;
    }
    
public:
    MultiPhysicsCoordinator() : globalTime_(0.0), globalTimeStep_(1e-4), couplingIteration_(0) {}
    
    void addSolver(std::shared_ptr<PhysicsSolver> solver) override {
        solvers_.push_back(solver);
        solver->setCoordinator(shared_from_this());
        
        std::string notification = solver->getName() + " (" + solver->getPhysicsType() + ") added to simulation";
        couplingLog_.push_back("[COORD] " + notification);
        std::cout << "\n[COORD] " << notification << "\n";
        
        // Determine global time step (minimum of all solvers)
        globalTimeStep_ = std::min(globalTimeStep_, solver->getTimeStep());
        std::cout << "[COORD] Global time step adjusted to " << std::scientific 
                  << globalTimeStep_ << " s\n";
    }
    
    void exchangeCouplingData(const std::string& dataType, 
                             const std::vector<double>& data,
                             PhysicsSolver* fromSolver) override {
        PhysicsData couplingData(dataType, data, globalTime_, 
                               fromSolver->getName(), "SI");
        
        std::string logEntry = fromSolver->getName() + " sent " + dataType + " data";
        couplingLog_.push_back(logEntry);
        
        // Send to all other solvers that might need this data
        for (const auto& solver : solvers_) {
            if (solver.get() != fromSolver && solver->isActive()) {
                bool needsData = shouldReceiveData(solver->getPhysicsType(), dataType);
                if (needsData) {
                    solver->receiveCouplingData(couplingData);
                }
            }
        }
    }
    
    void synchronizeAllSolvers() override {
        std::cout << "\n[COORD] Synchronizing all solvers to time " 
                  << std::scientific << globalTime_ << " s\n";
        
        for (const auto& solver : solvers_) {
            if (solver->isActive()) {
                solver->synchronizeTime(globalTime_);
            }
        }
    }
    
    void requestDataExchange(const std::string& dataType,
                           PhysicsSolver* requester,
                           const std::string& targetSolver) override {
        auto target = findSolver(targetSolver);
        if (target) {
            std::cout << "[COORD] Data exchange requested: " << dataType 
                      << " from " << targetSolver << " to " << requester->getName() << "\n";
            // In a real implementation, this would trigger specific data transfer
        }
    }
    
    void coordinateTimeStep() override {
        couplingIteration_++;
        std::cout << "\n=== Coupling Iteration " << couplingIteration_ 
                  << " (t = " << std::scientific << globalTime_ << " s) ===\n";
        
        // Solve all physics in sequence (could be parallelized)
        for (const auto& solver : solvers_) {
            if (solver->isActive()) {
                solver->solveTimeStep();
            }
        }
        
        // Advance global time
        globalTime_ += globalTimeStep_;
        
        // Synchronize all solvers
        synchronizeAllSolvers();
        
        std::cout << "[COORD] Coupling iteration " << couplingIteration_ 
                  << " completed\n";
    }
    
    void showCouplingLog() const {
        std::cout << "\n=== Multi-Physics Coupling Log ===\n";
        for (const auto& entry : couplingLog_) {
            std::cout << entry << "\n";
        }
    }
    
    void displayGlobalStatus() const {
        std::cout << "\n=== Global Simulation Status ===\n";
        std::cout << "Global time: " << std::scientific << globalTime_ << " s\n";
        std::cout << "Global time step: " << globalTimeStep_ << " s\n";
        std::cout << "Coupling iterations: " << couplingIteration_ << "\n";
        std::cout << "Active solvers: " << solvers_.size() << "\n";
        std::cout << "Coupling exchanges: " << couplingLog_.size() << "\n";
        std::cout << "==============================\n";
    }
    
private:
    bool shouldReceiveData(const std::string& solverType, const std::string& dataType) {
        // Define coupling relationships
        if (solverType == "Computational Fluid Dynamics") {
            return (dataType == "stress" || dataType == "displacement" || dataType == "temperature");
        } else if (solverType == "Structural Mechanics") {
            return (dataType == "pressure" || dataType == "temperature" || dataType == "heat_flux");
        } else if (solverType == "Heat Transfer") {
            return (dataType == "stress" || dataType == "pressure" || dataType == "velocity");
        }
        return false;
    }
    
public:
    // Factory method
    static std::shared_ptr<MultiPhysicsCoordinator> create() {
        return std::shared_ptr<MultiPhysicsCoordinator>(new MultiPhysicsCoordinator());
    }
};

// Additional example - Simulation Control Interface
class SimulationController {
public:
    virtual ~SimulationController() = default;
    virtual void notify(class ControlComponent* sender, const std::string& event) = 0;
};

class ControlComponent {
protected:
    SimulationController* controller_;
    std::string name_;
    
public:
    ControlComponent(const std::string& name) : name_(name), controller_(nullptr) {}
    
    void setController(SimulationController* controller) {
        controller_ = controller;
    }
    
    const std::string& getName() const { return name_; }
};

class SimulationButton : public ControlComponent {
public:
    SimulationButton(const std::string& name) : ControlComponent(name) {}
    
    void click() {
        std::cout << "[UI] Button '" << name_ << "' clicked\n";
        if (controller_) {
            controller_->notify(this, "click");
        }
    }
};

class ParameterSlider : public ControlComponent {
private:
    double value_;
    double minValue_, maxValue_;
    
public:
    ParameterSlider(const std::string& name, double min, double max, double initial)
        : ControlComponent(name), value_(initial), minValue_(min), maxValue_(max) {}
    
    void setValue(double value) {
        value_ = std::max(minValue_, std::min(maxValue_, value));
        std::cout << "[UI] Slider '" << name_ << "' set to " 
                  << std::scientific << value_ << "\n";
        if (controller_) {
            controller_->notify(this, "valueChanged");
        }
    }
    
    double getValue() const { return value_; }
};

class StatusDisplay : public ControlComponent {
private:
    std::string status_;
    
public:
    StatusDisplay(const std::string& name) : ControlComponent(name), status_("Ready") {}
    
    void updateStatus(const std::string& status) {
        status_ = status;
        std::cout << "[UI] Status '" << name_ << "': " << status << "\n";
        if (controller_) {
            controller_->notify(this, "statusUpdated");
        }
    }
    
    const std::string& getStatus() const { return status_; }
};

// Concrete simulation controller
class MultiPhysicsSimulationController : public SimulationController {
private:
    SimulationButton* startButton_;
    SimulationButton* stopButton_;
    ParameterSlider* timeStepSlider_;
    ParameterSlider* couplingToleranceSlider_;
    StatusDisplay* statusDisplay_;
    std::shared_ptr<MultiPhysicsCoordinator> coordinator_;
    bool simulationRunning_;
    
public:
    MultiPhysicsSimulationController(SimulationButton* start, SimulationButton* stop,
                                   ParameterSlider* timeStep, ParameterSlider* tolerance,
                                   StatusDisplay* status,
                                   std::shared_ptr<MultiPhysicsCoordinator> coordinator)
        : startButton_(start), stopButton_(stop), timeStepSlider_(timeStep),
          couplingToleranceSlider_(tolerance), statusDisplay_(status),
          coordinator_(coordinator), simulationRunning_(false) {
        
        startButton_->setController(this);
        stopButton_->setController(this);
        timeStepSlider_->setController(this);
        couplingToleranceSlider_->setController(this);
        statusDisplay_->setController(this);
    }
    
    void notify(ControlComponent* sender, const std::string& event) override {
        if (sender == startButton_ && event == "click") {
            if (!simulationRunning_) {
                std::cout << "  [CTRL] Starting multi-physics simulation...\n";
                simulationRunning_ = true;
                statusDisplay_->updateStatus("Running");
                
                // Run a few coupling iterations
                for (int i = 0; i < 3; ++i) {
                    coordinator_->coordinateTimeStep();
                }
            } else {
                std::cout << "  [CTRL] Simulation already running\n";
            }
        }
        else if (sender == stopButton_ && event == "click") {
            if (simulationRunning_) {
                std::cout << "  [CTRL] Stopping simulation...\n";
                simulationRunning_ = false;
                statusDisplay_->updateStatus("Stopped");
            } else {
                std::cout << "  [CTRL] Simulation not running\n";
            }
        }
        else if (sender == timeStepSlider_ && event == "valueChanged") {
            double newTimeStep = timeStepSlider_->getValue();
            std::cout << "  [CTRL] Time step updated to " 
                      << std::scientific << newTimeStep << " s\n";
            // In real implementation, would update solver time steps
        }
        else if (sender == couplingToleranceSlider_ && event == "valueChanged") {
            double newTolerance = couplingToleranceSlider_->getValue();
            std::cout << "  [CTRL] Coupling tolerance updated to " 
                      << std::scientific << newTolerance << "\n";
            // In real implementation, would update convergence criteria
        }
        else if (sender == statusDisplay_ && event == "statusUpdated") {
            std::cout << "  [CTRL] Status display updated\n";
        }
    }
};

int main() {
    std::cout << "=== Multi-Physics Simulation Coordinator ===\n";
    
    // Create simulation coordinator
    auto coordinator = MultiPhysicsCoordinator::create();
    
    // Create physics solvers
    auto cfdSolver = std::make_shared<FluidDynamicsSolver>("NavierStokes_3D");
    auto femSolver = std::make_shared<StructuralMechanicsSolver>("StructuralFEM");
    auto heatSolver = std::make_shared<HeatTransferSolver>("ThermalAnalysis");
    
    // Add solvers to coordinator
    coordinator->addSolver(cfdSolver);
    coordinator->addSolver(femSolver);
    coordinator->addSolver(heatSolver);
    
    // Display initial status
    coordinator->displayGlobalStatus();
    std::cout << "\n";
    cfdSolver->displayStatus();
    std::cout << "\n";
    femSolver->displayStatus();
    std::cout << "\n";
    heatSolver->displayStatus();
    
    // Run coupled simulation
    std::cout << "\n=== Multi-Physics Coupling Simulation ===\n";
    
    // Run several coupling iterations
    for (int iter = 1; iter <= 5; ++iter) {
        coordinator->coordinateTimeStep();
        
        if (iter == 3) {
            std::cout << "\n=== Mid-simulation Status Check ===\n";
            coordinator->displayGlobalStatus();
        }
    }
    
    // Show coupling log
    coordinator->showCouplingLog();
    
    // Final status
    std::cout << "\n=== Final Solver Status ===\n";
    cfdSolver->displayStatus();
    std::cout << "\n";
    femSolver->displayStatus();
    std::cout << "\n";
    heatSolver->displayStatus();
    
    // Simulation Control Interface Example
    std::cout << "\n\n=== Simulation Control Interface ===\n";
    
    SimulationButton startBtn("StartSimulation");
    SimulationButton stopBtn("StopSimulation");
    ParameterSlider timeStepSlider("TimeStepControl", 1e-6, 1e-3, 1e-4);
    ParameterSlider toleranceSlider("CouplingTolerance", 1e-8, 1e-3, 1e-6);
    StatusDisplay statusDisplay("SimulationStatus");
    
    MultiPhysicsSimulationController simController(
        &startBtn, &stopBtn, &timeStepSlider, &toleranceSlider, 
        &statusDisplay, coordinator);
    
    std::cout << "\n--- User adjusts parameters ---\n";
    timeStepSlider.setValue(5e-5);
    toleranceSlider.setValue(1e-7);
    
    std::cout << "\n--- User starts simulation ---\n";
    startBtn.click();
    
    std::cout << "\n--- User tries to start again (already running) ---\n";
    startBtn.click();
    
    std::cout << "\n--- User stops simulation ---\n";
    stopBtn.click();
    
    std::cout << "\n--- User adjusts parameters while stopped ---\n";
    timeStepSlider.setValue(2e-4);
    
    std::cout << "\nMediator pattern enables coordinated multi-physics simulations\n";
    std::cout << "with seamless data exchange and synchronized time stepping!\n";
    
    return 0;
}