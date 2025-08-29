// Minimal Dependency Injection Pattern Implementation
#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeinfo>
#include <any>
#include <string>

// Service interfaces
class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(const std::string& message) = 0;
};

class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual void connect() = 0;
    virtual void execute(const std::string& query) = 0;
    virtual std::string fetch(const std::string& query) = 0;
};

class IEmailService {
public:
    virtual ~IEmailService() = default;
    virtual void sendEmail(const std::string& to, const std::string& subject, 
                          const std::string& body) = 0;
};

class ICache {
public:
    virtual ~ICache() = default;
    virtual void set(const std::string& key, const std::string& value) = 0;
    virtual std::string get(const std::string& key) = 0;
    virtual bool contains(const std::string& key) = 0;
};

class IConfiguration {
public:
    virtual ~IConfiguration() = default;
    virtual std::string getValue(const std::string& key) = 0;
    virtual int getInt(const std::string& key) = 0;
    virtual bool getBool(const std::string& key) = 0;
};

// Concrete implementations
class ConsoleLogger : public ILogger {
private:
    std::string prefix_;
    
public:
    explicit ConsoleLogger(const std::string& prefix = "[LOG]") : prefix_(prefix) {}
    
    void log(const std::string& message) override {
        std::cout << prefix_ << " " << message << "\n";
    }
};

class FileLogger : public ILogger {
private:
    std::string filename_;
    
public:
    explicit FileLogger(const std::string& filename) : filename_(filename) {}
    
    void log(const std::string& message) override {
        std::cout << "Writing to " << filename_ << ": " << message << "\n";
    }
};

class MySQLDatabase : public IDatabase {
private:
    std::string connectionString_;
    bool connected_ = false;
    
public:
    explicit MySQLDatabase(const std::string& connectionString) 
        : connectionString_(connectionString) {}
    
    void connect() override {
        std::cout << "Connecting to MySQL: " << connectionString_ << "\n";
        connected_ = true;
    }
    
    void execute(const std::string& query) override {
        if (!connected_) connect();
        std::cout << "Executing MySQL query: " << query << "\n";
    }
    
    std::string fetch(const std::string& query) override {
        if (!connected_) connect();
        std::cout << "Fetching from MySQL: " << query << "\n";
        return "MySQL result data";
    }
};

class PostgreSQLDatabase : public IDatabase {
private:
    std::string host_;
    int port_;
    std::string database_;
    bool connected_ = false;
    
public:
    PostgreSQLDatabase(const std::string& host, int port, const std::string& database)
        : host_(host), port_(port), database_(database) {}
    
    void connect() override {
        std::cout << "Connecting to PostgreSQL: " << host_ << ":" << port_ 
                  << "/" << database_ << "\n";
        connected_ = true;
    }
    
    void execute(const std::string& query) override {
        if (!connected_) connect();
        std::cout << "Executing PostgreSQL query: " << query << "\n";
    }
    
    std::string fetch(const std::string& query) override {
        if (!connected_) connect();
        std::cout << "Fetching from PostgreSQL: " << query << "\n";
        return "PostgreSQL result data";
    }
};

class SMTPEmailService : public IEmailService {
private:
    std::string server_;
    int port_;
    
public:
    SMTPEmailService(const std::string& server, int port) 
        : server_(server), port_(port) {}
    
    void sendEmail(const std::string& to, const std::string& subject, 
                  const std::string& body) override {
        std::cout << "Sending email via SMTP " << server_ << ":" << port_ << "\n";
        std::cout << "  To: " << to << "\n";
        std::cout << "  Subject: " << subject << "\n";
    }
};

class MemoryCache : public ICache {
private:
    std::unordered_map<std::string, std::string> cache_;
    
public:
    void set(const std::string& key, const std::string& value) override {
        cache_[key] = value;
        std::cout << "Cached: " << key << " = " << value << "\n";
    }
    
    std::string get(const std::string& key) override {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            std::cout << "Cache hit: " << key << "\n";
            return it->second;
        }
        std::cout << "Cache miss: " << key << "\n";
        return "";
    }
    
    bool contains(const std::string& key) override {
        return cache_.find(key) != cache_.end();
    }
};

