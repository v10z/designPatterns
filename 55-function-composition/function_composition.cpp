// Minimal Function Composition Pattern Implementation
#include <iostream>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <cmath>
#include <memory>
#include <type_traits>
#include <thread>
#include <chrono>

// Example 1: Basic Function Composition
namespace BasicComposition {
    // Function composition operator
    template<typename F, typename G>
    auto compose(F&& f, G&& g) {
        return [f = std::forward<F>(f), g = std::forward<G>(g)](auto&& x) {
            return f(g(std::forward<decltype(x)>(x)));
        };
    }
    
    // Pipe operator (left-to-right composition)
    template<typename F, typename G>
    auto pipe(F&& f, G&& g) {
        return [f = std::forward<F>(f), g = std::forward<G>(g)](auto&& x) {
            return g(f(std::forward<decltype(x)>(x)));
        };
    }
    
    // Basic mathematical functions
    auto add(int n) {
        return [n](int x) {
            std::cout << "Adding " << n << " to " << x << " = " << (x + n) << "\n";
            return x + n;
        };
    }
    
    auto multiply(int n) {
        return [n](int x) {
            std::cout << "Multiplying " << x << " by " << n << " = " << (x * n) << "\n";
            return x * n;
        };
    }
    
    auto square() {
        return [](int x) {
            std::cout << "Squaring " << x << " = " << (x * x) << "\n";
            return x * x;
        };
    }
    
    auto toString() {
        return [](int x) {
            std::string result = std::to_string(x);
            std::cout << "Converting " << x << " to string: \"" << result << "\"\n";
            return result;
        };
    }
    
    auto addPrefix(const std::string& prefix) {
        return [prefix](const std::string& s) {
            std::string result = prefix + s;
            std::cout << "Adding prefix \"" << prefix << "\" to \"" << s << "\" = \"" << result << "\"\n";
            return result;
        };
    }
}

// Example 2: Variadic Function Composition
namespace VariadicComposition {
    // Identity function
    template<typename T>
    constexpr auto identity = [](T&& x) -> decltype(auto) {
        return std::forward<T>(x);
    };
    
    // Compose multiple functions (right-to-left)
    template<typename F>
    constexpr auto compose(F&& f) {
        return std::forward<F>(f);
    }
    
    template<typename F, typename... Fs>
    constexpr auto compose(F&& f, Fs&&... fs) {
        return [f = std::forward<F>(f), composition = compose(std::forward<Fs>(fs)...)](auto&& x) {
            return f(composition(std::forward<decltype(x)>(x)));
        };
    }
    
    // Pipe multiple functions (left-to-right)
    template<typename F>
    constexpr auto pipe(F&& f) {
        return std::forward<F>(f);
    }
    
    template<typename F, typename... Fs>
    constexpr auto pipe(F&& f, Fs&&... fs) {
        return [f = std::forward<F>(f), rest = pipe(std::forward<Fs>(fs)...)](auto&& x) {
            return rest(f(std::forward<decltype(x)>(x)));
        };
    }
    
    // String processing functions
    auto toUpper() {
        return [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(), ::toupper);
            std::cout << "Converting to uppercase: \"" << s << "\"\n";
            return s;
        };
    }
    
    auto trim() {
        return [](std::string s) {
            // Remove leading whitespace
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            
            // Remove trailing whitespace
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
            
            std::cout << "Trimmed string: \"" << s << "\"\n";
            return s;
        };
    }
    
    auto replace(const std::string& from, const std::string& to) {
        return [from, to](std::string s) {
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != std::string::npos) {
                s.replace(pos, from.length(), to);
                pos += to.length();
            }
            std::cout << "Replaced \"" << from << "\" with \"" << to << "\": \"" << s << "\"\n";
            return s;
        };
    }
    
    auto addBrackets() {
        return [](const std::string& s) {
            std::string result = "[" + s + "]";
            std::cout << "Added brackets: \"" << result << "\"\n";
            return result;
        };
    }
}

