// Minimal Service Locator Pattern Implementation
#include <iostream>
#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

// Forward declarations
class ILogger;
class IDatabase;
class IEmailService;
class IPaymentGateway;
class IAuthService;

// Service interfaces
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
    virtual void error(const std::string& message) = 0;
    virtual void debug(const std::string& message) = 0;
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual void executeQuery(const std::string& query) = 0;
    virtual std::string fetchData(const std::string& table, int id) = 0;
};

class IEmailService {
public:
    virtual ~IEmailService() = default;
    virtual void sendEmail(const std::string& to, const std::string& subject, 
                          const std::string& body) = 0;
    virtual void sendBulkEmail(const std::vector<std::string>& recipients,
                              const std::string& subject, const std::string& body) = 0;
};

class IPaymentGateway {
public:
    virtual ~IPaymentGateway() = default;
    virtual bool processPayment(double amount, const std::string& cardNumber) = 0;
    virtual bool refund(const std::string& transactionId, double amount) = 0;
    virtual std::string getTransactionStatus(const std::string& transactionId) = 0;
};

class IAuthService {
public:
    virtual ~IAuthService() = default;
    virtual bool authenticate(const std::string& username, const std::string& password) = 0;
    virtual bool authorize(const std::string& username, const std::string& resource) = 0;
    virtual void logout(const std::string& username) = 0;
};

// Service Locator
class ServiceLocator {
private:
    // Static instance
    static std::unique_ptr<ServiceLocator> instance_;
    
    // Service registry
    std::unordered_map<std::string, std::shared_ptr<void>> services_;
    std::unordered_map<std::string, std::function<std::shared_ptr<void>()>> factories_;
    
    // Constructor is private for singleton
    ServiceLocator() = default;
    
public:
    // Delete copy constructor and assignment
    ServiceLocator(const ServiceLocator&) = delete;
    ServiceLocator& operator=(const ServiceLocator&) = delete;
    
    // Get singleton instance
    static ServiceLocator& getInstance() {
        if (!instance_) {
            instance_ = std::unique_ptr<ServiceLocator>(new ServiceLocator());
        }
        return *instance_;
    }
    
    // Register a service instance
    template<typename T>
    void registerService(std::shared_ptr<T> service) {
        std::string typeName = typeid(T).name();
        services_[typeName] = std::static_pointer_cast<void>(service);
        std::cout << "Registered service: " << typeName << "\n";
    }
    
    // Register a service factory (lazy initialization)
    template<typename T>
    void registerFactory(std::function<std::shared_ptr<T>()> factory) {
        std::string typeName = typeid(T).name();
        factories_[typeName] = [factory]() {
            return std::static_pointer_cast<void>(factory());
        };
        std::cout << "Registered factory for: " << typeName << "\n";
    }
    
    // Get a service
    template<typename T>
    std::shared_ptr<T> getService() {
        std::string typeName = typeid(T).name();
        
        // Check if service already exists
        auto it = services_.find(typeName);
        if (it != services_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        
        // Check if factory exists
        auto factoryIt = factories_.find(typeName);
        if (factoryIt != factories_.end()) {
            // Create service using factory
            auto service = factoryIt->second();
            services_[typeName] = service;
            std::cout << "Created service using factory: " << typeName << "\n";
            return std::static_pointer_cast<T>(service);
        }
        
        throw std::runtime_error("Service not found: " + typeName);
    }
    
    // Check if service is registered
    template<typename T>
    bool hasService() const {
        std::string typeName = typeid(T).name();
        return services_.find(typeName) != services_.end() ||
               factories_.find(typeName) != factories_.end();
    }
    
    // Remove a service
    template<typename T>
    void removeService() {
        std::string typeName = typeid(T).name();
        services_.erase(typeName);
        factories_.erase(typeName);
        std::cout << "Removed service: " << typeName << "\n";
    }
    
    // Clear all services
    void clear() {
        services_.clear();
        factories_.clear();
        std::cout << "Cleared all services\n";
    }
    
    // Get service count
    size_t getServiceCount() const {
        return services_.size() + factories_.size();
    }
};

// Initialize static member
std::unique_ptr<ServiceLocator> ServiceLocator::instance_ = nullptr;

// Concrete service implementations
class ConsoleLogger : public ILogger {
private:
    std::string prefix_;
    
public:
    explicit ConsoleLogger(const std::string& prefix = "[LOG]") : prefix_(prefix) {}
    
