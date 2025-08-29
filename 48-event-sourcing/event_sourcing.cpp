// Minimal Event Sourcing Pattern Implementation
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <functional>
#include <any>
#include <typeindex>
#include <algorithm>
#include <sstream>
#include <iomanip>

// Example 1: Basic Event Sourcing for Bank Account
namespace BasicEventSourcing {
    // Base Event class
    class Event {
    protected:
        std::string aggregateId_;
        int version_;
        std::chrono::system_clock::time_point timestamp_;
        
    public:
        Event(const std::string& aggregateId, int version)
            : aggregateId_(aggregateId), version_(version),
              timestamp_(std::chrono::system_clock::now()) {}
        
        virtual ~Event() = default;
        
        const std::string& getAggregateId() const { return aggregateId_; }
        int getVersion() const { return version_; }
        std::chrono::system_clock::time_point getTimestamp() const { return timestamp_; }
        
        virtual std::string getEventType() const = 0;
        virtual std::string toString() const = 0;
    };
    
    // Concrete Events
    class AccountCreatedEvent : public Event {
    private:
        std::string accountHolder_;
        double initialBalance_;
        
    public:
        AccountCreatedEvent(const std::string& aggregateId, int version,
                           const std::string& holder, double balance)
            : Event(aggregateId, version), 
              accountHolder_(holder), initialBalance_(balance) {}
        
        std::string getEventType() const override { return "AccountCreated"; }
        
        std::string toString() const override {
            std::stringstream ss;
            ss << "AccountCreated: " << accountHolder_ 
               << " with balance $" << initialBalance_;
            return ss.str();
        }
        
        const std::string& getAccountHolder() const { return accountHolder_; }
        double getInitialBalance() const { return initialBalance_; }
    };
    
    class MoneyDepositedEvent : public Event {
    private:
        double amount_;
        std::string description_;
        
    public:
        MoneyDepositedEvent(const std::string& aggregateId, int version,
                           double amount, const std::string& desc)
            : Event(aggregateId, version), amount_(amount), description_(desc) {}
        
        std::string getEventType() const override { return "MoneyDeposited"; }
        
        std::string toString() const override {
            std::stringstream ss;
            ss << "MoneyDeposited: $" << amount_ << " - " << description_;
            return ss.str();
        }
        
        double getAmount() const { return amount_; }
    };
    
    class MoneyWithdrawnEvent : public Event {
    private:
        double amount_;
        std::string description_;
        
    public:
        MoneyWithdrawnEvent(const std::string& aggregateId, int version,
                           double amount, const std::string& desc)
            : Event(aggregateId, version), amount_(amount), description_(desc) {}
        
        std::string getEventType() const override { return "MoneyWithdrawn"; }
        
        std::string toString() const override {
            std::stringstream ss;
            ss << "MoneyWithdrawn: $" << amount_ << " - " << description_;
            return ss.str();
        }
        
        double getAmount() const { return amount_; }
    };
    
    // Event Store
    class EventStore {
    private:
        std::unordered_map<std::string, std::vector<std::shared_ptr<Event>>> events_;
        
    public:
        void append(std::shared_ptr<Event> event) {
            events_[event->getAggregateId()].push_back(event);
            
            std::cout << "Event stored: " << event->toString() 
                      << " (v" << event->getVersion() << ")\n";
        }
        
        std::vector<std::shared_ptr<Event>> getEvents(const std::string& aggregateId) {
            auto it = events_.find(aggregateId);
            if (it != events_.end()) {
                return it->second;
            }
            return {};
        }
        
        std::vector<std::shared_ptr<Event>> getAllEvents() {
            std::vector<std::shared_ptr<Event>> allEvents;
            for (const auto& [id, events] : events_) {
                allEvents.insert(allEvents.end(), events.begin(), events.end());
            }
            
            // Sort by timestamp
            std::sort(allEvents.begin(), allEvents.end(),
                [](const auto& a, const auto& b) {
                    return a->getTimestamp() < b->getTimestamp();
                });
            
            return allEvents;
        }
    };
    
    // Aggregate Root
    class BankAccount {
    private:
        std::string id_;
        std::string accountHolder_;
        double balance_;
        int version_;
        std::vector<std::shared_ptr<Event>> uncommittedEvents_;
        