class AppConfiguration : public IConfiguration {
private:
    std::unordered_map<std::string, std::string> config_;
    
public:
    AppConfiguration() {
        // Default configuration
        config_["app.name"] = "DI Demo App";
        config_["app.version"] = "1.0.0";
        config_["db.type"] = "mysql";
        config_["cache.enabled"] = "true";
        config_["email.smtp.port"] = "587";
    }
    
    std::string getValue(const std::string& key) override {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "";
    }
    
    int getInt(const std::string& key) override {
        std::string value = getValue(key);
        return value.empty() ? 0 : std::stoi(value);
    }
    
    bool getBool(const std::string& key) override {
        std::string value = getValue(key);
        return value == "true" || value == "1";
    }
};

// Business classes with dependencies
// Constructor Injection
class UserService {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IDatabase> database_;
    std::shared_ptr<IEmailService> emailService_;
    
public:
    // Dependencies injected through constructor
    UserService(std::shared_ptr<ILogger> logger,
                std::shared_ptr<IDatabase> database,
                std::shared_ptr<IEmailService> emailService)
        : logger_(logger), database_(database), emailService_(emailService) {
        logger_->log("UserService initialized");
    }
    
    void createUser(const std::string& username, const std::string& email) {
        logger_->log("Creating user: " + username);
        
        // Check if user exists
        std::string checkQuery = "SELECT * FROM users WHERE username = '" + username + "'";
        std::string result = database_->fetch(checkQuery);
        
        // Create user
        std::string insertQuery = "INSERT INTO users (username, email) VALUES ('" + 
                                username + "', '" + email + "')";
        database_->execute(insertQuery);
        
        // Send welcome email
        emailService_->sendEmail(email, "Welcome!", 
                               "Welcome to our service, " + username);
        
        logger_->log("User created successfully");
    }
};

// Setter Injection
class ProductService {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IDatabase> database_;
    std::shared_ptr<ICache> cache_;
    
public:
    // Default constructor
    ProductService() = default;
    
    // Setter injection methods
    void setLogger(std::shared_ptr<ILogger> logger) {
        logger_ = logger;
        if (logger_) {
            logger_->log("Logger injected into ProductService");
        }
    }
    
    void setDatabase(std::shared_ptr<IDatabase> database) {
        database_ = database;
    }
    
    void setCache(std::shared_ptr<ICache> cache) {
        cache_ = cache;
    }
    
    std::string getProduct(int productId) {
        if (!logger_ || !database_) {
            throw std::runtime_error("Dependencies not injected");
        }
        
        std::string key = "product_" + std::to_string(productId);
        
        // Check cache first
        if (cache_ && cache_->contains(key)) {
            return cache_->get(key);
        }
        
        // Fetch from database
        logger_->log("Fetching product: " + std::to_string(productId));
        std::string query = "SELECT * FROM products WHERE id = " + std::to_string(productId);
        std::string result = database_->fetch(query);
        
        // Cache the result
        if (cache_) {
            cache_->set(key, result);
        }
        
        return result;
    }
};

// Interface Injection
class ILoggerAware {
public:
    virtual ~ILoggerAware() = default;
    virtual void setLogger(std::shared_ptr<ILogger> logger) = 0;
};

class OrderService : public ILoggerAware {
private:
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IDatabase> database_;
    std::shared_ptr<IEmailService> emailService_;
    
public:
    // Constructor injection for required dependencies
    OrderService(std::shared_ptr<IDatabase> database,
                std::shared_ptr<IEmailService> emailService)
        : database_(database), emailService_(emailService) {}
    
    // Interface injection for optional dependency
    void setLogger(std::shared_ptr<ILogger> logger) override {
        logger_ = logger;
    }
    
