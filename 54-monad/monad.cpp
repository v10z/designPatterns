// Minimal Monad Pattern Implementation
#include <iostream>
#include <functional>
#include <optional>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <variant>

// Example 1: Maybe Monad (Optional)
namespace MaybeMonad {
    template<typename T>
    class Maybe {
    private:
        std::optional<T> value_;
        
    public:
        // Constructors
        Maybe() : value_(std::nullopt) {}
        explicit Maybe(const T& value) : value_(value) {}
        explicit Maybe(T&& value) : value_(std::move(value)) {}
        explicit Maybe(std::nullopt_t) : value_(std::nullopt) {}
        
        // Static factory methods
        static Maybe<T> some(const T& value) {
            return Maybe<T>(value);
        }
        
        static Maybe<T> none() {
            return Maybe<T>();
        }
        
        // Monad operations
        template<typename F>
        auto bind(F&& func) const -> decltype(func(*value_)) {
            using ReturnType = decltype(func(*value_));
            
            if (value_) {
                return func(*value_);
            }
            return ReturnType::none();
        }
        
        template<typename F>
        auto map(F&& func) const -> Maybe<decltype(func(*value_))> {
            using ResultType = decltype(func(*value_));
            
            if (value_) {
                return Maybe<ResultType>::some(func(*value_));
            }
            return Maybe<ResultType>::none();
        }
        
        // Utility methods
        bool hasValue() const {
            return value_.has_value();
        }
        
        const T& getValue() const {
            if (!value_) {
                throw std::runtime_error("No value present");
            }
            return *value_;
        }
        
        T getValueOr(const T& defaultValue) const {
            return value_.value_or(defaultValue);
        }
        
        void print() const {
            if (value_) {
                std::cout << "Some(" << *value_ << ")";
            } else {
                std::cout << "None";
            }
        }
    };
    
    // Helper functions for Maybe
    template<typename T>
    Maybe<T> some(const T& value) {
        return Maybe<T>::some(value);
    }
    
    template<typename T>
    Maybe<T> none() {
        return Maybe<T>::none();
    }
    
    // Example usage functions
    Maybe<int> safeDivide(int a, int b) {
        if (b == 0) {
            std::cout << "Division by zero - returning None\n";
            return none<int>();
        }
        std::cout << "Dividing " << a << " by " << b << " = " << (a / b) << "\n";
        return some(a / b);
    }
    
    Maybe<double> squareRoot(double x) {
        if (x < 0) {
            std::cout << "Negative square root - returning None\n";
            return none<double>();
        }
        double result = std::sqrt(x);
        std::cout << "Square root of " << x << " = " << result << "\n";
        return some(result);
    }
}

// Example 2: Either Monad (Result/Error)
namespace EitherMonad {
    template<typename L, typename R>
    class Either {
    private:
        std::variant<L, R> value_;
        
    public:
        // Constructors
        explicit Either(const L& left) : value_(left) {}
        explicit Either(const R& right) : value_(right) {}
        explicit Either(L&& left) : value_(std::move(left)) {}
        explicit Either(R&& right) : value_(std::move(right)) {}
        
        // Static factory methods
        static Either<L, R> left(const L& value) {
            return Either<L, R>(value);
        }
        
        static Either<L, R> right(const R& value) {
            return Either<L, R>(value);
        }
        
        // Type predicates
        bool isLeft() const {
            return std::holds_alternative<L>(value_);
        }
        
        bool isRight() const {
            return std::holds_alternative<R>(value_);
        }
        
        // Value accessors
        const L& getLeft() const {
            if (!isLeft()) {
                throw std::runtime_error("Not a left value");
            }
            return std::get<L>(value_);
        }
        
        const R& getRight() const {
            if (!isRight()) {
                throw std::runtime_error("Not a right value");
            }
            return std::get<R>(value_);
        }
        