    void log(const std::string& message) override {
        std::cout << prefix_ << " INFO: " << message << "\n";
    }
    
    void error(const std::string& message) override {
        std::cout << prefix_ << " ERROR: " << message << "\n";
    }
    
    void debug(const std::string& message) override {
        std::cout << prefix_ << " DEBUG: " << message << "\n";
    }
};

class FileLogger : public ILogger {
private:
    std::string filename_;
    
public:
    explicit FileLogger(const std::string& filename) : filename_(filename) {}
    
    void log(const std::string& message) override {
        std::cout << "Writing to " << filename_ << ": INFO: " << message << "\n";
    }
    
    void error(const std::string& message) override {
        std::cout << "Writing to " << filename_ << ": ERROR: " << message << "\n";
    }
    
    void debug(const std::string& message) override {
        std::cout << "Writing to " << filename_ << ": DEBUG: " << message << "\n";
    }
};

class SQLDatabase : public IDatabase {
private:
    std::string connectionString_;
    bool connected_ = false;
    
public:
    explicit SQLDatabase(const std::string& connStr) : connectionString_(connStr) {}
    
    void connect() override {
        std::cout << "Connecting to database: " << connectionString_ << "\n";
        connected_ = true;
    }
    
    void disconnect() override {
        std::cout << "Disconnecting from database\n";
        connected_ = false;
    }
    
    void executeQuery(const std::string& query) override {
        if (!connected_) {
            throw std::runtime_error("Database not connected");
        }
        std::cout << "Executing query: " << query << "\n";
    }
    
    std::string fetchData(const std::string& table, int id) override {
        if (!connected_) {
            throw std::runtime_error("Database not connected");
        }
        return "Data from " + table + " with id=" + std::to_string(id);
    }
};

class SMTPEmailService : public IEmailService {
private:
    std::string smtpServer_;
    int port_;
    
public:
    SMTPEmailService(const std::string& server, int port) 
        : smtpServer_(server), port_(port) {}
    
    void sendEmail(const std::string& to, const std::string& subject, 
                  const std::string& body) override {
        std::cout << "Sending email via " << smtpServer_ << ":" << port_ << "\n";
        std::cout << "  To: " << to << "\n";
        std::cout << "  Subject: " << subject << "\n";
        std::cout << "  Body: " << body << "\n";
    }
    
    void sendBulkEmail(const std::vector<std::string>& recipients,
                      const std::string& subject, const std::string& body) override {
        std::cout << "Sending bulk email to " << recipients.size() << " recipients\n";
        for (const auto& recipient : recipients) {
            sendEmail(recipient, subject, body);
        }
    }
};

class StripePaymentGateway : public IPaymentGateway {
private:
    std::string apiKey_;
    int transactionCounter_ = 1000;
    
public:
    explicit StripePaymentGateway(const std::string& apiKey) : apiKey_(apiKey) {}
    
    bool processPayment(double amount, const std::string& cardNumber) override {
        std::cout << "Processing payment of $" << amount << " via Stripe\n";
        std::cout << "  Card: ****" << cardNumber.substr(cardNumber.length() - 4) << "\n";
        
        // Simulate processing
        if (amount > 0 && !cardNumber.empty()) {
            std::cout << "  Transaction ID: TXN" << transactionCounter_++ << "\n";
            return true;
        }
        return false;
    }
    
    bool refund(const std::string& transactionId, double amount) override {
        std::cout << "Processing refund of $" << amount 
                  << " for transaction: " << transactionId << "\n";
        return true;
    }
    
