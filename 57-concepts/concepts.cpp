// Minimal Concepts Pattern Implementation (C++20)
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <type_traits>
#include <concepts>
#include <iterator>
#include <ranges>
#include <algorithm>
#include <numeric>
#include <unordered_map>

// Example 1: Basic Concepts for Type Constraints
namespace BasicConcepts {
    // Simple arithmetic concept
    template<typename T>
    concept Arithmetic = std::integral<T> || std::floating_point<T>;
    
    // Concept for types that can be compared
    template<typename T>
    concept Comparable = requires(T a, T b) {
        { a < b } -> std::convertible_to<bool>;
        { a > b } -> std::convertible_to<bool>;
        { a <= b } -> std::convertible_to<bool>;
        { a >= b } -> std::convertible_to<bool>;
        { a == b } -> std::convertible_to<bool>;
        { a != b } -> std::convertible_to<bool>;
    };
    
    // Concept for printable types
    template<typename T>
    concept Printable = requires(T t, std::ostream& os) {
        { os << t } -> std::same_as<std::ostream&>;
    };
    
    // Container concept
    template<typename T>
    concept Container = requires(T t) {
        typename T::value_type;
        typename T::iterator;
        { t.begin() } -> std::same_as<typename T::iterator>;
        { t.end() } -> std::same_as<typename T::iterator>;
        { t.size() } -> std::convertible_to<std::size_t>;
    };
    
    // Function templates using concepts
    template<Arithmetic T>
    T add(T a, T b) {
        std::cout << "Adding arithmetic types: " << a << " + " << b << " = ";
        return a + b;
    }
    
    template<Comparable T>
    T max(T a, T b) {
        std::cout << "Finding max of comparable types: ";
        return (a > b) ? a : b;
    }
    
    template<Printable T>
    void print(const T& value) {
        std::cout << "Printing: " << value << "\n";
    }
    
    template<Container C>
    void printContainer(const C& container) {
        std::cout << "Container contents: [";
        bool first = true;
        for (const auto& item : container) {
            if (!first) std::cout << ", ";
            std::cout << item;
            first = false;
        }
        std::cout << "]\n";
    }
}

// Example 2: Advanced Concepts with Requires Clauses
namespace AdvancedConcepts {
    // Concept for types that support mathematical operations
    template<typename T>
    concept Numeric = requires(T a, T b) {
        { a + b } -> std::same_as<T>;
        { a - b } -> std::same_as<T>;
        { a * b } -> std::same_as<T>;
        { a / b } -> std::same_as<T>;
        { +a } -> std::same_as<T>;
        { -a } -> std::same_as<T>;
    } && std::default_initializable<T> && std::copyable<T>;
    
    // Concept for hashable types
    template<typename T>
    concept Hashable = requires(T t) {
        { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
    };
    
    // Concept for serializable types
    template<typename T>
    concept Serializable = requires(T t, std::ostream& os, std::istream& is) {
        { t.serialize(os) } -> std::same_as<void>;
        { T::deserialize(is) } -> std::same_as<T>;
    };
    
    // Iterator concept refinement
    template<typename It, typename T>
    concept InputIteratorOf = std::input_iterator<It> && 
                             std::same_as<std::iter_value_t<It>, T>;
    
    // Range concept
    template<typename R, typename T>
    concept RangeOf = std::ranges::range<R> && 
                     std::same_as<std::ranges::range_value_t<R>, T>;
    
    // Mathematical vector concept
    template<typename V>
    concept Vector = requires(V v, typename V::value_type scalar) {
        typename V::value_type;
        { v.size() } -> std::convertible_to<std::size_t>;
        { v[0] } -> std::convertible_to<typename V::value_type&>;
        { v + v } -> std::same_as<V>;
        { v - v } -> std::same_as<V>;
        { v * scalar } -> std::same_as<V>;
        { scalar * v } -> std::same_as<V>;
    } && Numeric<typename V::value_type>;
    
    // Generic mathematical operations using concepts
    template<Vector V>
    auto dot_product(const V& a, const V& b) -> typename V::value_type {
        if (a.size() != b.size()) {
            throw std::invalid_argument("Vector sizes must match");
        }
        
        typename V::value_type result{};
        for (std::size_t i = 0; i < a.size(); ++i) {
            result += a[i] * b[i];
        }
        
        std::cout << "Dot product calculated\n";
        return result;
    }
    
    template<Vector V>
    V normalize(const V& v) {
        auto magnitude = std::sqrt(dot_product(v, v));
        std::cout << "Normalizing vector with magnitude: " << magnitude << "\n";
        return v * (1.0 / magnitude);
    }
    
    // Hash map concept
    template<typename M, typename K, typename V>
    concept HashMap = requires(M m, K key, V value) {
        typename M::key_type;
        typename M::mapped_type;
        { m[key] } -> std::convertible_to<V&>;
        { m.find(key) } -> std::input_iterator;
        { m.insert({key, value}) };
        { m.size() } -> std::convertible_to<std::size_t>;
    } && std::same_as<typename M::key_type, K> &&
         std::same_as<typename M::mapped_type, V> &&
         Hashable<K>;
    
    template<HashMap<std::string, int> Map>
    void printStringIntMap(const Map& map) {
        std::cout << "String-Int Map contents:\n";
        for (const auto& [key, value] : map) {
            std::cout << "  " << key << " -> " << value << "\n";
        }
    }
}

// Example 3: Concept-Based Design Patterns
namespace ConceptPatterns {
    // Strategy pattern with concepts
    template<typename T>
    concept SortStrategy = requires(T strategy, std::vector<int>& data) {
        { strategy.sort(data) } -> std::same_as<void>;
        { strategy.name() } -> std::convertible_to<std::string>;
    };
    