    void createOrder(int customerId, const std::vector<int>& productIds) {
        if (logger_) {
            logger_->log("Creating order for customer: " + std::to_string(customerId));
        }
        
        // Create order
        database_->execute("INSERT INTO orders (customer_id) VALUES (" + 
                         std::to_string(customerId) + ")");
        
        // Add order items
        for (int productId : productIds) {
            database_->execute("INSERT INTO order_items (order_id, product_id) VALUES (LAST_INSERT_ID(), " +
                             std::to_string(productId) + ")");
        }
        
        // Send confirmation
        emailService_->sendEmail("customer@example.com", "Order Confirmation",
                               "Your order has been placed!");
        
        if (logger_) {
            logger_->log("Order created successfully");
        }
    }
};

// Simple DI Container
class DIContainer {
private:
    std::unordered_map<std::string, std::any> services_;
    std::unordered_map<std::string, std::function<std::any()>> factories_;
    
public:
    // Register a singleton service
    template<typename T>
    void registerSingleton(std::shared_ptr<T> service) {
        std::string typeName = typeid(T).name();
        services_[typeName] = service;
        std::cout << "Registered singleton: " << typeName << "\n";
    }
    
    // Register a factory for transient services
    template<typename T>
    void registerFactory(std::function<std::shared_ptr<T>()> factory) {
        std::string typeName = typeid(T).name();
        factories_[typeName] = [factory]() -> std::any {
            return factory();
        };
        std::cout << "Registered factory: " << typeName << "\n";
    }
    
    // Resolve a service
    template<typename T>
    std::shared_ptr<T> resolve() {
        std::string typeName = typeid(T).name();
        
        // Check singletons first
        auto it = services_.find(typeName);
        if (it != services_.end()) {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        }
        
        // Check factories
        auto factoryIt = factories_.find(typeName);
        if (factoryIt != factories_.end()) {
            return std::any_cast<std::shared_ptr<T>>(factoryIt->second());
        }
        
        throw std::runtime_error("Service not registered: " + typeName);
    }
    
    // Build object with dependencies
    template<typename T, typename... Args>
    std::shared_ptr<T> build(Args&&... args) {
        return std::make_shared<T>(resolve<typename std::decay<Args>::type>()...);
    }
};

// Advanced DI Container with lifetime management
enum class ServiceLifetime {
    Singleton,
    Transient,
    Scoped
};

class ServiceDescriptor {
public:
    std::string typeName;
    ServiceLifetime lifetime;
    std::function<std::any(DIContainer&)> factory;
    std::any instance; // For singleton
};

class AdvancedDIContainer {
private:
    std::unordered_map<std::string, ServiceDescriptor> descriptors_;
    std::unordered_map<std::string, std::any> scopedInstances_;
    
public:
    template<typename TInterface, typename TImplementation>
    void addSingleton() {
        ServiceDescriptor descriptor;
        descriptor.typeName = typeid(TInterface).name();
        descriptor.lifetime = ServiceLifetime::Singleton;
        descriptor.factory = [](DIContainer& container) -> std::any {
            return std::static_pointer_cast<TInterface>(
                std::make_shared<TImplementation>());
        };
        descriptors_[descriptor.typeName] = descriptor;
    }
    
    template<typename TInterface, typename TImplementation, typename... TDeps>
    void addSingletonWithDeps() {
        ServiceDescriptor descriptor;
        descriptor.typeName = typeid(TInterface).name();
        descriptor.lifetime = ServiceLifetime::Singleton;
        descriptor.factory = [this](DIContainer& container) -> std::any {
            return std::static_pointer_cast<TInterface>(
                std::make_shared<TImplementation>(
                    resolve<TDeps>()...
                ));
        };
        descriptors_[descriptor.typeName] = descriptor;
    }
    
    template<typename T>
    std::shared_ptr<T> resolve() {
        std::string typeName = typeid(T).name();
        auto it = descriptors_.find(typeName);
        
        if (it == descriptors_.end()) {
            throw std::runtime_error("Service not registered: " + typeName);
        }
        
        auto& descriptor = it->second;
        
        switch (descriptor.lifetime) {
            case ServiceLifetime::Singleton:
                if (!descriptor.instance.has_value()) {
                    DIContainer temp;
                    descriptor.instance = descriptor.factory(temp);
                }
                return std::any_cast<std::shared_ptr<T>>(descriptor.instance);
                
            case ServiceLifetime::Scoped:
                // Implementation for scoped services
                break;
                
            case ServiceLifetime::Transient:
                DIContainer temp;
                return std::any_cast<std::shared_ptr<T>>(descriptor.factory(temp));
        }
        
        return nullptr;
    }
};