    std::string getTransactionStatus(const std::string& transactionId) override {
        return "COMPLETED";
    }
};

class SimpleAuthService : public IAuthService {
private:
    std::unordered_map<std::string, std::string> users_;
    std::unordered_map<std::string, std::vector<std::string>> permissions_;
    std::unordered_map<std::string, bool> sessions_;
    
public:
    SimpleAuthService() {
        // Add some test users
        users_["admin"] = "admin123";
        users_["user"] = "pass123";
        
        permissions_["admin"] = {"read", "write", "delete"};
        permissions_["user"] = {"read"};
    }
    
    bool authenticate(const std::string& username, const std::string& password) override {
        auto it = users_.find(username);
        if (it != users_.end() && it->second == password) {
            sessions_[username] = true;
            std::cout << "User " << username << " authenticated successfully\n";
            return true;
        }
        std::cout << "Authentication failed for user: " << username << "\n";
        return false;
    }
    
    bool authorize(const std::string& username, const std::string& resource) override {
        if (sessions_.find(username) == sessions_.end() || !sessions_[username]) {
            std::cout << "User " << username << " not authenticated\n";
            return false;
        }
        
        auto it = permissions_.find(username);
        if (it != permissions_.end()) {
            for (const auto& perm : it->second) {
                if (perm == resource) {
                    std::cout << "User " << username << " authorized for: " << resource << "\n";
                    return true;
                }
            }
        }
        
        std::cout << "User " << username << " not authorized for: " << resource << "\n";
        return false;
    }
    
    void logout(const std::string& username) override {
        sessions_[username] = false;
        std::cout << "User " << username << " logged out\n";
    }
};

// Application classes using Service Locator
class OrderService {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IDatabase> database_;
    std::shared_ptr<IEmailService> emailService_;
    std::shared_ptr<IPaymentGateway> paymentGateway_;
    
public:
    OrderService() {
        // Get services from Service Locator
        auto& locator = ServiceLocator::getInstance();
        logger_ = locator.getService<ILogger>();
        database_ = locator.getService<IDatabase>();
        emailService_ = locator.getService<IEmailService>();
        paymentGateway_ = locator.getService<IPaymentGateway>();
    }
    
    void processOrder(int customerId, double amount, const std::string& cardNumber) {
        logger_->log("Processing order for customer: " + std::to_string(customerId));
        
        try {
            // Get customer data
            std::string customerData = database_->fetchData("customers", customerId);
            logger_->debug("Retrieved customer data: " + customerData);
            
            // Process payment
            bool paymentSuccess = paymentGateway_->processPayment(amount, cardNumber);
            
            if (paymentSuccess) {
                logger_->log("Payment successful");
                
                // Save order to database
                database_->executeQuery("INSERT INTO orders (customer_id, amount) VALUES (" +
                                      std::to_string(customerId) + ", " + 
                                      std::to_string(amount) + ")");
                
                // Send confirmation email
                emailService_->sendEmail("customer@example.com", 
                                       "Order Confirmation",
                                       "Your order has been processed successfully!");
                
                logger_->log("Order completed successfully");
            } else {
                logger_->error("Payment failed");
            }
        } catch (const std::exception& e) {
            logger_->error("Order processing failed: " + std::string(e.what()));
        }
    }
};

// Example with lazy initialization
class ReportingService {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IDatabase> database_;
    
public:
    ReportingService() {
        auto& locator = ServiceLocator::getInstance();
        logger_ = locator.getService<ILogger>();
        // Database will be lazy-loaded when first used
    }
    
    void generateReport(const std::string& reportType) {
        logger_->log("Generating report: " + reportType);
        
        // Lazy load database service
        if (!database_) {
            auto& locator = ServiceLocator::getInstance();
            database_ = locator.getService<IDatabase>();
        }
        
        database_->executeQuery("SELECT * FROM " + reportType + "_data");
        logger_->log("Report generated successfully");
    }
};

// Service Provider - Alternative to static Service Locator
class ServiceProvider {
private:
    std::unordered_map<std::string, std::shared_ptr<void>> services_;
    
public:
    template<typename T>
    void addService(std::shared_ptr<T> service) {
        services_[typeid(T).name()] = std::static_pointer_cast<void>(service);
    }
    
