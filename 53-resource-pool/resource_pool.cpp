// Minimal Resource Pool Pattern Implementation
#include <iostream>
#include <memory>
#include <vector>
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <unordered_set>
#include <algorithm>
#include <random>

// Example 1: Basic Connection Pool
namespace BasicResourcePool {
    // Mock database connection
    class DatabaseConnection {
    private:
        std::string connectionId_;
        bool connected_;
        
    public:
        explicit DatabaseConnection(const std::string& id) 
            : connectionId_(id), connected_(false) {
            connect();
        }
        
        ~DatabaseConnection() {
            if (connected_) {
                disconnect();
            }
        }
        
        void connect() {
            std::cout << "Connection " << connectionId_ << ": Connecting to database...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate connection time
            connected_ = true;
            std::cout << "Connection " << connectionId_ << ": Connected\n";
        }
        
        void disconnect() {
            if (connected_) {
                std::cout << "Connection " << connectionId_ << ": Disconnecting\n";
                connected_ = false;
            }
        }
        
        void executeQuery(const std::string& query) {
            if (connected_) {
                std::cout << "Connection " << connectionId_ << ": Executing '" << query << "'\n";
            } else {
                std::cout << "Connection " << connectionId_ << ": Not connected!\n";
            }
        }
        
        const std::string& getId() const { return connectionId_; }
        bool isConnected() const { return connected_; }
    };
    
    // Basic resource pool
    template<typename T>
    class ResourcePool {
    private:
        std::queue<std::unique_ptr<T>> available_;
        std::unordered_set<T*> inUse_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        size_t maxSize_;
        std::function<std::unique_ptr<T>()> factory_;
        
    public:
        ResourcePool(size_t maxSize, std::function<std::unique_ptr<T>()> factory)
            : maxSize_(maxSize), factory_(factory) {
            std::cout << "ResourcePool: Created with max size " << maxSize_ << "\n";
        }
        
        ~ResourcePool() {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << "ResourcePool: Destroying pool with " 
                      << available_.size() << " available and " 
                      << inUse_.size() << " in-use resources\n";
        }
        
        std::unique_ptr<T> acquire() {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Wait for available resource or until we can create a new one
            cv_.wait(lock, [this] { 
                return !available_.empty() || (available_.size() + inUse_.size()) < maxSize_; 
            });
            
            std::unique_ptr<T> resource;
            
            if (!available_.empty()) {
                resource = std::move(available_.front());
                available_.pop();
                std::cout << "ResourcePool: Reusing existing resource\n";
            } else {
                // Create new resource
                resource = factory_();
                std::cout << "ResourcePool: Created new resource\n";
            }
            
            inUse_.insert(resource.get());
            std::cout << "ResourcePool: Acquired resource (" << inUse_.size() 
                      << " in use, " << available_.size() << " available)\n";
            
            return resource;
        }
        
        void release(std::unique_ptr<T> resource) {
            if (!resource) return;
            
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto it = inUse_.find(resource.get());
            if (it != inUse_.end()) {
                inUse_.erase(it);
                available_.push(std::move(resource));
                
                std::cout << "ResourcePool: Released resource (" << inUse_.size() 
                          << " in use, " << available_.size() << " available)\n";
                
                cv_.notify_one();
            } else {
                std::cout << "ResourcePool: Warning - releasing unknown resource\n";
            }
        }
        
        size_t getAvailableCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return available_.size();
        }
        
        size_t getInUseCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return inUse_.size();
        }
        
        size_t getTotalCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return available_.size() + inUse_.size();
        }
    };
}

// Example 2: Thread Pool
namespace ThreadPool {
    class ThreadPool {
    private:
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        mutable std::mutex queueMutex_;
        std::condition_variable cv_;
        std::atomic<bool> stop_{false};
        
