# Modern C++ Design Patterns

A comprehensive collection of 58 design patterns implemented in modern C++ (C++11/14/17/20), demonstrating best practices and real-world applications.

## 📚 Overview

This repository contains implementations of classic Gang of Four patterns, concurrency patterns, architectural patterns, and modern C++ idioms. Each pattern includes:
- Complete working example
- Detailed explanation
- Use cases and when to apply
- UML diagrams
- Compilation instructions

## 🚀 Quick Start

### Prerequisites

#### Minimum Requirements
- C++ compiler with C++11 support (for basic patterns)
- C++14 support recommended (GCC 5+, Clang 3.4+, MSVC 2015+)
- C++17 support for modern patterns (GCC 7+, Clang 5+, MSVC 2017 15.7+)
- C++20 support for cutting-edge patterns (GCC 10+, Clang 11+, MSVC 2019 16.8+)

#### Compiler Support by Pattern
| C++ Standard | Required For | Compiler Versions |
|--------------|--------------|-------------------|
| C++11 | Most patterns (1-45) | GCC 4.8+, Clang 3.3+, MSVC 2013+ |
| C++14 | Advanced patterns (46-55) | GCC 5+, Clang 3.4+, MSVC 2015+ |
| C++17 | Modern idioms | GCC 7+, Clang 5+, MSVC 2017 15.7+ |
| C++20 | Consteval (56), Concepts (57), Coroutines (58) | GCC 10+, Clang 11+, MSVC 2019 16.8+ |

### Building All Examples

#### Windows (Visual Studio)
```batch
REM From Developer Command Prompt:
build_all_vs.bat

REM Or auto-detect compiler:
build_all.bat
```

#### Windows (Visual Studio with Clang)
```bash
./build_vs_clang.sh
```

#### Linux/macOS
```bash
chmod +x build_all.sh
./build_all.sh
```

### Building Individual Examples
```bash
# Linux/macOS
g++ -std=c++17 -Wall -O2 01-singleton/singleton.cpp -o singleton

# Windows (MinGW)
g++ -std=c++17 -Wall -O2 01-singleton\singleton.cpp -o singleton.exe

# Windows (MSVC)
cl /std:c++17 /EHsc 01-singleton\singleton.cpp
```

## 📋 Pattern Categories

### 🏗️ Creational Patterns (5)
Patterns that deal with object creation mechanisms.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [01-singleton](01-singleton/) | Ensures a class has only one instance | C++11 |
| [02-factory-method](02-factory-method/) | Defines an interface for creating objects | C++11 |
| [03-abstract-factory](03-abstract-factory/) | Creates families of related objects | C++11 |
| [04-builder](04-builder/) | Constructs complex objects step by step | C++11 |
| [05-prototype](05-prototype/) | Creates objects by cloning existing instances | C++11 |

### 🔧 Structural Patterns (7)
Patterns that deal with object composition and relationships.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [06-adapter](06-adapter/) | Allows incompatible interfaces to work together | C++11 |
| [07-bridge](07-bridge/) | Separates abstraction from implementation | C++11 |
| [08-composite](08-composite/) | Composes objects into tree structures | C++11 |
| [09-decorator](09-decorator/) | Adds new functionality to objects dynamically | C++11 |
| [10-facade](10-facade/) | Provides a simplified interface to a complex subsystem | C++11 |
| [11-flyweight](11-flyweight/) | Reduces memory usage by sharing data | C++11 |
| [12-proxy](12-proxy/) | Provides a placeholder/representative for another object | C++11 |

### 🎯 Behavioral Patterns (11)
Patterns that identify common communication patterns between objects.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [13-chain-of-responsibility](13-chain-of-responsibility/) | Passes requests along a chain of handlers | C++11 |
| [14-command](14-command/) | Encapsulates requests as objects | C++11 |
| [15-iterator](15-iterator/) | Provides a way to access elements sequentially | C++11 |
| [16-mediator](16-mediator/) | Defines how objects interact | C++11 |
| [17-memento](17-memento/) | Captures and restores object's internal state | C++11 |
| [18-observer](18-observer/) | Notifies multiple objects about state changes | C++11 |
| [19-state](19-state/) | Changes object behavior when state changes | C++11 |
| [20-strategy](20-strategy/) | Defines a family of algorithms | C++11 |
| [21-template-method](21-template-method/) | Defines skeleton of algorithm in base class | C++11 |
| [22-visitor](22-visitor/) | Separates algorithms from object structure | C++11 |
| [23-interpreter](23-interpreter/) | Implements a specialized language | C++11 |