    class BubbleSort {
    public:
        void sort(std::vector<int>& data) {
            std::cout << "Performing bubble sort\n";
            for (size_t i = 0; i < data.size(); ++i) {
                for (size_t j = 0; j < data.size() - 1 - i; ++j) {
                    if (data[j] > data[j + 1]) {
                        std::swap(data[j], data[j + 1]);
                    }
                }
            }
        }
        
        std::string name() const { return "Bubble Sort"; }
    };
    
    class QuickSort {
    public:
        void sort(std::vector<int>& data) {
            std::cout << "Performing quick sort\n";
            std::sort(data.begin(), data.end());
        }
        
        std::string name() const { return "Quick Sort"; }
    };
    
    template<SortStrategy Strategy>
    class Sorter {
    private:
        Strategy strategy_;
        
    public:
        explicit Sorter(Strategy strategy) : strategy_(strategy) {}
        
        void performSort(std::vector<int>& data) {
            std::cout << "Using strategy: " << strategy_.name() << "\n";
            strategy_.sort(data);
        }
    };
    
    // Observer pattern with concepts
    template<typename T>
    concept Observer = requires(T observer, const std::string& event) {
        { observer.notify(event) } -> std::same_as<void>;
        { observer.getId() } -> std::convertible_to<std::string>;
    };
    
    template<typename T>
    concept Observable = requires(T observable) {
        { observable.notifyObservers(std::string{}) } -> std::same_as<void>;
    };
    
    class Logger {
    public:
        void notify(const std::string& event) {
            std::cout << "Logger: " << event << "\n";
        }
        
        std::string getId() const { return "Logger"; }
    };
    
    class EmailNotifier {
    public:
        void notify(const std::string& event) {
            std::cout << "Email: " << event << "\n";
        }
        
        std::string getId() const { return "EmailNotifier"; }
    };
    
    template<Observer... Observers>
    class Subject {
    private:
        std::tuple<Observers...> observers_;
        
    public:
        explicit Subject(Observers... observers) : observers_(observers...) {}
        
        void notifyObservers(const std::string& event) {
            std::cout << "Notifying all observers about: " << event << "\n";
            std::apply([&event](auto&... observers) {
                (observers.notify(event), ...);
            }, observers_);
        }
    };
    
    // Factory pattern with concepts
    template<typename T>
    concept Product = requires {
        typename T::product_type;
    } && requires(typename T::product_type product) {
        { product.use() } -> std::same_as<void>;
        { product.getType() } -> std::convertible_to<std::string>;
    };
    
    template<typename T>
    concept Factory = requires(T factory) {
        typename T::product_type;
        { factory.create() } -> std::same_as<std::unique_ptr<typename T::product_type>>;
    } && Product<T>;
    