    template<typename T>
    std::shared_ptr<T> getService() const {
        auto it = services_.find(typeid(T).name());
        if (it != services_.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
};

// Example using dependency injection instead
class UserService {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IAuthService> authService_;
    
public:
    // Constructor injection - preferred over Service Locator
    UserService(std::shared_ptr<ILogger> logger, 
                std::shared_ptr<IAuthService> authService)
        : logger_(logger), authService_(authService) {}
    
    bool login(const std::string& username, const std::string& password) {
        logger_->log("Login attempt for user: " + username);
        
        if (authService_->authenticate(username, password)) {
            logger_->log("Login successful for user: " + username);
            return true;
        } else {
            logger_->error("Login failed for user: " + username);
            return false;
        }
    }
    
    bool hasPermission(const std::string& username, const std::string& resource) {
        return authService_->authorize(username, resource);
    }
};

int main() {
    std::cout << "=== Service Locator Pattern Demo ===\n\n";
    
    // Get Service Locator instance
    auto& locator = ServiceLocator::getInstance();
    
    std::cout << "=== Registering Services ===\n";
    
    // Register services directly
    locator.registerService<ILogger>(std::make_shared<ConsoleLogger>("[APP]"));
    locator.registerService<IEmailService>(
        std::make_shared<SMTPEmailService>("smtp.example.com", 587));
    locator.registerService<IPaymentGateway>(
        std::make_shared<StripePaymentGateway>("sk_test_123456"));
    locator.registerService<IAuthService>(std::make_shared<SimpleAuthService>());
    
    // Register factory for lazy initialization
    locator.registerFactory<IDatabase>([]() {
        auto db = std::make_shared<SQLDatabase>("server=localhost;db=myapp");
        db->connect();
        return db;
    });
    
    std::cout << "\nTotal services registered: " << locator.getServiceCount() << "\n";
    
    // Using services through Service Locator
    std::cout << "\n=== Using Order Service ===\n";
    OrderService orderService;
    orderService.processOrder(123, 99.99, "4111111111111111");
    
    // Demonstrating lazy initialization
    std::cout << "\n=== Using Reporting Service (Lazy Loading) ===\n";
    ReportingService reportingService;
    reportingService.generateReport("sales");
    
    // Direct service access
    std::cout << "\n=== Direct Service Access ===\n";
    auto logger = locator.getService<ILogger>();
    logger->log("Direct access to logger service");
    
    // Auth service demo
    std::cout << "\n=== Authentication Demo ===\n";
    auto authService = locator.getService<IAuthService>();
    authService->authenticate("admin", "admin123");
    authService->authorize("admin", "write");
    authService->authorize("admin", "execute");
    
    // Alternative: Using dependency injection
    std::cout << "\n=== Dependency Injection Alternative ===\n";
    UserService userService(
        std::make_shared<ConsoleLogger>("[USER]"),
        std::make_shared<SimpleAuthService>()
    );
    
    userService.login("user", "pass123");
    std::cout << "Has read permission: " 
              << userService.hasPermission("user", "read") << "\n";
    std::cout << "Has write permission: " 
              << userService.hasPermission("user", "write") << "\n";
    
    // Service replacement
    std::cout << "\n=== Service Replacement ===\n";
    locator.removeService<ILogger>();
    locator.registerService<ILogger>(std::make_shared<FileLogger>("app.log"));
    
    auto newLogger = locator.getService<ILogger>();
    newLogger->log("Using new file logger");
    
    // Anti-pattern warning
    std::cout << "\n=== Anti-Pattern Warning ===\n";
    std::cout << "Service Locator can be considered an anti-pattern because:\n";
    std::cout << "- It creates hidden dependencies\n";
    std::cout << "- Makes testing harder (need to mock the locator)\n";
    std::cout << "- Violates Dependency Inversion Principle\n";
    std::cout << "- Runtime errors instead of compile-time errors\n";
    std::cout << "\nPrefer Dependency Injection when possible!\n";
    
    return 0;
}