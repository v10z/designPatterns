// Minimal Unit of Work Pattern Implementation
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <typeinfo>
#include <functional>
#include <optional>
#include <string>

// Entity base class
class Entity {
protected:
    int id_;
    int version_ = 0;
    
public:
    Entity(int id = 0) : id_(id) {}
    virtual ~Entity() = default;
    
    int getId() const { return id_; }
    void setId(int id) { id_ = id; }
    
    int getVersion() const { return version_; }
    void incrementVersion() { version_++; }
    
    virtual std::string toString() const = 0;
    virtual std::unique_ptr<Entity> clone() const = 0;
};

// Domain entities
class Customer : public Entity {
private:
    std::string name_;
    std::string email_;
    double creditLimit_;
    
public:
    Customer(int id = 0, const std::string& name = "", 
             const std::string& email = "", double creditLimit = 0.0)
        : Entity(id), name_(name), email_(email), creditLimit_(creditLimit) {}
    
    std::string getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }
    
    std::string getEmail() const { return email_; }
    void setEmail(const std::string& email) { email_ = email; }
    
    double getCreditLimit() const { return creditLimit_; }
    void setCreditLimit(double limit) { creditLimit_ = limit; }
    
    std::string toString() const override {
        return "Customer{id=" + std::to_string(id_) + 
               ", name='" + name_ + 
               "', email='" + email_ + 
               "', creditLimit=" + std::to_string(creditLimit_) + 
               ", version=" + std::to_string(version_) + "}";
    }
    
    std::unique_ptr<Entity> clone() const override {
        return std::make_unique<Customer>(*this);
    }
};

class Order : public Entity {
private:
    int customerId_;
    std::string orderNumber_;
    double totalAmount_;
    std::string status_;
    
public:
    Order(int id = 0, int customerId = 0, const std::string& orderNumber = "",
          double totalAmount = 0.0, const std::string& status = "PENDING")
        : Entity(id), customerId_(customerId), orderNumber_(orderNumber),
          totalAmount_(totalAmount), status_(status) {}
    
    int getCustomerId() const { return customerId_; }
    void setCustomerId(int id) { customerId_ = id; }
    
    std::string getOrderNumber() const { return orderNumber_; }
    void setOrderNumber(const std::string& number) { orderNumber_ = number; }
    
    double getTotalAmount() const { return totalAmount_; }
    void setTotalAmount(double amount) { totalAmount_ = amount; }
    
    std::string getStatus() const { return status_; }
    void setStatus(const std::string& status) { status_ = status; }
    
    std::string toString() const override {
        return "Order{id=" + std::to_string(id_) + 
               ", customerId=" + std::to_string(customerId_) +
               ", orderNumber='" + orderNumber_ + 
               "', totalAmount=" + std::to_string(totalAmount_) + 
               ", status='" + status_ + 
               "', version=" + std::to_string(version_) + "}";
    }
    
    std::unique_ptr<Entity> clone() const override {
        return std::make_unique<Order>(*this);
    }
};

// Repository interfaces
template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual std::optional<T> findById(int id) = 0;
    virtual std::vector<T> findAll() = 0;
    virtual void insert(const T& entity) = 0;
    virtual void update(const T& entity) = 0;
    virtual void remove(int id) = 0;
};

// Identity Map to prevent duplicate objects
template<typename T>
class IdentityMap {
private:
    std::unordered_map<int, std::shared_ptr<T>> map_;
    
public:
    void add(int id, std::shared_ptr<T> entity) {
        map_[id] = entity;
    }
    
    std::shared_ptr<T> get(int id) {
        auto it = map_.find(id);
        return (it != map_.end()) ? it->second : nullptr;
    }
    
    void remove(int id) {
        map_.erase(id);
    }
    
    void clear() {
        map_.clear();
    }
    
    bool contains(int id) const {
        return map_.find(id) != map_.end();
    }
};

// Change tracking
enum class EntityState {
    UNCHANGED,
    ADDED,
    MODIFIED,
    DELETED
};

template<typename T>
struct TrackedEntity {
    std::shared_ptr<T> entity;
    std::shared_ptr<T> originalEntity;  // For optimistic concurrency
    EntityState state;
    
    TrackedEntity() : state(EntityState::UNCHANGED) {}
    
