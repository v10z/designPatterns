// Command Pattern - Scientific Simulation Control System
// Encapsulates simulation operations as commands for batch processing and reproducibility
#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <string>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cmath>

// Receiver - Computational simulation engine
class SimulationEngine {
private:
    // Simulation state
    double currentTime_;
    double timeStep_;
    double temperature_;    // Kelvin
    double pressure_;       // Pascal
    double volume_;         // m³
    int particleCount_;
    std::string solver_;
    std::vector<double> energyHistory_;
    std::vector<double> temperatureHistory_;
    bool isRunning_;
    
public:
    SimulationEngine() 
        : currentTime_(0.0), timeStep_(1e-15), temperature_(300.0),
          pressure_(101325.0), volume_(1e-6), particleCount_(1000),
          solver_("Velocity-Verlet"), isRunning_(false) {}
    
    // Simulation operations
    void setTimeStep(double dt) {
        double oldDt = timeStep_;
        timeStep_ = dt;
        std::cout << "Time step changed from " << std::scientific << std::setprecision(2) 
                  << oldDt << " to " << dt << " seconds\n";
    }
    
    void setTemperature(double temp) {
        double oldTemp = temperature_;
        temperature_ = temp;
        std::cout << "Temperature changed from " << std::fixed << std::setprecision(2)
                  << oldTemp << " to " << temp << " K\n";
    }
    
    void setPressure(double pressure) {
        double oldPressure = pressure_;
        pressure_ = pressure;
        std::cout << "Pressure changed from " << std::scientific << std::setprecision(2)
                  << oldPressure << " to " << pressure << " Pa\n";
    }
    
    void setParticleCount(int count) {
        int oldCount = particleCount_;
        particleCount_ = count;
        std::cout << "Particle count changed from " << oldCount 
                  << " to " << count << "\n";
    }
    
    void setSolver(const std::string& solver) {
        std::string oldSolver = solver_;
        solver_ = solver;
        std::cout << "Solver changed from " << oldSolver 
                  << " to " << solver << "\n";
    }
    
    void runSimulation(double duration) {
        std::cout << "\n--- Running simulation for " << std::scientific 
                  << duration << " seconds ---\n";
        std::cout << "Parameters: T=" << std::fixed << temperature_ 
                  << "K, P=" << std::scientific << pressure_ 
                  << "Pa, N=" << particleCount_ << "\n";
        std::cout << "Using " << solver_ << " integrator\n";
        
        isRunning_ = true;
        int steps = static_cast<int>(duration / timeStep_);
        
        for (int i = 0; i < steps && i < 10; ++i) { // Simulate first 10 steps
            currentTime_ += timeStep_;
            
            // Simulate energy calculation
            double kineticEnergy = 0.5 * particleCount_ * 1.38e-23 * temperature_;
            double potentialEnergy = -particleCount_ * std::exp(-currentTime_ * 1e12);
            double totalEnergy = kineticEnergy + potentialEnergy;
            
            energyHistory_.push_back(totalEnergy);
            temperatureHistory_.push_back(temperature_ + 0.1 * std::sin(i));
            
            if (i % 3 == 0) {
                std::cout << "  Step " << i << ": E=" << std::scientific 
                          << totalEnergy << " J, T=" << std::fixed 
                          << temperatureHistory_.back() << " K\n";
            }
        }
        
        std::cout << "Simulation completed. Total time: " 
                  << std::scientific << currentTime_ << " s\n";
        isRunning_ = false;
    }
    
    void saveCheckpoint(const std::string& filename) {
        std::cout << "Saving checkpoint to " << filename << "\n";
        std::cout << "  Current time: " << std::scientific << currentTime_ << " s\n";
        std::cout << "  Energy history points: " << energyHistory_.size() << "\n";
        std::cout << "  Checkpoint saved successfully\n";
    }
    
