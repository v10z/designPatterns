// Minimal Saga Pattern Implementation
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <queue>
#include <any>
#include <optional>
#include <chrono>
#include <thread>

// Example 1: Basic Saga Pattern for Order Processing
namespace BasicSaga {
    // Forward declarations
    class SagaStep;
    class SagaContext;
    
    // Saga transaction result
    enum class StepResult {
        SUCCESS,
        FAILURE,
        COMPENSATE
    };
    
    // Saga context holds shared state
    class SagaContext {
    private:
        std::unordered_map<std::string, std::any> data_;
        std::vector<std::string> completedSteps_;
        bool failed_ = false;
        
    public:
        template<typename T>
        void set(const std::string& key, const T& value) {
            data_[key] = value;
        }
        
        template<typename T>
        std::optional<T> get(const std::string& key) const {
            auto it = data_.find(key);
            if (it != data_.end()) {
                try {
                    return std::any_cast<T>(it->second);
                } catch (...) {
                    return std::nullopt;
                }
            }
            return std::nullopt;
        }
        
        void markStepCompleted(const std::string& stepName) {
            completedSteps_.push_back(stepName);
        }
        
        const std::vector<std::string>& getCompletedSteps() const {
            return completedSteps_;
        }
        
        void setFailed() { failed_ = true; }
        bool isFailed() const { return failed_; }
    };
    
    // Saga step interface
    class SagaStep {
    protected:
        std::string name_;
        
    public:
        explicit SagaStep(const std::string& name) : name_(name) {}
        virtual ~SagaStep() = default;
        
        virtual StepResult execute(SagaContext& context) = 0;
        virtual void compensate(SagaContext& context) = 0;
        
        const std::string& getName() const { return name_; }
    };
    
    // Concrete saga steps for order processing
    class ReserveInventoryStep : public SagaStep {
    public:
        ReserveInventoryStep() : SagaStep("ReserveInventory") {}
        
        StepResult execute(SagaContext& context) override {
            auto orderId = context.get<std::string>("orderId");
            auto items = context.get<int>("itemCount");
            
            std::cout << "Reserving inventory for order " << orderId.value() 
                      << " (" << items.value() << " items)\n";
            
            // Simulate inventory check
            if (items.value() > 5) {
                std::cout << "  Failed: Not enough inventory\n";
                return StepResult::FAILURE;
            }
            
            context.set("inventoryReserved", true);
            std::cout << "  Success: Inventory reserved\n";
            return StepResult::SUCCESS;
        }
        
        void compensate(SagaContext& context) override {
            auto orderId = context.get<std::string>("orderId");
            std::cout << "Compensating: Releasing inventory for order " 
                      << orderId.value() << "\n";
            context.set("inventoryReserved", false);
        }
    };
    
    class ProcessPaymentStep : public SagaStep {
    public:
        ProcessPaymentStep() : SagaStep("ProcessPayment") {}
        
        StepResult execute(SagaContext& context) override {
            auto orderId = context.get<std::string>("orderId");
            auto amount = context.get<double>("amount");
            
            std::cout << "Processing payment for order " << orderId.value() 
                      << " ($" << amount.value() << ")\n";
            
            // Simulate payment processing
            if (amount.value() > 1000) {
                std::cout << "  Failed: Payment declined\n";
                return StepResult::FAILURE;
            }
            
            context.set("paymentProcessed", true);
            context.set("transactionId", "TXN-12345");
            std::cout << "  Success: Payment processed\n";
            return StepResult::SUCCESS;
        }
        
        void compensate(SagaContext& context) override {
            auto orderId = context.get<std::string>("orderId");
            auto txnId = context.get<std::string>("transactionId");
            std::cout << "Compensating: Refunding payment for order " 
                      << orderId.value();
            if (txnId) {
                std::cout << " (Transaction: " << txnId.value() << ")";
            }
            std::cout << "\n";
            context.set("paymentProcessed", false);
        }
    };
    
    class CreateShipmentStep : public SagaStep {
    public:
        CreateShipmentStep() : SagaStep("CreateShipment") {}
        