        void apply(std::shared_ptr<AccountCreatedEvent> event) {
            accountHolder_ = event->getAccountHolder();
            balance_ = event->getInitialBalance();
        }
        
        void apply(std::shared_ptr<MoneyDepositedEvent> event) {
            balance_ += event->getAmount();
        }
        
        void apply(std::shared_ptr<MoneyWithdrawnEvent> event) {
            balance_ -= event->getAmount();
        }
        
    public:
        BankAccount(const std::string& id) : id_(id), balance_(0), version_(0) {}
        
        // Command handlers
        void create(const std::string& holder, double initialBalance) {
            if (version_ > 0) {
                throw std::runtime_error("Account already created");
            }
            
            auto event = std::make_shared<AccountCreatedEvent>(
                id_, ++version_, holder, initialBalance);
            applyEvent(event);
            uncommittedEvents_.push_back(event);
        }
        
        void deposit(double amount, const std::string& description) {
            if (version_ == 0) {
                throw std::runtime_error("Account not created");
            }
            if (amount <= 0) {
                throw std::invalid_argument("Amount must be positive");
            }
            
            auto event = std::make_shared<MoneyDepositedEvent>(
                id_, ++version_, amount, description);
            applyEvent(event);
            uncommittedEvents_.push_back(event);
        }
        
        void withdraw(double amount, const std::string& description) {
            if (version_ == 0) {
                throw std::runtime_error("Account not created");
            }
            if (amount <= 0) {
                throw std::invalid_argument("Amount must be positive");
            }
            if (amount > balance_) {
                throw std::runtime_error("Insufficient funds");
            }
            
            auto event = std::make_shared<MoneyWithdrawnEvent>(
                id_, ++version_, amount, description);
            applyEvent(event);
            uncommittedEvents_.push_back(event);
        }
        
        // Apply event polymorphically
        void applyEvent(std::shared_ptr<Event> event) {
            if (auto e = std::dynamic_pointer_cast<AccountCreatedEvent>(event)) {
                apply(e);
            } else if (auto e = std::dynamic_pointer_cast<MoneyDepositedEvent>(event)) {
                apply(e);
            } else if (auto e = std::dynamic_pointer_cast<MoneyWithdrawnEvent>(event)) {
                apply(e);
            }
        }
        
        // Load from event stream
        void loadFromHistory(const std::vector<std::shared_ptr<Event>>& events) {
            for (const auto& event : events) {
                applyEvent(event);
                version_ = event->getVersion();
            }
        }
        
        // Get uncommitted events
        std::vector<std::shared_ptr<Event>> getUncommittedEvents() {
            return uncommittedEvents_;
        }
        
        void markEventsAsCommitted() {
            uncommittedEvents_.clear();
        }
        
        // Getters
        double getBalance() const { return balance_; }
        const std::string& getAccountHolder() const { return accountHolder_; }
        int getVersion() const { return version_; }
    };
}

// Example 2: Event Sourcing with Snapshots
namespace SnapshotEventSourcing {
    // Snapshot class
    class Snapshot {
    private:
        std::string aggregateId_;
        int version_;
        std::any state_;
        std::chrono::system_clock::time_point timestamp_;
        
    public:
        Snapshot(const std::string& id, int version, std::any state)
            : aggregateId_(id), version_(version), state_(state),
              timestamp_(std::chrono::system_clock::now()) {}
        
        const std::string& getAggregateId() const { return aggregateId_; }
        int getVersion() const { return version_; }
        const std::any& getState() const { return state_; }
    };
    
    // Shopping Cart Events
    class ItemAddedEvent {
    public:
        std::string itemId;
        std::string itemName;
        double price;
        int quantity;
        
        ItemAddedEvent(const std::string& id, const std::string& name, 
                      double p, int q)
            : itemId(id), itemName(name), price(p), quantity(q) {}
    };
    
    class ItemRemovedEvent {
    public:
        std::string itemId;
        int quantity;
        
        ItemRemovedEvent(const std::string& id, int q)
            : itemId(id), quantity(q) {}
    };
    
