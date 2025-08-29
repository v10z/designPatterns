// Minimal Type Erasure Pattern Implementation
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <typeinfo>
#include <any>

// Example 1: Basic Type Erasure for Drawable Objects
namespace BasicTypeErasure {
    // Type-erased interface
    class Drawable {
    private:
        // Concept - internal interface
        struct Concept {
            virtual ~Concept() = default;
            virtual void draw() const = 0;
            virtual std::unique_ptr<Concept> clone() const = 0;
        };
        
        // Model - concrete implementation for type T
        template<typename T>
        struct Model : Concept {
            T object;
            
            explicit Model(T obj) : object(std::move(obj)) {}
            
            void draw() const override {
                object.draw();
            }
            
            std::unique_ptr<Concept> clone() const override {
                return std::make_unique<Model>(*this);
            }
        };
        
        std::unique_ptr<Concept> pimpl;
        
    public:
        // Constructor accepts any type with draw() method
        template<typename T>
        Drawable(T object) 
            : pimpl(std::make_unique<Model<T>>(std::move(object))) {}
        
        // Copy constructor
        Drawable(const Drawable& other) 
            : pimpl(other.pimpl->clone()) {}
        
        // Move constructor
        Drawable(Drawable&&) noexcept = default;
        
        // Assignment operators
        Drawable& operator=(const Drawable& other) {
            pimpl = other.pimpl->clone();
            return *this;
        }
        
        Drawable& operator=(Drawable&&) noexcept = default;
        
        // Public interface
        void draw() const {
            pimpl->draw();
        }
    };
    
    // Concrete types with draw() method
    class Circle {
    private:
        double radius_;
        
    public:
        explicit Circle(double radius) : radius_(radius) {}
        
        void draw() const {
            std::cout << "Drawing Circle with radius " << radius_ << "\n";
        }
    };
    
    class Rectangle {
    private:
        double width_, height_;
        
    public:
        Rectangle(double width, double height) 
            : width_(width), height_(height) {}
        
        void draw() const {
            std::cout << "Drawing Rectangle " << width_ << "x" << height_ << "\n";
        }
    };
    
    class Text {
    private:
        std::string content_;
        
    public:
        explicit Text(const std::string& content) : content_(content) {}
        
        void draw() const {
            std::cout << "Drawing Text: \"" << content_ << "\"\n";
        }
    };
}

// Example 2: Type Erasure with Multiple Methods
namespace MultiMethodTypeErasure {
    // Type-erased Animal
    class Animal {
    private:
        struct Concept {
            virtual ~Concept() = default;
            virtual void makeSound() const = 0;
            virtual std::string getName() const = 0;
            virtual int getAge() const = 0;
            virtual std::unique_ptr<Concept> clone() const = 0;
        };
        
        template<typename T>
        struct Model : Concept {
            T object;
            
            explicit Model(T obj) : object(std::move(obj)) {}
            
            void makeSound() const override {
                object.makeSound();
            }
            
            std::string getName() const override {
                return object.getName();
            }
            
            int getAge() const override {
                return object.getAge();
            }
            
            std::unique_ptr<Concept> clone() const override {
                return std::make_unique<Model>(*this);
            }
        };
        
        std::unique_ptr<Concept> pimpl;
        
    public:
        template<typename T>
        Animal(T object)
            : pimpl(std::make_unique<Model<T>>(std::move(object))) {}
        
        Animal(const Animal& other) : pimpl(other.pimpl->clone()) {}
        Animal(Animal&&) noexcept = default;
        
        Animal& operator=(const Animal& other) {
            pimpl = other.pimpl->clone();
            return *this;
        }
        Animal& operator=(Animal&&) noexcept = default;
        
        void makeSound() const { pimpl->makeSound(); }
        std::string getName() const { return pimpl->getName(); }
        int getAge() const { return pimpl->getAge(); }
    };
    
    // Concrete animal types
    class Dog {
    private:
        std::string name_;
        int age_;
        
    public:
        Dog(const std::string& name, int age) : name_(name), age_(age) {}
        
