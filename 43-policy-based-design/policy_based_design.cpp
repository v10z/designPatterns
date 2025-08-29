// Minimal Policy-Based Design Pattern Implementation
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <mutex>
#include <thread>
#include <cstring>
#include <stdexcept>

// Example 1: Smart Pointer with Policies
namespace SmartPointerPolicies {
    // Storage Policies
    template<typename T>
    class DefaultStorage {
    protected:
        T* ptr_;
        
    public:
        DefaultStorage() : ptr_(nullptr) {}
        explicit DefaultStorage(T* p) : ptr_(p) {}
        
        ~DefaultStorage() {
            delete ptr_;
        }
        
        T* get() const { return ptr_; }
        void reset(T* p = nullptr) {
            delete ptr_;
            ptr_ = p;
        }
        
        T* release() {
            T* tmp = ptr_;
            ptr_ = nullptr;
            return tmp;
        }
    };
    
    template<typename T>
    class ArrayStorage {
    protected:
        T* ptr_;
        
    public:
        ArrayStorage() : ptr_(nullptr) {}
        explicit ArrayStorage(T* p) : ptr_(p) {}
        
        ~ArrayStorage() {
            delete[] ptr_;
        }
        
        T* get() const { return ptr_; }
        void reset(T* p = nullptr) {
            delete[] ptr_;
            ptr_ = p;
        }
        
        T* release() {
            T* tmp = ptr_;
            ptr_ = nullptr;
            return tmp;
        }
    };
    
    // Ownership Policies
    template<typename T>
    class ExclusiveOwnership {
    public:
        ExclusiveOwnership() = default;
        
        // Delete copy operations
        ExclusiveOwnership(const ExclusiveOwnership&) = delete;
        ExclusiveOwnership& operator=(const ExclusiveOwnership&) = delete;
        
        // Allow move operations
        ExclusiveOwnership(ExclusiveOwnership&&) = default;
        ExclusiveOwnership& operator=(ExclusiveOwnership&&) = default;
    };
    
    template<typename T>
    class RefCountedOwnership {
    private:
        mutable size_t* refCount_;
        
    public:
        RefCountedOwnership() : refCount_(new size_t(1)) {}
        
        RefCountedOwnership(const RefCountedOwnership& other) 
            : refCount_(other.refCount_) {
            ++(*refCount_);
        }
        
        RefCountedOwnership& operator=(const RefCountedOwnership& other) {
            if (this != &other) {
                decrementRef();
                refCount_ = other.refCount_;
                ++(*refCount_);
            }
            return *this;
        }
        
        ~RefCountedOwnership() {
            decrementRef();
        }
        
        size_t getRefCount() const { return *refCount_; }
        
    protected:
        bool isLastRef() const { return *refCount_ == 1; }
        
        void decrementRef() {
            if (--(*refCount_) == 0) {
                delete refCount_;
            }
        }
    };
    
    // Checking Policy
    template<typename T>
    class NoChecking {
    public:
        static void checkPointer(T* ptr) {
            // No checking
        }
    };
    
    template<typename T>
    class EnforceNotNull {
    public:
        static void checkPointer(T* ptr) {
            if (!ptr) {
                throw std::runtime_error("Null pointer access");
            }
        }
    };
    
    // Smart Pointer Template
    template<
        typename T,
        template<typename> class StoragePolicy = DefaultStorage,
        template<typename> class OwnershipPolicy = ExclusiveOwnership,
        template<typename> class CheckingPolicy = NoChecking
    >
    class SmartPtr : public StoragePolicy<T>, 
                     public OwnershipPolicy<T>,
                     public CheckingPolicy<T> {
    private:
        using Storage = StoragePolicy<T>;
        using Ownership = OwnershipPolicy<T>;
        using Checking = CheckingPolicy<T>;
        
    public:
        SmartPtr() = default;
        explicit SmartPtr(T* p) : Storage(p) {}
        
        T& operator*() {
            Checking::checkPointer(this->get());
            return *this->get();
        }
        
        T* operator->() {
            Checking::checkPointer(this->get());
            return this->get();
        }
        
        const T& operator*() const {
            Checking::checkPointer(this->get());
            return *this->get();
        }
        
        const T* operator->() const {
            Checking::checkPointer(this->get());
            return this->get();
        }
    };
}

// Example 2: Thread-Safe Container with Policies
namespace ThreadSafeContainer {
    // Locking Policies
    class NoLocking {
    public:
        void lock() {}
        void unlock() {}
    };
    