    public:
        explicit ThreadPool(size_t numThreads) {
            std::cout << "ThreadPool: Creating pool with " << numThreads << " threads\n";
            
            for (size_t i = 0; i < numThreads; ++i) {
                workers_.emplace_back([this, i] {
                    std::cout << "Worker thread " << i << " started\n";
                    
                    while (true) {
                        std::function<void()> task;
                        
                        {
                            std::unique_lock<std::mutex> lock(queueMutex_);
                            cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                            
                            if (stop_ && tasks_.empty()) {
                                std::cout << "Worker thread " << i << " stopping\n";
                                return;
                            }
                            
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                        
                        std::cout << "Worker thread " << i << " executing task\n";
                        task();
                    }
                });
            }
        }
        
        ~ThreadPool() {
            std::cout << "ThreadPool: Shutting down\n";
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                stop_ = true;
            }
            
            cv_.notify_all();
            
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
            
            std::cout << "ThreadPool: All threads stopped\n";
        }
        
        template<typename F>
        void enqueue(F&& task) {
            if (stop_) {
                std::cout << "ThreadPool: Cannot enqueue task - pool is stopping\n";
                return;
            }
            
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                tasks_.emplace(std::forward<F>(task));
            }
            
            std::cout << "ThreadPool: Task enqueued (" << getQueueSize() << " in queue)\n";
            cv_.notify_one();
        }
        
        size_t getQueueSize() const {
            std::lock_guard<std::mutex> lock(queueMutex_);
            return tasks_.size();
        }
        
        size_t getThreadCount() const {
            return workers_.size();
        }
    };
}

// Example 3: Memory Pool
namespace MemoryPool {
    class MemoryPool {
    private:
        struct Block {
            char* data;
            size_t size;
            bool inUse;
            
            Block(size_t s) : size(s), inUse(false) {
                data = new char[size];
                std::cout << "MemoryPool: Allocated block of " << size << " bytes\n";
            }
            
            ~Block() {
                delete[] data;
                std::cout << "MemoryPool: Deallocated block of " << size << " bytes\n";
            }
        };
        
        std::vector<std::unique_ptr<Block>> blocks_;
        mutable std::mutex mutex_;
        size_t blockSize_;
        size_t maxBlocks_;
        
    public:
        MemoryPool(size_t blockSize, size_t maxBlocks)
            : blockSize_(blockSize), maxBlocks_(maxBlocks) {
            std::cout << "MemoryPool: Created with block size " << blockSize_ 
                      << " and max " << maxBlocks_ << " blocks\n";
        }
        
        void* allocate() {
            std::lock_guard<std::mutex> lock(mutex_);
            
            // Look for available block
            for (auto& block : blocks_) {
                if (!block->inUse) {
                    block->inUse = true;
                    std::cout << "MemoryPool: Reusing existing block\n";
                    return block->data;
                }
            }
            
            // Create new block if under limit
            if (blocks_.size() < maxBlocks_) {
                auto block = std::make_unique<Block>(blockSize_);
                void* ptr = block->data;
                block->inUse = true;
                blocks_.push_back(std::move(block));
                std::cout << "MemoryPool: Created new block (" << blocks_.size() 
                          << "/" << maxBlocks_ << ")\n";
                return ptr;
            }
            
            std::cout << "MemoryPool: No blocks available\n";
            return nullptr;
        }
        
        void deallocate(void* ptr) {
            if (!ptr) return;
            
            std::lock_guard<std::mutex> lock(mutex_);
            
            for (auto& block : blocks_) {
                if (block->data == ptr && block->inUse) {
                    block->inUse = false;
                    std::cout << "MemoryPool: Block returned to pool\n";
                    return;
                }
            }
            
            std::cout << "MemoryPool: Warning - deallocating unknown pointer\n";
        }
        
        size_t getAvailableBlocks() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return std::count_if(blocks_.begin(), blocks_.end(),
                [](const auto& block) { return !block->inUse; });
        }
        
        size_t getUsedBlocks() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return std::count_if(blocks_.begin(), blocks_.end(),
                [](const auto& block) { return block->inUse; });
        }
        
        size_t getTotalBlocks() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return blocks_.size();
        }
    };
}