        StepResult execute(SagaContext& context) override {
            auto orderId = context.get<std::string>("orderId");
            
            std::cout << "Creating shipment for order " << orderId.value() << "\n";
            
            // Simulate shipment creation
            context.set("shipmentId", "SHIP-67890");
            context.set("shipmentCreated", true);
            std::cout << "  Success: Shipment created\n";
            return StepResult::SUCCESS;
        }
        
        void compensate(SagaContext& context) override {
            auto orderId = context.get<std::string>("orderId");
            auto shipId = context.get<std::string>("shipmentId");
            std::cout << "Compensating: Cancelling shipment for order " 
                      << orderId.value();
            if (shipId) {
                std::cout << " (Shipment: " << shipId.value() << ")";
            }
            std::cout << "\n";
            context.set("shipmentCreated", false);
        }
    };
    
    // Saga orchestrator
    class SagaOrchestrator {
    private:
        std::vector<std::unique_ptr<SagaStep>> steps_;
        
    public:
        void addStep(std::unique_ptr<SagaStep> step) {
            steps_.push_back(std::move(step));
        }
        
        bool execute(SagaContext& context) {
            std::cout << "\nStarting saga execution...\n";
            std::cout << "==========================\n";
            
            // Execute each step
            for (auto& step : steps_) {
                std::cout << "\nExecuting step: " << step->getName() << "\n";
                
                StepResult result = step->execute(context);
                
                if (result == StepResult::SUCCESS) {
                    context.markStepCompleted(step->getName());
                } else {
                    std::cout << "\nStep failed! Starting compensation...\n";
                    std::cout << "===================================\n";
                    context.setFailed();
                    compensate(context);
                    return false;
                }
            }
            
            std::cout << "\nSaga completed successfully!\n";
            return true;
        }
        
    private:
        void compensate(SagaContext& context) {
            const auto& completedSteps = context.getCompletedSteps();
            
            // Compensate in reverse order
            for (auto it = completedSteps.rbegin(); it != completedSteps.rend(); ++it) {
                for (auto& step : steps_) {
                    if (step->getName() == *it) {
                        std::cout << "\nCompensating step: " << *it << "\n";
                        step->compensate(context);
                        break;
                    }
                }
            }
            
            std::cout << "\nCompensation completed\n";
        }
    };
}

// Example 2: Choreography-Based Saga
namespace ChoreographySaga {
    // Event types
    enum class EventType {
        ORDER_PLACED,
        INVENTORY_RESERVED,
        INVENTORY_FAILED,
        PAYMENT_PROCESSED,
        PAYMENT_FAILED,
        SHIPMENT_CREATED,
        SHIPMENT_FAILED,
        ORDER_COMPLETED,
        ORDER_CANCELLED
    };
    
    struct Event {
        EventType type;
        std::string orderId;
        std::unordered_map<std::string, std::any> data;
        
        Event(EventType t, const std::string& id) : type(t), orderId(id) {}
    };
    
    // Event bus for choreography
    class EventBus {
    private:
        using Handler = std::function<void(const Event&)>;
        std::unordered_map<EventType, std::vector<Handler>> handlers_;
        std::queue<Event> eventQueue_;
        
    public:
        void subscribe(EventType type, Handler handler) {
            handlers_[type].push_back(handler);
        }
        
        void publish(const Event& event) {
            eventQueue_.push(event);
        }
        
        void processEvents() {
            while (!eventQueue_.empty()) {
                Event event = eventQueue_.front();
                eventQueue_.pop();
                
                std::cout << "Processing event: " << static_cast<int>(event.type) 
                          << " for order " << event.orderId << "\n";
                
                auto it = handlers_.find(event.type);
                if (it != handlers_.end()) {
                    for (const auto& handler : it->second) {
                        handler(event);
                    }
                }
            }
        }
    };
    
    // Service participants in choreography
    class InventoryService {
    private:
        EventBus* eventBus_;
        
