// Minimal Monostate Pattern Implementation
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <functional>

// Example 1: Basic Monostate Configuration
namespace BasicMonostate {
    class Configuration {
    private:
        // All data members are static (shared state)
        static std::string appName_;
        static std::string version_;
        static int maxConnections_;
        static bool debugMode_;
        static std::string logLevel_;
        
    public:
        // Constructors - create instances but share state
        Configuration() = default;
        
        Configuration(const std::string& appName, const std::string& version)
            : Configuration() {
            appName_ = appName;
            version_ = version;
        }
        
        // All instances share the same data
        void setAppName(const std::string& name) { appName_ = name; }
        std::string getAppName() const { return appName_; }
        
        void setVersion(const std::string& version) { version_ = version; }
        std::string getVersion() const { return version_; }
        
        void setMaxConnections(int max) { maxConnections_ = max; }
        int getMaxConnections() const { return maxConnections_; }
        
        void setDebugMode(bool debug) { debugMode_ = debug; }
        bool isDebugMode() const { return debugMode_; }
        
        void setLogLevel(const std::string& level) { logLevel_ = level; }
        std::string getLogLevel() const { return logLevel_; }
        
        void print() const {
            std::cout << "Configuration:\n";
            std::cout << "  App Name: " << appName_ << "\n";
            std::cout << "  Version: " << version_ << "\n";
            std::cout << "  Max Connections: " << maxConnections_ << "\n";
            std::cout << "  Debug Mode: " << (debugMode_ ? "ON" : "OFF") << "\n";
            std::cout << "  Log Level: " << logLevel_ << "\n";
        }
    };
    
    // Static member definitions
    std::string Configuration::appName_ = "DefaultApp";
    std::string Configuration::version_ = "1.0.0";
    int Configuration::maxConnections_ = 100;
    bool Configuration::debugMode_ = false;
    std::string Configuration::logLevel_ = "INFO";
}

// Example 2: Thread-Safe Monostate Logger
namespace ThreadSafeMonostate {
    class Logger {
    private:
        // Shared state with synchronization
        static std::vector<std::string> logs_;
        static std::mutex mutex_;
        static std::string filename_;
        static bool consoleOutput_;
        static std::chrono::steady_clock::time_point startTime_;
        
        static std::string getCurrentTimestamp() {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                           (now - startTime_);
            
            return "[" + std::to_string(duration.count()) + "ms]";
        }
        
    public:
        Logger() = default;
        
        void log(const std::string& message) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            std::string logEntry = getCurrentTimestamp() + " " + message;
            logs_.push_back(logEntry);
            
            if (consoleOutput_) {
                std::cout << logEntry << "\n";
            }
        }
        
        void setFilename(const std::string& filename) {
            std::lock_guard<std::mutex> lock(mutex_);
            filename_ = filename;
        }
        
        void setConsoleOutput(bool enable) {
            std::lock_guard<std::mutex> lock(mutex_);
            consoleOutput_ = enable;
        }
        
        std::vector<std::string> getLogs() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return logs_;
        }
        
        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            logs_.clear();
        }
        
        size_t getLogCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return logs_.size();
        }
    };
    
    // Static member definitions
    std::vector<std::string> Logger::logs_;
    std::mutex Logger::mutex_;
    std::string Logger::filename_ = "app.log";
    bool Logger::consoleOutput_ = true;
    std::chrono::steady_clock::time_point Logger::startTime_ = 
        std::chrono::steady_clock::now();
}

// Example 3: Monostate vs Singleton Comparison
namespace MonostateVsSingleton {
    // Monostate implementation
    class MonostateCounter {
    private:
        static int count_;
        static int instances_;
        
    public:
        MonostateCounter() { 
            instances_++; 
            std::cout << "MonostateCounter instance #" << instances_ << " created\n";
        }
        
        ~MonostateCounter() {
            instances_--;
            std::cout << "MonostateCounter instance destroyed (remaining: " 
                      << instances_ << ")\n";
        }
        
        void increment() { count_++; }
        void decrement() { count_--; }
        int getCount() const { return count_; }
        static int getInstanceCount() { return instances_; }
    };
    
    int MonostateCounter::count_ = 0;
    int MonostateCounter::instances_ = 0;
    
    // Singleton implementation for comparison
    class SingletonCounter {
    private:
        int count_ = 0;
        static SingletonCounter* instance_;
        static int accessCount_;
        
        SingletonCounter() {
            std::cout << "SingletonCounter created\n";
        }
        
    public:
        static SingletonCounter& getInstance() {
            if (!instance_) {
                instance_ = new SingletonCounter();
            }
            accessCount_++;
            return *instance_;
        }
        
        void increment() { count_++; }
        void decrement() { count_--; }
        int getCount() const { return count_; }
        static int getAccessCount() { return accessCount_; }
        
