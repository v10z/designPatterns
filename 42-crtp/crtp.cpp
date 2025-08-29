// Minimal CRTP (Curiously Recurring Template Pattern) Implementation
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cmath>
#include <memory>
#include <algorithm>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Example 1: Static Polymorphism with CRTP
namespace StaticPolymorphism {
    // CRTP base class
    template<typename Derived>
    class Shape {
    public:
        double area() const {
            return static_cast<const Derived*>(this)->area_impl();
        }
        
        double perimeter() const {
            return static_cast<const Derived*>(this)->perimeter_impl();
        }
        
        void draw() const {
            static_cast<const Derived*>(this)->draw_impl();
        }
        
        std::string name() const {
            return static_cast<const Derived*>(this)->name_impl();
        }
    };
    
    // Derived classes
    class Circle : public Shape<Circle> {
    private:
        double radius_;
        
    public:
        explicit Circle(double radius) : radius_(radius) {}
        
        double area_impl() const {
            return M_PI * radius_ * radius_;
        }
        
        double perimeter_impl() const {
            return 2 * M_PI * radius_;
        }
        
        void draw_impl() const {
            std::cout << "Drawing circle with radius " << radius_ << "\n";
        }
        
        std::string name_impl() const {
            return "Circle";
        }
    };
    
    class Rectangle : public Shape<Rectangle> {
    private:
        double width_, height_;
        
    public:
        Rectangle(double width, double height) 
            : width_(width), height_(height) {}
        
        double area_impl() const {
            return width_ * height_;
        }
        
        double perimeter_impl() const {
            return 2 * (width_ + height_);
        }
        
        void draw_impl() const {
            std::cout << "Drawing rectangle " << width_ << "x" << height_ << "\n";
        }
        
        std::string name_impl() const {
            return "Rectangle";
        }
    };
    
    class Triangle : public Shape<Triangle> {
    private:
        double base_, height_;
        double side1_, side2_, side3_;
        
    public:
        Triangle(double base, double height, double s1, double s2, double s3)
            : base_(base), height_(height), side1_(s1), side2_(s2), side3_(s3) {}
        
        double area_impl() const {
            return 0.5 * base_ * height_;
        }
        
        double perimeter_impl() const {
            return side1_ + side2_ + side3_;
        }
        
        void draw_impl() const {
            std::cout << "Drawing triangle with base " << base_ 
                      << " and height " << height_ << "\n";
        }
        
        std::string name_impl() const {
            return "Triangle";
        }
    };
    
    // Template function that works with any shape
    template<typename T>
    void printShapeInfo(const Shape<T>& shape) {
        std::cout << shape.name() << ":\n";
        std::cout << "  Area: " << shape.area() << "\n";
        std::cout << "  Perimeter: " << shape.perimeter() << "\n";
        shape.draw();
    }
}

// Example 2: Mixin-style functionality with CRTP
namespace MixinStyle {
    // CRTP mixin for comparison operators
    template<typename Derived>
    class Comparable {
    public:
        bool operator!=(const Derived& other) const {
            return !static_cast<const Derived*>(this)->operator==(other);
        }
        
        bool operator<=(const Derived& other) const {
            const Derived& self = *static_cast<const Derived*>(this);
            return self < other || self == other;
        }
        
        bool operator>(const Derived& other) const {
            return !static_cast<const Derived*>(this)->operator<=(other);
        }
        
        bool operator>=(const Derived& other) const {
            return !static_cast<const Derived*>(this)->operator<(other);
        }
    };
    
    // CRTP mixin for arithmetic operators
    template<typename Derived>
    class Arithmetic {
    public:
        Derived operator-(const Derived& other) const {
            Derived result = *static_cast<const Derived*>(this);
            result -= other;
            return result;
        }
        
        Derived operator*(double scalar) const {
            Derived result = *static_cast<const Derived*>(this);
            result *= scalar;
            return result;
        }
        
        Derived operator/(double scalar) const {
            Derived result = *static_cast<const Derived*>(this);
            result /= scalar;
            return result;
        }
    };
    
    // Point class using mixins
    class Point : public Comparable<Point>, public Arithmetic<Point> {
    private:
        double x_, y_;
        