    public:
        explicit InventoryService(EventBus* bus) : eventBus_(bus) {
            // Subscribe to relevant events
            bus->subscribe(EventType::ORDER_PLACED, 
                [this](const Event& e) { handleOrderPlaced(e); });
            bus->subscribe(EventType::ORDER_CANCELLED,
                [this](const Event& e) { handleOrderCancelled(e); });
        }
        
        void handleOrderPlaced(const Event& event) {
            std::cout << "  InventoryService: Checking inventory for order " 
                      << event.orderId << "\n";
            
            // Simulate inventory check
            auto items = event.data.find("items");
            if (items != event.data.end()) {
                int count = std::any_cast<int>(items->second);
                if (count <= 3) {
                    std::cout << "    Inventory available, reserving...\n";
                    Event success(EventType::INVENTORY_RESERVED, event.orderId);
                    eventBus_->publish(success);
                } else {
                    std::cout << "    Insufficient inventory!\n";
                    Event failure(EventType::INVENTORY_FAILED, event.orderId);
                    eventBus_->publish(failure);
                }
            }
        }
        
        void handleOrderCancelled(const Event& event) {
            std::cout << "  InventoryService: Releasing inventory for cancelled order " 
                      << event.orderId << "\n";
        }
    };
    
    class PaymentService {
    private:
        EventBus* eventBus_;
        
    public:
        explicit PaymentService(EventBus* bus) : eventBus_(bus) {
            bus->subscribe(EventType::INVENTORY_RESERVED,
                [this](const Event& e) { handleInventoryReserved(e); });
            bus->subscribe(EventType::ORDER_CANCELLED,
                [this](const Event& e) { handleOrderCancelled(e); });
        }
        
        void handleInventoryReserved(const Event& event) {
            std::cout << "  PaymentService: Processing payment for order " 
                      << event.orderId << "\n";
            
            // Always succeed for demo
            std::cout << "    Payment successful\n";
            Event success(EventType::PAYMENT_PROCESSED, event.orderId);
            eventBus_->publish(success);
        }
        
        void handleOrderCancelled(const Event& event) {
            std::cout << "  PaymentService: Refunding payment for cancelled order " 
                      << event.orderId << "\n";
        }
    };
    
    class ShippingService {
    private:
        EventBus* eventBus_;
        
    public:
        explicit ShippingService(EventBus* bus) : eventBus_(bus) {
            bus->subscribe(EventType::PAYMENT_PROCESSED,
                [this](const Event& e) { handlePaymentProcessed(e); });
        }
        
        void handlePaymentProcessed(const Event& event) {
            std::cout << "  ShippingService: Creating shipment for order " 
                      << event.orderId << "\n";
            
            Event success(EventType::SHIPMENT_CREATED, event.orderId);
            eventBus_->publish(success);
            
            // Order complete
            Event complete(EventType::ORDER_COMPLETED, event.orderId);
            eventBus_->publish(complete);
        }
    };
    
    // Saga coordinator monitors and handles failures
    class SagaCoordinator {
    private:
        EventBus* eventBus_;
        
    public:
        explicit SagaCoordinator(EventBus* bus) : eventBus_(bus) {
            // Monitor failure events
            bus->subscribe(EventType::INVENTORY_FAILED,
                [this](const Event& e) { handleFailure(e); });
            bus->subscribe(EventType::PAYMENT_FAILED,
                [this](const Event& e) { handleFailure(e); });
            bus->subscribe(EventType::SHIPMENT_FAILED,
                [this](const Event& e) { handleFailure(e); });
        }
        
        void handleFailure(const Event& event) {
            std::cout << "\n  SagaCoordinator: Handling failure for order " 
                      << event.orderId << "\n";
            std::cout << "  Initiating compensation...\n";
            
            Event cancelEvent(EventType::ORDER_CANCELLED, event.orderId);
            eventBus_->publish(cancelEvent);
        }
    };
}

// Example 3: Saga with Timeout and Retry
namespace TimeoutSaga {
    class TimedSagaStep {
    private:
        std::string name_;
        std::chrono::milliseconds timeout_;
        int maxRetries_;
        
    public:
        TimedSagaStep(const std::string& name, 
                      std::chrono::milliseconds timeout,
                      int maxRetries = 3)
            : name_(name), timeout_(timeout), maxRetries_(maxRetries) {}
        