### 🔄 Concurrency Patterns (7)
Patterns for multi-threaded and concurrent programming.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [26-active-object](26-active-object/) | Decouples method execution from invocation | C++11 |
| [27-monitor-object](27-monitor-object/) | Synchronizes method execution | C++11 |
| [28-producer-consumer](28-producer-consumer/) | Separates data production from consumption | C++11 |
| [29-read-write-lock](29-read-write-lock/) | Allows concurrent reads, exclusive writes | C++14 |
| [30-thread-pool](30-thread-pool/) | Reuses threads for multiple tasks | C++11 |
| [31-future-promise](31-future-promise/) | Provides asynchronous value communication | C++11 |
| [52-double-checked-locking](52-double-checked-locking/) | Reduces locking overhead | C++11 |

### 📊 Data Patterns (4)
Patterns for data access and management.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [32-repository](32-repository/) | Encapsulates data access logic | C++11 |
| [33-unit-of-work](33-unit-of-work/) | Maintains list of business objects changes | C++11 |
| [34-data-mapper](34-data-mapper/) | Maps between objects and database | C++11 |
| [50-lazy-loading](50-lazy-loading/) | Defers initialization until needed | C++11 |

### 🏛️ Architectural Patterns (8)
High-level patterns for application architecture.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [35-service-locator](35-service-locator/) | Provides global access to services | C++11 |
| [36-dependency-injection](36-dependency-injection/) | Provides dependencies from external sources | C++11 |
| [37-mvc](37-mvc/) | Model-View-Controller architecture | C++11 |
| [38-mvp](38-mvp/) | Model-View-Presenter architecture | C++11 |
| [39-mvvm](39-mvvm/) | Model-View-ViewModel architecture | C++11 |
| [46-publish-subscribe](46-publish-subscribe/) | Loose coupling between publishers and subscribers | C++11 |
| [47-message-queue](47-message-queue/) | Asynchronous message-based communication | C++11 |
| [48-event-sourcing](48-event-sourcing/) | Stores state changes as events | C++11 |

### 💎 Modern C++ Patterns (10)
Patterns leveraging modern C++ features.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [40-raii](40-raii/) | Resource Acquisition Is Initialization | C++11 |
| [41-type-erasure](41-type-erasure/) | Hides concrete types behind interfaces | C++11 |
| [42-crtp](42-crtp/) | Curiously Recurring Template Pattern | C++11 |
| [43-policy-based-design](43-policy-based-design/) | Compile-time strategy pattern | C++11 |
| [45-pimpl](45-pimpl/) | Pointer to Implementation idiom | C++11 |
| [54-monad](54-monad/) | Functional programming pattern | C++14 |
| [55-function-composition](55-function-composition/) | Combines simple functions | C++14 |
| [56-consteval](56-consteval/) | Compile-time computation | C++20 |
| [57-concepts](57-concepts/) | Template constraints | C++20 |
| [58-coroutines](58-coroutines/) | Cooperative multitasking | C++20 |

### 🔧 Additional Patterns (7)
Other useful patterns and idioms.

| Pattern | Description | C++ Version |
|---------|-------------|-------------|
| [24-null-object](24-null-object/) | Provides default behavior for null references | C++11 |
| [25-object-pool](25-object-pool/) | Reuses expensive objects | C++11 |
| [44-monostate](44-monostate/) | Alternative to singleton | C++11 |
| [49-saga](49-saga/) | Manages long-running transactions | C++11 |
| [51-copy-on-write](51-copy-on-write/) | Delays copying until modification | C++11 |
| [53-resource-pool](53-resource-pool/) | Manages reusable resources | C++11 |

## 📖 Learning Path

### Beginner
Start with these fundamental patterns:
1. **Singleton** - Basic creational pattern
2. **Factory Method** - Object creation
3. **Observer** - Event handling
4. **Strategy** - Algorithm selection
5. **RAII** - C++ resource management

