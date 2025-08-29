// Minimal Pimpl (Pointer to Implementation) Pattern Implementation
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <unordered_map>

// Example 1: Basic Pimpl - Widget Class
// widget.h - Public header (what clients see)
namespace BasicPimpl {
    class Widget {
    private:
        // Forward declaration of implementation
        class Impl;
        std::unique_ptr<Impl> pImpl;
        
    public:
        // Constructor/Destructor must be defined in .cpp
        Widget();
        explicit Widget(const std::string& name);
        ~Widget();
        
        // Copy and move operations
        Widget(const Widget& other);
        Widget& operator=(const Widget& other);
        Widget(Widget&& other) noexcept;
        Widget& operator=(Widget&& other) noexcept;
        
        // Public interface
        void setName(const std::string& name);
        std::string getName() const;
        void performOperation();
        void addValue(int value);
        int getTotal() const;
    };
}

// widget.cpp - Implementation file
namespace BasicPimpl {
    // Private implementation class
    class Widget::Impl {
    public:
        std::string name_;
        std::vector<int> values_;
        int total_ = 0;
        
        Impl() : name_("Default") {
            std::cout << "Widget::Impl default constructed\n";
        }
        
        explicit Impl(const std::string& name) : name_(name) {
            std::cout << "Widget::Impl constructed with name: " << name << "\n";
        }
        
        ~Impl() {
            std::cout << "Widget::Impl destroyed: " << name_ << "\n";
        }
        
        void performOperation() {
            std::cout << "Performing operation on widget: " << name_ << "\n";
            // Complex implementation details hidden from client
            total_ = 0;
            for (int val : values_) {
                total_ += val;
            }
        }
    };
    
    // Widget public methods implementation
    Widget::Widget() : pImpl(std::make_unique<Impl>()) {}
    
    Widget::Widget(const std::string& name) 
        : pImpl(std::make_unique<Impl>(name)) {}
    
    Widget::~Widget() = default;
    
    Widget::Widget(const Widget& other) 
        : pImpl(std::make_unique<Impl>(*other.pImpl)) {}
    
    Widget& Widget::operator=(const Widget& other) {
        if (this != &other) {
            pImpl = std::make_unique<Impl>(*other.pImpl);
        }
        return *this;
    }
    
    Widget::Widget(Widget&& other) noexcept = default;
    Widget& Widget::operator=(Widget&& other) noexcept = default;
    
    void Widget::setName(const std::string& name) {
        pImpl->name_ = name;
    }
    
    std::string Widget::getName() const {
        return pImpl->name_;
    }
    
    void Widget::performOperation() {
        pImpl->performOperation();
    }
    
    void Widget::addValue(int value) {
        pImpl->values_.push_back(value);
    }
    
    int Widget::getTotal() const {
        return pImpl->total_;
    }
}

// Example 2: Pimpl with Complex Dependencies
namespace ComplexPimpl {
    // database.h - Public interface
    class Database {
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
        
    public:
        Database(const std::string& connectionString);
        ~Database();
        
        // Delete copy, allow move
        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
        Database(Database&&) noexcept;
        Database& operator=(Database&&) noexcept;
        
        void connect();
        void disconnect();
        void executeQuery(const std::string& query);
        std::vector<std::string> fetchResults();
        bool isConnected() const;
    };
    
    // database.cpp - Implementation
    class Database::Impl {
    private:
        // Complex internal dependencies (simulated)
        struct ConnectionPool {
            std::vector<int> connections;
            void initialize() {
                std::cout << "Initializing connection pool\n";
                for (int i = 0; i < 5; ++i) {
                    connections.push_back(i);
                }
            }
        };
        
        struct QueryCache {
            std::vector<std::pair<std::string, std::string>> cache;
            void clear() {
                cache.clear();
            }
        };
        
        std::string connectionString_;
        bool connected_ = false;
        ConnectionPool pool_;
        QueryCache cache_;
        std::vector<std::string> results_;
        
    public:
        explicit Impl(const std::string& connStr) 
            : connectionString_(connStr) {
            std::cout << "Database implementation created\n";
        }
        
        ~Impl() {
            if (connected_) {
                disconnect();
            }
            std::cout << "Database implementation destroyed\n";
        }
        