    void loadCheckpoint(const std::string& filename) {
        std::cout << "Loading checkpoint from " << filename << "\n";
        // Simulate loading
        std::cout << "  Checkpoint loaded successfully\n";
        std::cout << "  Resuming from time: " << std::scientific << currentTime_ << " s\n";
    }
    
    void clearHistory() {
        size_t oldSize = energyHistory_.size();
        energyHistory_.clear();
        temperatureHistory_.clear();
        currentTime_ = 0.0;
        std::cout << "Cleared " << oldSize << " history points\n";
        std::cout << "Reset simulation time to 0\n";
    }
    
    // Getters for undo operations
    double getTimeStep() const { return timeStep_; }
    double getTemperature() const { return temperature_; }
    double getPressure() const { return pressure_; }
    int getParticleCount() const { return particleCount_; }
    std::string getSolver() const { return solver_; }
    double getCurrentTime() const { return currentTime_; }
    size_t getHistorySize() const { return energyHistory_.size(); }
    
    void displayStatus() const {
        std::cout << "\n=== Simulation Status ===\n";
        std::cout << "Time: " << std::scientific << std::setprecision(3) 
                  << currentTime_ << " s\n";
        std::cout << "Time step: " << timeStep_ << " s\n";
        std::cout << "Temperature: " << std::fixed << std::setprecision(2) 
                  << temperature_ << " K\n";
        std::cout << "Pressure: " << std::scientific << pressure_ << " Pa\n";
        std::cout << "Particles: " << particleCount_ << "\n";
        std::cout << "Solver: " << solver_ << "\n";
        std::cout << "History points: " << energyHistory_.size() << "\n";
        std::cout << "Status: " << (isRunning_ ? "Running" : "Idle") << "\n";
        std::cout << "=======================\n\n";
    }
};

// Command interface
class SimulationCommand {
public:
    virtual ~SimulationCommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getParameters() const = 0;
    
    // For logging and reproducibility
    std::chrono::system_clock::time_point getTimestamp() const {
        return timestamp_;
    }
    
protected:
    std::chrono::system_clock::time_point timestamp_;
    
    void recordTimestamp() {
        timestamp_ = std::chrono::system_clock::now();
    }
};

// Concrete commands
class SetTimeStepCommand : public SimulationCommand {
private:
    SimulationEngine& engine_;
    double newTimeStep_;
    double oldTimeStep_;
    
public:
    SetTimeStepCommand(SimulationEngine& engine, double timeStep)
        : engine_(engine), newTimeStep_(timeStep) {
        oldTimeStep_ = engine.getTimeStep();
    }
    
    void execute() override {
        recordTimestamp();
        engine_.setTimeStep(newTimeStep_);
    }
    
    void undo() override {
        engine_.setTimeStep(oldTimeStep_);
    }
    
    std::string getName() const override {
        return "SetTimeStep";
    }
    
    std::string getParameters() const override {
        std::stringstream ss;
        ss << "dt=" << std::scientific << std::setprecision(2) << newTimeStep_ << "s";
        return ss.str();
    }
};

class SetTemperatureCommand : public SimulationCommand {
private:
    SimulationEngine& engine_;
    double newTemperature_;
    double oldTemperature_;
    
public:
    SetTemperatureCommand(SimulationEngine& engine, double temperature)
        : engine_(engine), newTemperature_(temperature) {
        oldTemperature_ = engine.getTemperature();
    }
    
    void execute() override {
        recordTimestamp();
        engine_.setTemperature(newTemperature_);
    }
    
    void undo() override {
        engine_.setTemperature(oldTemperature_);
    }
    
    std::string getName() const override {
        return "SetTemperature";
    }
    
    std::string getParameters() const override {
        std::stringstream ss;
        ss << "T=" << std::fixed << std::setprecision(2) << newTemperature_ << "K";
        return ss.str();
    }
};