    TrackedEntity(std::shared_ptr<T> e, EntityState s) 
        : entity(e), state(s) {
        if (s == EntityState::UNCHANGED || s == EntityState::MODIFIED) {
            originalEntity = std::make_shared<T>(*e);
        }
    }
};

// Unit of Work interface
class IUnitOfWork {
public:
    virtual ~IUnitOfWork() = default;
    virtual void commit() = 0;
    virtual void rollback() = 0;
    
    virtual std::shared_ptr<IRepository<Customer>> customers() = 0;
    virtual std::shared_ptr<IRepository<Order>> orders() = 0;
};

// Forward declaration
class UnitOfWork;

// Repository wrapper that tracks changes
class TrackingCustomerRepository : public IRepository<Customer> {
    private:
        UnitOfWork* uow_;
        std::shared_ptr<IRepository<Customer>> innerRepo_;
        
    public:
        TrackingCustomerRepository(UnitOfWork* uow, 
                                 std::shared_ptr<IRepository<Customer>> repo)
            : uow_(uow), innerRepo_(repo) {}
        
        std::optional<Customer> findById(int id) override;
        
        std::vector<Customer> findAll() override;
        
        void insert(const Customer& entity) override;
        
        void update(const Customer& entity) override;
        
        void remove(int id) override;
};

// Unit of Work implementation
class UnitOfWork : public IUnitOfWork {
private:
    // Repositories
    std::shared_ptr<IRepository<Customer>> customerRepo_;
    std::shared_ptr<IRepository<Order>> orderRepo_;
    
    // Identity maps
    IdentityMap<Customer> customerIdentityMap_;
    IdentityMap<Order> orderIdentityMap_;
    
    // Change tracking
    std::unordered_map<int, TrackedEntity<Customer>> trackedCustomers_;
    std::unordered_map<int, TrackedEntity<Order>> trackedOrders_;
    
    // Transaction state
    bool inTransaction_ = false;
    
    // Make TrackingCustomerRepository a friend
    friend class TrackingCustomerRepository;
    
public:
    UnitOfWork(std::shared_ptr<IRepository<Customer>> customerRepo,
              std::shared_ptr<IRepository<Order>> orderRepo)
        : customerRepo_(std::make_shared<TrackingCustomerRepository>(this, customerRepo)),
          orderRepo_(orderRepo) {}
    
    void commit() override {
        std::cout << "\n=== Committing Unit of Work ===\n";
        inTransaction_ = true;
        
        try {
            // Process customers
            for (const auto& [id, tracked] : trackedCustomers_) {
                switch (tracked.state) {
                    case EntityState::ADDED:
                        std::cout << "INSERT: " << tracked.entity->toString() << "\n";
                        // In real implementation, would call actual repo
                        break;
                        
                    case EntityState::MODIFIED:
                        // Check for optimistic concurrency
                        if (tracked.originalEntity && 
                            tracked.originalEntity->getVersion() != tracked.entity->getVersion()) {
                            throw std::runtime_error("Concurrency conflict detected!");
                        }
                        tracked.entity->incrementVersion();
                        std::cout << "UPDATE: " << tracked.entity->toString() << "\n";
                        break;
                        
                    case EntityState::DELETED:
                        std::cout << "DELETE: Customer with ID " << id << "\n";
                        break;
                        
                    case EntityState::UNCHANGED:
                        // No action needed
                        break;
                }
            }
            
            // Clear tracking after successful commit
            clear();
            std::cout << "Commit successful!\n";
            
        } catch (const std::exception& e) {
            std::cout << "Commit failed: " << e.what() << "\n";
            rollback();
            throw;
        }
        
        inTransaction_ = false;
    }
    
    void rollback() override {
        std::cout << "\n=== Rolling back Unit of Work ===\n";
        
        // Restore original values for modified entities
        for (auto& [id, tracked] : trackedCustomers_) {
            if (tracked.state == EntityState::MODIFIED && tracked.originalEntity) {
                *tracked.entity = *tracked.originalEntity;
            }
        }
        
        // Clear all tracking
        clear();
        std::cout << "Rollback complete!\n";
    }
    
    std::shared_ptr<IRepository<Customer>> customers() override {
        return customerRepo_;
    }
    
    std::shared_ptr<IRepository<Order>> orders() override {
        return orderRepo_;
    }
    
private:
    void clear() {
        trackedCustomers_.clear();
        trackedOrders_.clear();
        customerIdentityMap_.clear();
        orderIdentityMap_.clear();
    }
};