    public:
        Point(double x = 0, double y = 0) : x_(x), y_(y) {}
        
        // Required for Comparable
        bool operator==(const Point& other) const {
            return x_ == other.x_ && y_ == other.y_;
        }
        
        bool operator<(const Point& other) const {
            if (x_ != other.x_) return x_ < other.x_;
            return y_ < other.y_;
        }
        
        // Required for Arithmetic
        Point operator+(const Point& other) const {
            return Point(x_ + other.x_, y_ + other.y_);
        }
        
        Point& operator+=(const Point& other) {
            x_ += other.x_;
            y_ += other.y_;
            return *this;
        }
        
        Point& operator-=(const Point& other) {
            x_ -= other.x_;
            y_ -= other.y_;
            return *this;
        }
        
        Point& operator*=(double scalar) {
            x_ *= scalar;
            y_ *= scalar;
            return *this;
        }
        
        Point& operator/=(double scalar) {
            x_ /= scalar;
            y_ /= scalar;
            return *this;
        }
        
        void print() const {
            std::cout << "(" << x_ << ", " << y_ << ")";
        }
    };
}

// Example 3: Clone Pattern with CRTP
namespace ClonePattern {
    template<typename Derived>
    class Cloneable {
    public:
        std::unique_ptr<Derived> clone() const {
            return std::make_unique<Derived>(*static_cast<const Derived*>(this));
        }
    };
    
    // Concrete classes
    class Document : public Cloneable<Document> {
    private:
        std::string title_;
        std::string content_;
        
    public:
        Document(const std::string& title, const std::string& content)
            : title_(title), content_(content) {}
        
        void setContent(const std::string& content) { content_ = content; }
        
        void print() const {
            std::cout << "Document: " << title_ << "\n";
            std::cout << "Content: " << content_ << "\n";
        }
    };
    
    class Image : public Cloneable<Image> {
    private:
        std::string filename_;
        int width_, height_;
        
    public:
        Image(const std::string& filename, int width, int height)
            : filename_(filename), width_(width), height_(height) {}
        
        void resize(int width, int height) {
            width_ = width;
            height_ = height;
        }
        
        void print() const {
            std::cout << "Image: " << filename_ << " (" 
                      << width_ << "x" << height_ << ")\n";
        }
    };
}

// Example 4: Counter Pattern with CRTP
namespace CounterPattern {
    template<typename Derived>
    class InstanceCounter {
    private:
        inline static size_t count_ = 0;
        inline static size_t totalCreated_ = 0;
        
    protected:
        InstanceCounter() {
            ++count_;
            ++totalCreated_;
        }
        
        InstanceCounter(const InstanceCounter&) {
            ++count_;
            ++totalCreated_;
        }
        
        ~InstanceCounter() {
            --count_;
        }
        
    public:
        static size_t getInstanceCount() { return count_; }
        static size_t getTotalCreated() { return totalCreated_; }
    };
    
    // Classes that track their instances
    class Widget : public InstanceCounter<Widget> {
    private:
        std::string name_;
        
    public:
        explicit Widget(const std::string& name) : name_(name) {
            std::cout << "Widget '" << name_ << "' created\n";
        }
        
        ~Widget() {
            std::cout << "Widget '" << name_ << "' destroyed\n";
        }
    };
    
    class Gadget : public InstanceCounter<Gadget> {
    private:
        int id_;
        
    public:
        explicit Gadget(int id) : id_(id) {
            std::cout << "Gadget " << id_ << " created\n";
        }
        
        ~Gadget() {
            std::cout << "Gadget " << id_ << " destroyed\n";
        }
    };
}

// Example 5: Singleton Pattern with CRTP
namespace SingletonPattern {
    template<typename Derived>
    class Singleton {
    protected:
        Singleton() = default;
        
    public:
        // Delete copy/move operations
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;
        Singleton(Singleton&&) = delete;
        Singleton& operator=(Singleton&&) = delete;
        
        static Derived& getInstance() {
            static Derived instance;
            return instance;
        }
    };
    
