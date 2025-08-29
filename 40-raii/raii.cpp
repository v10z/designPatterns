// Minimal RAII (Resource Acquisition Is Initialization) Pattern Implementation
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <stdexcept>

// Example 1: File Handle RAII
class FileHandle {
private:
    std::FILE* file_;
    std::string filename_;
    
public:
    // Resource acquisition in constructor
    explicit FileHandle(const std::string& filename, const std::string& mode) 
        : filename_(filename), file_(nullptr) {
        file_ = std::fopen(filename.c_str(), mode.c_str());
        if (!file_) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        std::cout << "File opened: " << filename_ << "\n";
    }
    
    // Resource release in destructor
    ~FileHandle() {
        if (file_) {
            std::fclose(file_);
            std::cout << "File closed: " << filename_ << "\n";
        }
    }
    
    // Delete copy operations to prevent double-close
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    
    // Move operations
    FileHandle(FileHandle&& other) noexcept 
        : file_(other.file_), filename_(std::move(other.filename_)) {
        other.file_ = nullptr;
    }
    
    FileHandle& operator=(FileHandle&& other) noexcept {
        if (this != &other) {
            if (file_) {
                std::fclose(file_);
            }
            file_ = other.file_;
            filename_ = std::move(other.filename_);
            other.file_ = nullptr;
        }
        return *this;
    }
    
    // File operations
    void write(const std::string& data) {
        if (file_) {
            std::fputs(data.c_str(), file_);
            std::fflush(file_);
        }
    }
    
    std::string read(size_t size) {
        if (!file_) return "";
        
        std::vector<char> buffer(size + 1, '\0');
        size_t bytesRead = std::fread(buffer.data(), 1, size, file_);
        return std::string(buffer.data(), bytesRead);
    }
    
    bool isOpen() const { return file_ != nullptr; }
};

// Example 2: Memory Buffer RAII
template<typename T>
class MemoryBuffer {
private:
    T* data_;
    size_t size_;
    
public:
    // Acquire memory in constructor
    explicit MemoryBuffer(size_t size) : size_(size), data_(nullptr) {
        if (size > 0) {
            data_ = new T[size];
            std::cout << "Allocated " << size << " elements of type " 
                      << typeid(T).name() << "\n";
        }
    }
    
    // Initialize with value
    MemoryBuffer(size_t size, const T& value) : MemoryBuffer(size) {
        std::fill(data_, data_ + size_, value);
    }
    
    // Release memory in destructor
    ~MemoryBuffer() {
        if (data_) {
            delete[] data_;
            std::cout << "Deallocated " << size_ << " elements\n";
        }
    }
    
    // Delete copy constructor and assignment
    MemoryBuffer(const MemoryBuffer&) = delete;
    MemoryBuffer& operator=(const MemoryBuffer&) = delete;
    
    // Move constructor
    MemoryBuffer(MemoryBuffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    // Move assignment
    MemoryBuffer& operator=(MemoryBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    // Access operations
    T& operator[](size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        return data_[index];
    }
    
    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        return data_[index];
    }
    
    T* data() { return data_; }
    const T* data() const { return data_; }
    size_t size() const { return size_; }
};

// Example 3: Lock Guard RAII
class MutexGuard {
private:
    std::mutex& mutex_;
    bool locked_;
    
public:
    // Lock in constructor
    explicit MutexGuard(std::mutex& mutex) : mutex_(mutex), locked_(true) {
        mutex_.lock();
        std::cout << "Mutex locked by thread " << std::this_thread::get_id() << "\n";
    }
    
    // Unlock in destructor
    ~MutexGuard() {
        if (locked_) {
            mutex_.unlock();
            std::cout << "Mutex unlocked by thread " << std::this_thread::get_id() << "\n";
        }
    }
    
    // Delete copy operations
    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;
    
    // Allow early unlock
    void unlock() {
        if (locked_) {
            mutex_.unlock();
            locked_ = false;
            std::cout << "Mutex explicitly unlocked by thread " 
                      << std::this_thread::get_id() << "\n";
        }
    }
};

// Example 4: Database Connection RAII
class DatabaseConnection {
private:
    std::string connectionString_;
    bool connected_;
    
    void connect() {
        std::cout << "Connecting to database: " << connectionString_ << "\n";
        // Simulate connection
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        connected_ = true;
    }
    