    class MutexLocking {
    private:
        mutable std::mutex mutex_;
        
    public:
        void lock() { mutex_.lock(); }
        void unlock() { mutex_.unlock(); }
        std::mutex& getMutex() { return mutex_; }
    };
    
    class RecursiveLocking {
    private:
        mutable std::recursive_mutex mutex_;
        
    public:
        void lock() { mutex_.lock(); }
        void unlock() { mutex_.unlock(); }
    };
    
    // Growth Policies
    class FixedSize {
    public:
        static size_t getNewCapacity(size_t current, size_t required) {
            if (required > 1000) {
                throw std::length_error("Container size limit exceeded");
            }
            return required;
        }
    };
    
    class ExponentialGrowth {
    public:
        static size_t getNewCapacity(size_t current, size_t required) {
            size_t newCapacity = current;
            while (newCapacity < required) {
                newCapacity = newCapacity * 2 + 1;
            }
            return newCapacity;
        }
    };
    
    class LinearGrowth {
    public:
        static size_t getNewCapacity(size_t current, size_t required) {
            return required + 10; // Add 10 extra elements
        }
    };
    
    // Thread-Safe Vector
    template<
        typename T,
        typename LockingPolicy = MutexLocking,
        typename GrowthPolicy = ExponentialGrowth
    >
    class SafeVector : private LockingPolicy {
    private:
        std::vector<T> data_;
        using Lock = LockingPolicy;
        
        class LockGuard {
        private:
            LockingPolicy* policy_;
            
        public:
            explicit LockGuard(LockingPolicy* p) : policy_(p) {
                policy_->lock();
            }
            
            ~LockGuard() {
                policy_->unlock();
            }
        };
        
    public:
        void push_back(const T& value) {
            LockGuard guard(this);
            
            if (data_.size() == data_.capacity()) {
                size_t newCapacity = GrowthPolicy::getNewCapacity(
                    data_.capacity(), data_.size() + 1);
                data_.reserve(newCapacity);
            }
            
            data_.push_back(value);
        }
        
        T pop_back() {
            LockGuard guard(this);
            
            if (data_.empty()) {
                throw std::out_of_range("Container is empty");
            }
            
            T value = data_.back();
            data_.pop_back();
            return value;
        }
        
        size_t size() const {
            LockGuard guard(const_cast<SafeVector*>(this));
            return data_.size();
        }
        
        bool empty() const {
            LockGuard guard(const_cast<SafeVector*>(this));
            return data_.empty();
        }
    };
}

// Example 3: String Class with Policies
namespace StringPolicies {
    // Character Case Policies
    class PreserveCase {
    public:
        static char processChar(char c) { return c; }
        static std::string processString(const std::string& s) { return s; }
    };
    
    class UpperCase {
    public:
        static char processChar(char c) { return std::toupper(c); }
        static std::string processString(const std::string& s) {
            std::string result = s;
            std::transform(result.begin(), result.end(), result.begin(), ::toupper);
            return result;
        }
    };
    
    class LowerCase {
    public:
        static char processChar(char c) { return std::tolower(c); }
        static std::string processString(const std::string& s) {
            std::string result = s;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return result;
        }
    };
    
    // Storage Optimization Policies
    class EagerCopy {
    protected:
        std::string data_;
        
    public:
        EagerCopy() = default;
        explicit EagerCopy(const std::string& s) : data_(s) {}
        
        std::string& getData() { return data_; }
        const std::string& getData() const { return data_; }
        
        void ensureUnique() {} // Always unique
    };
    
    class CopyOnWrite {
    protected:
        std::shared_ptr<std::string> data_;
        
    public:
        CopyOnWrite() : data_(std::make_shared<std::string>()) {}
        explicit CopyOnWrite(const std::string& s) 
            : data_(std::make_shared<std::string>(s)) {}
        
        std::string& getData() {
            ensureUnique();
            return *data_;
        }
        
        const std::string& getData() const { return *data_; }
        
        void ensureUnique() {
            if (data_.use_count() > 1) {
                data_ = std::make_shared<std::string>(*data_);
            }
        }
    };
    
    // Policy-Based String
    template<
        typename CasePolicy = PreserveCase,
        typename StoragePolicy = EagerCopy
    >
    class String : private StoragePolicy {
    private:
        using Storage = StoragePolicy;
        
    public:
        String() = default;
        
        explicit String(const std::string& s) 
            : Storage(CasePolicy::processString(s)) {}
        
        String(const char* s) 
            : Storage(CasePolicy::processString(std::string(s))) {}
        