        void connect() {
            if (!connected_) {
                std::cout << "Connecting to: " << connectionString_ << "\n";
                pool_.initialize();
                connected_ = true;
            }
        }
        
        void disconnect() {
            if (connected_) {
                std::cout << "Disconnecting from database\n";
                cache_.clear();
                connected_ = false;
            }
        }
        
        void executeQuery(const std::string& query) {
            if (!connected_) {
                throw std::runtime_error("Not connected to database");
            }
            
            std::cout << "Executing: " << query << "\n";
            // Simulate query execution
            results_.clear();
            results_.push_back("Result 1");
            results_.push_back("Result 2");
            results_.push_back("Result 3");
        }
        
        std::vector<std::string> fetchResults() {
            return results_;
        }
        
        bool isConnected() const {
            return connected_;
        }
    };
    
    Database::Database(const std::string& connectionString)
        : pImpl(std::make_unique<Impl>(connectionString)) {}
    
    Database::~Database() = default;
    
    Database::Database(Database&&) noexcept = default;
    Database& Database::operator=(Database&&) noexcept = default;
    
    void Database::connect() {
        pImpl->connect();
    }
    
    void Database::disconnect() {
        pImpl->disconnect();
    }
    
    void Database::executeQuery(const std::string& query) {
        pImpl->executeQuery(query);
    }
    
    std::vector<std::string> Database::fetchResults() {
        return pImpl->fetchResults();
    }
    
    bool Database::isConnected() const {
        return pImpl->isConnected();
    }
}

// Example 3: Fast Pimpl (without heap allocation)
namespace FastPimpl {
    // timer.h - Public interface
    class Timer {
    private:
        // Fixed-size buffer for implementation
        static constexpr size_t ImplSize = 64;
        alignas(8) char implBuffer_[ImplSize];
        
        class Impl;
        Impl* impl();
        const Impl* impl() const;
        
    public:
        Timer();
        ~Timer();
        
        // Delete copy/move for simplicity
        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;
        
        void start();
        void stop();
        double getElapsedSeconds() const;
        void reset();
    };
    
    // timer.cpp - Implementation
    class Timer::Impl {
    private:
        using Clock = std::chrono::high_resolution_clock;
        Clock::time_point startTime_;
        Clock::time_point endTime_;
        bool running_ = false;
        
    public:
        Impl() {
            std::cout << "Timer implementation created (in-place)\n";
        }
        
        ~Impl() {
            std::cout << "Timer implementation destroyed\n";
        }
        
        void start() {
            startTime_ = Clock::now();
            running_ = true;
            std::cout << "Timer started\n";
        }
        
        void stop() {
            if (running_) {
                endTime_ = Clock::now();
                running_ = false;
                std::cout << "Timer stopped\n";
            }
        }
        
        double getElapsedSeconds() const {
            auto end = running_ ? Clock::now() : endTime_;
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>
                           (end - startTime_);
            return duration.count() / 1000000.0;
        }
        
        void reset() {
            running_ = false;
            startTime_ = Clock::time_point();
            endTime_ = Clock::time_point();
            std::cout << "Timer reset\n";
        }
    };
    
    Timer::Impl* Timer::impl() {
        return reinterpret_cast<Impl*>(implBuffer_);
    }
    
    const Timer::Impl* Timer::impl() const {
        return reinterpret_cast<const Impl*>(implBuffer_);
    }
    
    Timer::Timer() {
        new (impl()) Impl();
    }
    
    Timer::~Timer() {
        impl()->~Impl();
    }
    
    void Timer::start() {
        impl()->start();
    }
    
    void Timer::stop() {
        impl()->stop();
    }
    
    double Timer::getElapsedSeconds() const {
        return impl()->getElapsedSeconds();
    }
    
    void Timer::reset() {
        impl()->reset();
    }
}

// Example 4: Pimpl with Shared Implementation
namespace SharedPimpl {
    // document.h - Public interface
    class Document {
    private:
        class Impl;
        std::shared_ptr<Impl> pImpl;
        
    public:
        Document();
        explicit Document(const std::string& content);
        
        // Copy is cheap (shared implementation)
        Document(const Document&) = default;
        Document& operator=(const Document&) = default;
        