// Example 3: Function Composition with Containers
namespace ContainerComposition {
    // Map function over container
    template<typename F>
    auto map(F&& func) {
        return [func = std::forward<F>(func)](const auto& container) {
            using ContainerType = std::decay_t<decltype(container)>;
            using ValueType = typename ContainerType::value_type;
            using ResultType = std::decay_t<decltype(func(std::declval<ValueType>()))>;
            
            std::vector<ResultType> result;
            result.reserve(container.size());
            
            std::cout << "Mapping function over container with " << container.size() << " elements\n";
            for (const auto& item : container) {
                result.push_back(func(item));
            }
            
            return result;
        };
    }
    
    // Filter container elements
    template<typename Predicate>
    auto filter(Predicate&& pred) {
        return [pred = std::forward<Predicate>(pred)](const auto& container) {
            using ContainerType = std::decay_t<decltype(container)>;
            ContainerType result;
            
            std::cout << "Filtering container\n";
            std::copy_if(container.begin(), container.end(), 
                        std::back_inserter(result), pred);
            
            std::cout << "Filtered from " << container.size() << " to " << result.size() << " elements\n";
            return result;
        };
    }
    
    // Reduce container to single value
    template<typename T, typename BinaryOp>
    auto reduce(T init, BinaryOp&& op) {
        return [init, op = std::forward<BinaryOp>(op)](const auto& container) {
            std::cout << "Reducing container to single value\n";
            return std::accumulate(container.begin(), container.end(), init, op);
        };
    }
    
    // Sort container
    template<typename Compare = std::less<>>
    auto sort(Compare comp = {}) {
        return [comp](auto container) {
            std::cout << "Sorting container\n";
            std::sort(container.begin(), container.end(), comp);
            return container;
        };
    }
    
    // Take first n elements
    auto take(size_t n) {
        return [n](const auto& container) {
            using ContainerType = std::decay_t<decltype(container)>;
            ContainerType result;
            
            size_t count = std::min(n, container.size());
            result.reserve(count);
            
            std::copy_n(container.begin(), count, std::back_inserter(result));
            std::cout << "Taking first " << count << " elements\n";
            return result;
        };
    }
    
    // Print container contents
    auto print(const std::string& label = "") {
        return [label](const auto& container) {
            std::cout << label << "[";
            bool first = true;
            for (const auto& item : container) {
                if (!first) std::cout << ", ";
                std::cout << item;
                first = false;
            }
            std::cout << "]\n";
            return container;
        };
    }
}

// Example 4: Monadic Function Composition
namespace MonadicComposition {
    // Optional monad for composing potentially failing operations
    template<typename T>
    class Optional {
    private:
        std::optional<T> value_;
        
    public:
        Optional() = default;
        explicit Optional(const T& value) : value_(value) {}
        explicit Optional(T&& value) : value_(std::move(value)) {}
        
        static Optional<T> some(const T& value) {
            return Optional<T>(value);
        }
        
        static Optional<T> none() {
            return Optional<T>();
        }
        
        bool hasValue() const { return value_.has_value(); }
        const T& getValue() const { return *value_; }
        
        template<typename F>
        auto bind(F&& func) const -> decltype(func(getValue())) {
            using ReturnType = decltype(func(getValue()));
            if (hasValue()) {
                return func(getValue());
            }
            return ReturnType::none();
        }
        
        template<typename F>
        auto map(F&& func) const -> Optional<decltype(func(getValue()))> {
            using ResultType = decltype(func(getValue()));
            if (hasValue()) {
                return Optional<ResultType>::some(func(getValue()));
            }
            return Optional<ResultType>::none();
        }
        
        void print() const {
            if (hasValue()) {
                std::cout << "Some(" << getValue() << ")";
            } else {
                std::cout << "None";
            }
        }
    };
    
    // Safe division
    auto safeDivide(double divisor) {
        return [divisor](double x) {
            if (std::abs(divisor) < 1e-10) {
                std::cout << "Division by zero avoided\n";
                return Optional<double>::none();
            }
            double result = x / divisor;
            std::cout << "Safe division: " << x << " / " << divisor << " = " << result << "\n";
            return Optional<double>::some(result);
        };
    }
    
