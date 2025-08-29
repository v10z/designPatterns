// Minimal Coroutines Pattern Implementation (C++20)
#include <iostream>
#include <coroutine>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <queue>
#include <optional>
#include <exception>

// Example 1: Basic Generator Coroutine
namespace BasicGenerator {
    template<typename T>
    class Generator {
    public:
        struct promise_type {
            T current_value;
            
            Generator get_return_object() {
                return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            
            std::suspend_always initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            
            std::suspend_always yield_value(T value) {
                current_value = value;
                return {};
            }
            
            void return_void() {}
            void unhandled_exception() {}
        };
        
        using handle_type = std::coroutine_handle<promise_type>;
        
    private:
        handle_type coro_handle;
        
    public:
        explicit Generator(handle_type h) : coro_handle(h) {}
        
        ~Generator() {
            if (coro_handle) {
                coro_handle.destroy();
            }
        }
        
        // Move-only type
        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;
        
        Generator(Generator&& other) noexcept : coro_handle(other.coro_handle) {
            other.coro_handle = {};
        }
        
        Generator& operator=(Generator&& other) noexcept {
            if (this != &other) {
                if (coro_handle) {
                    coro_handle.destroy();
                }
                coro_handle = other.coro_handle;
                other.coro_handle = {};
            }
            return *this;
        }
        
        // Iterator interface
        class iterator {
        private:
            handle_type coro_handle;
            
        public:
            explicit iterator(handle_type h) : coro_handle(h) {}
            
            void operator++() {
                coro_handle.resume();
                if (coro_handle.done()) {
                    coro_handle = {};
                }
            }
            
            T operator*() const {
                return coro_handle.promise().current_value;
            }
            
            bool operator==(const iterator& other) const {
                return coro_handle == other.coro_handle;
            }
            
            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }
        };
        
        iterator begin() {
            if (coro_handle) {
                coro_handle.resume();
                if (coro_handle.done()) {
                    return iterator{nullptr};
                }
            }
            return iterator{coro_handle};
        }
        
        iterator end() {
            return iterator{nullptr};
        }
    };
    
    // Example generator functions
    Generator<int> fibonacci(int count) {
        std::cout << "Starting fibonacci generator\n";
        int a = 0, b = 1;
        
        for (int i = 0; i < count; ++i) {
            co_yield a;
            std::cout << "Generated fibonacci: " << a << "\n";
            
            int temp = a + b;
            a = b;
            b = temp;
        }
        
        std::cout << "Fibonacci generator finished\n";
    }
    
    Generator<int> range(int start, int end, int step = 1) {
        std::cout << "Starting range generator: " << start << " to " << end << " step " << step << "\n";
        
        for (int i = start; i < end; i += step) {
            co_yield i;
            std::cout << "Generated range value: " << i << "\n";
        }
        
        std::cout << "Range generator finished\n";
    }
    
    Generator<std::string> tokenize(const std::string& text, char delimiter) {
        std::cout << "Starting tokenization of: \"" << text << "\"\n";
        
        std::string token;
        for (char c : text) {
            if (c == delimiter) {
                if (!token.empty()) {
                    co_yield token;
                    std::cout << "Generated token: \"" << token << "\"\n";
                    token.clear();
                }
            } else {
                token += c;
            }
        }
        
        if (!token.empty()) {
            co_yield token;
            std::cout << "Generated final token: \"" << token << "\"\n";
        }
        
        std::cout << "Tokenization finished\n";
    }
}

// Example 2: Async Task Coroutine
namespace AsyncTask {
    template<typename T>
    class Task {
    public:
        struct promise_type {
            T result;
            std::exception_ptr exception;
            
            Task get_return_object() {
                return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            
            void return_value(T value) {
                result = std::move(value);
            }
            
            void unhandled_exception() {
                exception = std::current_exception();
            }
        };
        
        using handle_type = std::coroutine_handle<promise_type>;
        
    private:
        handle_type coro_handle;
        
    public:
        explicit Task(handle_type h) : coro_handle(h) {}
        
        ~Task() {
            if (coro_handle) {
                coro_handle.destroy();
            }
        }
        
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;
        
        Task(Task&& other) noexcept : coro_handle(other.coro_handle) {
            other.coro_handle = {};
        }
        
