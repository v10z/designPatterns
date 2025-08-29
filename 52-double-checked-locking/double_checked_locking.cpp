// Minimal Double-Checked Locking Pattern Implementation
#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <random>

// Example 1: Classic Double-Checked Locking Singleton
namespace ClassicDCL {
    class DatabaseConnection {
    private:
        std::string connectionString_;
        int connectionId_;
        
        explicit DatabaseConnection(const std::string& connStr) 
            : connectionString_(connStr) {
            // Simulate expensive connection setup
            static int idCounter = 1;
            connectionId_ = idCounter++;
            std::cout << "DatabaseConnection: Creating connection " 
                      << connectionId_ << " to " << connStr << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        static std::atomic<DatabaseConnection*> instance_;
        static std::mutex mutex_;
        
    public:
        static DatabaseConnection* getInstance(const std::string& connStr = "default") {
            // First check - no locking
            DatabaseConnection* result = instance_.load(std::memory_order_acquire);
            if (result == nullptr) {
                // Lock and check again
                std::lock_guard<std::mutex> lock(mutex_);
                result = instance_.load(std::memory_order_relaxed);
                if (result == nullptr) {
                    std::cout << "DCL: Creating singleton instance\n";
                    result = new DatabaseConnection(connStr);
                    instance_.store(result, std::memory_order_release);
                }
            }
            return result;
        }
        
        void executeQuery(const std::string& query) {
            std::cout << "Connection " << connectionId_ 
                      << ": Executing '" << query << "'\n";
        }
        
        int getConnectionId() const {
            return connectionId_;
        }
        
        const std::string& getConnectionString() const {
            return connectionString_;
        }
        
        // Cleanup method for demo purposes
        static void cleanup() {
            delete instance_.load();
            instance_.store(nullptr);
        }
    };
    
    // Static member definitions
    std::atomic<DatabaseConnection*> DatabaseConnection::instance_{nullptr};
    std::mutex DatabaseConnection::mutex_;
}

// Example 2: Modern C++ Double-Checked Locking with shared_ptr
namespace ModernDCL {
    class ConfigurationManager {
    private:
        std::string configPath_;
        std::map<std::string, std::string> settings_;
        
        explicit ConfigurationManager(const std::string& path) 
            : configPath_(path) {
            std::cout << "ConfigurationManager: Loading config from " << path << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            
            // Simulate loading configuration
            settings_["database.host"] = "localhost";
            settings_["database.port"] = "5432";
            settings_["app.name"] = "MyApplication";
            settings_["log.level"] = "INFO";
        }
        
        static std::atomic<std::shared_ptr<ConfigurationManager>> instance_;
        static std::mutex mutex_;
        
    public:
        static std::shared_ptr<ConfigurationManager> getInstance(const std::string& path = "config.json") {
            // First check without locking
            std::shared_ptr<ConfigurationManager> result = 
                std::atomic_load_explicit(&instance_, std::memory_order_acquire);
            
            if (!result) {
                // Lock and check again
                std::lock_guard<std::mutex> lock(mutex_);
                result = std::atomic_load_explicit(&instance_, std::memory_order_relaxed);
                
                if (!result) {
                    std::cout << "DCL: Creating ConfigurationManager instance\n";
                    result = std::shared_ptr<ConfigurationManager>(
                        new ConfigurationManager(path));
                    std::atomic_store_explicit(&instance_, result, std::memory_order_release);
                }
            }
            
            return result;
        }
        
        std::string getSetting(const std::string& key, const std::string& defaultValue = "") const {
            auto it = settings_.find(key);
            return (it != settings_.end()) ? it->second : defaultValue;
        }
        
        const std::map<std::string, std::string>& getAllSettings() const {
            return settings_;
        }
        
        const std::string& getConfigPath() const {
            return configPath_;
        }
        
        static void reset() {
            std::lock_guard<std::mutex> lock(mutex_);
            instance_.store(nullptr);
        }
    };
    