    // Singleton classes
    class Logger : public Singleton<Logger> {
        friend class Singleton<Logger>;
        
    private:
        Logger() {
            std::cout << "Logger initialized\n";
        }
        
    public:
        void log(const std::string& message) {
            std::cout << "[LOG] " << message << "\n";
        }
    };
    
    class ConfigManager : public Singleton<ConfigManager> {
        friend class Singleton<ConfigManager>;
        
    private:
        std::vector<std::pair<std::string, std::string>> settings_;
        
        ConfigManager() {
            std::cout << "ConfigManager initialized\n";
            // Load default settings
            settings_.push_back({"app.name", "CRTP Demo"});
            settings_.push_back({"version", "1.0"});
        }
        
    public:
        void setSetting(const std::string& key, const std::string& value) {
            auto it = std::find_if(settings_.begin(), settings_.end(),
                [&key](const auto& pair) { return pair.first == key; });
            
            if (it != settings_.end()) {
                it->second = value;
            } else {
                settings_.push_back({key, value});
            }
        }
        
        std::string getSetting(const std::string& key) const {
            auto it = std::find_if(settings_.begin(), settings_.end(),
                [&key](const auto& pair) { return pair.first == key; });
            
            return (it != settings_.end()) ? it->second : "";
        }
    };
}

// Example 6: Enable Shared From This Pattern
namespace SharedFromThis {
    template<typename Derived>
    class EnableSharedFromThis {
    private:
        mutable std::weak_ptr<Derived> weak_this_;
        
    public:
        std::shared_ptr<Derived> shared_from_this() {
            return std::shared_ptr<Derived>(weak_this_);
        }
        
        std::shared_ptr<const Derived> shared_from_this() const {
            return std::shared_ptr<const Derived>(weak_this_);
        }
        
        template<typename T>
        friend void setWeakPtr(EnableSharedFromThis<T>* obj, 
                              const std::shared_ptr<T>& ptr);
    };
    
    template<typename T>
    void setWeakPtr(EnableSharedFromThis<T>* obj, const std::shared_ptr<T>& ptr) {
        obj->weak_this_ = ptr;
    }
    
    template<typename T, typename... Args>
    std::shared_ptr<T> makeShared(Args&&... args) {
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        setWeakPtr(ptr.get(), ptr);
        return ptr;
    }
    
    // Example class
    class Node : public EnableSharedFromThis<Node> {
    private:
        std::string name_;
        std::vector<std::shared_ptr<Node>> children_;
        
    public:
        explicit Node(const std::string& name) : name_(name) {}
        
        void addChild(std::shared_ptr<Node> child) {
            children_.push_back(child);
        }
        
        std::shared_ptr<Node> getPtr() {
            return shared_from_this();
        }
        
        void print() const {
            std::cout << "Node: " << name_ << "\n";
        }
    };
}