// Example 4: Smart Resource Pool with RAII
namespace SmartResourcePool {
    template<typename T>
    class ResourcePool; // Forward declaration
    
    // RAII wrapper for pooled resources
    template<typename T>
    class PooledResource {
    private:
        std::unique_ptr<T> resource_;
        ResourcePool<T>* pool_;
        
    public:
        PooledResource(std::unique_ptr<T> resource, ResourcePool<T>* pool)
            : resource_(std::move(resource)), pool_(pool) {}
        
        ~PooledResource() {
            if (resource_ && pool_) {
                pool_->returnResource(std::move(resource_));
            }
        }
        
        // Move semantics
        PooledResource(PooledResource&& other) noexcept
            : resource_(std::move(other.resource_)), pool_(other.pool_) {
            other.pool_ = nullptr;
        }
        
        PooledResource& operator=(PooledResource&& other) noexcept {
            if (this != &other) {
                if (resource_ && pool_) {
                    pool_->returnResource(std::move(resource_));
                }
                resource_ = std::move(other.resource_);
                pool_ = other.pool_;
                other.pool_ = nullptr;
            }
            return *this;
        }
        
        // Disable copy
        PooledResource(const PooledResource&) = delete;
        PooledResource& operator=(const PooledResource&) = delete;
        
        T* operator->() { return resource_.get(); }
        const T* operator->() const { return resource_.get(); }
        T& operator*() { return *resource_; }
        const T& operator*() const { return *resource_; }
        
        bool valid() const { return resource_ != nullptr; }
    };
    
    template<typename T>
    class ResourcePool {
    private:
        std::queue<std::unique_ptr<T>> available_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        size_t maxSize_;
        size_t currentSize_;
        std::function<std::unique_ptr<T>()> factory_;
        
    public:
        ResourcePool(size_t maxSize, std::function<std::unique_ptr<T>()> factory)
            : maxSize_(maxSize), currentSize_(0), factory_(factory) {
            std::cout << "SmartResourcePool: Created with max size " << maxSize_ << "\n";
        }
        
        PooledResource<T> acquire() {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Wait for available resource or room to create new one
            cv_.wait(lock, [this] { 
                return !available_.empty() || currentSize_ < maxSize_; 
            });
            
            std::unique_ptr<T> resource;
            
            if (!available_.empty()) {
                resource = std::move(available_.front());
                available_.pop();
                std::cout << "SmartResourcePool: Reusing resource\n";
            } else {
                resource = factory_();
                currentSize_++;
                std::cout << "SmartResourcePool: Created new resource (" 
                          << currentSize_ << "/" << maxSize_ << ")\n";
            }
            
            return PooledResource<T>(std::move(resource), this);
        }
        
        void returnResource(std::unique_ptr<T> resource) {
            if (!resource) return;
            
            std::lock_guard<std::mutex> lock(mutex_);
            available_.push(std::move(resource));
            std::cout << "SmartResourcePool: Resource returned (" 
                      << available_.size() << " available)\n";
            cv_.notify_one();
        }
        
        size_t getAvailableCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return available_.size();
        }
        
        size_t getCurrentSize() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return currentSize_;
        }
    };
}

// Example 5: Resource Pool with Health Checking
namespace HealthCheckedPool {
    // Resource that can be health-checked
    class NetworkConnection {
    private:
        std::string endpoint_;
        bool healthy_;
        int connectionId_;
        static std::atomic<int> idCounter_;
        
    public:
        explicit NetworkConnection(const std::string& endpoint)
            : endpoint_(endpoint), healthy_(true), connectionId_(idCounter_++) {
            std::cout << "NetworkConnection " << connectionId_ 
                      << ": Connecting to " << endpoint_ << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        bool isHealthy() {
            // Simulate random health check failure (10% chance)
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(1, 10);
            
            if (dis(gen) == 1) {
                healthy_ = false;
                std::cout << "NetworkConnection " << connectionId_ 
                          << ": Health check failed\n";
            }
            
            return healthy_;
        }
        
        void sendData(const std::string& data) {
            if (healthy_) {
                std::cout << "NetworkConnection " << connectionId_ 
                          << ": Sending '" << data << "'\n";
            } else {
                std::cout << "NetworkConnection " << connectionId_ 
                          << ": Cannot send - unhealthy\n";
            }
        }
        
        int getId() const { return connectionId_; }
        const std::string& getEndpoint() const { return endpoint_; }
    };
    