    struct IWidget {
        virtual ~IWidget() = default;
        virtual void use() = 0;
        virtual std::string getType() const = 0;
    };
    
    class Button : public IWidget {
    public:
        void use() override {
            std::cout << "Button clicked\n";
        }
        
        std::string getType() const override {
            return "Button";
        }
    };
    
    class TextField : public IWidget {
    public:
        void use() override {
            std::cout << "Text entered\n";
        }
        
        std::string getType() const override {
            return "TextField";
        }
    };
    
    class ButtonFactory {
    public:
        using product_type = IWidget;
        
        std::unique_ptr<IWidget> create() {
            std::cout << "Creating button\n";
            return std::make_unique<Button>();
        }
    };
    
    class TextFieldFactory {
    public:
        using product_type = IWidget;
        
        std::unique_ptr<IWidget> create() {
            std::cout << "Creating text field\n";
            return std::make_unique<TextField>();
        }
    };
    
    template<Factory F>
    void useFactory(F factory) {
        auto product = factory.create();
        std::cout << "Using product of type: " << product->getType() << "\n";
        product->use();
    }
}

// Example 4: Concept-Based Template Specialization
namespace ConceptSpecialization {
    // Different processing strategies based on concepts
    template<typename T>
    concept Integral = std::integral<T>;
    
    template<typename T>
    concept FloatingPoint = std::floating_point<T>;
    
    template<typename T>
    concept StringLike = std::convertible_to<T, std::string>;
    
    // Generic processor with concept-based specialization
    template<typename T>
    class Processor {
    public:
        void process(const T& value) {
            if constexpr (Integral<T>) {
                processInteger(value);
            } else if constexpr (FloatingPoint<T>) {
                processFloat(value);
            } else if constexpr (StringLike<T>) {
                processString(value);
            } else {
                processGeneric(value);
            }
        }
        
    private:
        void processInteger(const auto& value) {
            std::cout << "Processing integer: " << value 
                      << " (even: " << (value % 2 == 0 ? "yes" : "no") << ")\n";
        }
        
        void processFloat(const auto& value) {
            std::cout << "Processing float: " << value 
                      << " (rounded: " << std::round(value) << ")\n";
        }
        
        void processString(const auto& value) {
            std::string str = value;
            std::cout << "Processing string: \"" << str 
                      << "\" (length: " << str.length() << ")\n";
        }
        
        void processGeneric(const auto& value) {
            std::cout << "Processing generic type\n";
        }
    };
    
    // Concept-constrained algorithms
    template<std::ranges::range R>
        requires std::integral<std::ranges::range_value_t<R>>
    auto sum_integers(const R& range) {
        std::cout << "Summing integer range\n";
        return std::accumulate(std::ranges::begin(range), 
                              std::ranges::end(range), 0);
    }
    
    template<std::ranges::range R>
        requires std::floating_point<std::ranges::range_value_t<R>>
    auto average_floats(const R& range) {
        if (std::ranges::empty(range)) return 0.0;
        
        std::cout << "Averaging floating-point range\n";
        auto sum = std::accumulate(std::ranges::begin(range), 
                                  std::ranges::end(range), 0.0);
        return sum / std::ranges::size(range);
    }
}

// Example 5: Concept Composition and Refinement
namespace ConceptComposition {
    // Base concepts
    template<typename T>
    concept Movable = std::move_constructible<T> && std::is_move_assignable_v<T>;
    
    template<typename T>
    concept Copyable = std::copy_constructible<T> && std::is_copy_assignable_v<T>;
    
    template<typename T>
    concept Regular = std::semiregular<T> && std::equality_comparable<T>;
    
    // Composed concepts
    template<typename T>
    concept Resource = Movable<T> && requires(T t) {
        { t.isValid() } -> std::convertible_to<bool>;
        { t.release() } -> std::same_as<void>;
    };
    
    template<typename T>
    concept Container = Regular<T> && requires(T t) {
        typename T::value_type;
        typename T::size_type;
        { t.size() } -> std::same_as<typename T::size_type>;
        { t.empty() } -> std::convertible_to<bool>;
        { t.clear() } -> std::same_as<void>;
    };
    