    // Shopping Cart State
    struct CartItem {
        std::string itemId;
        std::string itemName;
        double price;
        int quantity;
    };
    
    struct CartState {
        std::unordered_map<std::string, CartItem> items;
        double totalAmount = 0;
        
        void recalculateTotal() {
            totalAmount = 0;
            for (const auto& [id, item] : items) {
                totalAmount += item.price * item.quantity;
            }
        }
    };
    
    // Shopping Cart Aggregate
    class ShoppingCart {
    private:
        std::string id_;
        CartState state_;
        int version_ = 0;
        std::vector<std::any> events_;
        
    public:
        explicit ShoppingCart(const std::string& id) : id_(id) {}
        
        void addItem(const std::string& itemId, const std::string& itemName,
                    double price, int quantity) {
            ItemAddedEvent event(itemId, itemName, price, quantity);
            applyEvent(event);
            events_.push_back(event);
            version_++;
        }
        
        void removeItem(const std::string& itemId, int quantity) {
            if (state_.items.find(itemId) == state_.items.end()) {
                throw std::runtime_error("Item not in cart");
            }
            
            ItemRemovedEvent event(itemId, quantity);
            applyEvent(event);
            events_.push_back(event);
            version_++;
        }
        
        void applyEvent(const std::any& event) {
            if (event.type() == typeid(ItemAddedEvent)) {
                auto e = std::any_cast<ItemAddedEvent>(event);
                auto& item = state_.items[e.itemId];
                item.itemId = e.itemId;
                item.itemName = e.itemName;
                item.price = e.price;
                item.quantity += e.quantity;
            } else if (event.type() == typeid(ItemRemovedEvent)) {
                auto e = std::any_cast<ItemRemovedEvent>(event);
                auto& item = state_.items[e.itemId];
                item.quantity -= e.quantity;
                if (item.quantity <= 0) {
                    state_.items.erase(e.itemId);
                }
            }
            
            state_.recalculateTotal();
        }
        
        // Create snapshot
        Snapshot createSnapshot() {
            return Snapshot(id_, version_, state_);
        }
        
        // Restore from snapshot
        void restoreFromSnapshot(const Snapshot& snapshot) {
            state_ = std::any_cast<CartState>(snapshot.getState());
            version_ = snapshot.getVersion();
        }
        
        // Getters
        const CartState& getState() const { return state_; }
        int getVersion() const { return version_; }
        
        void printCart() const {
            std::cout << "\nShopping Cart (v" << version_ << "):\n";
            for (const auto& [id, item] : state_.items) {
                std::cout << "  " << item.itemName << " - $" << item.price 
                          << " x " << item.quantity << " = $" 
                          << (item.price * item.quantity) << "\n";
            }
            std::cout << "Total: $" << state_.totalAmount << "\n";
        }
    };
}

// Example 3: Event Sourcing with Projections
namespace ProjectionEventSourcing {
    // Order Events
    class OrderEvent {
    public:
        std::string orderId;
        std::chrono::system_clock::time_point timestamp;
        
        OrderEvent(const std::string& id) 
            : orderId(id), timestamp(std::chrono::system_clock::now()) {}
        virtual ~OrderEvent() = default;
    };
    
    class OrderPlacedEvent : public OrderEvent {
    public:
        std::string customerId;
        std::vector<std::pair<std::string, int>> items;
        
        OrderPlacedEvent(const std::string& orderId, const std::string& custId)
            : OrderEvent(orderId), customerId(custId) {}
    };
    
    class OrderShippedEvent : public OrderEvent {
    public:
        std::string trackingNumber;
        
        OrderShippedEvent(const std::string& orderId, const std::string& tracking)
            : OrderEvent(orderId), trackingNumber(tracking) {}
    };
    
    class OrderDeliveredEvent : public OrderEvent {
    public:
        OrderDeliveredEvent(const std::string& orderId) : OrderEvent(orderId) {}
    };
    
    // Projections
    class OrderStatusProjection {
    private:
        struct OrderStatus {
            std::string orderId;
            std::string customerId;
            std::string status;
            std::string trackingNumber;
        };
        
        std::unordered_map<std::string, OrderStatus> orders_;
        