        // Public interface
        void setContent(const std::string& content);
        std::string getContent() const;
        void append(const std::string& text);
        size_t getLength() const;
        
        // Copy-on-write
        void makeUnique();
    };
    
    // document.cpp - Implementation
    class Document::Impl {
    public:
        std::string content_;
        mutable size_t accessCount_ = 0;
        
        Impl() {
            std::cout << "Document implementation created (empty)\n";
        }
        
        explicit Impl(const std::string& content) : content_(content) {
            std::cout << "Document implementation created with content\n";
        }
        
        ~Impl() {
            std::cout << "Document implementation destroyed (accessed " 
                      << accessCount_ << " times)\n";
        }
    };
    
    Document::Document() : pImpl(std::make_shared<Impl>()) {}
    
    Document::Document(const std::string& content)
        : pImpl(std::make_shared<Impl>(content)) {}
    
    void Document::setContent(const std::string& content) {
        makeUnique();
        pImpl->content_ = content;
    }
    
    std::string Document::getContent() const {
        pImpl->accessCount_++;
        return pImpl->content_;
    }
    
    void Document::append(const std::string& text) {
        makeUnique();
        pImpl->content_ += text;
    }
    
    size_t Document::getLength() const {
        pImpl->accessCount_++;
        return pImpl->content_.length();
    }
    
    void Document::makeUnique() {
        if (pImpl.use_count() > 1) {
            std::cout << "Making document unique (copy-on-write)\n";
            pImpl = std::make_shared<Impl>(*pImpl);
        }
    }
}

// Example 5: Compilation Firewall Demo
namespace CompilationFirewall {
    // engine.h - Public interface (minimal dependencies)
    class Engine {
    private:
        class Impl;
        std::unique_ptr<Impl> pImpl;
        
    public:
        Engine();
        ~Engine();
        
        void initialize();
        void run();
        void shutdown();
        void setParameter(const std::string& name, double value);
        double getParameter(const std::string& name) const;
    };
    
    // engine.cpp - Implementation (can have heavy dependencies)
    // Simulating heavy dependencies that clients don't need to know about
    namespace HeavyDependencies {
        class PhysicsEngine {
        public:
            void initialize() { std::cout << "Physics engine initialized\n"; }
            void simulate() { std::cout << "Running physics simulation\n"; }
        };
        
        class RenderEngine {
        public:
            void initialize() { std::cout << "Render engine initialized\n"; }
            void render() { std::cout << "Rendering frame\n"; }
        };
        
        class AudioEngine {
        public:
            void initialize() { std::cout << "Audio engine initialized\n"; }
            void process() { std::cout << "Processing audio\n"; }
        };
    }
    
    class Engine::Impl {
    private:
        // Heavy dependencies hidden from clients
        HeavyDependencies::PhysicsEngine physics_;
        HeavyDependencies::RenderEngine render_;
        HeavyDependencies::AudioEngine audio_;
        std::unordered_map<std::string, double> parameters_;
        bool initialized_ = false;
        
    public:
        void initialize() {
            if (!initialized_) {
                physics_.initialize();
                render_.initialize();
                audio_.initialize();
                initialized_ = true;
            }
        }
        
        void run() {
            if (!initialized_) {
                throw std::runtime_error("Engine not initialized");
            }
            
            physics_.simulate();
            render_.render();
            audio_.process();
        }
        
        void shutdown() {
            if (initialized_) {
                std::cout << "Engine shutting down\n";
                initialized_ = false;
            }
        }
        
        void setParameter(const std::string& name, double value) {
            parameters_[name] = value;
        }
        
        double getParameter(const std::string& name) const {
            auto it = parameters_.find(name);
            return (it != parameters_.end()) ? it->second : 0.0;
        }
    };
    
    Engine::Engine() : pImpl(std::make_unique<Impl>()) {}
    Engine::~Engine() = default;
    
    void Engine::initialize() { pImpl->initialize(); }
    void Engine::run() { pImpl->run(); }
    void Engine::shutdown() { pImpl->shutdown(); }
    
    void Engine::setParameter(const std::string& name, double value) {
        pImpl->setParameter(name, value);
    }
    
    double Engine::getParameter(const std::string& name) const {
        return pImpl->getParameter(name);
    }
}