        void makeSound() const {
            std::cout << name_ << " says: Woof! Woof!\n";
        }
        
        std::string getName() const { return name_; }
        int getAge() const { return age_; }
    };
    
    class Cat {
    private:
        std::string name_;
        int age_;
        
    public:
        Cat(const std::string& name, int age) : name_(name), age_(age) {}
        
        void makeSound() const {
            std::cout << name_ << " says: Meow!\n";
        }
        
        std::string getName() const { return name_; }
        int getAge() const { return age_; }
    };
    
    class Parrot {
    private:
        std::string name_;
        int age_;
        std::string phrase_;
        
    public:
        Parrot(const std::string& name, int age, const std::string& phrase)
            : name_(name), age_(age), phrase_(phrase) {}
        
        void makeSound() const {
            std::cout << name_ << " says: " << phrase_ << "\n";
        }
        
        std::string getName() const { return name_; }
        int getAge() const { return age_; }
    };
}

// Example 3: Function Type Erasure
namespace FunctionTypeErasure {
    // Type-erased function wrapper
    class Function {
    private:
        struct Concept {
            virtual ~Concept() = default;
            virtual int invoke(int x) const = 0;
            virtual std::unique_ptr<Concept> clone() const = 0;
        };
        
        template<typename F>
        struct Model : Concept {
            F function;
            
            explicit Model(F f) : function(std::move(f)) {}
            
            int invoke(int x) const override {
                return function(x);
            }
            
            std::unique_ptr<Concept> clone() const override {
                return std::make_unique<Model>(*this);
            }
        };
        
        std::unique_ptr<Concept> pimpl;
        
    public:
        template<typename F>
        Function(F f) : pimpl(std::make_unique<Model<F>>(std::move(f))) {}
        
        Function(const Function& other) : pimpl(other.pimpl->clone()) {}
        Function(Function&&) noexcept = default;
        
        Function& operator=(const Function& other) {
            pimpl = other.pimpl->clone();
            return *this;
        }
        Function& operator=(Function&&) noexcept = default;
        
        int operator()(int x) const {
            return pimpl->invoke(x);
        }
    };
    
    // Various callable types
    int square(int x) { return x * x; }
    
    struct Multiplier {
        int factor;
        explicit Multiplier(int f) : factor(f) {}
        int operator()(int x) const { return x * factor; }
    };
    
    class Counter {
    private:
        mutable int count_ = 0;
        
    public:
        int operator()(int x) const {
            count_++;
            return x + count_;
        }
    };
}

// Example 4: Advanced Type Erasure with Small Buffer Optimization
namespace AdvancedTypeErasure {
    template<typename Signature>
    class FunctionRef;
    
    template<typename R, typename... Args>
    class FunctionRef<R(Args...)> {
    private:
        void* object_;
        R (*invoke_)(void*, Args...);
        
        template<typename F>
        static R invoker(void* obj, Args... args) {
            return (*static_cast<F*>(obj))(std::forward<Args>(args)...);
        }
        
    public:
        template<typename F>
        FunctionRef(F& f) noexcept
            : object_(&f), invoke_(invoker<F>) {}
        
        R operator()(Args... args) const {
            return invoke_(object_, std::forward<Args>(args)...);
        }
    };
    
    // Type-erased container with small buffer optimization
    template<size_t Size = 32>
    class AnySmall {
    private:
        static constexpr size_t BufferSize = Size;
        alignas(std::max_align_t) char buffer_[BufferSize];
        
        struct VTable {
            void (*destroy)(void*);
            void (*copy)(const void*, void*);
            void (*move)(void*, void*);
            const std::type_info* (*type)();
        };
        
        template<typename T>
        static constexpr VTable vtableFor() {
            return VTable{
                [](void* p) { static_cast<T*>(p)->~T(); },
                [](const void* src, void* dst) { 
                    new(dst) T(*static_cast<const T*>(src)); 
                },
                [](void* src, void* dst) { 
                    new(dst) T(std::move(*static_cast<T*>(src))); 
                },
                []() -> const std::type_info* { return &typeid(T); }
            };
        }
        