// Implementation of TrackingCustomerRepository methods
std::optional<Customer> TrackingCustomerRepository::findById(int id) {
    // Check identity map first
    auto cached = uow_->customerIdentityMap_.get(id);
    if (cached) {
        return *cached;
    }
    
    // Load from repository
    auto result = innerRepo_->findById(id);
    if (result.has_value()) {
        auto entity = std::make_shared<Customer>(result.value());
        uow_->customerIdentityMap_.add(id, entity);
        uow_->trackedCustomers_[id] = TrackedEntity<Customer>(
            entity, EntityState::UNCHANGED);
        return *entity;
    }
    
    return std::nullopt;
}

std::vector<Customer> TrackingCustomerRepository::findAll() {
    return innerRepo_->findAll();
}

void TrackingCustomerRepository::insert(const Customer& entity) {
    auto tracked = std::make_shared<Customer>(entity);
    if (tracked->getId() != 0) {
        uow_->customerIdentityMap_.add(tracked->getId(), tracked);
    }
    uow_->trackedCustomers_[tracked->getId()] = TrackedEntity<Customer>(
        tracked, EntityState::ADDED);
}

void TrackingCustomerRepository::update(const Customer& entity) {
    auto id = entity.getId();
    auto existing = uow_->customerIdentityMap_.get(id);
    
    if (existing) {
        // Update existing tracked entity
        *existing = entity;
        
        auto& tracked = uow_->trackedCustomers_[id];
        if (tracked.state == EntityState::UNCHANGED) {
            tracked.state = EntityState::MODIFIED;
        }
    } else {
        // Not tracked yet, add to tracking
        auto tracked = std::make_shared<Customer>(entity);
        uow_->customerIdentityMap_.add(id, tracked);
        uow_->trackedCustomers_[id] = TrackedEntity<Customer>(
            tracked, EntityState::MODIFIED);
    }
}

void TrackingCustomerRepository::remove(int id) {
    auto it = uow_->trackedCustomers_.find(id);
    if (it != uow_->trackedCustomers_.end()) {
        if (it->second.state == EntityState::ADDED) {
            // If it was added in this UoW, just remove from tracking
            uow_->trackedCustomers_.erase(it);
            uow_->customerIdentityMap_.remove(id);
        } else {
            // Mark for deletion
            it->second.state = EntityState::DELETED;
        }
    }
}

// In-memory repository implementations for testing
class InMemoryCustomerRepository : public IRepository<Customer> {
private:
    std::unordered_map<int, Customer> data_;
    int nextId_ = 1;
    
public:
    std::optional<Customer> findById(int id) override {
        auto it = data_.find(id);
        return (it != data_.end()) ? std::optional<Customer>(it->second) : std::nullopt;
    }
    
    std::vector<Customer> findAll() override {
        std::vector<Customer> result;
        for (const auto& [id, customer] : data_) {
            result.push_back(customer);
        }
        return result;
    }
    
    void insert(const Customer& entity) override {
        Customer newEntity = entity;
        if (newEntity.getId() == 0) {
            newEntity.setId(nextId_++);
        }
        data_[newEntity.getId()] = newEntity;
    }
    
    void update(const Customer& entity) override {
        data_[entity.getId()] = entity;
    }
    
    void remove(int id) override {
        data_.erase(id);
    }
    
    // Helper method to populate test data
    void seedData() {
        insert(Customer(1, "John Doe", "john@example.com", 5000.0));
        insert(Customer(2, "Jane Smith", "jane@example.com", 10000.0));
        insert(Customer(3, "Bob Johnson", "bob@example.com", 7500.0));
    }
};

// Service layer using Unit of Work
class CustomerService {
private:
    std::function<std::unique_ptr<IUnitOfWork>()> uowFactory_;
    
public:
    explicit CustomerService(std::function<std::unique_ptr<IUnitOfWork>()> factory)
        : uowFactory_(factory) {}
    
    void updateCustomerCredit(int customerId, double newCreditLimit) {
        auto uow = uowFactory_();
        
        auto customer = uow->customers()->findById(customerId);
        if (!customer.has_value()) {
            throw std::runtime_error("Customer not found");
        }
        
        customer->setCreditLimit(newCreditLimit);
        uow->customers()->update(customer.value());
        
        // Could update related orders here...
        
        uow->commit();
    }
    