        struct ExecutionResult {
            bool success;
            std::string error;
            std::chrono::milliseconds duration;
        };
        
        ExecutionResult executeWithTimeout(
            std::function<bool()> operation,
            std::function<void()> compensation) {
            
            for (int attempt = 1; attempt <= maxRetries_; ++attempt) {
                std::cout << "Executing " << name_ << " (attempt " 
                          << attempt << "/" << maxRetries_ << ")\n";
                
                auto start = std::chrono::steady_clock::now();
                
                // Simulate timeout check
                bool completed = operation();
                
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                                (end - start);
                
                if (completed && duration <= timeout_) {
                    std::cout << "  Success in " << duration.count() << "ms\n";
                    return {true, "", duration};
                } else if (duration > timeout_) {
                    std::cout << "  Timeout after " << duration.count() << "ms\n";
                    if (attempt < maxRetries_) {
                        std::cout << "  Retrying...\n";
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                } else {
                    std::cout << "  Failed\n";
                    if (attempt < maxRetries_) {
                        std::cout << "  Retrying...\n";
                    }
                }
            }
            
            std::cout << "  All attempts failed. Compensating...\n";
            compensation();
            return {false, "Max retries exceeded", timeout_};
        }
        
        const std::string& getName() const { return name_; }
    };
}

// Example 4: Distributed Saga Pattern
namespace DistributedSaga {
    // Saga log for durability
    class SagaLog {
    public:
        enum class LogType {
            SAGA_STARTED,
            STEP_STARTED,
            STEP_COMPLETED,
            STEP_FAILED,
            COMPENSATION_STARTED,
            COMPENSATION_COMPLETED,
            SAGA_COMPLETED,
            SAGA_ABORTED
        };
        
        struct LogEntry {
            std::string sagaId;
            LogType type;
            std::string stepName;
            std::chrono::system_clock::time_point timestamp;
            std::unordered_map<std::string, std::string> metadata;
        };
        
    private:
        std::vector<LogEntry> entries_;
        
    public:
        void log(const std::string& sagaId, LogType type, 
                const std::string& stepName = "",
                const std::unordered_map<std::string, std::string>& metadata = {}) {
            
            entries_.push_back({
                sagaId, type, stepName, 
                std::chrono::system_clock::now(), metadata
            });
            
            std::cout << "[SagaLog] " << sagaId << " - "
                      << logTypeToString(type);
            if (!stepName.empty()) {
                std::cout << " - " << stepName;
            }
            std::cout << "\n";
        }
        
        std::vector<LogEntry> getEntriesForSaga(const std::string& sagaId) const {
            std::vector<LogEntry> result;
            for (const auto& entry : entries_) {
                if (entry.sagaId == sagaId) {
                    result.push_back(entry);
                }
            }
            return result;
        }
        
    private:
        std::string logTypeToString(LogType type) const {
            switch (type) {
                case LogType::SAGA_STARTED: return "SAGA_STARTED";
                case LogType::STEP_STARTED: return "STEP_STARTED";
                case LogType::STEP_COMPLETED: return "STEP_COMPLETED";
                case LogType::STEP_FAILED: return "STEP_FAILED";
                case LogType::COMPENSATION_STARTED: return "COMPENSATION_STARTED";
                case LogType::COMPENSATION_COMPLETED: return "COMPENSATION_COMPLETED";
                case LogType::SAGA_COMPLETED: return "SAGA_COMPLETED";
                case LogType::SAGA_ABORTED: return "SAGA_ABORTED";
                default: return "UNKNOWN";
            }
        }
    };
    
    // Distributed saga coordinator
    class DistributedSagaCoordinator {
    private:
        SagaLog sagaLog_;
        std::string sagaId_;
        
    public:
        explicit DistributedSagaCoordinator(const std::string& sagaId) 
            : sagaId_(sagaId) {}
        