    // Static member definitions
    std::atomic<std::shared_ptr<ConfigurationManager>> 
        ConfigurationManager::instance_{nullptr};
    std::mutex ConfigurationManager::mutex_;
}

// Example 3: Generic Double-Checked Locking Template
namespace GenericDCL {
    template<typename T>
    class DCLSingleton {
    private:
        static std::atomic<T*> instance_;
        static std::mutex mutex_;
        
    protected:
        DCLSingleton() = default;
        
    public:
        template<typename... Args>
        static T* getInstance(Args&&... args) {
            // First check
            T* result = instance_.load(std::memory_order_acquire);
            if (result == nullptr) {
                // Lock and double-check
                std::lock_guard<std::mutex> lock(mutex_);
                result = instance_.load(std::memory_order_relaxed);
                if (result == nullptr) {
                    std::cout << "DCL Template: Creating instance of " 
                              << typeid(T).name() << "\n";
                    result = new T(std::forward<Args>(args)...);
                    instance_.store(result, std::memory_order_release);
                }
            }
            return result;
        }
        
        static void destroyInstance() {
            std::lock_guard<std::mutex> lock(mutex_);
            T* instance = instance_.load();
            if (instance) {
                delete instance;
                instance_.store(nullptr);
            }
        }
        
        // Delete copy operations
        DCLSingleton(const DCLSingleton&) = delete;
        DCLSingleton& operator=(const DCLSingleton&) = delete;
    };
    
    // Static member definitions for template
    template<typename T>
    std::atomic<T*> DCLSingleton<T>::instance_{nullptr};
    
    template<typename T>
    std::mutex DCLSingleton<T>::mutex_;
    
    // Example usage classes
    class Logger : public DCLSingleton<Logger> {
    private:
        std::string logFile_;
        std::mutex logMutex_;
        
        friend class DCLSingleton<Logger>;
        
        explicit Logger(const std::string& logFile = "app.log") 
            : logFile_(logFile) {
            std::cout << "Logger: Initializing with file " << logFile_ << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
    public:
        void log(const std::string& message) {
            std::lock_guard<std::mutex> lock(logMutex_);
            std::cout << "[LOG] " << message << " (to " << logFile_ << ")\n";
        }
        
        const std::string& getLogFile() const {
            return logFile_;
        }
    };
    
    class MetricsCollector : public DCLSingleton<MetricsCollector> {
    private:
        std::map<std::string, int> metrics_;
        std::mutex metricsMutex_;
        
        friend class DCLSingleton<MetricsCollector>;
        
        MetricsCollector() {
            std::cout << "MetricsCollector: Initializing metrics system\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(75));
        }
        
    public:
        void increment(const std::string& metric) {
            std::lock_guard<std::mutex> lock(metricsMutex_);
            metrics_[metric]++;
        }
        
        int getMetric(const std::string& metric) {
            std::lock_guard<std::mutex> lock(metricsMutex_);
            return metrics_[metric];
        }
        
        void printMetrics() {
            std::lock_guard<std::mutex> lock(metricsMutex_);
            std::cout << "Metrics:\n";
            for (const auto& [name, value] : metrics_) {
                std::cout << "  " << name << ": " << value << "\n";
            }
        }
    };
}

// Example 4: Double-Checked Locking for Resource Pools
namespace ResourcePoolDCL {
    class ConnectionPool {
    private:
        std::vector<std::unique_ptr<std::string>> connections_;
        std::mutex poolMutex_;
        int maxConnections_;
        std::atomic<bool> initialized_{false};
        static std::mutex initMutex_;
        