        // Monad operations
        template<typename F>
        auto bind(F&& func) const -> decltype(func(getRight())) {
            using ReturnType = decltype(func(getRight()));
            
            if (isRight()) {
                return func(getRight());
            }
            return ReturnType::left(getLeft());
        }
        
        template<typename F>
        auto map(F&& func) const -> Either<L, decltype(func(getRight()))> {
            using ResultType = decltype(func(getRight()));
            
            if (isRight()) {
                return Either<L, ResultType>::right(func(getRight()));
            }
            return Either<L, ResultType>::left(getLeft());
        }
        
        template<typename F>
        Either<L, R> mapLeft(F&& func) const {
            if (isLeft()) {
                return Either<L, R>::left(func(getLeft()));
            }
            return Either<L, R>::right(getRight());
        }
        
        void print() const {
            if (isLeft()) {
                std::cout << "Left(" << getLeft() << ")";
            } else {
                std::cout << "Right(" << getRight() << ")";
            }
        }
    };
    
    // Type aliases for common patterns
    template<typename T>
    using Result = Either<std::string, T>;
    
    template<typename T>
    Result<T> success(const T& value) {
        return Result<T>::right(value);
    }
    
    template<typename T>
    Result<T> failure(const std::string& error) {
        return Result<T>::left(error);
    }
    
    // Example usage functions
    Result<int> parseInteger(const std::string& str) {
        try {
            int value = std::stoi(str);
            std::cout << "Parsed '" << str << "' as " << value << "\n";
            return success(value);
        } catch (const std::exception& e) {
            std::string error = "Failed to parse '" + str + "': " + e.what();
            std::cout << error << "\n";
            return failure<int>(error);
        }
    }
    
    Result<double> safeDivision(double a, double b) {
        if (b == 0.0) {
            std::string error = "Division by zero: " + std::to_string(a) + " / " + std::to_string(b);
            std::cout << error << "\n";
            return failure<double>(error);
        }
        double result = a / b;
        std::cout << "Division: " << a << " / " << b << " = " << result << "\n";
        return success(result);
    }
}

// Example 3: List Monad
namespace ListMonad {
    template<typename T>
    class List {
    private:
        std::vector<T> items_;
        
    public:
        // Constructors
        List() = default;
        explicit List(const std::vector<T>& items) : items_(items) {}
        explicit List(std::vector<T>&& items) : items_(std::move(items)) {}
        List(std::initializer_list<T> items) : items_(items) {}
        
        // Static factory
        static List<T> pure(const T& value) {
            return List<T>({value});
        }
        
        static List<T> empty() {
            return List<T>();
        }
        
        // Monad operations
        template<typename F>
        auto bind(F&& func) const -> decltype(func(T{})) {
            using ReturnType = decltype(func(T{}));
            ReturnType result = ReturnType::empty();
            
            for (const auto& item : items_) {
                auto subList = func(item);
                result.append(subList);
            }
            
            return result;
        }
        
        template<typename F>
        auto map(F&& func) const -> List<decltype(func(T{}))> {
            using ResultType = decltype(func(T{}));
            std::vector<ResultType> results;
            
            for (const auto& item : items_) {
                results.push_back(func(item));
            }
            
            return List<ResultType>(std::move(results));
        }
        
        // List-specific operations
        void append(const List<T>& other) {
            items_.insert(items_.end(), other.items_.begin(), other.items_.end());
        }
        
        List<T> filter(std::function<bool(const T&)> predicate) const {
            std::vector<T> filtered;
            for (const auto& item : items_) {
                if (predicate(item)) {
                    filtered.push_back(item);
                }
            }
            return List<T>(std::move(filtered));
        }
        
        template<typename U, typename F>
        U fold(const U& initial, F func) const {
            U accumulator = initial;
            for (const auto& item : items_) {
                accumulator = func(accumulator, item);
            }
            return accumulator;
        }
        
        // Utility methods
        size_t size() const { return items_.size(); }
        bool isEmpty() const { return items_.empty(); }
        