        void executeSaga(const std::vector<std::function<bool()>>& steps,
                        const std::vector<std::function<void()>>& compensations) {
            
            sagaLog_.log(sagaId_, SagaLog::LogType::SAGA_STARTED);
            
            size_t completedSteps = 0;
            bool failed = false;
            
            // Execute steps
            for (size_t i = 0; i < steps.size(); ++i) {
                std::string stepName = "Step" + std::to_string(i + 1);
                sagaLog_.log(sagaId_, SagaLog::LogType::STEP_STARTED, stepName);
                
                if (steps[i]()) {
                    sagaLog_.log(sagaId_, SagaLog::LogType::STEP_COMPLETED, stepName);
                    completedSteps++;
                } else {
                    sagaLog_.log(sagaId_, SagaLog::LogType::STEP_FAILED, stepName);
                    failed = true;
                    break;
                }
            }
            
            if (failed) {
                // Compensate completed steps
                sagaLog_.log(sagaId_, SagaLog::LogType::COMPENSATION_STARTED);
                
                for (int i = completedSteps - 1; i >= 0; --i) {
                    std::string stepName = "Step" + std::to_string(i + 1);
                    compensations[i]();
                    sagaLog_.log(sagaId_, SagaLog::LogType::COMPENSATION_COMPLETED, 
                                stepName);
                }
                
                sagaLog_.log(sagaId_, SagaLog::LogType::SAGA_ABORTED);
            } else {
                sagaLog_.log(sagaId_, SagaLog::LogType::SAGA_COMPLETED);
            }
        }
        
        void printSagaHistory() const {
            std::cout << "\nSaga History for " << sagaId_ << ":\n";
            std::cout << "================================\n";
            
            auto entries = sagaLog_.getEntriesForSaga(sagaId_);
            for (const auto& entry : entries) {
                auto time = std::chrono::system_clock::to_time_t(entry.timestamp);
                std::cout << std::put_time(std::localtime(&time), "%H:%M:%S")
                          << " - " << entry.stepName << "\n";
            }
        }
    };
}

// Demo functions
void demonstrateBasicSaga() {
    using namespace BasicSaga;
    
    std::cout << "=== Basic Saga Pattern (Orchestration) ===\n";
    
    // Successful saga
    {
        std::cout << "\n--- Successful Order Processing ---\n";
        SagaContext context;
        context.set("orderId", std::string("ORDER-001"));
        context.set("itemCount", 3);
        context.set("amount", 150.0);
        
        SagaOrchestrator saga;
        saga.addStep(std::make_unique<ReserveInventoryStep>());
        saga.addStep(std::make_unique<ProcessPaymentStep>());
        saga.addStep(std::make_unique<CreateShipmentStep>());
        
        saga.execute(context);
    }
    
    // Failed saga with compensation
    {
        std::cout << "\n--- Failed Order Processing (Payment Failure) ---\n";
        SagaContext context;
        context.set("orderId", std::string("ORDER-002"));
        context.set("itemCount", 2);
        context.set("amount", 1500.0); // Will fail payment
        
        SagaOrchestrator saga;
        saga.addStep(std::make_unique<ReserveInventoryStep>());
        saga.addStep(std::make_unique<ProcessPaymentStep>());
        saga.addStep(std::make_unique<CreateShipmentStep>());
        
        saga.execute(context);
    }
}

void demonstrateChoreographySaga() {
    using namespace ChoreographySaga;
    
    std::cout << "\n=== Choreography-Based Saga ===\n";
    
    EventBus eventBus;
    
    // Create services
    InventoryService inventory(&eventBus);
    PaymentService payment(&eventBus);
    ShippingService shipping(&eventBus);
    SagaCoordinator coordinator(&eventBus);
    
    // Successful order
    {
        std::cout << "\n--- Successful Order Flow ---\n";
        Event orderPlaced(EventType::ORDER_PLACED, "ORDER-101");
        orderPlaced.data["items"] = 2;
        eventBus.publish(orderPlaced);
        eventBus.processEvents();
    }
    
    // Failed order
    {
        std::cout << "\n--- Failed Order Flow (Insufficient Inventory) ---\n";
        Event orderPlaced(EventType::ORDER_PLACED, "ORDER-102");
        orderPlaced.data["items"] = 10; // Will fail inventory check
        eventBus.publish(orderPlaced);
        eventBus.processEvents();
    }
}

void demonstrateTimeoutSaga() {
    using namespace TimeoutSaga;
    
    std::cout << "\n=== Saga with Timeout and Retry ===\n";
    
    // API call with retry
    {
        std::cout << "\n--- External API Call with Retry ---\n";
        TimedSagaStep apiStep("External API Call", 
                             std::chrono::milliseconds(500), 3);
        
        int attemptCount = 0;
        auto result = apiStep.executeWithTimeout(
            [&attemptCount]() {
                attemptCount++;
                // Fail first 2 attempts
                if (attemptCount < 3) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(600)); // Timeout
                    return false;
                }
                return true;
            },
            []() {
                std::cout << "  Compensating: Logging API failure\n";
            }
        );
        