        // Delete copy/move
        SingletonCounter(const SingletonCounter&) = delete;
        SingletonCounter& operator=(const SingletonCounter&) = delete;
    };
    
    SingletonCounter* SingletonCounter::instance_ = nullptr;
    int SingletonCounter::accessCount_ = 0;
}

// Example 4: Monostate with Template
namespace TemplateMonostate {
    template<typename Tag>
    class Settings {
    private:
        static std::unordered_map<std::string, std::string> settings_;
        static std::mutex mutex_;
        
    public:
        Settings() = default;
        
        void set(const std::string& key, const std::string& value) {
            std::lock_guard<std::mutex> lock(mutex_);
            settings_[key] = value;
        }
        
        std::string get(const std::string& key, 
                        const std::string& defaultValue = "") const {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = settings_.find(key);
            return (it != settings_.end()) ? it->second : defaultValue;
        }
        
        bool has(const std::string& key) const {
            std::lock_guard<std::mutex> lock(mutex_);
            return settings_.find(key) != settings_.end();
        }
        
        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            settings_.clear();
        }
        
        size_t size() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return settings_.size();
        }
        
        void printAll() const {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << "Settings for " << typeid(Tag).name() << ":\n";
            for (const auto& [key, value] : settings_) {
                std::cout << "  " << key << " = " << value << "\n";
            }
        }
    };
    
    // Static member definitions
    template<typename Tag>
    std::unordered_map<std::string, std::string> Settings<Tag>::settings_;
    
    template<typename Tag>
    std::mutex Settings<Tag>::mutex_;
    
    // Tags for different setting domains
    struct AppSettings {};
    struct UserSettings {};
    struct NetworkSettings {};
}

// Example 5: Monostate Registry Pattern
namespace MonostateRegistry {
    class ServiceRegistry {
    private:
        using ServiceFactory = std::function<std::shared_ptr<void>()>;
        static std::unordered_map<std::string, ServiceFactory> factories_;
        static std::unordered_map<std::string, std::shared_ptr<void>> instances_;
        static std::mutex mutex_;
        
    public:
        ServiceRegistry() = default;
        
        template<typename T>
        void registerService(const std::string& name, ServiceFactory factory) {
            std::lock_guard<std::mutex> lock(mutex_);
            factories_[name] = factory;
            std::cout << "Registered service: " << name << "\n";
        }
        
        template<typename T>
        void registerSingleton(const std::string& name, std::shared_ptr<T> instance) {
            std::lock_guard<std::mutex> lock(mutex_);
            instances_[name] = instance;
            std::cout << "Registered singleton: " << name << "\n";
        }
        
        template<typename T>
        std::shared_ptr<T> getService(const std::string& name) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            // Check singletons first
            auto instIt = instances_.find(name);
            if (instIt != instances_.end()) {
                return std::static_pointer_cast<T>(instIt->second);
            }
            
            // Check factories
            auto factIt = factories_.find(name);
            if (factIt != factories_.end()) {
                auto instance = factIt->second();
                return std::static_pointer_cast<T>(instance);
            }
            
            return nullptr;
        }
        
        bool hasService(const std::string& name) const {
            std::lock_guard<std::mutex> lock(mutex_);
            return instances_.find(name) != instances_.end() ||
                   factories_.find(name) != factories_.end();
        }
        
        std::vector<std::string> getRegisteredServices() const {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<std::string> names;
            
            for (const auto& [name, _] : instances_) {
                names.push_back(name + " (singleton)");
            }
            
            for (const auto& [name, _] : factories_) {
                if (instances_.find(name) == instances_.end()) {
                    names.push_back(name + " (factory)");
                }
            }
            
            return names;
        }
    };
    
    // Static member definitions
    std::unordered_map<std::string, ServiceRegistry::ServiceFactory> 
        ServiceRegistry::factories_;
    std::unordered_map<std::string, std::shared_ptr<void>> 
        ServiceRegistry::instances_;
    std::mutex ServiceRegistry::mutex_;
    
    // Example services
    class DatabaseService {
    public:
        void connect() { std::cout << "Database connected\n"; }
    };
    
    class CacheService {
    public:
        void put(const std::string& key, const std::string& value) {
            std::cout << "Cached: " << key << " = " << value << "\n";
        }
    };
}

// Demo functions
void demonstrateBasicMonostate() {
    using namespace BasicMonostate;
    
    std::cout << "=== Basic Monostate Configuration ===\n";
    
    // Create multiple instances
    Configuration config1;
    Configuration config2("MyApp", "2.0.0");
    Configuration config3;
    
    std::cout << "Initial state from config1:\n";
    config1.print();
    
    // Modify through different instances
    config2.setMaxConnections(200);
    config3.setDebugMode(true);
    config1.setLogLevel("DEBUG");
    
    std::cout << "\nAfter modifications from different instances:\n";
    config1.print();
    
    // All instances see the same state
    std::cout << "\nconfig2 sees: " << config2.getMaxConnections() << " connections\n";
    std::cout << "config3 sees debug mode: " << config3.isDebugMode() << "\n";
}