// Demo functions
void demonstrateBasicPimpl() {
    using namespace BasicPimpl;
    
    std::cout << "=== Basic Pimpl Pattern ===\n";
    
    Widget w1;
    Widget w2("Custom Widget");
    
    w1.setName("Modified Widget");
    w1.addValue(10);
    w1.addValue(20);
    w1.addValue(30);
    w1.performOperation();
    
    std::cout << "Widget 1 name: " << w1.getName() << "\n";
    std::cout << "Widget 1 total: " << w1.getTotal() << "\n";
    
    // Test copy
    Widget w3 = w2;
    w3.setName("Copied Widget");
    
    std::cout << "Widget 2 name: " << w2.getName() << "\n";
    std::cout << "Widget 3 name: " << w3.getName() << "\n";
}

void demonstrateComplexPimpl() {
    using namespace ComplexPimpl;
    
    std::cout << "\n=== Complex Pimpl with Dependencies ===\n";
    
    Database db("server=localhost;database=testdb");
    
    db.connect();
    std::cout << "Connected: " << db.isConnected() << "\n";
    
    db.executeQuery("SELECT * FROM users");
    auto results = db.fetchResults();
    
    std::cout << "Query results:\n";
    for (const auto& result : results) {
        std::cout << "  " << result << "\n";
    }
    
    db.disconnect();
}

void demonstrateFastPimpl() {
    using namespace FastPimpl;
    
    std::cout << "\n=== Fast Pimpl (Stack Allocated) ===\n";
    
    Timer timer;
    
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    timer.stop();
    
    std::cout << "Elapsed time: " << timer.getElapsedSeconds() << " seconds\n";
    
    timer.reset();
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "Running time: " << timer.getElapsedSeconds() << " seconds\n";
    timer.stop();
}

void demonstrateSharedPimpl() {
    using namespace SharedPimpl;
    
    std::cout << "\n=== Shared Pimpl with Copy-on-Write ===\n";
    
    Document doc1("Original content");
    Document doc2 = doc1; // Cheap copy (shared implementation)
    
    std::cout << "Doc1 content: " << doc1.getContent() << "\n";
    std::cout << "Doc2 content: " << doc2.getContent() << "\n";
    std::cout << "Doc1 length: " << doc1.getLength() << "\n";
    
    // Trigger copy-on-write
    doc2.append(" - Modified");
    
    std::cout << "\nAfter modification:\n";
    std::cout << "Doc1 content: " << doc1.getContent() << "\n";
    std::cout << "Doc2 content: " << doc2.getContent() << "\n";
}

void demonstrateCompilationFirewall() {
    using namespace CompilationFirewall;
    
    std::cout << "\n=== Compilation Firewall ===\n";
    
    Engine engine;
    
    engine.initialize();
    engine.setParameter("speed", 100.0);
    engine.setParameter("quality", 0.8);
    
    std::cout << "\nRunning engine:\n";
    engine.run();
    
    std::cout << "\nParameters:\n";
    std::cout << "Speed: " << engine.getParameter("speed") << "\n";
    std::cout << "Quality: " << engine.getParameter("quality") << "\n";
    
    engine.shutdown();
}

int main() {
    std::cout << "=== Pimpl (Pointer to Implementation) Pattern Demo ===\n\n";
    
    demonstrateBasicPimpl();
    demonstrateComplexPimpl();
    demonstrateFastPimpl();
    demonstrateSharedPimpl();
    demonstrateCompilationFirewall();
    
    std::cout << "\n=== Pimpl Pattern Benefits ===\n";
    std::cout << "1. Compilation firewall (faster builds)\n";
    std::cout << "2. ABI stability\n";
    std::cout << "3. Hidden implementation details\n";
    std::cout << "4. Reduced header dependencies\n";
    std::cout << "5. Value semantics with pointer performance\n";
    
    std::cout << "\n=== Pimpl Variations ===\n";
    std::cout << "1. Basic Pimpl: unique_ptr to heap implementation\n";
    std::cout << "2. Fast Pimpl: Stack-allocated implementation\n";
    std::cout << "3. Shared Pimpl: shared_ptr with copy-on-write\n";
    std::cout << "4. Const Pimpl: Separate const/non-const implementations\n";
    
    return 0;
}