class SetPressureCommand : public SimulationCommand {
private:
    SimulationEngine& engine_;
    double newPressure_;
    double oldPressure_;
    
public:
    SetPressureCommand(SimulationEngine& engine, double pressure)
        : engine_(engine), newPressure_(pressure) {
        oldPressure_ = engine.getPressure();
    }
    
    void execute() override {
        recordTimestamp();
        engine_.setPressure(newPressure_);
    }
    
    void undo() override {
        engine_.setPressure(oldPressure_);
    }
    
    std::string getName() const override {
        return "SetPressure";
    }
    
    std::string getParameters() const override {
        std::stringstream ss;
        ss << "P=" << std::scientific << std::setprecision(2) << newPressure_ << "Pa";
        return ss.str();
    }
};

class RunSimulationCommand : public SimulationCommand {
private:
    SimulationEngine& engine_;
    double duration_;
    double previousTime_;
    size_t previousHistorySize_;
    
public:
    RunSimulationCommand(SimulationEngine& engine, double duration)
        : engine_(engine), duration_(duration) {
        previousTime_ = engine.getCurrentTime();
        previousHistorySize_ = engine.getHistorySize();
    }
    
    void execute() override {
        recordTimestamp();
        engine_.runSimulation(duration_);
    }
    
    void undo() override {
        std::cout << "Undoing simulation run (reverting to previous state)\n";
        // In real implementation, would restore previous state
        std::cout << "  Restored to time: " << std::scientific 
                  << previousTime_ << " s\n";
    }
    
    std::string getName() const override {
        return "RunSimulation";
    }
    
    std::string getParameters() const override {
        std::stringstream ss;
        ss << "duration=" << std::scientific << std::setprecision(2) 
           << duration_ << "s";
        return ss.str();
    }
};

class ChangeSolverCommand : public SimulationCommand {
private:
    SimulationEngine& engine_;
    std::string newSolver_;
    std::string oldSolver_;
    
public:
    ChangeSolverCommand(SimulationEngine& engine, const std::string& solver)
        : engine_(engine), newSolver_(solver) {
        oldSolver_ = engine.getSolver();
    }
    
    void execute() override {
        recordTimestamp();
        engine_.setSolver(newSolver_);
    }
    
    void undo() override {
        engine_.setSolver(oldSolver_);
    }
    
    std::string getName() const override {
        return "ChangeSolver";
    }
    
    std::string getParameters() const override {
        return "solver=" + newSolver_;
    }
};

class SaveCheckpointCommand : public SimulationCommand {
private:
    SimulationEngine& engine_;
    std::string filename_;
    
public:
    SaveCheckpointCommand(SimulationEngine& engine, const std::string& filename)
        : engine_(engine), filename_(filename) {}
    
    void execute() override {
        recordTimestamp();
        engine_.saveCheckpoint(filename_);
    }
    
    void undo() override {
        std::cout << "Cannot undo checkpoint save (file: " << filename_ << ")\n";
    }
    
    std::string getName() const override {
        return "SaveCheckpoint";
    }
    
    std::string getParameters() const override {
        return "file=" + filename_;
    }
};

// Batch command - composite command for scientific workflows
class BatchCommand : public SimulationCommand {
private:
    std::vector<std::shared_ptr<SimulationCommand>> commands_;
    std::string workflowName_;
    std::string description_;
    
public:
    BatchCommand(const std::string& name, const std::string& description)
        : workflowName_(name), description_(description) {}
    
    void addCommand(std::shared_ptr<SimulationCommand> command) {
        commands_.push_back(command);
    }
    
    void execute() override {
        recordTimestamp();
        std::cout << "\n=== Executing Workflow: " << workflowName_ << " ===\n";
        std::cout << "Description: " << description_ << "\n";
        std::cout << "Steps: " << commands_.size() << "\n\n";
        
        for (size_t i = 0; i < commands_.size(); ++i) {
            std::cout << "Step " << (i+1) << "/" << commands_.size() 
                      << ": " << commands_[i]->getName() 
                      << " (" << commands_[i]->getParameters() << ")\n";
            commands_[i]->execute();
            std::cout << "\n";
        }
        
        std::cout << "=== Workflow Complete ===\n\n";
    }
    