    public:
        void apply(std::shared_ptr<OrderEvent> event) {
            if (auto e = std::dynamic_pointer_cast<OrderPlacedEvent>(event)) {
                orders_[e->orderId] = {
                    e->orderId, e->customerId, "Placed", ""
                };
            } else if (auto e = std::dynamic_pointer_cast<OrderShippedEvent>(event)) {
                if (orders_.find(e->orderId) != orders_.end()) {
                    orders_[e->orderId].status = "Shipped";
                    orders_[e->orderId].trackingNumber = e->trackingNumber;
                }
            } else if (auto e = std::dynamic_pointer_cast<OrderDeliveredEvent>(event)) {
                if (orders_.find(e->orderId) != orders_.end()) {
                    orders_[e->orderId].status = "Delivered";
                }
            }
        }
        
        void printOrderStatuses() const {
            std::cout << "\nOrder Status Projection:\n";
            for (const auto& [id, status] : orders_) {
                std::cout << "  Order " << status.orderId 
                          << " (Customer: " << status.customerId << ")"
                          << " - Status: " << status.status;
                if (!status.trackingNumber.empty()) {
                    std::cout << " (Tracking: " << status.trackingNumber << ")";
                }
                std::cout << "\n";
            }
        }
    };
    
    class CustomerOrderCountProjection {
    private:
        std::unordered_map<std::string, int> orderCounts_;
        
    public:
        void apply(std::shared_ptr<OrderEvent> event) {
            if (auto e = std::dynamic_pointer_cast<OrderPlacedEvent>(event)) {
                orderCounts_[e->customerId]++;
            }
        }
        
        void printCustomerOrderCounts() const {
            std::cout << "\nCustomer Order Count Projection:\n";
            for (const auto& [customerId, count] : orderCounts_) {
                std::cout << "  Customer " << customerId 
                          << ": " << count << " orders\n";
            }
        }
    };
}

// Example 4: Event Store with Replay
namespace EventReplay {
    class AuditLog {
    private:
        struct AuditEvent {
            std::string userId;
            std::string action;
            std::string resource;
            std::chrono::system_clock::time_point timestamp;
            bool success;
        };
        
        std::vector<AuditEvent> events_;
        
    public:
        void recordEvent(const std::string& userId, const std::string& action,
                        const std::string& resource, bool success) {
            events_.push_back({
                userId, action, resource, 
                std::chrono::system_clock::now(), success
            });
        }
        
        // Replay events with time travel
        void replayUntil(std::chrono::system_clock::time_point pointInTime) {
            std::cout << "\nReplaying events until specified time:\n";
            
            for (const auto& event : events_) {
                if (event.timestamp <= pointInTime) {
                    auto time_t = std::chrono::system_clock::to_time_t(event.timestamp);
                    std::cout << "  [" << std::put_time(std::localtime(&time_t), "%H:%M:%S") 
                              << "] User " << event.userId << " " << event.action 
                              << " " << event.resource 
                              << " - " << (event.success ? "SUCCESS" : "FAILED") << "\n";
                }
            }
        }
        
        // Query events
        std::vector<AuditEvent> getUserEvents(const std::string& userId) {
            std::vector<AuditEvent> userEvents;
            std::copy_if(events_.begin(), events_.end(), 
                        std::back_inserter(userEvents),
                        [&userId](const AuditEvent& e) { 
                            return e.userId == userId; 
                        });
            return userEvents;
        }
    };
}

// Demo functions
void demonstrateBasicEventSourcing() {
    using namespace BasicEventSourcing;
    
    std::cout << "=== Basic Event Sourcing ===\n";
    
    EventStore store;
    
    // Create account
    BankAccount account("ACC001");
    account.create("John Doe", 1000.0);
    
    // Perform operations
    account.deposit(500.0, "Salary");
    account.withdraw(200.0, "Groceries");
    account.deposit(100.0, "Gift");
    account.withdraw(50.0, "Gas");
    
    // Save events to store
    for (const auto& event : account.getUncommittedEvents()) {
        store.append(event);
    }
    account.markEventsAsCommitted();
    
    std::cout << "\nCurrent balance: $" << account.getBalance() << "\n";
    
    // Rebuild account from events
    std::cout << "\nRebuilding account from event store:\n";
    BankAccount rebuiltAccount("ACC001");
    auto events = store.getEvents("ACC001");
    rebuiltAccount.loadFromHistory(events);
    
    std::cout << "Rebuilt account balance: $" << rebuiltAccount.getBalance() << "\n";
    std::cout << "Account holder: " << rebuiltAccount.getAccountHolder() << "\n";
    std::cout << "Version: " << rebuiltAccount.getVersion() << "\n";
}