        const VTable* vtable_ = nullptr;
        
    public:
        AnySmall() = default;
        
        template<typename T>
        AnySmall(T&& value) {
            static_assert(sizeof(T) <= BufferSize, "Type too large for small buffer");
            static_assert(alignof(T) <= alignof(std::max_align_t), "Type has excessive alignment");
            
            new(buffer_) std::decay_t<T>(std::forward<T>(value));
            vtable_ = &vtableFor<std::decay_t<T>>();
        }
        
        ~AnySmall() {
            if (vtable_) {
                vtable_->destroy(buffer_);
            }
        }
        
        AnySmall(const AnySmall& other) {
            if (other.vtable_) {
                other.vtable_->copy(other.buffer_, buffer_);
                vtable_ = other.vtable_;
            }
        }
        
        AnySmall(AnySmall&& other) noexcept {
            if (other.vtable_) {
                other.vtable_->move(other.buffer_, buffer_);
                vtable_ = other.vtable_;
                other.vtable_ = nullptr;
            }
        }
        
        template<typename T>
        T* get() {
            if (vtable_ && vtable_->type() == &typeid(T)) {
                return static_cast<T*>(static_cast<void*>(buffer_));
            }
            return nullptr;
        }
        
        bool has_value() const { return vtable_ != nullptr; }
        
        const std::type_info& type() const {
            return vtable_ ? *vtable_->type() : typeid(void);
        }
    };
}

// Example 5: Visitor Pattern with Type Erasure
namespace VisitorTypeErasure {
    class ShapeVisitor {
    private:
        struct Concept {
            virtual ~Concept() = default;
            virtual void visitCircle(double radius) = 0;
            virtual void visitRectangle(double width, double height) = 0;
            virtual void visitTriangle(double base, double height) = 0;
        };
        
        template<typename T>
        struct Model : Concept {
            T visitor;
            
            explicit Model(T v) : visitor(std::move(v)) {}
            
            void visitCircle(double radius) override {
                visitor.visitCircle(radius);
            }
            
            void visitRectangle(double width, double height) override {
                visitor.visitRectangle(width, height);
            }
            
            void visitTriangle(double base, double height) override {
                visitor.visitTriangle(base, height);
            }
        };
        
        std::unique_ptr<Concept> pimpl;
        
    public:
        template<typename T>
        ShapeVisitor(T visitor)
            : pimpl(std::make_unique<Model<T>>(std::move(visitor))) {}
        
        void visitCircle(double radius) {
            pimpl->visitCircle(radius);
        }
        
        void visitRectangle(double width, double height) {
            pimpl->visitRectangle(width, height);
        }
        
        void visitTriangle(double base, double height) {
            pimpl->visitTriangle(base, height);
        }
    };
    
    // Concrete visitors
    struct AreaCalculator {
        double totalArea = 0;
        
        void visitCircle(double radius) {
            totalArea += 3.14159 * radius * radius;
            std::cout << "Circle area: " << 3.14159 * radius * radius << "\n";
        }
        
        void visitRectangle(double width, double height) {
            totalArea += width * height;
            std::cout << "Rectangle area: " << width * height << "\n";
        }
        
        void visitTriangle(double base, double height) {
            totalArea += 0.5 * base * height;
            std::cout << "Triangle area: " << 0.5 * base * height << "\n";
        }
    };
    
    struct ShapeDrawer {
        void visitCircle(double radius) {
            std::cout << "Drawing circle with radius " << radius << "\n";
        }
        
        void visitRectangle(double width, double height) {
            std::cout << "Drawing rectangle " << width << "x" << height << "\n";
        }
        
        void visitTriangle(double base, double height) {
            std::cout << "Drawing triangle with base " << base 
                      << " and height " << height << "\n";
        }
    };
}