    std::atomic<int> NetworkConnection::idCounter_{1};
    
    class HealthCheckedResourcePool {
    private:
        std::queue<std::unique_ptr<NetworkConnection>> available_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        size_t maxSize_;
        std::string endpoint_;
        
        std::unique_ptr<NetworkConnection> createResource() {
            return std::make_unique<NetworkConnection>(endpoint_);
        }
        
    public:
        HealthCheckedResourcePool(size_t maxSize, const std::string& endpoint)
            : maxSize_(maxSize), endpoint_(endpoint) {
            std::cout << "HealthCheckedResourcePool: Created for " << endpoint_ 
                      << " with max size " << maxSize_ << "\n";
        }
        
        std::unique_ptr<NetworkConnection> acquire() {
            std::unique_lock<std::mutex> lock(mutex_);
            
            while (true) {
                // Try to find healthy resource
                while (!available_.empty()) {
                    auto resource = std::move(available_.front());
                    available_.pop();
                    
                    if (resource->isHealthy()) {
                        std::cout << "HealthCheckedResourcePool: Acquired healthy resource " 
                                  << resource->getId() << "\n";
                        return resource;
                    } else {
                        std::cout << "HealthCheckedResourcePool: Discarding unhealthy resource " 
                                  << resource->getId() << "\n";
                        // Resource is destroyed here
                    }
                }
                
                // Create new resource if we have capacity
                if (getTotalCount() < maxSize_) {
                    auto resource = createResource();
                    std::cout << "HealthCheckedResourcePool: Created new resource " 
                              << resource->getId() << "\n";
                    return resource;
                }
                
                // Wait for resources to become available
                std::cout << "HealthCheckedResourcePool: Waiting for resources...\n";
                cv_.wait(lock);
            }
        }
        
        void release(std::unique_ptr<NetworkConnection> resource) {
            if (!resource) return;
            
            std::lock_guard<std::mutex> lock(mutex_);
            
            if (resource->isHealthy()) {
                available_.push(std::move(resource));
                std::cout << "HealthCheckedResourcePool: Returned healthy resource\n";
            } else {
                std::cout << "HealthCheckedResourcePool: Discarding unhealthy resource " 
                          << resource->getId() << "\n";
                // Resource is destroyed
            }
            
            cv_.notify_one();
        }
        
        size_t getTotalCount() const {
            // This is a simplified count - in real implementation,
            // you'd track both available and in-use resources
            return available_.size();
        }
    };
}

// Demo functions
void demonstrateBasicResourcePool() {
    using namespace BasicResourcePool;
    
    std::cout << "=== Basic Resource Pool ===\n";
    
    // Create connection pool
    int connectionCounter = 1;
    ResourcePool<DatabaseConnection> pool(3, [&connectionCounter]() {
        return std::make_unique<DatabaseConnection>("conn-" + std::to_string(connectionCounter++));
    });
    
    std::vector<std::thread> threads;
    
    // Multiple threads using the pool
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&pool, i]() {
            std::cout << "Thread " << i << " acquiring connection...\n";
            auto conn = pool.acquire();
            
            conn->executeQuery("SELECT * FROM users WHERE thread_id = " + std::to_string(i));
            
            // Hold connection for a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            std::cout << "Thread " << i << " releasing connection...\n";
            pool.release(std::move(conn));
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final pool state: " << pool.getAvailableCount() 
              << " available, " << pool.getTotalCount() << " total\n";
}

void demonstrateThreadPool() {
    std::cout << "\n=== Thread Pool ===\n";
    
    ThreadPool::ThreadPool pool(3);
    
    // Submit various tasks
    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i]() {
            std::cout << "Task " << i << " executing\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "Task " << i << " completed\n";
        });
    }
    
    // Let tasks complete
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Thread pool demo completed\n";
}