    // Safe square root
    auto safeSqrt() {
        return [](double x) {
            if (x < 0) {
                std::cout << "Negative square root avoided\n";
                return Optional<double>::none();
            }
            double result = std::sqrt(x);
            std::cout << "Safe sqrt: sqrt(" << x << ") = " << result << "\n";
            return Optional<double>::some(result);
        };
    }
    
    // Safe logarithm
    auto safeLog() {
        return [](double x) {
            if (x <= 0) {
                std::cout << "Non-positive logarithm avoided\n";
                return Optional<double>::none();
            }
            double result = std::log(x);
            std::cout << "Safe log: log(" << x << ") = " << result << "\n";
            return Optional<double>::some(result);
        };
    }
}

// Example 5: Async Function Composition
namespace AsyncComposition {
    // Simple future-like monad for async composition
    template<typename T>
    class Future {
    private:
        std::function<T()> computation_;
        
    public:
        explicit Future(std::function<T()> comp) : computation_(comp) {}
        
        T get() const {
            return computation_();
        }
        
        template<typename F>
        auto then(F&& func) const -> Future<decltype(func(get()))> {
            using ResultType = decltype(func(get()));
            
            return Future<ResultType>([computation = computation_, func = std::forward<F>(func)]() {
                return func(computation());
            });
        }
        
        template<typename F>
        auto bind(F&& func) const -> decltype(func(get())) {
            using ReturnType = decltype(func(get()));
            
            return ReturnType([computation = computation_, func = std::forward<F>(func)]() {
                return func(computation()).get();
            });
        }
        
        static Future<T> pure(const T& value) {
            return Future<T>([value]() { return value; });
        }
    };
    
    // Async operations
    auto asyncAdd(int n) {
        return [n](int x) {
            return Future<int>([x, n]() {
                std::cout << "Async add: " << x << " + " << n << " = " << (x + n) << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate async work
                return x + n;
            });
        };
    }
    
    auto asyncMultiply(int n) {
        return [n](int x) {
            return Future<int>([x, n]() {
                std::cout << "Async multiply: " << x << " * " << n << " = " << (x * n) << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate async work
                return x * n;
            });
        };
    }
    
    auto asyncToString() {
        return [](int x) {
            return Future<std::string>([x]() {
                std::string result = "Result: " + std::to_string(x);
                std::cout << "Async toString: " << x << " -> \"" << result << "\"\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                return result;
            });
        };
    }
}

// Demo functions
void demonstrateBasicComposition() {
    using namespace BasicComposition;
    
    std::cout << "=== Basic Function Composition ===\n";
    
    // Right-to-left composition (mathematical style)
    std::cout << "\nRight-to-left composition:\n";
    auto f1 = compose(addPrefix("Result: "), compose(toString(), compose(square(), add(3))));
    auto result1 = f1(5);
    std::cout << "Final result: \"" << result1 << "\"\n";
    
    // Left-to-right composition (pipeline style)
    std::cout << "\nLeft-to-right composition (pipeline):\n";
    auto f2 = pipe(add(2), pipe(multiply(3), pipe(square(), toString())));
    auto result2 = f2(4);
    std::cout << "Final result: \"" << result2 << "\"\n";
}

void demonstrateVariadicComposition() {
    using namespace VariadicComposition;
    
    std::cout << "\n=== Variadic Function Composition ===\n";
    
    // String processing pipeline
    std::cout << "\nString processing pipeline:\n";
    auto stringProcessor = pipe(
        trim(),
        toUpper(),
        replace("HELLO", "GREETINGS"),
        addBrackets()
    );
    
    std::string input = "  hello world from composition  ";
    std::cout << "Input: \"" << input << "\"\n";
    auto result = stringProcessor(input);
    std::cout << "Final result: \"" << result << "\"\n";
    
    // Mathematical composition
    std::cout << "\nMathematical composition:\n";
    auto mathPipeline = compose(
        [](int x) { std::cout << "Final: " << x << "\n"; return x; },
        [](int x) { std::cout << "Cube: " << (x*x*x) << "\n"; return x*x*x; },
        [](int x) { std::cout << "Add 10: " << (x+10) << "\n"; return x + 10; },
        [](int x) { std::cout << "Double: " << (x*2) << "\n"; return x * 2; }
    );
    
    mathPipeline(3);
}