        void initializePool() {
            std::cout << "ConnectionPool: Initializing pool with " 
                      << maxConnections_ << " connections\n";
            
            connections_.reserve(maxConnections_);
            for (int i = 0; i < maxConnections_; ++i) {
                connections_.push_back(
                    std::make_unique<std::string>("Connection-" + std::to_string(i + 1))
                );
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            std::cout << "ConnectionPool: Initialization complete\n";
        }
        
    public:
        explicit ConnectionPool(int maxConnections = 10) 
            : maxConnections_(maxConnections) {}
        
        std::string* getConnection() {
            // First check - is pool initialized?
            if (!initialized_.load(std::memory_order_acquire)) {
                // Lock and double-check
                std::lock_guard<std::mutex> lock(initMutex_);
                if (!initialized_.load(std::memory_order_relaxed)) {
                    initializePool();
                    initialized_.store(true, std::memory_order_release);
                }
            }
            
            // Get connection from pool
            std::lock_guard<std::mutex> lock(poolMutex_);
            if (!connections_.empty()) {
                auto connection = connections_.back().release();
                connections_.pop_back();
                std::cout << "ConnectionPool: Borrowed connection: " << *connection << "\n";
                return connection;
            }
            
            std::cout << "ConnectionPool: No connections available\n";
            return nullptr;
        }
        
        void returnConnection(std::string* connection) {
            if (connection) {
                std::lock_guard<std::mutex> lock(poolMutex_);
                connections_.push_back(std::unique_ptr<std::string>(connection));
                std::cout << "ConnectionPool: Returned connection: " << *connection << "\n";
            }
        }
        
        size_t getAvailableConnections() {
            // Ensure pool is initialized
            if (!initialized_.load(std::memory_order_acquire)) {
                std::lock_guard<std::mutex> lock(initMutex_);
                if (!initialized_.load(std::memory_order_relaxed)) {
                    initializePool();
                    initialized_.store(true, std::memory_order_release);
                }
            }
            
            std::lock_guard<std::mutex> lock(poolMutex_);
            return connections_.size();
        }
    };
    
    std::mutex ConnectionPool::initMutex_;
}

// Example 5: Double-Checked Locking with Call-Once
namespace CallOnceDCL {
    class ServiceRegistry {
    private:
        std::map<std::string, std::string> services_;
        static std::once_flag initialized_;
        static ServiceRegistry* instance_;
        static std::mutex instanceMutex_;
        
        ServiceRegistry() = default;
        
        void initialize() {
            std::cout << "ServiceRegistry: Initializing service registry\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Register default services
            services_["database"] = "postgresql://localhost:5432";
            services_["cache"] = "redis://localhost:6379";
            services_["queue"] = "rabbitmq://localhost:5672";
            services_["search"] = "elasticsearch://localhost:9200";
            
            std::cout << "ServiceRegistry: Registered " << services_.size() << " services\n";
        }
        
    public:
        static ServiceRegistry* getInstance() {
            // Use call_once for initialization
            std::call_once(initialized_, []() {
                std::lock_guard<std::mutex> lock(instanceMutex_);
                if (!instance_) {
                    std::cout << "CallOnce DCL: Creating ServiceRegistry instance\n";
                    instance_ = new ServiceRegistry();
                    instance_->initialize();
                }
            });
            
            return instance_;
        }
        
        std::string getService(const std::string& name) {
            auto it = services_.find(name);
            return (it != services_.end()) ? it->second : "";
        }
        
        void registerService(const std::string& name, const std::string& endpoint) {
            services_[name] = endpoint;
            std::cout << "ServiceRegistry: Registered " << name << " -> " << endpoint << "\n";
        }
        
        const std::map<std::string, std::string>& getAllServices() const {
            return services_;
        }
        
        static void cleanup() {
            std::lock_guard<std::mutex> lock(instanceMutex_);
            delete instance_;
            instance_ = nullptr;
        }
    };
    