void demonstrateMemoryPool() {
    std::cout << "\n=== Memory Pool ===\n";
    
    MemoryPool::MemoryPool pool(1024, 5); // 1KB blocks, max 5 blocks
    
    std::vector<void*> allocations;
    
    // Allocate several blocks
    for (int i = 0; i < 7; ++i) {
        std::cout << "Allocating block " << i << "...\n";
        void* ptr = pool.allocate();
        if (ptr) {
            allocations.push_back(ptr);
            std::cout << "Allocated block " << i << "\n";
        } else {
            std::cout << "Failed to allocate block " << i << "\n";
        }
    }
    
    std::cout << "\nPool status: " << pool.getUsedBlocks() << " used, "
              << pool.getAvailableBlocks() << " available\n";
    
    // Deallocate some blocks
    for (size_t i = 0; i < allocations.size() / 2; ++i) {
        std::cout << "Deallocating block " << i << "...\n";
        pool.deallocate(allocations[i]);
    }
    
    std::cout << "\nAfter partial deallocation: " << pool.getUsedBlocks() << " used, "
              << pool.getAvailableBlocks() << " available\n";
    
    // Try allocating again
    void* newPtr = pool.allocate();
    if (newPtr) {
        std::cout << "Successfully allocated from freed blocks\n";
        pool.deallocate(newPtr);
    }
    
    // Clean up remaining allocations
    for (size_t i = allocations.size() / 2; i < allocations.size(); ++i) {
        pool.deallocate(allocations[i]);
    }
}

void demonstrateSmartResourcePool() {
    using namespace SmartResourcePool;
    using BasicResourcePool::DatabaseConnection;
    
    std::cout << "\n=== Smart Resource Pool (RAII) ===\n";
    
    int connectionCounter = 1;
    BasicResourcePool::ResourcePool<DatabaseConnection> pool(2, [&connectionCounter]() {
        return std::make_unique<DatabaseConnection>("smart-conn-" + std::to_string(connectionCounter++));
    });
    
    {
        std::cout << "\nAcquiring resources in scope...\n";
        auto conn1 = pool.acquire();
        conn1->executeQuery("SELECT 1");
        
        {
            auto conn2 = pool.acquire();
            conn2->executeQuery("SELECT 2");
            
            std::cout << "Inner scope ending - conn2 auto-released\n";
        }
        
        conn1->executeQuery("SELECT 3");
        std::cout << "Outer scope ending - conn1 auto-released\n";
    }
    
    std::cout << "\nAll resources should be back in pool\n";
    std::cout << "Available resources: " << pool.getAvailableCount() << "\n";
}

void demonstrateHealthCheckedPool() {
    using namespace HealthCheckedPool;
    
    std::cout << "\n=== Health-Checked Resource Pool ===\n";
    
    HealthCheckedResourcePool pool(3, "api.example.com:443");
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&pool, i]() {
            for (int attempt = 0; attempt < 3; ++attempt) {
                std::cout << "Thread " << i << " attempt " << attempt << "\n";
                
                auto conn = pool.acquire();
                conn->sendData("Hello from thread " + std::to_string(i));
                
                // Hold connection briefly
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                
                pool.release(std::move(conn));
                
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    std::cout << "=== Resource Pool Pattern Demo ===\n\n";
    
    demonstrateBasicResourcePool();
    demonstrateThreadPool();
    demonstrateMemoryPool();
    demonstrateSmartResourcePool();
    demonstrateHealthCheckedPool();
    
    std::cout << "\n=== Resource Pool Benefits ===\n";
    std::cout << "1. Efficient resource reuse\n";
    std::cout << "2. Controlled resource creation\n";
    std::cout << "3. Better performance and scalability\n";
    std::cout << "4. Resource lifecycle management\n";
    std::cout << "5. Thread-safe resource sharing\n";
    
    return 0;
}