void demonstrateThreadSafeMonostate() {
    using namespace ThreadSafeMonostate;
    
    std::cout << "\n=== Thread-Safe Monostate Logger ===\n";
    
    Logger logger1;
    Logger logger2;
    
    // Launch multiple threads
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i]() {
            Logger threadLogger;
            for (int j = 0; j < 3; ++j) {
                threadLogger.log("Thread " + std::to_string(i) + 
                               " message " + std::to_string(j));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Total log entries: " << logger1.getLogCount() << "\n";
    std::cout << "Logger2 also sees: " << logger2.getLogCount() << " entries\n";
}

void demonstrateMonostateVsSingleton() {
    using namespace MonostateVsSingleton;
    
    std::cout << "\n=== Monostate vs Singleton ===\n";
    
    std::cout << "Monostate pattern:\n";
    {
        MonostateCounter m1;
        MonostateCounter m2;
        MonostateCounter m3;
        
        m1.increment();
        m2.increment();
        m3.increment();
        
        std::cout << "Count from m1: " << m1.getCount() << "\n";
        std::cout << "Count from m2: " << m2.getCount() << "\n";
        std::cout << "Active instances: " << MonostateCounter::getInstanceCount() << "\n";
    }
    std::cout << "After scope: " << MonostateCounter::getInstanceCount() << " instances\n";
    
    std::cout << "\nSingleton pattern:\n";
    SingletonCounter::getInstance().increment();
    SingletonCounter::getInstance().increment();
    SingletonCounter::getInstance().increment();
    
    std::cout << "Count: " << SingletonCounter::getInstance().getCount() << "\n";
    std::cout << "Access count: " << SingletonCounter::getAccessCount() << "\n";
}

void demonstrateTemplateMonostate() {
    using namespace TemplateMonostate;
    
    std::cout << "\n=== Template Monostate ===\n";
    
    // Different setting domains
    Settings<AppSettings> appSettings1;
    Settings<AppSettings> appSettings2;
    Settings<UserSettings> userSettings;
    Settings<NetworkSettings> networkSettings;
    
    // App settings
    appSettings1.set("theme", "dark");
    appSettings2.set("language", "en-US");
    appSettings1.set("autosave", "true");
    
    // User settings
    userSettings.set("username", "john_doe");
    userSettings.set("email", "john@example.com");
    
    // Network settings
    networkSettings.set("proxy", "proxy.company.com:8080");
    networkSettings.set("timeout", "30");
    
    std::cout << "App settings (from instance 2):\n";
    appSettings2.printAll();
    
    std::cout << "\nUser settings:\n";
    userSettings.printAll();
    
    std::cout << "\nNetwork settings:\n";
    networkSettings.printAll();
}

void demonstrateMonostateRegistry() {
    using namespace MonostateRegistry;
    
    std::cout << "\n=== Monostate Registry ===\n";
    
    ServiceRegistry registry1;
    ServiceRegistry registry2;
    
    // Register services through different instances
    registry1.registerSingleton<DatabaseService>("database", 
        std::make_shared<DatabaseService>());
    
    registry2.registerService<CacheService>("cache", []() {
        return std::make_shared<CacheService>();
    });
    
    // Access from any instance
    auto db = registry1.getService<DatabaseService>("database");
    if (db) db->connect();
    
    auto cache = registry2.getService<CacheService>("cache");
    if (cache) cache->put("key1", "value1");
    
    // List all services
    std::cout << "\nRegistered services:\n";
    for (const auto& name : registry1.getRegisteredServices()) {
        std::cout << "  - " << name << "\n";
    }
}

int main() {
    std::cout << "=== Monostate Pattern Demo ===\n\n";
    
    demonstrateBasicMonostate();
    demonstrateThreadSafeMonostate();
    demonstrateMonostateVsSingleton();
    demonstrateTemplateMonostate();
    demonstrateMonostateRegistry();
    
    std::cout << "\n=== Monostate Pattern Characteristics ===\n";
    std::cout << "1. All instances share the same state\n";
    std::cout << "2. Normal constructor/destructor semantics\n";
    std::cout << "3. Can be passed by value or reference\n";
    std::cout << "4. Supports inheritance\n";
    std::cout << "5. Thread safety must be explicitly handled\n";
    
    std::cout << "\n=== Monostate vs Singleton ===\n";
    std::cout << "Monostate advantages:\n";
    std::cout << "- Normal object semantics\n";
    std::cout << "- Can have multiple instances\n";
    std::cout << "- Easier to test\n";
    std::cout << "- Works with existing code\n";
    
    std::cout << "\nMonostate disadvantages:\n";
    std::cout << "- Hidden global state\n";
    std::cout << "- Initialization order issues\n";
    std::cout << "- Memory overhead per instance\n";
    
    return 0;
}