    void disconnect() {
        if (connected_) {
            std::cout << "Disconnecting from database\n";
            connected_ = false;
        }
    }
    
public:
    // Connect in constructor
    explicit DatabaseConnection(const std::string& connectionString)
        : connectionString_(connectionString), connected_(false) {
        connect();
    }
    
    // Disconnect in destructor
    ~DatabaseConnection() {
        disconnect();
    }
    
    // Delete copy operations
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    
    // Move operations
    DatabaseConnection(DatabaseConnection&& other) noexcept
        : connectionString_(std::move(other.connectionString_)),
          connected_(other.connected_) {
        other.connected_ = false;
    }
    
    DatabaseConnection& operator=(DatabaseConnection&& other) noexcept {
        if (this != &other) {
            disconnect();
            connectionString_ = std::move(other.connectionString_);
            connected_ = other.connected_;
            other.connected_ = false;
        }
        return *this;
    }
    
    void executeQuery(const std::string& query) {
        if (!connected_) {
            throw std::runtime_error("Not connected to database");
        }
        std::cout << "Executing query: " << query << "\n";
    }
    
    bool isConnected() const { return connected_; }
};

// Example 5: Socket RAII
class Socket {
private:
    int socket_fd_;
    std::string address_;
    int port_;
    
public:
    // Create socket in constructor
    Socket(const std::string& address, int port) 
        : address_(address), port_(port), socket_fd_(-1) {
        // Simulate socket creation
        socket_fd_ = 42; // Dummy socket descriptor
        std::cout << "Socket created for " << address_ << ":" << port_ << "\n";
    }
    
    // Close socket in destructor
    ~Socket() {
        if (socket_fd_ >= 0) {
            std::cout << "Socket closed for " << address_ << ":" << port_ << "\n";
            socket_fd_ = -1;
        }
    }
    
    // Delete copy operations
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    // Move operations
    Socket(Socket&& other) noexcept
        : socket_fd_(other.socket_fd_), 
          address_(std::move(other.address_)),
          port_(other.port_) {
        other.socket_fd_ = -1;
    }
    
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            if (socket_fd_ >= 0) {
                // Close current socket
                socket_fd_ = -1;
            }
            socket_fd_ = other.socket_fd_;
            address_ = std::move(other.address_);
            port_ = other.port_;
            other.socket_fd_ = -1;
        }
        return *this;
    }
    
    void send(const std::string& data) {
        if (socket_fd_ < 0) {
            throw std::runtime_error("Socket not connected");
        }
        std::cout << "Sending data: " << data << "\n";
    }
    
    bool isConnected() const { return socket_fd_ >= 0; }
};

// Example 6: Transaction RAII
class Transaction {
private:
    DatabaseConnection& connection_;
    bool committed_;
    bool active_;
    
public:
    // Begin transaction in constructor
    explicit Transaction(DatabaseConnection& conn) 
        : connection_(conn), committed_(false), active_(true) {
        connection_.executeQuery("BEGIN TRANSACTION");
    }
    
    // Rollback in destructor if not committed
    ~Transaction() {
        if (active_ && !committed_) {
            try {
                connection_.executeQuery("ROLLBACK");
                std::cout << "Transaction rolled back\n";
            } catch (...) {
                // Ignore exceptions in destructor
            }
        }
    }
    
    // Delete copy operations
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;
    
    void commit() {
        if (active_ && !committed_) {
            connection_.executeQuery("COMMIT");
            committed_ = true;
            active_ = false;
            std::cout << "Transaction committed\n";
        }
    }
    
    void rollback() {
        if (active_ && !committed_) {
            connection_.executeQuery("ROLLBACK");
            active_ = false;
            std::cout << "Transaction explicitly rolled back\n";
        }
    }
};

// Example 7: Scoped Timer RAII
class ScopedTimer {
private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_;
    
public:
    // Start timer in constructor
    explicit ScopedTimer(const std::string& name) 
        : name_(name), start_(std::chrono::high_resolution_clock::now()) {
        std::cout << "Timer '" << name_ << "' started\n";
    }
    
    // Report elapsed time in destructor
    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        std::cout << "Timer '" << name_ << "' elapsed: " 
                  << duration.count() << " microseconds\n";
    }
    
    // Delete copy operations
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
};