    template<typename T>
    concept SequenceContainer = Container<T> && requires(T t, typename T::value_type v) {
        { t.push_back(v) } -> std::same_as<void>;
        { t.pop_back() } -> std::same_as<void>;
        { t.front() } -> std::convertible_to<typename T::value_type&>;
        { t.back() } -> std::convertible_to<typename T::value_type&>;
    };
    
    // RAII wrapper using Resource concept
    template<Resource R>
    class RAIIWrapper {
    private:
        R resource_;
        
    public:
        explicit RAIIWrapper(R&& resource) : resource_(std::move(resource)) {
            std::cout << "RAII: Acquired resource\n";
        }
        
        ~RAIIWrapper() {
            if (resource_.isValid()) {
                std::cout << "RAII: Releasing resource\n";
                resource_.release();
            }
        }
        
        RAIIWrapper(const RAIIWrapper&) = delete;
        RAIIWrapper& operator=(const RAIIWrapper&) = delete;
        
        RAIIWrapper(RAIIWrapper&& other) noexcept 
            : resource_(std::move(other.resource_)) {
            std::cout << "RAII: Moved resource\n";
        }
        
        R& get() { return resource_; }
        const R& get() const { return resource_; }
    };
    
    // Mock resource for demonstration
    class FileHandle {
    private:
        std::string filename_;
        bool valid_ = true;
        
    public:
        explicit FileHandle(std::string filename) : filename_(std::move(filename)) {}
        
        FileHandle(FileHandle&& other) noexcept 
            : filename_(std::move(other.filename_)), valid_(other.valid_) {
            other.valid_ = false;
        }
        
        FileHandle& operator=(FileHandle&& other) noexcept {
            if (this != &other) {
                filename_ = std::move(other.filename_);
                valid_ = other.valid_;
                other.valid_ = false;
            }
            return *this;
        }
        
        bool isValid() const { return valid_; }
        
        void release() {
            if (valid_) {
                std::cout << "Closing file: " << filename_ << "\n";
                valid_ = false;
            }
        }
        
        const std::string& getFilename() const { return filename_; }
    };
    
    // Generic algorithm for sequence containers
    template<SequenceContainer C>
    void demonstrateSequenceOps(C& container) {
        using ValueType = typename C::value_type;
        
        std::cout << "Demonstrating sequence container operations:\n";
        std::cout << "  Initial size: " << container.size() << "\n";
        
        if constexpr (std::is_same_v<ValueType, int>) {
            container.push_back(42);
            container.push_back(84);
            std::cout << "  Added integers, new size: " << container.size() << "\n";
            std::cout << "  Front: " << container.front() << ", Back: " << container.back() << "\n";
        }
    }
}

// Demo functions
void demonstrateBasicConcepts() {
    using namespace BasicConcepts;
    
    std::cout << "=== Basic Concepts ===\n";
    
    // Arithmetic concept
    auto result1 = add(5, 3);
    std::cout << result1 << "\n";
    
    auto result2 = add(2.5, 1.7);
    std::cout << result2 << "\n\n";
    
    // Comparable concept
    auto maxInt = max(10, 7);
    std::cout << maxInt << "\n";
    
    auto maxString = BasicConcepts::max(std::string("apple"), std::string("banana"));
    std::cout << maxString << "\n\n";
    
    // Printable concept
    print(42);
    print(std::string("Hello Concepts!"));
    
    // Container concept
    std::vector<int> vec = {1, 2, 3, 4, 5};
    printContainer(vec);
}

void demonstrateAdvancedConcepts() {
    using namespace AdvancedConcepts;
    
    std::cout << "\n=== Advanced Concepts ===\n";
    
    // Simple vector class for demonstration
    class SimpleVector {
    public:
        using value_type = double;
        
    private:
        std::vector<double> data_;
        
    public:
        explicit SimpleVector(std::vector<double> data) : data_(std::move(data)) {}
        
        std::size_t size() const { return data_.size(); }
        
        double& operator[](std::size_t index) { return data_[index]; }
        const double& operator[](std::size_t index) const { return data_[index]; }
        
        SimpleVector operator+(const SimpleVector& other) const {
            if (size() != other.size()) {
                throw std::invalid_argument("Vector sizes must match");
            }
            
            std::vector<double> result;
            result.reserve(size());
            for (std::size_t i = 0; i < size(); ++i) {
                result.push_back(data_[i] + other.data_[i]);
            }
            return SimpleVector(std::move(result));
        }
        
        SimpleVector operator-(const SimpleVector& other) const {
            if (size() != other.size()) {
                throw std::invalid_argument("Vector sizes must match");
            }
            
            std::vector<double> result;
            result.reserve(size());
            for (std::size_t i = 0; i < size(); ++i) {
                result.push_back(data_[i] - other.data_[i]);
            }
            return SimpleVector(std::move(result));
        }
        
        SimpleVector operator*(double scalar) const {
            std::vector<double> result;
            result.reserve(size());
            for (double val : data_) {
                result.push_back(val * scalar);
            }
            return SimpleVector(std::move(result));
        }
        
        // Note: friend functions cannot be defined in local classes
        // MSVC doesn't support scalar * vec for local classes
        // The Vector concept test will be limited
    };
    
    SimpleVector v1({1.0, 2.0, 3.0});
    SimpleVector v2({4.0, 5.0, 6.0});
    
    // Note: dot_product requires scalar * vector which can't be implemented for local classes
    // Manually computing dot product instead
    double dot = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
        dot += v1[i] * v2[i];
    }
    std::cout << "Dot product result: " << dot << "\n";
    