        Task& operator=(Task&& other) noexcept {
            if (this != &other) {
                if (coro_handle) {
                    coro_handle.destroy();
                }
                coro_handle = other.coro_handle;
                other.coro_handle = {};
            }
            return *this;
        }
        
        bool is_ready() const {
            return !coro_handle || coro_handle.done();
        }
        
        T get() {
            if (!coro_handle.done()) {
                throw std::runtime_error("Task not completed");
            }
            
            if (coro_handle.promise().exception) {
                std::rethrow_exception(coro_handle.promise().exception);
            }
            
            return std::move(coro_handle.promise().result);
        }
        
        // Make Task awaitable
        bool await_ready() {
            return coro_handle.done();
        }
        
        void await_suspend(std::coroutine_handle<> awaiting) {
            // In a real implementation, you'd schedule the awaiting coroutine
            // to resume when this task completes
            coro_handle.resume();
        }
        
        T await_resume() {
            return get();
        }
    };
    
    // Awaitable for sleeping
    class SleepAwaiter {
    private:
        std::chrono::milliseconds duration;
        
    public:
        explicit SleepAwaiter(std::chrono::milliseconds d) : duration(d) {}
        
        bool await_ready() const { return duration.count() <= 0; }
        
        void await_suspend(std::coroutine_handle<> handle) {
            std::thread([handle, duration = this->duration]() {
                std::this_thread::sleep_for(duration);
                handle.resume();
            }).detach();
        }
        
        void await_resume() {}
    };
    
    SleepAwaiter sleep_for(std::chrono::milliseconds duration) {
        return SleepAwaiter{duration};
    }
    
    // Example async tasks
    Task<int> computeAsync(int n) {
        std::cout << "Starting async computation for: " << n << "\n";
        
        co_await sleep_for(std::chrono::milliseconds(100));
        std::cout << "Halfway through computation...\n";
        
        co_await sleep_for(std::chrono::milliseconds(100));
        std::cout << "Computation completed\n";
        
        co_return n * n;
    }
    
    Task<std::string> fetchDataAsync(const std::string& url) {
        std::cout << "Fetching data from: " << url << "\n";
        
        co_await sleep_for(std::chrono::milliseconds(200));
        std::cout << "Simulating network delay...\n";
        
        co_await sleep_for(std::chrono::milliseconds(100));
        std::cout << "Data fetched successfully\n";
        
        co_return "Data from " + url;
    }
    
    Task<int> processChainAsync() {
        std::cout << "Starting processing chain\n";
        
        auto result1 = co_await computeAsync(5);
        std::cout << "First computation result: " << result1 << "\n";
        
        auto result2 = co_await computeAsync(result1 / 5);
        std::cout << "Second computation result: " << result2 << "\n";
        
        co_return result1 + result2;
    }
}

// Example 3: Coroutine-Based State Machine
namespace StateMachine {
    enum class State { Start, Processing, Waiting, Completed, Error };
    
    class StateMachine {
    public:
        struct promise_type {
            State current_state = State::Start;
            std::string message;
            
            StateMachine get_return_object() {
                return StateMachine{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            
            std::suspend_always yield_value(State state) {
                current_state = state;
                return {};
            }
            
            void return_void() {}
            void unhandled_exception() {}
        };
        
        using handle_type = std::coroutine_handle<promise_type>;
        
    private:
        handle_type coro_handle;
        
    public:
        explicit StateMachine(handle_type h) : coro_handle(h) {}
        
        ~StateMachine() {
            if (coro_handle) {
                coro_handle.destroy();
            }
        }
        
        StateMachine(const StateMachine&) = delete;
        StateMachine& operator=(const StateMachine&) = delete;
        
        StateMachine(StateMachine&& other) noexcept : coro_handle(other.coro_handle) {
            other.coro_handle = {};
        }
        
        State getCurrentState() const {
            if (!coro_handle) return State::Error;
            return coro_handle.promise().current_state;
        }
        
        bool advance() {
            if (coro_handle && !coro_handle.done()) {
                coro_handle.resume();
                return !coro_handle.done();
            }
            return false;
        }
        
        bool isFinished() const {
            return !coro_handle || coro_handle.done();
        }
    };
    
    const char* stateToString(State state) {
        switch (state) {
            case State::Start: return "Start";
            case State::Processing: return "Processing";
            case State::Waiting: return "Waiting";
            case State::Completed: return "Completed";
            case State::Error: return "Error";
            default: return "Unknown";
        }
    }
    