// Example 8: Array Pointer RAII Wrapper
template<typename T>
class ArrayPtr {
private:
    std::unique_ptr<T[]> data_;
    size_t size_;
    
public:
    explicit ArrayPtr(size_t size) : size_(size) {
        if (size > 0) {
            data_ = std::make_unique<T[]>(size);
            std::cout << "Array allocated with " << size << " elements\n";
        }
    }
    
    // Destructor automatically called by unique_ptr
    ~ArrayPtr() {
        std::cout << "Array deallocated\n";
    }
    
    T& operator[](size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        return data_[index];
    }
    
    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of bounds");
        }
        return data_[index];
    }
    
    size_t size() const { return size_; }
};

// Demo functions
void demonstrateFileHandling() {
    std::cout << "=== File Handling RAII ===\n";
    
    try {
        FileHandle file("test.txt", "w");
        file.write("Hello, RAII!\n");
        file.write("Resource management made easy.\n");
        
        // File automatically closed when going out of scope
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    
    // Read the file
    try {
        FileHandle file("test.txt", "r");
        std::string content = file.read(100);
        std::cout << "File content: " << content << "\n";
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
}

void demonstrateMemoryBuffer() {
    std::cout << "\n=== Memory Buffer RAII ===\n";
    
    {
        MemoryBuffer<int> buffer(10, 42);
        
        std::cout << "Buffer contents: ";
        for (size_t i = 0; i < buffer.size(); ++i) {
            std::cout << buffer[i] << " ";
        }
        std::cout << "\n";
        
        // Modify some values
        buffer[0] = 100;
        buffer[5] = 200;
        
        // Memory automatically freed when going out of scope
    }
    
    // Move semantics
    {
        MemoryBuffer<double> buffer1(5);
        MemoryBuffer<double> buffer2(std::move(buffer1));
        
        std::cout << "Buffer2 size: " << buffer2.size() << "\n";
        // buffer1 is now empty
    }
}

void threadFunction(std::mutex& mutex, int id) {
    MutexGuard guard(mutex);
    
    std::cout << "Thread " << id << " is working...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Mutex automatically unlocked when guard goes out of scope
}

void demonstrateLocking() {
    std::cout << "\n=== Mutex Guard RAII ===\n";
    
    std::mutex mutex;
    std::vector<std::thread> threads;
    
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(threadFunction, std::ref(mutex), i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

void demonstrateDatabaseTransaction() {
    std::cout << "\n=== Database Transaction RAII ===\n";
    
    try {
        DatabaseConnection db("server=localhost;db=test");
        
        // Successful transaction
        {
            Transaction txn(db);
            db.executeQuery("INSERT INTO users VALUES ('john', 'john@example.com')");
            db.executeQuery("UPDATE stats SET count = count + 1");
            txn.commit();
        }
        
        // Failed transaction (automatic rollback)
        {
            Transaction txn(db);
            db.executeQuery("INSERT INTO users VALUES ('jane', 'jane@example.com')");
            // Simulate error - transaction will rollback automatically
            std::cout << "Simulating error - transaction will rollback\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "Database error: " << e.what() << "\n";
    }
}

void demonstrateScopedTimer() {
    std::cout << "\n=== Scoped Timer RAII ===\n";
    
    {
        ScopedTimer timer("Overall operation");
        
        {
            ScopedTimer timer2("Step 1");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        {
            ScopedTimer timer3("Step 2");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

int main() {
    std::cout << "=== RAII (Resource Acquisition Is Initialization) Pattern Demo ===\n\n";
    
    demonstrateFileHandling();
    demonstrateMemoryBuffer();
    demonstrateLocking();
    demonstrateDatabaseTransaction();
    demonstrateScopedTimer();
    
    std::cout << "\n=== RAII Benefits ===\n";
    std::cout << "1. Automatic resource management\n";
    std::cout << "2. Exception safety\n";
    std::cout << "3. No resource leaks\n";
    std::cout << "4. Clear ownership semantics\n";
    std::cout << "5. Simplified code\n";
    
    std::cout << "\n=== RAII Rules ===\n";
    std::cout << "1. Acquire resources in constructor\n";
    std::cout << "2. Release resources in destructor\n";
    std::cout << "3. Never throw from destructor\n";
    std::cout << "4. Delete or implement copy operations\n";
    std::cout << "5. Implement move operations when appropriate\n";
    
    return 0;
}