void demonstrateContainerComposition() {
    using namespace ContainerComposition;
    
    std::cout << "\n=== Container Function Composition ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    std::cout << "\nProcessing pipeline:\n";
    auto pipeline = VariadicComposition::pipe(
        print("Input: "),
        filter([](int x) { return x % 2 == 0; }),  // Keep even numbers
        map([](int x) { return x * x; }),          // Square them
        sort(std::greater<int>{}),                  // Sort descending
        take(3),                                    // Take first 3
        print("Output: ")
    );
    
    auto result = pipeline(numbers);
    
    // Reduction example
    std::cout << "\nReduction pipeline:\n";
    auto sumPipeline = VariadicComposition::pipe(
        filter([](int x) { return x > 5; }),
        print("Filtered: "),
        reduce(0, [](int acc, int x) { return acc + x; })
    );
    
    int sum = sumPipeline(numbers);
    std::cout << "Sum of filtered numbers: " << sum << "\n";
}

void demonstrateMonadicComposition() {
    using namespace MonadicComposition;
    
    std::cout << "\n=== Monadic Function Composition ===\n";
    
    // Safe mathematical operations
    std::cout << "\nSafe computation chain (success case):\n";
    auto safeComputation1 = Optional<double>::some(100.0)
        .bind(safeDivide(4.0))
        .bind(safeSqrt())
        .bind(safeLog());
    
    std::cout << "Result 1: ";
    safeComputation1.print();
    std::cout << "\n";
    
    // Chain with failure
    std::cout << "\nSafe computation chain (failure case):\n";
    auto safeComputation2 = Optional<double>::some(100.0)
        .bind(safeDivide(0.0))  // This will fail
        .bind(safeSqrt())       // This won't execute
        .bind(safeLog());       // This won't execute
    
    std::cout << "Result 2: ";
    safeComputation2.print();
    std::cout << "\n";
    
    // Chain with negative square root
    std::cout << "\nSafe computation chain (negative sqrt):\n";
    auto safeComputation3 = Optional<double>::some(-25.0)
        .bind(safeSqrt())       // This will fail
        .bind(safeDivide(2.0)); // This won't execute
    
    std::cout << "Result 3: ";
    safeComputation3.print();
    std::cout << "\n";
}

void demonstrateAsyncComposition() {
    using namespace AsyncComposition;
    
    std::cout << "\n=== Async Function Composition ===\n";
    
    // Async computation chain
    std::cout << "\nAsync computation pipeline:\n";
    auto asyncPipeline = Future<int>::pure(5)
        .bind(asyncAdd(10))
        .bind(asyncMultiply(3))
        .bind(asyncAdd(7))
        .bind(asyncToString());
    
    std::string result = asyncPipeline.get();
    std::cout << "Final async result: \"" << result << "\"\n";
    
    // Another async chain
    std::cout << "\nAnother async chain:\n";
    auto asyncChain = Future<int>::pure(2)
        .then([](int x) { 
            std::cout << "Step 1: doubling " << x << "\n";
            return x * 2; 
        })
        .then([](int x) { 
            std::cout << "Step 2: adding 5 to " << x << "\n";
            return x + 5; 
        })
        .then([](int x) { 
            std::cout << "Step 3: squaring " << x << "\n";
            return x * x; 
        });
    
    int finalResult = asyncChain.get();
    std::cout << "Final result: " << finalResult << "\n";
}

int main() {
    std::cout << "=== Function Composition Pattern Demo ===\n\n";
    
    demonstrateBasicComposition();
    demonstrateVariadicComposition();
    demonstrateContainerComposition();
    demonstrateMonadicComposition();
    demonstrateAsyncComposition();
    
    std::cout << "\n=== Function Composition Benefits ===\n";
    std::cout << "1. Modular and reusable code\n";
    std::cout << "2. Declarative programming style\n";
    std::cout << "3. Easy to test individual functions\n";
    std::cout << "4. Improved code readability\n";
    std::cout << "5. Functional programming paradigms\n";
    
    return 0;
}