    // Hash map demonstration
    std::unordered_map<std::string, int> map = {
        {"apple", 5},
        {"banana", 3},
        {"cherry", 8}
    };
    
    printStringIntMap(map);
}

void demonstrateConceptPatterns() {
    using namespace ConceptPatterns;
    
    std::cout << "\n=== Concept-Based Design Patterns ===\n";
    
    // Strategy pattern
    std::vector<int> data = {64, 34, 25, 12, 22, 11, 90};
    std::cout << "Original data: ";
    for (int x : data) std::cout << x << " ";
    std::cout << "\n";
    
    auto bubbleSorter = Sorter(BubbleSort{});
    bubbleSorter.performSort(data);
    
    std::cout << "Sorted data: ";
    for (int x : data) std::cout << x << " ";
    std::cout << "\n\n";
    
    // Observer pattern
    auto subject = Subject(Logger{}, EmailNotifier{});
    subject.notifyObservers("System startup completed");
    
    std::cout << "\n";
    
    // Factory pattern
    useFactory(ButtonFactory{});
    useFactory(TextFieldFactory{});
}

void demonstrateConceptSpecialization() {
    using namespace ConceptSpecialization;
    
    std::cout << "\n=== Concept-Based Specialization ===\n";
    
    Processor<int> intProcessor;
    Processor<double> floatProcessor;
    Processor<std::string> stringProcessor;
    
    intProcessor.process(42);
    floatProcessor.process(3.14159);
    stringProcessor.process("Hello World");
    
    std::cout << "\n";
    
    // Range algorithms
    std::vector<int> integers = {1, 2, 3, 4, 5};
    auto sum = sum_integers(integers);
    std::cout << "Sum: " << sum << "\n";
    
    std::vector<double> floats = {1.1, 2.2, 3.3, 4.4, 5.5};
    auto avg = average_floats(floats);
    std::cout << "Average: " << avg << "\n";
}

void demonstrateConceptComposition() {
    using namespace ConceptComposition;
    
    std::cout << "\n=== Concept Composition ===\n";
    
    // RAII with concepts
    {
        auto wrapper = RAIIWrapper(FileHandle("test.txt"));
        std::cout << "Working with file: " << wrapper.get().getFilename() << "\n";
    } // Resource automatically released here
    
    // Sequence container operations
    std::vector<int> vec;
    demonstrateSequenceOps(vec);
}

int main() {
    std::cout << "=== Concepts Pattern Demo (C++20) ===\n\n";
    
    demonstrateBasicConcepts();
    demonstrateAdvancedConcepts();
    demonstrateConceptPatterns();
    demonstrateConceptSpecialization();
    demonstrateConceptComposition();
    
    std::cout << "\n=== Concepts Benefits ===\n";
    std::cout << "1. Clear, readable template constraints\n";
    std::cout << "2. Better error messages\n";
    std::cout << "3. Self-documenting code\n";
    std::cout << "4. Compile-time interface checking\n";
    std::cout << "5. Enhanced template metaprogramming\n";
    
    return 0;
}