// Demo functions
void demonstrateStaticPolymorphism() {
    using namespace StaticPolymorphism;
    
    std::cout << "=== Static Polymorphism with CRTP ===\n";
    
    Circle circle(5.0);
    Rectangle rectangle(4.0, 6.0);
    Triangle triangle(3.0, 4.0, 3.0, 4.0, 5.0);
    
    printShapeInfo(circle);
    std::cout << "\n";
    printShapeInfo(rectangle);
    std::cout << "\n";
    printShapeInfo(triangle);
    
    // Performance: No virtual function overhead
    auto start = std::chrono::high_resolution_clock::now();
    double totalArea = 0;
    for (int i = 0; i < 1000000; ++i) {
        totalArea += circle.area() + rectangle.area() + triangle.area();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "\nPerformance test (1M iterations): " 
              << duration.count() << " microseconds\n";
}

void demonstrateMixinStyle() {
    using namespace MixinStyle;
    
    std::cout << "\n=== Mixin-style CRTP ===\n";
    
    Point p1(3, 4);
    Point p2(1, 2);
    
    std::cout << "p1 = "; p1.print(); std::cout << "\n";
    std::cout << "p2 = "; p2.print(); std::cout << "\n";
    
    // Using Comparable mixin
    std::cout << "\nComparison operations:\n";
    std::cout << "p1 == p2: " << (p1 == p2) << "\n";
    std::cout << "p1 != p2: " << (p1 != p2) << "\n";
    std::cout << "p1 < p2: " << (p1 < p2) << "\n";
    std::cout << "p1 > p2: " << (p1 > p2) << "\n";
    
    // Using Arithmetic mixin
    std::cout << "\nArithmetic operations:\n";
    Point p3 = p1 + p2;
    std::cout << "p1 + p2 = "; p3.print(); std::cout << "\n";
    
    Point p4 = p1 - p2;
    std::cout << "p1 - p2 = "; p4.print(); std::cout << "\n";
    
    Point p5 = p1 * 2.0;
    std::cout << "p1 * 2 = "; p5.print(); std::cout << "\n";
}

void demonstrateClonePattern() {
    using namespace ClonePattern;
    
    std::cout << "\n=== Clone Pattern with CRTP ===\n";
    
    Document doc("Report", "This is the original content");
    doc.print();
    
    auto docClone = doc.clone();
    docClone->setContent("This is the modified content");
    
    std::cout << "\nOriginal after clone modification:\n";
    doc.print();
    std::cout << "\nClone:\n";
    docClone->print();
    
    Image img("photo.jpg", 1920, 1080);
    auto imgClone = img.clone();
    imgClone->resize(800, 600);
    
    std::cout << "\nOriginal image:\n";
    img.print();
    std::cout << "Cloned image:\n";
    imgClone->print();
}

void demonstrateCounterPattern() {
    using namespace CounterPattern;
    
    std::cout << "\n=== Counter Pattern with CRTP ===\n";
    
    {
        Widget w1("First");
        Widget w2("Second");
        
        std::cout << "Widget instances: " << Widget::getInstanceCount() << "\n";
        
        {
            Widget w3("Third");
            Gadget g1(101);
            
            std::cout << "Widget instances: " << Widget::getInstanceCount() << "\n";
            std::cout << "Gadget instances: " << Gadget::getInstanceCount() << "\n";
        }
        
        std::cout << "After scope - Widget instances: " 
                  << Widget::getInstanceCount() << "\n";
        std::cout << "After scope - Gadget instances: " 
                  << Gadget::getInstanceCount() << "\n";
    }
    
    std::cout << "\nTotal widgets created: " << Widget::getTotalCreated() << "\n";
    std::cout << "Total gadgets created: " << Gadget::getTotalCreated() << "\n";
    std::cout << "Current widget instances: " << Widget::getInstanceCount() << "\n";
    std::cout << "Current gadget instances: " << Gadget::getInstanceCount() << "\n";
}

void demonstrateSingletonPattern() {
    using namespace SingletonPattern;
    
    std::cout << "\n=== Singleton Pattern with CRTP ===\n";
    
    // Get logger instance
    Logger& logger1 = Logger::getInstance();
    logger1.log("First message");
    
    // Get same instance
    Logger& logger2 = Logger::getInstance();
    logger2.log("Second message");
    
    std::cout << "Same logger instance: " << (&logger1 == &logger2) << "\n";
    
    // Config manager
    ConfigManager& config = ConfigManager::getInstance();
    std::cout << "\nApp name: " << config.getSetting("app.name") << "\n";
    std::cout << "Version: " << config.getSetting("version") << "\n";
    
    config.setSetting("user", "John Doe");
    std::cout << "User: " << config.getSetting("user") << "\n";
}

int main() {
    std::cout << "=== CRTP (Curiously Recurring Template Pattern) Demo ===\n\n";
    
    demonstrateStaticPolymorphism();
    demonstrateMixinStyle();
    demonstrateClonePattern();
    demonstrateCounterPattern();
    demonstrateSingletonPattern();
    
    std::cout << "\n=== CRTP Benefits ===\n";
    std::cout << "1. Static polymorphism (no virtual function overhead)\n";
    std::cout << "2. Compile-time interface enforcement\n";
    std::cout << "3. Mixin-style functionality\n";
    std::cout << "4. Code reuse without runtime cost\n";
    std::cout << "5. Type safety\n";
    
    return 0;
}