### Intermediate
Progress to more complex patterns:
1. **Decorator** - Dynamic behavior
2. **Composite** - Tree structures
3. **Command** - Undo/redo operations
4. **Template Method** - Algorithm frameworks
5. **CRTP** - Static polymorphism

### Advanced
Master sophisticated patterns:
1. **Visitor** - Double dispatch
2. **Type Erasure** - Runtime polymorphism
3. **Coroutines** - Asynchronous programming
4. **Policy-Based Design** - Compile-time configuration
5. **Concepts** - Template constraints

## 🛠️ Development Environment

### Recommended IDEs
- **Visual Studio Code** with C++ extension
- **CLion** (cross-platform)
- **Visual Studio** (Windows)
- **Xcode** (macOS)

### Compiler Flags

#### GCC/Clang
```bash
# Basic compilation
g++ -std=c++17 -o program source.cpp

# Development build with warnings
g++ -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -o program source.cpp

# Release build with optimizations
g++ -std=c++17 -O3 -DNDEBUG -march=native -o program source.cpp

# Debug build with sanitizers
g++ -std=c++17 -g -fsanitize=address,undefined -fno-omit-frame-pointer -o program source.cpp

# Thread-safe build
g++ -std=c++17 -pthread -D_REENTRANT -o program source.cpp

# Static analysis
g++ -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wnull-dereference -Wdouble-promotion -Wformat=2 -o program source.cpp

# Link-time optimization
g++ -std=c++17 -O3 -flto -o program source.cpp

# Profile-guided optimization (PGO)
g++ -std=c++17 -fprofile-generate -o program source.cpp
./program  # Run with typical workload
g++ -std=c++17 -fprofile-use -O3 -o program source.cpp
```

#### MSVC (Windows)
```batch
# Basic compilation
cl /std:c++17 /EHsc source.cpp

# Development build
cl /std:c++17 /EHsc /W4 /Od /Zi /MDd /D_DEBUG source.cpp

# Release build
cl /std:c++17 /EHsc /O2 /MD /DNDEBUG source.cpp

# With additional warnings
cl /std:c++17 /EHsc /W4 /permissive- /analyze source.cpp

# Static linking
cl /std:c++17 /EHsc /MT source.cpp

# Enable all optimizations
cl /std:c++17 /EHsc /Ox /GL /Gy source.cpp
```

#### Common Warning Flags Explained
- `-Wall`: Enable most warning messages
- `-Wextra`: Enable extra warning messages
- `-Wpedantic`: Ensure ISO C++ compliance
- `-Werror`: Treat warnings as errors
- `-Wshadow`: Warn about variable shadowing
- `-Wnon-virtual-dtor`: Warn about missing virtual destructors
- `-Wold-style-cast`: Warn about C-style casts
- `-Wcast-align`: Warn about pointer cast alignment issues
- `-Wunused`: Warn about unused variables/functions
- `-Woverloaded-virtual`: Warn about hidden virtual functions

#### Optimization Levels
- `-O0`: No optimization (fastest compilation)
- `-O1`: Basic optimization
- `-O2`: Recommended optimization level
- `-O3`: Aggressive optimization
- `-Os`: Optimize for size
- `-Ofast`: `-O3` with fast math

#### C++ Standard Selection
- `-std=c++11`: C++11 standard
- `-std=c++14`: C++14 standard
- `-std=c++17`: C++17 standard
- `-std=c++20`: C++20 standard
- `-std=c++23`: C++23 standard (experimental)

## 📚 Resources

### Books
- "Design Patterns: Elements of Reusable Object-Oriented Software" - Gang of Four
- "Modern C++ Design" - Andrei Alexandrescu
- "C++ Concurrency in Action" - Anthony Williams

### Online Resources
- [cppreference.com](https://en.cppreference.com/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Refactoring Guru](https://refactoring.guru/design-patterns)

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Follow the existing code style
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Gang of Four for the original design patterns
- The C++ community for modern idioms and patterns
- Contributors and reviewers

---

**Note**: This is an educational resource. While the patterns are production-ready, always consider your specific use case and requirements when applying them in real projects.