void demonstrateSnapshotEventSourcing() {
    using namespace SnapshotEventSourcing;
    
    std::cout << "\n=== Event Sourcing with Snapshots ===\n";
    
    ShoppingCart cart("CART001");
    
    // Add items
    cart.addItem("ITEM1", "Laptop", 999.99, 1);
    cart.addItem("ITEM2", "Mouse", 29.99, 2);
    cart.addItem("ITEM3", "Keyboard", 79.99, 1);
    cart.printCart();
    
    // Create snapshot
    auto snapshot = cart.createSnapshot();
    std::cout << "\nSnapshot created at version " << snapshot.getVersion() << "\n";
    
    // Continue adding items
    cart.addItem("ITEM4", "Monitor", 299.99, 1);
    cart.removeItem("ITEM2", 1);
    cart.printCart();
    
    // Restore from snapshot
    std::cout << "\nRestoring from snapshot:\n";
    ShoppingCart restoredCart("CART001");
    restoredCart.restoreFromSnapshot(snapshot);
    restoredCart.printCart();
}

void demonstrateProjections() {
    using namespace ProjectionEventSourcing;
    
    std::cout << "\n=== Event Sourcing with Projections ===\n";
    
    std::vector<std::shared_ptr<OrderEvent>> eventStream;
    
    // Create events
    auto order1 = std::make_shared<OrderPlacedEvent>("ORD001", "CUST001");
    auto order2 = std::make_shared<OrderPlacedEvent>("ORD002", "CUST002");
    auto order3 = std::make_shared<OrderPlacedEvent>("ORD003", "CUST001");
    
    auto ship1 = std::make_shared<OrderShippedEvent>("ORD001", "TRACK123");
    auto ship2 = std::make_shared<OrderShippedEvent>("ORD003", "TRACK456");
    
    auto deliver1 = std::make_shared<OrderDeliveredEvent>("ORD001");
    
    eventStream.push_back(order1);
    eventStream.push_back(order2);
    eventStream.push_back(order3);
    eventStream.push_back(ship1);
    eventStream.push_back(ship2);
    eventStream.push_back(deliver1);
    
    // Apply to projections
    OrderStatusProjection statusProjection;
    CustomerOrderCountProjection countProjection;
    
    for (const auto& event : eventStream) {
        statusProjection.apply(event);
        countProjection.apply(event);
    }
    
    statusProjection.printOrderStatuses();
    countProjection.printCustomerOrderCounts();
}

void demonstrateEventReplay() {
    using namespace EventReplay;
    
    std::cout << "\n=== Event Replay ===\n";
    
    AuditLog auditLog;
    
    // Record events
    auditLog.recordEvent("user1", "LOGIN", "system", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auditLog.recordEvent("user1", "READ", "document1", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auditLog.recordEvent("user2", "LOGIN", "system", true);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auditLog.recordEvent("user1", "WRITE", "document1", false);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auditLog.recordEvent("user2", "DELETE", "document2", true);
    
    // Replay events
    auto replayTime = std::chrono::system_clock::now() - std::chrono::milliseconds(150);
    auditLog.replayUntil(replayTime);
}

int main() {
    std::cout << "=== Event Sourcing Pattern Demo ===\n\n";
    
    demonstrateBasicEventSourcing();
    demonstrateSnapshotEventSourcing();
    demonstrateProjections();
    demonstrateEventReplay();
    
    std::cout << "\n=== Event Sourcing Benefits ===\n";
    std::cout << "1. Complete audit trail\n";
    std::cout << "2. Time travel debugging\n";
    std::cout << "3. Event replay capability\n";
    std::cout << "4. Multiple projections\n";
    std::cout << "5. Natural fit for CQRS\n";
    
    return 0;
}