        const std::vector<T>& getItems() const { return items_; }
        
        void print() const {
            std::cout << "[";
            for (size_t i = 0; i < items_.size(); ++i) {
                std::cout << items_[i];
                if (i < items_.size() - 1) std::cout << ", ";
            }
            std::cout << "]";
        }
    };
    
    // Helper functions
    template<typename T>
    List<T> pure(const T& value) {
        return List<T>::pure(value);
    }
    
    template<typename T>
    List<T> empty() {
        return List<T>::empty();
    }
    
    // Example functions that return Lists
    List<int> divisors(int n) {
        std::vector<int> result;
        std::cout << "Finding divisors of " << n << ": ";
        
        for (int i = 1; i <= n; ++i) {
            if (n % i == 0) {
                result.push_back(i);
            }
        }
        
        List<int> list(result);
        list.print();
        std::cout << "\n";
        return list;
    }
    
    List<std::string> words(const std::string& sentence) {
        std::vector<std::string> result;
        std::string word;
        
        for (char c : sentence) {
            if (c == ' ') {
                if (!word.empty()) {
                    result.push_back(word);
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        
        if (!word.empty()) {
            result.push_back(word);
        }
        
        std::cout << "Words in '" << sentence << "': ";
        List<std::string> list(result);
        list.print();
        std::cout << "\n";
        return list;
    }
}

// Example 4: State Monad
namespace StateMonad {
    template<typename S, typename A>
    class State {
    private:
        std::function<std::pair<A, S>(const S&)> runState_;
        
    public:
        explicit State(std::function<std::pair<A, S>(const S&)> func)
            : runState_(func) {}
        
        // Run the state computation
        std::pair<A, S> runState(const S& initialState) const {
            return runState_(initialState);
        }
        
        // Get just the result value
        A evalState(const S& initialState) const {
            return runState(initialState).first;
        }
        
        // Get just the final state
        S execState(const S& initialState) const {
            return runState(initialState).second;
        }
        
        // Monad operations
        template<typename F>
        auto bind(F&& func) const -> decltype(func(A{})) {
            using ReturnType = decltype(func(A{}));
            
            return ReturnType([this, func](const S& state) {
                auto [value, newState] = runState_(state);
                return func(value).runState(newState);
            });
        }
        
        template<typename F>
        auto map(F&& func) const -> State<S, decltype(func(A{}))> {
            using ResultType = decltype(func(A{}));
            
            return State<S, ResultType>([this, func](const S& state) {
                auto [value, newState] = runState_(state);
                return std::make_pair(func(value), newState);
            });
        }
        
        // Static factory methods
        static State<S, A> pure(const A& value) {
            return State<S, A>([value](const S& state) {
                return std::make_pair(value, state);
            });
        }
        
        static State<S, S> get() {
            return State<S, S>([](const S& state) {
                return std::make_pair(state, state);
            });
        }
        
        static State<S, void> put(const S& newState) {
            return State<S, void>([newState](const S&) {
                return std::make_pair(static_cast<void*>(nullptr), newState);
            });
        }
        
        static State<S, S> modify(std::function<S(const S&)> func) {
            return State<S, S>([func](const S& state) {
                S newState = func(state);
                return std::make_pair(newState, newState);
            });
        }
    };
    
    // Example: Counter state
    struct Counter {
        int value;
        
        Counter(int v = 0) : value(v) {}
        
        void print() const {
            std::cout << "Counter: " << value;
        }
    };
    
    // State operations for Counter
    State<Counter, int> increment() {
        return State<Counter, int>([](const Counter& counter) {
            Counter newCounter(counter.value + 1);
            std::cout << "Incrementing: " << counter.value << " -> " << newCounter.value << "\n";
            return std::make_pair(newCounter.value, newCounter);
        });
    }
    
    State<Counter, int> add(int n) {
        return State<Counter, int>([n](const Counter& counter) {
            Counter newCounter(counter.value + n);
            std::cout << "Adding " << n << ": " << counter.value << " -> " << newCounter.value << "\n";
            return std::make_pair(newCounter.value, newCounter);
        });
    }
    
    State<Counter, int> multiply(int n) {
        return State<Counter, int>([n](const Counter& counter) {
            Counter newCounter(counter.value * n);
            std::cout << "Multiplying by " << n << ": " << counter.value << " -> " << newCounter.value << "\n";
            return std::make_pair(newCounter.value, newCounter);
        });
    }
}

// Example 5: IO Monad (Simplified)
namespace IOMonad {
    template<typename T>
    class IO {
    private:
        std::function<T()> action_;
        
    public:
        explicit IO(std::function<T()> action) : action_(action) {}
        
        // Execute the IO action
        T run() const {
            return action_();
        }
        
        // Monad operations
        template<typename F>
        auto bind(F&& func) const -> decltype(func(T{})) {
            using ReturnType = decltype(func(T{}));
            
            return ReturnType([this, func]() {
                T value = action_();
                return func(value).run();
            });
        }
        
        template<typename F>
        auto map(F&& func) const -> IO<decltype(func(T{}))> {
            using ResultType = decltype(func(T{}));
            
            return IO<ResultType>([this, func]() {
                return func(action_());
            });
        }
        
        // Static factory
        static IO<T> pure(const T& value) {
            return IO<T>([value]() { return value; });
        }
    };
    
    // Specialization for void
    template<>
    class IO<void> {
    private:
        std::function<void()> action_;
        
    public:
        explicit IO(std::function<void()> action) : action_(action) {}
        
        // Execute the IO action
        void run() const {
            action_();
        }
        
        // Monad operations
        template<typename F>
        auto bind(F&& func) const -> decltype(func()) {
            using ReturnType = decltype(func());
            
            return ReturnType([this, func]() {
                action_();
                return func().run();
            });
        }
        
        template<typename F>
        auto map(F&& func) const -> IO<decltype(func())> {
            using ResultType = decltype(func());
            
            return IO<ResultType>([this, func]() {
                action_();
                return func();
            });
        }
        
        // Static factory for void
        static IO<void> pure() {
            return IO<void>([]() {});
        }
    };
    
    // IO operations
    IO<std::string> readLine(const std::string& prompt) {
        return IO<std::string>([prompt]() {
            std::cout << prompt;
            std::string input;
            std::getline(std::cin, input);
            return input;
        });
    }
    
    IO<void> writeLine(const std::string& text) {
        return IO<void>([text]() {
            std::cout << text << "\n";
        });
    }
    
    template<typename T>
    IO<void> print(const T& value) {
        return IO<void>([value]() {
            std::cout << value;
        });
    }
    
    // Helper function for pure
    template<typename T>
    IO<T> pure(const T& value) {
        return IO<T>::pure(value);
    }
}

// Demo functions
void demonstrateMaybeMonad() {
    using namespace MaybeMonad;
    
    std::cout << "=== Maybe Monad ===\n";
    
    // Chaining operations with Maybe
    auto result1 = some(20)
        .bind([](int x) { return safeDivide(x, 4); })
        .bind([](int x) { return safeDivide(x, 2); })
        .map([](int x) { return x * 2; });
    
    std::cout << "Result 1: ";
    result1.print();
    std::cout << "\n\n";
    
    // Chain that fails
    auto result2 = some(10)
        .bind([](int x) { return safeDivide(x, 0); })  // This will fail
        .map([](int x) { return x * 100; });           // This won't execute
    
    std::cout << "Result 2: ";
    result2.print();
    std::cout << "\n\n";
    
    // Working with square roots
    auto result3 = some(16.0)
        .bind([](double x) { return squareRoot(x); })
        .map([](double x) { return x * 2; });
    
    std::cout << "Result 3: ";
    result3.print();
    std::cout << "\n";
}

void demonstrateEitherMonad() {
    using namespace EitherMonad;
    
    std::cout << "\n=== Either Monad ===\n";
    
    // Successful chain
    auto result1 = parseInteger("42")
        .bind([](int x) { return safeDivision(x, 2); })
        .map([](double x) { return x + 10.5; });
    
    std::cout << "Result 1: ";
    result1.print();
    std::cout << "\n\n";
    
    // Chain with parse error
    auto result2 = parseInteger("not_a_number")
        .bind([](int x) { return safeDivision(x, 2); })
        .map([](double x) { return x + 10.5; });
    
    std::cout << "Result 2: ";
    result2.print();
    std::cout << "\n\n";
    
    // Chain with division error
    auto result3 = parseInteger("100")
        .bind([](int x) { return safeDivision(x, 0); })
        .map([](double x) { return x + 10.5; });
    
    std::cout << "Result 3: ";
    result3.print();
    std::cout << "\n";
}

void demonstrateListMonad() {
    using namespace ListMonad;
    
    std::cout << "\n=== List Monad ===\n";
    
    // Generate lists and bind
    auto result1 = List<int>({2, 3, 4})
        .bind([](int x) { return divisors(x); })
        .map([](int x) { return x * x; });
    
    std::cout << "Result 1 (squared divisors): ";
    result1.print();
    std::cout << "\n\n";
    
    // String processing
    auto result2 = List<std::string>({"hello world", "functional programming"})
        .bind([](const std::string& s) { return words(s); })
        .map([](const std::string& w) { return w + "!"; });
    
    std::cout << "Result 2 (words with exclamation): ";
    result2.print();
    std::cout << "\n\n";
    
    // Filtering and folding
    auto numbers = List<int>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    auto evens = numbers.filter([](int x) { return x % 2 == 0; });
    int sum = evens.fold(0, [](int acc, int x) { return acc + x; });
    
    std::cout << "Original: ";
    numbers.print();
    std::cout << "\nEvens: ";
    evens.print();
    std::cout << "\nSum of evens: " << sum << "\n";
}

void demonstrateStateMonad() {
    using namespace StateMonad;
    
    std::cout << "\n=== State Monad ===\n";
    
    // Chain state operations
    auto computation = increment()
        .bind([](int) { return add(5); })
        .bind([](int) { return multiply(3); })
        .bind([](int) { return increment(); });
    
    Counter initialState(0);
    std::cout << "Initial state: ";
    initialState.print();
    std::cout << "\n\n";
    
    auto [finalValue, finalState] = computation.runState(initialState);
    
    std::cout << "\nFinal value: " << finalValue << "\n";
    std::cout << "Final state: ";
    finalState.print();
    std::cout << "\n";
}

void demonstrateIOMonad() {
    using namespace IOMonad;
    
    std::cout << "\n=== IO Monad ===\n";
    
    // Simple IO chain
    auto program = writeLine("Welcome to the IO Monad demo!")
        .bind([]() { return writeLine("This demonstrates pure functional IO."); })
        .bind([]() { return pure(42); })
        .bind([](int x) { return writeLine("The answer is: " + std::to_string(x)); });
    
    std::cout << "Running IO program:\n";
    program.run();
    
    std::cout << "\nIO computation completed.\n";
}

int main() {
    std::cout << "=== Monad Pattern Demo ===\n\n";
    
    demonstrateMaybeMonad();
    demonstrateEitherMonad();
    demonstrateListMonad();
    demonstrateStateMonad();
    demonstrateIOMonad();
    
    std::cout << "\n=== Monad Benefits ===\n";
    std::cout << "1. Composable computations\n";
    std::cout << "2. Error handling without exceptions\n";
    std::cout << "3. Functional programming patterns\n";
    std::cout << "4. Safe chaining of operations\n";
    std::cout << "5. Abstraction over computational contexts\n";
    
    return 0;
}