    void undo() override {
        std::cout << "\n=== Undoing Workflow: " << workflowName_ << " ===\n";
        // Undo in reverse order
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            (*it)->undo();
        }
        std::cout << "=== Workflow Undone ===\n\n";
    }
    
    std::string getName() const override {
        return "Workflow";
    }
    
    std::string getParameters() const override {
        return workflowName_ + " (" + std::to_string(commands_.size()) + " steps)";
    }
};

// Command manager - handles execution, history, and reproducibility
class SimulationCommandManager {
private:
    std::stack<std::shared_ptr<SimulationCommand>> undoStack_;
    std::stack<std::shared_ptr<SimulationCommand>> redoStack_;
    std::vector<std::shared_ptr<SimulationCommand>> commandHistory_;
    bool recordingScript_;
    std::string scriptFilename_;
    
public:
    SimulationCommandManager() : recordingScript_(false) {}
    
    void executeCommand(std::shared_ptr<SimulationCommand> command) {
        command->execute();
        undoStack_.push(command);
        commandHistory_.push_back(command);
        
        // Clear redo stack on new command
        while (!redoStack_.empty()) {
            redoStack_.pop();
        }
        
        std::cout << "\n[EXECUTED] " << command->getName() 
                  << " (" << command->getParameters() << ")\n";
        
        if (recordingScript_) {
            logCommandToScript(command);
        }
    }
    
    void undo() {
        if (undoStack_.empty()) {
            std::cout << "\n[INFO] Nothing to undo\n";
            return;
        }
        
        auto command = undoStack_.top();
        undoStack_.pop();
        std::cout << "\n[UNDO] " << command->getName() 
                  << " (" << command->getParameters() << ")\n";
        command->undo();
        redoStack_.push(command);
    }
    
    void redo() {
        if (redoStack_.empty()) {
            std::cout << "\n[INFO] Nothing to redo\n";
            return;
        }
        
        auto command = redoStack_.top();
        redoStack_.pop();
        std::cout << "\n[REDO] " << command->getName() 
                  << " (" << command->getParameters() << ")\n";
        command->execute();
        undoStack_.push(command);
    }
    
    void startScriptRecording(const std::string& filename) {
        scriptFilename_ = filename;
        recordingScript_ = true;
        std::cout << "\n[RECORDING] Started recording commands to " 
                  << filename << "\n";
    }
    
    void stopScriptRecording() {
        if (recordingScript_) {
            recordingScript_ = false;
            std::cout << "\n[RECORDING] Stopped recording to " 
                      << scriptFilename_ << "\n";
        }
    }
    
    void saveCommandHistory(const std::string& filename) {
        std::cout << "\n[SAVE] Saving command history to " << filename << "\n";
        std::cout << "Total commands: " << commandHistory_.size() << "\n";
        
        // Simulate saving
        for (size_t i = 0; i < commandHistory_.size(); ++i) {
            std::cout << "  " << std::setw(3) << i+1 << ": " 
                      << commandHistory_[i]->getName() 
                      << " (" << commandHistory_[i]->getParameters() << ")\n";
        }
        std::cout << "History saved successfully\n";
    }
    