        String& operator+=(const String& other) {
            this->getData() += other.getData();
            return *this;
        }
        
        String& operator+=(char c) {
            this->getData() += CasePolicy::processChar(c);
            return *this;
        }
        
        char operator[](size_t index) const {
            return this->getData()[index];
        }
        
        size_t length() const {
            return this->getData().length();
        }
        
        const std::string& str() const {
            return this->getData();
        }
        
        void append(const std::string& s) {
            this->getData() += CasePolicy::processString(s);
        }
    };
}

// Example 4: Allocator with Policies
namespace AllocatorPolicies {
    // Allocation Policies
    template<typename T>
    class MallocAllocator {
    public:
        static T* allocate(size_t n) {
            void* p = std::malloc(n * sizeof(T));
            if (!p) throw std::bad_alloc();
            std::cout << "Malloc: Allocated " << n << " objects\n";
            return static_cast<T*>(p);
        }
        
        static void deallocate(T* p, size_t n) {
            std::cout << "Malloc: Deallocated " << n << " objects\n";
            std::free(p);
        }
    };
    
    template<typename T>
    class NewAllocator {
    public:
        static T* allocate(size_t n) {
            std::cout << "New: Allocated " << n << " objects\n";
            return new T[n];
        }
        
        static void deallocate(T* p, size_t n) {
            std::cout << "New: Deallocated " << n << " objects\n";
            delete[] p;
        }
    };
    
    template<typename T, size_t PoolSize = 1024>
    class PoolAllocator {
    private:
        static inline char memory_[PoolSize * sizeof(T)];
        static inline bool used_[PoolSize] = {false};
        
    public:
        static T* allocate(size_t n) {
            if (n > PoolSize) throw std::bad_alloc();
            
            // Find contiguous free blocks
            for (size_t i = 0; i <= PoolSize - n; ++i) {
                bool canAllocate = true;
                for (size_t j = 0; j < n; ++j) {
                    if (used_[i + j]) {
                        canAllocate = false;
                        break;
                    }
                }
                
                if (canAllocate) {
                    for (size_t j = 0; j < n; ++j) {
                        used_[i + j] = true;
                    }
                    std::cout << "Pool: Allocated " << n << " objects\n";
                    return reinterpret_cast<T*>(&memory_[i * sizeof(T)]);
                }
            }
            
            throw std::bad_alloc();
        }
        
        static void deallocate(T* p, size_t n) {
            size_t index = (reinterpret_cast<char*>(p) - memory_) / sizeof(T);
            for (size_t i = 0; i < n; ++i) {
                used_[index + i] = false;
            }
            std::cout << "Pool: Deallocated " << n << " objects\n";
        }
    };
    
    // Tracking Policy
    class NoTracking {
    public:
        static void trackAllocation(size_t) {}
        static void trackDeallocation(size_t) {}
        static size_t getCurrentUsage() { return 0; }
        static size_t getPeakUsage() { return 0; }
    };
    
    class MemoryTracking {
    private:
        static inline size_t currentUsage_ = 0;
        static inline size_t peakUsage_ = 0;
        
    public:
        static void trackAllocation(size_t bytes) {
            currentUsage_ += bytes;
            if (currentUsage_ > peakUsage_) {
                peakUsage_ = currentUsage_;
            }
        }
        
        static void trackDeallocation(size_t bytes) {
            currentUsage_ -= bytes;
        }
        
        static size_t getCurrentUsage() { return currentUsage_; }
        static size_t getPeakUsage() { return peakUsage_; }
    };
    
    // Policy-Based Allocator
    template<
        typename T,
        template<typename> class AllocationPolicy = NewAllocator,
        typename TrackingPolicy = NoTracking
    >
    class Allocator {
    private:
        using Allocation = AllocationPolicy<T>;
        
    public:
        using value_type = T;
        
        Allocator() = default;
        
        template<typename U>
        Allocator(const Allocator<U, AllocationPolicy, TrackingPolicy>&) {}
        
        T* allocate(size_t n) {
            TrackingPolicy::trackAllocation(n * sizeof(T));
            return Allocation::allocate(n);
        }
        
        void deallocate(T* p, size_t n) {
            Allocation::deallocate(p, n);
            TrackingPolicy::trackDeallocation(n * sizeof(T));
        }
        
        template<typename U>
        struct rebind {
            using other = Allocator<U, AllocationPolicy, TrackingPolicy>;
        };
    };
}