    void createCustomerWithOrder(const std::string& name, const std::string& email,
                                double creditLimit, const std::string& orderNumber,
                                double orderAmount) {
        auto uow = uowFactory_();
        
        // Create customer
        Customer customer(0, name, email, creditLimit);
        uow->customers()->insert(customer);
        
        // Create order (in real implementation)
        // Order order(0, customer.getId(), orderNumber, orderAmount);
        // uow->orders()->insert(order);
        
        // Commit both operations together
        uow->commit();
    }
};

// Example usage
void demonstrateBasicUnitOfWork() {
    std::cout << "=== Basic Unit of Work Demo ===\n";
    
    // Create repositories
    auto customerRepo = std::make_shared<InMemoryCustomerRepository>();
    customerRepo->seedData();
    auto orderRepo = std::shared_ptr<IRepository<Order>>(); // Placeholder - null for now
    
    // Create unit of work
    auto uow = std::make_unique<UnitOfWork>(customerRepo, orderRepo);
    
    // Load and modify customer
    auto customer = uow->customers()->findById(1);
    if (customer.has_value()) {
        std::cout << "Loaded: " << customer->toString() << "\n";
        
        customer->setEmail("john.doe@newdomain.com");
        customer->setCreditLimit(7500.0);
        uow->customers()->update(customer.value());
    }
    
    // Add new customer
    Customer newCustomer(0, "Alice Brown", "alice@example.com", 3000.0);
    uow->customers()->insert(newCustomer);
    
    // Delete customer
    uow->customers()->remove(3);
    
    // Commit all changes
    uow->commit();
}

void demonstrateIdentityMap() {
    std::cout << "\n=== Identity Map Demo ===\n";
    
    auto customerRepo = std::make_shared<InMemoryCustomerRepository>();
    customerRepo->seedData();
    auto orderRepo = std::shared_ptr<IRepository<Order>>();
    
    auto uow = std::make_unique<UnitOfWork>(customerRepo, orderRepo);
    
    // Load same customer twice
    auto customer1 = uow->customers()->findById(1);
    auto customer2 = uow->customers()->findById(1);
    
    if (customer1.has_value() && customer2.has_value()) {
        std::cout << "First load: " << customer1->toString() << "\n";
        std::cout << "Second load: " << customer2->toString() << "\n";
        
        // Modify through first reference
        customer1->setEmail("modified@example.com");
        
        // Check second reference (should see the change due to identity map)
        std::cout << "After modification: " << customer2->toString() << "\n";
        std::cout << "Identity map ensures single instance per entity!\n";
    }
}

void demonstrateRollback() {
    std::cout << "\n=== Rollback Demo ===\n";
    
    auto customerRepo = std::make_shared<InMemoryCustomerRepository>();
    customerRepo->seedData();
    auto orderRepo = std::shared_ptr<IRepository<Order>>();
    
    auto uow = std::make_unique<UnitOfWork>(customerRepo, orderRepo);
    
    // Load and modify customer
    auto customer = uow->customers()->findById(2);
    if (customer.has_value()) {
        std::cout << "Original: " << customer->toString() << "\n";
        
        customer->setCreditLimit(20000.0);
        uow->customers()->update(customer.value());
        
        std::cout << "Modified: " << customer->toString() << "\n";
        
        // Rollback changes
        uow->rollback();
        
        std::cout << "After rollback: " << customer->toString() << "\n";
    }
}

int main() {
    std::cout << "=== Unit of Work Pattern Demo ===\n\n";
    
    demonstrateBasicUnitOfWork();
    demonstrateIdentityMap();
    demonstrateRollback();
    
    // Service layer example
    std::cout << "\n=== Service Layer with Unit of Work ===\n";
    
    auto customerRepo = std::make_shared<InMemoryCustomerRepository>();
    customerRepo->seedData();
    auto orderRepo = std::shared_ptr<IRepository<Order>>();
    
    CustomerService service([customerRepo, orderRepo]() {
        return std::make_unique<UnitOfWork>(customerRepo, orderRepo);
    });
    
    try {
        service.updateCustomerCredit(1, 8000.0);
        service.createCustomerWithOrder("New Customer", "new@example.com", 
                                      5000.0, "ORD-001", 150.0);
    } catch (const std::exception& e) {
        std::cout << "Service operation failed: " << e.what() << "\n";
    }
    
    return 0;
}