    // Static member definitions
    std::once_flag ServiceRegistry::initialized_;
    ServiceRegistry* ServiceRegistry::instance_ = nullptr;
    std::mutex ServiceRegistry::instanceMutex_;
}

// Demo functions
void demonstrateClassicDCL() {
    using namespace ClassicDCL;
    
    std::cout << "=== Classic Double-Checked Locking ===\n";
    
    // Test concurrent access
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i]() {
            std::cout << "Thread " << i << " requesting database connection\n";
            auto* db = DatabaseConnection::getInstance("postgresql://localhost:5432/mydb");
            db->executeQuery("SELECT * FROM users WHERE id = " + std::to_string(i));
            std::cout << "Thread " << i << " got connection ID: " << db->getConnectionId() << "\n";
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    DatabaseConnection::cleanup();
}

void demonstrateModernDCL() {
    using namespace ModernDCL;
    
    std::cout << "\n=== Modern C++ Double-Checked Locking ===\n";
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([i]() {
            std::cout << "Thread " << i << " requesting configuration\n";
            auto config = ConfigurationManager::getInstance("app_config.json");
            
            std::cout << "Thread " << i << " - App name: " 
                      << config->getSetting("app.name") << "\n";
            std::cout << "Thread " << i << " - DB host: " 
                      << config->getSetting("database.host") << "\n";
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    ConfigurationManager::reset();
}

void demonstrateGenericDCL() {
    using namespace GenericDCL;
    
    std::cout << "\n=== Generic DCL Template ===\n";
    
    std::vector<std::thread> threads;
    
    // Test Logger singleton
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i]() {
            auto* logger = Logger::getInstance("application.log");
            logger->log("Message from thread " + std::to_string(i));
        });
    }
    
    // Test MetricsCollector singleton
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i]() {
            auto* metrics = MetricsCollector::getInstance();
            metrics->increment("requests");
            metrics->increment("thread_" + std::to_string(i));
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nMetrics after all threads:\n";
    MetricsCollector::getInstance()->printMetrics();
    
    Logger::destroyInstance();
    MetricsCollector::destroyInstance();
}

void demonstrateResourcePoolDCL() {
    using namespace ResourcePoolDCL;
    
    std::cout << "\n=== Resource Pool DCL ===\n";
    
    ConnectionPool pool(5); // Pool with 5 connections
    std::vector<std::thread> threads;
    std::vector<std::string*> borrowedConnections;
    std::mutex connectionsMutex;
    
    // Multiple threads trying to get connections
    for (int i = 0; i < 7; ++i) {
        threads.emplace_back([&pool, &borrowedConnections, &connectionsMutex, i]() {
            std::cout << "Thread " << i << " requesting connection\n";
            auto* conn = pool.getConnection();
            
            if (conn) {
                // Hold connection briefly
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                std::lock_guard<std::mutex> lock(connectionsMutex);
                borrowedConnections.push_back(conn);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nReturning all borrowed connections:\n";
    for (auto* conn : borrowedConnections) {
        pool.returnConnection(conn);
    }
    
    std::cout << "Available connections: " << pool.getAvailableConnections() << "\n";
}

void demonstrateCallOnceDCL() {
    using namespace CallOnceDCL;
    
    std::cout << "\n=== Call-Once DCL ===\n";
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([i]() {
            std::cout << "Thread " << i << " requesting service registry\n";
            auto* registry = ServiceRegistry::getInstance();
            
            std::cout << "Thread " << i << " - Database service: " 
                      << registry->getService("database") << "\n";
            
            if (i == 2) {
                registry->registerService("monitoring", "prometheus://localhost:9090");
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nAll registered services:\n";
    auto* registry = ServiceRegistry::getInstance();
    for (const auto& [name, endpoint] : registry->getAllServices()) {
        std::cout << "  " << name << " -> " << endpoint << "\n";
    }
    
    ServiceRegistry::cleanup();
}

int main() {
    std::cout << "=== Double-Checked Locking Pattern Demo ===\n\n";
    
    demonstrateClassicDCL();
    demonstrateModernDCL();
    demonstrateGenericDCL();
    demonstrateResourcePoolDCL();
    demonstrateCallOnceDCL();
    
    std::cout << "\n=== Double-Checked Locking Benefits ===\n";
    std::cout << "1. Reduces synchronization overhead\n";
    std::cout << "2. Thread-safe lazy initialization\n";
    std::cout << "3. Avoids unnecessary locking\n";
    std::cout << "4. Maintains singleton guarantees\n";
    std::cout << "5. Better performance than simple locking\n";
    
    return 0;
}