    StateMachine fileProcessingStateMachine(const std::string& filename) {
        std::cout << "Starting file processing for: " << filename << "\n";
        
        co_yield State::Start;
        std::cout << "State: Start - Initializing\n";
        
        co_yield State::Processing;
        std::cout << "State: Processing - Reading file\n";
        
        // Simulate processing time
        for (int i = 0; i < 3; ++i) {
            co_yield State::Processing;
            std::cout << "State: Processing - Step " << (i + 1) << "/3\n";
        }
        
        co_yield State::Waiting;
        std::cout << "State: Waiting - External validation\n";
        
        co_yield State::Waiting;
        std::cout << "State: Waiting - Validation complete\n";
        
        co_yield State::Completed;
        std::cout << "State: Completed - File processed successfully\n";
    }
}

// Example 4: Producer-Consumer with Coroutines
namespace ProducerConsumer {
    template<typename T>
    class Channel {
    private:
        std::queue<T> buffer_;
        std::queue<std::coroutine_handle<>> waiting_producers_;
        std::queue<std::coroutine_handle<>> waiting_consumers_;
        size_t capacity_;
        mutable std::mutex mutex_;
        
    public:
        explicit Channel(size_t capacity = 10) : capacity_(capacity) {}
        
        class SendAwaiter {
        private:
            Channel& channel_;
            T value_;
            
        public:
            SendAwaiter(Channel& channel, T value) : channel_(channel), value_(std::move(value)) {}
            
            bool await_ready() {
                std::lock_guard<std::mutex> lock(channel_.mutex_);
                return channel_.buffer_.size() < channel_.capacity_;
            }
            
            void await_suspend(std::coroutine_handle<> handle) {
                std::lock_guard<std::mutex> lock(channel_.mutex_);
                channel_.waiting_producers_.push(handle);
            }
            
            void await_resume() {
                std::lock_guard<std::mutex> lock(channel_.mutex_);
                channel_.buffer_.push(std::move(value_));
                
                if (!channel_.waiting_consumers_.empty()) {
                    auto consumer = channel_.waiting_consumers_.front();
                    channel_.waiting_consumers_.pop();
                    consumer.resume();
                }
            }
        };
        
        class ReceiveAwaiter {
        private:
            Channel& channel_;
            T result_;
            
        public:
            explicit ReceiveAwaiter(Channel& channel) : channel_(channel) {}
            
            bool await_ready() {
                std::lock_guard<std::mutex> lock(channel_.mutex_);
                return !channel_.buffer_.empty();
            }
            
            void await_suspend(std::coroutine_handle<> handle) {
                std::lock_guard<std::mutex> lock(channel_.mutex_);
                channel_.waiting_consumers_.push(handle);
            }
            
            T await_resume() {
                std::lock_guard<std::mutex> lock(channel_.mutex_);
                result_ = std::move(channel_.buffer_.front());
                channel_.buffer_.pop();
                
                if (!channel_.waiting_producers_.empty()) {
                    auto producer = channel_.waiting_producers_.front();
                    channel_.waiting_producers_.pop();
                    producer.resume();
                }
                
                return std::move(result_);
            }
        };
        
        SendAwaiter send(T value) {
            return SendAwaiter{*this, std::move(value)};
        }
        
        ReceiveAwaiter receive() {
            return ReceiveAwaiter{*this};
        }
    };
    
    // Simple task type for producer-consumer
    class SimpleTask {
    public:
        struct promise_type {
            SimpleTask get_return_object() {
                return SimpleTask{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void return_void() {}
            void unhandled_exception() {}
        };
        
        using handle_type = std::coroutine_handle<promise_type>;
        
    private:
        handle_type coro_handle;
        
    public:
        explicit SimpleTask(handle_type h) : coro_handle(h) {}
        
        ~SimpleTask() {
            if (coro_handle) {
                coro_handle.destroy();
            }
        }
        
        SimpleTask(const SimpleTask&) = delete;
        SimpleTask& operator=(const SimpleTask&) = delete;
        
        SimpleTask(SimpleTask&& other) noexcept : coro_handle(other.coro_handle) {
            other.coro_handle = {};
        }
    };
    