    void showStatistics() {
        std::cout << "\n=== Command Statistics ===\n";
        std::cout << "Total commands executed: " << commandHistory_.size() << "\n";
        std::cout << "Undo stack size: " << undoStack_.size() << "\n";
        std::cout << "Redo stack size: " << redoStack_.size() << "\n";
        
        // Count command types
        std::map<std::string, int> commandCounts;
        for (const auto& cmd : commandHistory_) {
            commandCounts[cmd->getName()]++;
        }
        
        std::cout << "\nCommand frequency:\n";
        for (const auto& [name, count] : commandCounts) {
            std::cout << "  " << name << ": " << count << "\n";
        }
        std::cout << "========================\n";
    }
    
private:
    void logCommandToScript(std::shared_ptr<SimulationCommand> command) {
        // In real implementation, would write to file
        std::cout << "[SCRIPT] Logged: " << command->getName() 
                  << " (" << command->getParameters() << ")\n";
    }
};

int main() {
    SimulationEngine engine;
    SimulationCommandManager manager;
    
    std::cout << "=== Scientific Simulation Command System ===\n\n";
    
    // Display initial state
    engine.displayStatus();
    
    // Start recording for reproducibility
    manager.startScriptRecording("experiment_2024_01.sim");
    
    // Basic parameter setup
    std::cout << "Setting up simulation parameters...\n";
    
    manager.executeCommand(
        std::make_shared<SetTemperatureCommand>(engine, 350.0)
    );
    
    manager.executeCommand(
        std::make_shared<SetPressureCommand>(engine, 2e5)  // 2 bar
    );
    
    manager.executeCommand(
        std::make_shared<SetTimeStepCommand>(engine, 0.5e-15)  // 0.5 fs
    );
    
    // Run initial equilibration
    manager.executeCommand(
        std::make_shared<RunSimulationCommand>(engine, 1e-12)  // 1 ps
    );
    
    engine.displayStatus();
    
    // Test undo functionality
    std::cout << "\n=== Testing Undo/Redo ===\n";
    manager.undo();  // Undo simulation run
    manager.undo();  // Undo time step change
    
    manager.redo();  // Redo time step change
    
    // Change solver
    manager.executeCommand(
        std::make_shared<ChangeSolverCommand>(engine, "Leapfrog")
    );
    
    // Save checkpoint
    manager.executeCommand(
        std::make_shared<SaveCheckpointCommand>(engine, "checkpoint_001.chk")
    );
    
    // Create a workflow for temperature scanning
    std::cout << "\n=== Creating Temperature Scan Workflow ===\n";
    auto tempScan = std::make_shared<BatchCommand>(
        "Temperature Scan",
        "Scan temperatures from 300K to 400K"
    );
    
    // Add workflow steps
    tempScan->addCommand(std::make_shared<SetTemperatureCommand>(engine, 300.0));
    tempScan->addCommand(std::make_shared<RunSimulationCommand>(engine, 1e-12));
    tempScan->addCommand(std::make_shared<SaveCheckpointCommand>(engine, "T300K.chk"));
    
    tempScan->addCommand(std::make_shared<SetTemperatureCommand>(engine, 350.0));
    tempScan->addCommand(std::make_shared<RunSimulationCommand>(engine, 1e-12));
    tempScan->addCommand(std::make_shared<SaveCheckpointCommand>(engine, "T350K.chk"));
    
    tempScan->addCommand(std::make_shared<SetTemperatureCommand>(engine, 400.0));
    tempScan->addCommand(std::make_shared<RunSimulationCommand>(engine, 1e-12));
    tempScan->addCommand(std::make_shared<SaveCheckpointCommand>(engine, "T400K.chk"));
    
    // Execute workflow
    manager.executeCommand(tempScan);
    
    // Stop recording
    manager.stopScriptRecording();
    
    // Show final status
    engine.displayStatus();
    
    // Save command history for paper supplementary
    manager.saveCommandHistory("simulation_history.log");
    
    // Show statistics
    manager.showStatistics();
    
    // Demonstrate workflow undo
    std::cout << "\n=== Undoing Entire Workflow ===\n";
    manager.undo();
    
    std::cout << "\nCommand pattern enables reproducible scientific simulations\n";
    std::cout << "with full undo/redo capability and workflow automation!\n";
    
    return 0;
}