// Demo functions
void demonstrateBasicTypeErasure() {
    using namespace BasicTypeErasure;
    
    std::cout << "=== Basic Type Erasure ===\n";
    
    std::vector<Drawable> shapes;
    shapes.emplace_back(Circle(5.0));
    shapes.emplace_back(Rectangle(10.0, 20.0));
    shapes.emplace_back(Text("Hello, Type Erasure!"));
    
    std::cout << "Drawing all shapes:\n";
    for (const auto& shape : shapes) {
        shape.draw();
    }
    
    // Copy test
    std::cout << "\nTesting copy:\n";
    Drawable copiedShape = shapes[0];
    copiedShape.draw();
}

void demonstrateMultiMethodTypeErasure() {
    using namespace MultiMethodTypeErasure;
    
    std::cout << "\n=== Multi-Method Type Erasure ===\n";
    
    std::vector<Animal> zoo;
    zoo.emplace_back(Dog("Buddy", 5));
    zoo.emplace_back(Cat("Whiskers", 3));
    zoo.emplace_back(Parrot("Polly", 10, "Hello!"));
    
    std::cout << "Zoo animals:\n";
    for (const auto& animal : zoo) {
        std::cout << animal.getName() << " (age " << animal.getAge() << "): ";
        animal.makeSound();
    }
}

void demonstrateFunctionTypeErasure() {
    using namespace FunctionTypeErasure;
    
    std::cout << "\n=== Function Type Erasure ===\n";
    
    std::vector<Function> functions;
    functions.emplace_back(square);
    functions.emplace_back(Multiplier(3));
    functions.emplace_back(Counter());
    functions.emplace_back([](int x) { return x + 10; });
    
    std::cout << "Applying functions to value 5:\n";
    for (const auto& func : functions) {
        std::cout << "Result: " << func(5) << "\n";
    }
}

void demonstrateAdvancedTypeErasure() {
    using namespace AdvancedTypeErasure;
    
    std::cout << "\n=== Advanced Type Erasure ===\n";
    
    // Function reference
    auto lambda = [](int x, int y) { return x + y; };
    FunctionRef<int(int, int)> funcRef = lambda;
    std::cout << "Function reference result: " << funcRef(10, 20) << "\n";
    
    // Small buffer optimization
    AnySmall<32> small1(42);
    AnySmall<32> small2(std::string("Hello"));
    AnySmall<32> small3(3.14);
    
    if (auto* p = small1.get<int>()) {
        std::cout << "Integer value: " << *p << "\n";
    }
    
    if (auto* p = small2.get<std::string>()) {
        std::cout << "String value: " << *p << "\n";
    }
    
    if (auto* p = small3.get<double>()) {
        std::cout << "Double value: " << *p << "\n";
    }
}

void demonstrateVisitorTypeErasure() {
    using namespace VisitorTypeErasure;
    
    std::cout << "\n=== Visitor Type Erasure ===\n";
    
    AreaCalculator areaCalc;
    ShapeVisitor areaVisitor(areaCalc);
    
    std::cout << "Calculating areas:\n";
    areaVisitor.visitCircle(5.0);
    areaVisitor.visitRectangle(4.0, 6.0);
    areaVisitor.visitTriangle(3.0, 4.0);
    
    ShapeDrawer drawer;
    ShapeVisitor drawVisitor(drawer);
    
    std::cout << "\nDrawing shapes:\n";
    drawVisitor.visitCircle(5.0);
    drawVisitor.visitRectangle(4.0, 6.0);
    drawVisitor.visitTriangle(3.0, 4.0);
}

int main() {
    std::cout << "=== Type Erasure Pattern Demo ===\n\n";
    
    demonstrateBasicTypeErasure();
    demonstrateMultiMethodTypeErasure();
    demonstrateFunctionTypeErasure();
    demonstrateAdvancedTypeErasure();
    demonstrateVisitorTypeErasure();
    
    std::cout << "\n=== Type Erasure Benefits ===\n";
    std::cout << "1. Runtime polymorphism without inheritance\n";
    std::cout << "2. Value semantics (no pointers)\n";
    std::cout << "3. Decouples interface from implementation\n";
    std::cout << "4. No virtual function overhead in concrete types\n";
    std::cout << "5. Can work with third-party types\n";
    
    return 0;
}