// Demo functions
void demonstrateSmartPointers() {
    using namespace SmartPointerPolicies;
    
    std::cout << "=== Smart Pointer with Policies ===\n";
    
    // Exclusive ownership, no checking
    using UniquePtr = SmartPtr<int, DefaultStorage, ExclusiveOwnership, NoChecking>;
    UniquePtr p1(new int(42));
    std::cout << "Unique pointer value: " << *p1 << "\n";
    
    // Reference counted, with null checking
    using SharedPtr = SmartPtr<std::string, DefaultStorage, RefCountedOwnership, EnforceNotNull>;
    SharedPtr p2(new std::string("Hello"));
    std::cout << "Shared pointer value: " << *p2 << "\n";
    
    // Array storage
    using ArrayPtr = SmartPtr<int, ArrayStorage, ExclusiveOwnership, NoChecking>;
    ArrayPtr p3(new int[5]{1, 2, 3, 4, 5});
    std::cout << "Array pointer first element: " << *p3 << "\n";
    
    // Test null checking
    try {
        using CheckedPtr = SmartPtr<int, DefaultStorage, ExclusiveOwnership, EnforceNotNull>;
        CheckedPtr p4;
        *p4; // Should throw
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << "\n";
    }
}

void demonstrateThreadSafeContainer() {
    using namespace ThreadSafeContainer;
    
    std::cout << "\n=== Thread-Safe Container with Policies ===\n";
    
    // Thread-safe vector with mutex locking
    SafeVector<int, MutexLocking, ExponentialGrowth> safeVec;
    
    // Launch multiple threads
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&safeVec, i]() {
            for (int j = 0; j < 5; ++j) {
                safeVec.push_back(i * 10 + j);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Container size: " << safeVec.size() << "\n";
    
    // No locking vector (single-threaded)
    SafeVector<double, NoLocking, LinearGrowth> fastVec;
    for (int i = 0; i < 5; ++i) {
        fastVec.push_back(i * 1.1);
    }
    std::cout << "Fast vector size: " << fastVec.size() << "\n";
}

void demonstrateStringPolicies() {
    using namespace StringPolicies;
    
    std::cout << "\n=== String Class with Policies ===\n";
    
    // Normal string
    String<PreserveCase, EagerCopy> normal("Hello World");
    std::cout << "Normal string: " << normal.str() << "\n";
    
    // Uppercase string
    String<UpperCase, EagerCopy> upper("Hello World");
    upper += '!';
    std::cout << "Upper string: " << upper.str() << "\n";
    
    // Lowercase string with copy-on-write
    String<LowerCase, CopyOnWrite> lower("Hello World");
    lower.append(" from C++");
    std::cout << "Lower string: " << lower.str() << "\n";
    
    // Test case conversion
    String<UpperCase> upperName("john doe");
    std::cout << "Uppercase name: " << upperName.str() << "\n";
}

void demonstrateAllocatorPolicies() {
    using namespace AllocatorPolicies;
    
    std::cout << "\n=== Allocator with Policies ===\n";
    
    // Vector with custom allocator
    using TrackingAllocator = Allocator<int, NewAllocator, MemoryTracking>;
    std::vector<int, TrackingAllocator> vec1;
    
    for (int i = 0; i < 10; ++i) {
        vec1.push_back(i);
    }
    
    std::cout << "Current memory usage: " << MemoryTracking::getCurrentUsage() << " bytes\n";
    std::cout << "Peak memory usage: " << MemoryTracking::getPeakUsage() << " bytes\n";
    
    // Vector with pool allocator
    using PoolAlloc = Allocator<double, PoolAllocator, NoTracking>;
    std::vector<double, PoolAlloc> vec2;
    
    for (int i = 0; i < 5; ++i) {
        vec2.push_back(i * 1.5);
    }
    
    vec2.clear();
    
    // Vector with malloc allocator
    using MallocAlloc = Allocator<std::string, MallocAllocator, NoTracking>;
    std::vector<std::string, MallocAlloc> vec3;
    vec3.push_back("Test");
}

int main() {
    std::cout << "=== Policy-Based Design Pattern Demo ===\n\n";
    
    demonstrateSmartPointers();
    demonstrateThreadSafeContainer();
    demonstrateStringPolicies();
    demonstrateAllocatorPolicies();
    
    std::cout << "\n=== Policy-Based Design Benefits ===\n";
    std::cout << "1. Compile-time configuration\n";
    std::cout << "2. No runtime overhead\n";
    std::cout << "3. High flexibility\n";
    std::cout << "4. Type safety\n";
    std::cout << "5. Orthogonal design\n";
    
    return 0;
}