// Example of Poor Man's DI (manual wiring)
class ApplicationBootstrapper {
public:
    static void bootstrapApplication() {
        std::cout << "\n=== Poor Man's DI (Manual Wiring) ===\n";
        
        // Create dependencies manually
        auto logger = std::make_shared<ConsoleLogger>("[APP]");
        auto database = std::make_shared<MySQLDatabase>("localhost:3306/myapp");
        auto emailService = std::make_shared<SMTPEmailService>("smtp.gmail.com", 587);
        auto cache = std::make_shared<MemoryCache>();
        
        // Wire up services
        auto userService = std::make_shared<UserService>(logger, database, emailService);
        
        auto productService = std::make_shared<ProductService>();
        productService->setLogger(logger);
        productService->setDatabase(database);
        productService->setCache(cache);
        
        // Use services
        userService->createUser("john_doe", "john@example.com");
        productService->getProduct(123);
    }
};

int main() {
    std::cout << "=== Dependency Injection Pattern Demo ===\n";
    
    // Constructor Injection Demo
    std::cout << "\n=== Constructor Injection ===\n";
    auto logger = std::make_shared<ConsoleLogger>("[MAIN]");
    auto database = std::make_shared<PostgreSQLDatabase>("localhost", 5432, "testdb");
    auto emailService = std::make_shared<SMTPEmailService>("mail.example.com", 25);
    
    UserService userService(logger, database, emailService);
    userService.createUser("alice", "alice@example.com");
    
    // Setter Injection Demo
    std::cout << "\n=== Setter Injection ===\n";
    ProductService productService;
    productService.setLogger(std::make_shared<FileLogger>("product.log"));
    productService.setDatabase(database);
    productService.setCache(std::make_shared<MemoryCache>());
    
    std::string product = productService.getProduct(456);
    std::cout << "Retrieved: " << product << "\n";
    
    // Interface Injection Demo
    std::cout << "\n=== Interface Injection ===\n";
    OrderService orderService(database, emailService);
    orderService.setLogger(logger); // Optional dependency
    orderService.createOrder(789, {1, 2, 3});
    
    // Simple DI Container Demo
    std::cout << "\n=== Simple DI Container ===\n";
    DIContainer container;
    
    // Register services
    container.registerSingleton<ILogger>(std::make_shared<ConsoleLogger>("[DI]"));
    container.registerSingleton<IDatabase>(
        std::make_shared<MySQLDatabase>("di-server:3306/app"));
    container.registerSingleton<IEmailService>(
        std::make_shared<SMTPEmailService>("di-smtp.com", 587));
    
    // Register factory for transient services
    container.registerFactory<ICache>([]() {
        return std::make_shared<MemoryCache>();
    });
    
    // Resolve services
    auto diLogger = container.resolve<ILogger>();
    diLogger->log("Resolved from DI container");
    
    // Build service with dependencies
    auto diUserService = std::make_shared<UserService>(
        container.resolve<ILogger>(),
        container.resolve<IDatabase>(),
        container.resolve<IEmailService>()
    );
    diUserService->createUser("bob", "bob@example.com");
    
    // Poor Man's DI
    ApplicationBootstrapper::bootstrapApplication();
    
    // Benefits of DI
    std::cout << "\n=== Benefits of Dependency Injection ===\n";
    std::cout << "1. Loose coupling between classes\n";
    std::cout << "2. Easy to test (mock dependencies)\n";
    std::cout << "3. Flexible configuration\n";
    std::cout << "4. Follows SOLID principles\n";
    std::cout << "5. Clear dependencies\n";
    
    return 0;
}