    SimpleTask producer(Channel<int>& channel, int start, int count) {
        std::cout << "Producer starting: will produce " << count << " items starting from " << start << "\n";
        
        for (int i = 0; i < count; ++i) {
            int value = start + i;
            std::cout << "Producer: sending " << value << "\n";
            co_await channel.send(value);
            std::cout << "Producer: sent " << value << "\n";
        }
        
        std::cout << "Producer finished\n";
    }
    
    SimpleTask consumer(Channel<int>& channel, const std::string& name, int count) {
        std::cout << "Consumer " << name << " starting: will consume " << count << " items\n";
        
        for (int i = 0; i < count; ++i) {
            std::cout << "Consumer " << name << ": waiting for item\n";
            int value = co_await channel.receive();
            std::cout << "Consumer " << name << ": received " << value << "\n";
        }
        
        std::cout << "Consumer " << name << " finished\n";
    }
}

// Demo functions
void demonstrateBasicGenerator() {
    using namespace BasicGenerator;
    
    std::cout << "=== Basic Generator Coroutines ===\n";
    
    // Fibonacci generator
    std::cout << "\nFibonacci sequence (first 8 numbers):\n";
    auto fib = fibonacci(8);
    for (int value : fib) {
        std::cout << "Main: got fibonacci value " << value << "\n";
    }
    
    // Range generator
    std::cout << "\nRange generator (0 to 10, step 2):\n";
    auto range_gen = range(0, 10, 2);
    for (int value : range_gen) {
        std::cout << "Main: got range value " << value << "\n";
    }
    
    // Tokenizer generator
    std::cout << "\nTokenizer generator:\n";
    auto tokens = tokenize("hello,world,coroutines,are,awesome", ',');
    for (const std::string& token : tokens) {
        std::cout << "Main: got token \"" << token << "\"\n";
    }
}

void demonstrateAsyncTask() {
    using namespace AsyncTask;
    
    std::cout << "\n=== Async Task Coroutines ===\n";
    
    // Simple async computation
    std::cout << "\nStarting async computation...\n";
    auto task1 = computeAsync(7);
    
    // Wait for completion (busy wait for demo)
    while (!task1.is_ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "Async computation result: " << task1.get() << "\n";
    
    // Chained async operations
    std::cout << "\nStarting processing chain...\n";
    auto chain_task = processChainAsync();
    
    while (!chain_task.is_ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "Processing chain result: " << chain_task.get() << "\n";
}

void demonstrateStateMachine() {
    using namespace StateMachine;
    
    std::cout << "\n=== State Machine Coroutines ===\n";
    
    auto sm = fileProcessingStateMachine("example.txt");
    
    std::cout << "\nAdvancing through state machine:\n";
    while (!sm.isFinished()) {
        State current = sm.getCurrentState();
        std::cout << "Current state: " << stateToString(current) << "\n";
        
        bool hasMore = sm.advance();
        if (!hasMore) break;
        
        // Simulate some processing time
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    std::cout << "Final state: " << stateToString(sm.getCurrentState()) << "\n";
}

void demonstrateProducerConsumer() {
    using namespace ProducerConsumer;
    
    std::cout << "\n=== Producer-Consumer Coroutines ===\n";
    
    Channel<int> channel(3); // Buffer size 3
    
    // Create producer and consumer tasks
    auto prod_task = producer(channel, 100, 5);
    auto cons_task1 = consumer(channel, "A", 3);
    auto cons_task2 = consumer(channel, "B", 2);
    
    std::cout << "\nProducer-Consumer demo started\n";
    std::cout << "(Note: This is a simplified demo - real implementation would use proper scheduling)\n";
    
    // In a real implementation, you'd use a proper coroutine scheduler
    // This is just for demonstration
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Producer-Consumer demo completed\n";
}

int main() {
    std::cout << "=== Coroutines Pattern Demo (C++20) ===\n\n";
    
    demonstrateBasicGenerator();
    demonstrateAsyncTask();
    demonstrateStateMachine();
    demonstrateProducerConsumer();
    
    std::cout << "\n=== Coroutines Benefits ===\n";
    std::cout << "1. Simplified asynchronous programming\n";
    std::cout << "2. Natural sequential code structure\n";
    std::cout << "3. Efficient cooperative multitasking\n";
    std::cout << "4. Generator pattern implementation\n";
    std::cout << "5. State machine modeling\n";
    
    return 0;
}