        if (result.success) {
            std::cout << "API call eventually succeeded!\n";
        }
    }
    
    // Database operation with timeout
    {
        std::cout << "\n--- Database Operation with Timeout ---\n";
        TimedSagaStep dbStep("Database Update", 
                            std::chrono::milliseconds(100), 2);
        
        auto result = dbStep.executeWithTimeout(
            []() {
                // Always timeout
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(200));
                return true;
            },
            []() {
                std::cout << "  Compensating: Rolling back database changes\n";
            }
        );
    }
}

void demonstrateDistributedSaga() {
    using namespace DistributedSaga;
    
    std::cout << "\n=== Distributed Saga with Logging ===\n";
    
    // Successful distributed transaction
    {
        std::cout << "\n--- Successful Distributed Transaction ---\n";
        DistributedSagaCoordinator coordinator("SAGA-001");
        
        std::vector<std::function<bool()>> steps = {
            []() { 
                std::cout << "  Updating user account...\n";
                return true; 
            },
            []() { 
                std::cout << "  Charging credit card...\n";
                return true; 
            },
            []() { 
                std::cout << "  Sending confirmation email...\n";
                return true; 
            }
        };
        
        std::vector<std::function<void()>> compensations = {
            []() { std::cout << "  Reverting user account update\n"; },
            []() { std::cout << "  Refunding credit card\n"; },
            []() { std::cout << "  Sending cancellation email\n"; }
        };
        
        coordinator.executeSaga(steps, compensations);
        coordinator.printSagaHistory();
    }
    
    // Failed distributed transaction
    {
        std::cout << "\n--- Failed Distributed Transaction ---\n";
        DistributedSagaCoordinator coordinator("SAGA-002");
        
        std::vector<std::function<bool()>> steps = {
            []() { 
                std::cout << "  Creating order record...\n";
                return true; 
            },
            []() { 
                std::cout << "  Reserving items...\n";
                return true; 
            },
            []() { 
                std::cout << "  Processing payment...\n";
                std::cout << "    Payment gateway timeout!\n";
                return false; // Fail here
            },
            []() { 
                std::cout << "  Scheduling delivery...\n";
                return true; 
            }
        };
        
        std::vector<std::function<void()>> compensations = {
            []() { std::cout << "  Deleting order record\n"; },
            []() { std::cout << "  Releasing reserved items\n"; },
            []() { std::cout << "  Cancelling payment authorization\n"; },
            []() { std::cout << "  Cancelling delivery\n"; }
        };
        
        coordinator.executeSaga(steps, compensations);
        coordinator.printSagaHistory();
    }
}

int main() {
    std::cout << "=== Saga Pattern Demo ===\n\n";
    
    demonstrateBasicSaga();
    demonstrateChoreographySaga();
    demonstrateTimeoutSaga();
    demonstrateDistributedSaga();
    
    std::cout << "\n=== Saga Pattern Benefits ===\n";
    std::cout << "1. Manages distributed transactions\n";
    std::cout << "2. Provides compensation mechanism\n";
    std::cout << "3. Maintains data consistency\n";
    std::cout << "4. Handles partial failures\n";
    std::cout << "5. Supports long-running transactions\n";
    
    return 0;
}