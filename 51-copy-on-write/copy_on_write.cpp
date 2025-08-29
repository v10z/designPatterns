// Minimal Copy-on-Write Pattern Implementation
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <cassert>

// Example 1: Basic Copy-on-Write String
namespace BasicCopyOnWrite {
    class COWString {
    private:
        // Shared data structure
        struct SharedData {
            std::string data;
            std::atomic<int> refCount{1};
            
            explicit SharedData(const std::string& str) : data(str) {}
            explicit SharedData(const char* str) : data(str) {}
        };
        
        std::shared_ptr<SharedData> sharedData_;
        
        // Ensure we have a unique copy before modification
        void ensureUnique() {
            if (sharedData_.use_count() > 1) {
                std::cout << "COW: Creating copy for modification\n";
                sharedData_ = std::make_shared<SharedData>(sharedData_->data);
            }
        }
        
    public:
        // Constructors
        COWString() : sharedData_(std::make_shared<SharedData>("")) {
            std::cout << "COW: Creating empty string\n";
        }
        
        explicit COWString(const std::string& str) 
            : sharedData_(std::make_shared<SharedData>(str)) {
            std::cout << "COW: Creating string: \"" << str << "\"\n";
        }
        
        explicit COWString(const char* str) 
            : sharedData_(std::make_shared<SharedData>(str)) {
            std::cout << "COW: Creating string: \"" << str << "\"\n";
        }
        
        // Copy constructor - shallow copy
        COWString(const COWString& other) : sharedData_(other.sharedData_) {
            std::cout << "COW: Shallow copy (ref count: " 
                      << sharedData_.use_count() << ")\n";
        }
        
        // Assignment operator - shallow copy
        COWString& operator=(const COWString& other) {
            if (this != &other) {
                sharedData_ = other.sharedData_;
                std::cout << "COW: Shallow assignment (ref count: " 
                          << sharedData_.use_count() << ")\n";
            }
            return *this;
        }
        
        // Read-only access (no copy needed)
        const std::string& c_str() const {
            return sharedData_->data;
        }
        
        size_t length() const {
            return sharedData_->data.length();
        }
        
        char operator[](size_t index) const {
            return sharedData_->data[index];
        }
        
        // Modifying operations (trigger copy-on-write)
        void append(const std::string& str) {
            ensureUnique();
            sharedData_->data.append(str);
            std::cout << "COW: Appended \"" << str << "\"\n";
        }
        
        void insert(size_t pos, const std::string& str) {
            ensureUnique();
            sharedData_->data.insert(pos, str);
            std::cout << "COW: Inserted \"" << str << "\" at position " << pos << "\n";
        }
        
        void erase(size_t pos, size_t count = 1) {
            ensureUnique();
            sharedData_->data.erase(pos, count);
            std::cout << "COW: Erased " << count << " characters at position " << pos << "\n";
        }
        
        // Non-const reference access (triggers copy)
        char& operator[](size_t index) {
            ensureUnique();
            return sharedData_->data[index];
        }
        
        // Utility methods
        int getRefCount() const {
            return sharedData_.use_count();
        }
        
        void* getDataPtr() const {
            return sharedData_.get();
        }
        
        void print() const {
            std::cout << "String: \"" << sharedData_->data 
                      << "\" (refs: " << getRefCount() 
                      << ", ptr: " << getDataPtr() << ")\n";
        }
    };
}

// Example 2: Copy-on-Write Vector
namespace COWVector {
    template<typename T>
    class COWVector {
    private:
        struct SharedData {
            std::vector<T> data;
            std::atomic<int> refCount{1};
            
            SharedData() = default;
            explicit SharedData(const std::vector<T>& vec) : data(vec) {}
            explicit SharedData(std::vector<T>&& vec) : data(std::move(vec)) {}
        };
        
        std::shared_ptr<SharedData> sharedData_;
        
        void ensureUnique() {
            if (sharedData_.use_count() > 1) {
                std::cout << "COWVector: Creating copy for modification\n";
                sharedData_ = std::make_shared<SharedData>(sharedData_->data);
            }
        }
        
    public:
        // Constructors
        COWVector() : sharedData_(std::make_shared<SharedData>()) {
            std::cout << "COWVector: Creating empty vector\n";
        }
        
        explicit COWVector(const std::vector<T>& vec) 
            : sharedData_(std::make_shared<SharedData>(vec)) {
            std::cout << "COWVector: Creating vector with " << vec.size() << " elements\n";
        }
        
        explicit COWVector(std::initializer_list<T> init) 
            : sharedData_(std::make_shared<SharedData>(std::vector<T>(init))) {
            std::cout << "COWVector: Creating vector with " << init.size() << " elements\n";
        }
        
        // Copy operations (shallow)
        COWVector(const COWVector& other) : sharedData_(other.sharedData_) {
            std::cout << "COWVector: Shallow copy (ref count: " 
                      << sharedData_.use_count() << ")\n";
        }
        
        COWVector& operator=(const COWVector& other) {
            if (this != &other) {
                sharedData_ = other.sharedData_;
                std::cout << "COWVector: Shallow assignment (ref count: " 
                          << sharedData_.use_count() << ")\n";
            }
            return *this;
        }
        
        // Read-only access
        const T& operator[](size_t index) const {
            return sharedData_->data[index];
        }
        
        const T& at(size_t index) const {
            return sharedData_->data.at(index);
        }
        
        size_t size() const {
            return sharedData_->data.size();
        }
        
        bool empty() const {
            return sharedData_->data.empty();
        }
        
        const T& front() const {
            return sharedData_->data.front();
        }
        
        const T& back() const {
            return sharedData_->data.back();
        }
        
        // Iterator support (read-only)
        auto begin() const {
            return sharedData_->data.begin();
        }
        
        auto end() const {
            return sharedData_->data.end();
        }
        
        // Modifying operations (trigger COW)
        void push_back(const T& value) {
            ensureUnique();
            sharedData_->data.push_back(value);
            std::cout << "COWVector: Pushed back element\n";
        }
        
        void pop_back() {
            ensureUnique();
            sharedData_->data.pop_back();
            std::cout << "COWVector: Popped back element\n";
        }
        
        void insert(size_t pos, const T& value) {
            ensureUnique();
            sharedData_->data.insert(sharedData_->data.begin() + pos, value);
            std::cout << "COWVector: Inserted element at position " << pos << "\n";
        }
        
        void erase(size_t pos) {
            ensureUnique();
            sharedData_->data.erase(sharedData_->data.begin() + pos);
            std::cout << "COWVector: Erased element at position " << pos << "\n";
        }
        
        void clear() {
            ensureUnique();
            sharedData_->data.clear();
            std::cout << "COWVector: Cleared all elements\n";
        }
        
        // Non-const reference access (triggers COW)
        T& operator[](size_t index) {
            ensureUnique();
            return sharedData_->data[index];
        }
        
        T& at(size_t index) {
            ensureUnique();
            return sharedData_->data.at(index);
        }
        
        // Utility methods
        int getRefCount() const {
            return sharedData_.use_count();
        }
        
        void* getDataPtr() const {
            return sharedData_.get();
        }
        
        void print() const {
            std::cout << "Vector[" << size() << "] (refs: " << getRefCount() 
                      << ", ptr: " << getDataPtr() << "): ";
            for (const auto& item : *this) {
                std::cout << item << " ";
            }
            std::cout << "\n";
        }
    };
}

// Example 3: Copy-on-Write Document with Undo
namespace COWDocument {
    class Document {
    private:
        struct DocumentData {
            std::string content;
            std::vector<std::string> history;
            
            explicit DocumentData(const std::string& initialContent = "") 
                : content(initialContent) {
                history.push_back(content);
            }
        };
        
        std::shared_ptr<DocumentData> data_;
        
        void ensureUnique() {
            if (data_.use_count() > 1) {
                std::cout << "Document: Creating copy for modification\n";
                data_ = std::make_shared<DocumentData>(*data_);
            }
        }
        
    public:
        explicit Document(const std::string& initialContent = "") 
            : data_(std::make_shared<DocumentData>(initialContent)) {
            std::cout << "Document: Created with content: \"" << initialContent << "\"\n";
        }
        
        // Copy operations (shallow)
        Document(const Document& other) : data_(other.data_) {
            std::cout << "Document: Shallow copy (ref count: " 
                      << data_.use_count() << ")\n";
        }
        
        Document& operator=(const Document& other) {
            if (this != &other) {
                data_ = other.data_;
                std::cout << "Document: Shallow assignment (ref count: " 
                          << data_.use_count() << ")\n";
            }
            return *this;
        }
        
        // Read-only access
        const std::string& getContent() const {
            return data_->content;
        }
        
        size_t getLength() const {
            return data_->content.length();
        }
        
        const std::vector<std::string>& getHistory() const {
            return data_->history;
        }
        
        // Modifying operations
        void setText(const std::string& newContent) {
            ensureUnique();
            data_->content = newContent;
            data_->history.push_back(newContent);
            std::cout << "Document: Set text to: \"" << newContent << "\"\n";
        }
        
        void append(const std::string& text) {
            ensureUnique();
            data_->content += text;
            data_->history.push_back(data_->content);
            std::cout << "Document: Appended: \"" << text << "\"\n";
        }
        
        void insert(size_t pos, const std::string& text) {
            ensureUnique();
            data_->content.insert(pos, text);
            data_->history.push_back(data_->content);
            std::cout << "Document: Inserted \"" << text << "\" at position " << pos << "\n";
        }
        
        void erase(size_t pos, size_t count) {
            ensureUnique();
            data_->content.erase(pos, count);
            data_->history.push_back(data_->content);
            std::cout << "Document: Erased " << count << " characters at position " << pos << "\n";
        }
        
        bool undo() {
            if (data_->history.size() <= 1) {
                return false;
            }
            
            ensureUnique();
            data_->history.pop_back();
            data_->content = data_->history.back();
            std::cout << "Document: Undo - restored to: \"" << data_->content << "\"\n";
            return true;
        }
        
        // Utility methods
        int getRefCount() const {
            return data_.use_count();
        }
        
        void printInfo() const {
            std::cout << "Document (refs: " << getRefCount() << "):\n";
            std::cout << "  Content: \"" << data_->content << "\"\n";
            std::cout << "  History size: " << data_->history.size() << "\n";
        }
    };
}

// Example 4: Copy-on-Write with Smart Pointers
namespace SmartCOW {
    template<typename T>
    class COWPtr {
    private:
        struct COWData {
            T data;
            std::atomic<int> refCount{1};
            
            template<typename... Args>
            explicit COWData(Args&&... args) : data(std::forward<Args>(args)...) {}
        };
        
        std::shared_ptr<COWData> ptr_;
        
        void ensureUnique() {
            if (ptr_.use_count() > 1) {
                std::cout << "COWPtr: Creating deep copy\n";
                ptr_ = std::make_shared<COWData>(ptr_->data);
            }
        }
        
    public:
        // Constructor
        template<typename... Args>
        explicit COWPtr(Args&&... args) 
            : ptr_(std::make_shared<COWData>(std::forward<Args>(args)...)) {}
        
        // Copy operations
        COWPtr(const COWPtr& other) : ptr_(other.ptr_) {}
        
        COWPtr& operator=(const COWPtr& other) {
            if (this != &other) {
                ptr_ = other.ptr_;
            }
            return *this;
        }
        
        // Read-only access
        const T& operator*() const {
            return ptr_->data;
        }
        
        const T* operator->() const {
            return &ptr_->data;
        }
        
        const T& get() const {
            return ptr_->data;
        }
        
        // Mutable access (triggers COW)
        T& mutable_get() {
            ensureUnique();
            return ptr_->data;
        }
        
        T* mutable_ptr() {
            ensureUnique();
            return &ptr_->data;
        }
        
        // Utility
        int use_count() const {
            return ptr_.use_count();
        }
        
        bool unique() const {
            return ptr_.use_count() == 1;
        }
    };
    
    // Example usage with a complex object
    class ComplexObject {
    private:
        std::string name_;
        std::vector<int> data_;
        
    public:
        explicit ComplexObject(const std::string& name) : name_(name) {
            std::cout << "ComplexObject: Creating " << name_ << "\n";
            data_ = {1, 2, 3, 4, 5};
        }
        
        // Copy constructor
        ComplexObject(const ComplexObject& other) 
            : name_(other.name_ + "_copy"), data_(other.data_) {
            std::cout << "ComplexObject: Deep copying " << other.name_ 
                      << " to " << name_ << "\n";
        }
        
        const std::string& getName() const { return name_; }
        const std::vector<int>& getData() const { return data_; }
        
        void addData(int value) {
            data_.push_back(value);
            std::cout << "ComplexObject: Added " << value << " to " << name_ << "\n";
        }
        
        void setName(const std::string& newName) {
            name_ = newName;
            std::cout << "ComplexObject: Renamed to " << name_ << "\n";
        }
        
        void print() const {
            std::cout << "ComplexObject " << name_ << ": [";
            for (size_t i = 0; i < data_.size(); ++i) {
                std::cout << data_[i];
                if (i < data_.size() - 1) std::cout << ", ";
            }
            std::cout << "]\n";
        }
    };
}

// Demo functions
void demonstrateBasicCOW() {
    using namespace BasicCopyOnWrite;
    
    std::cout << "=== Basic Copy-on-Write String ===\n";
    
    std::cout << "\nCreating original string:\n";
    COWString str1("Hello World");
    str1.print();
    
    std::cout << "\nCreating copy (shallow copy):\n";
    COWString str2 = str1;
    std::cout << "Original: ";
    str1.print();
    std::cout << "Copy: ";
    str2.print();
    
    std::cout << "\nReading from both strings (no copy triggered):\n";
    std::cout << "str1 length: " << str1.length() << "\n";
    std::cout << "str2[0]: " << str2[0] << "\n";
    
    std::cout << "\nModifying copy (triggers COW):\n";
    str2.append(" - Modified");
    std::cout << "Original: ";
    str1.print();
    std::cout << "Modified: ";
    str2.print();
    
    std::cout << "\nFurther modifications (no more copying):\n";
    str2.insert(5, " Beautiful");
    str2.print();
}

void demonstrateCOWVector() {
    std::cout << "\n=== Copy-on-Write Vector ===\n";
    
    std::cout << "\nCreating original vector:\n";
    COWVector::COWVector<int> vec1{1, 2, 3, 4, 5};
    vec1.print();
    
    std::cout << "\nCreating copies (shallow copies):\n";
    COWVector::COWVector<int> vec2 = vec1;
    COWVector::COWVector<int> vec3 = vec1;
    
    std::cout << "All vectors:\n";
    std::cout << "vec1: ";
    vec1.print();
    std::cout << "vec2: ";
    vec2.print();
    std::cout << "vec3: ";
    vec3.print();
    
    std::cout << "\nReading from vectors (no copy):\n";
    std::cout << "vec1.size(): " << vec1.size() << "\n";
    std::cout << "vec2[2]: " << vec2[2] << "\n";
    std::cout << "vec3.back(): " << vec3.back() << "\n";
    
    std::cout << "\nModifying vec2 (triggers COW):\n";
    vec2.push_back(6);
    vec2.push_back(7);
    
    std::cout << "After modification:\n";
    std::cout << "vec1: ";
    vec1.print();
    std::cout << "vec2: ";
    vec2.print();
    std::cout << "vec3: ";
    vec3.print();
    
    std::cout << "\nModifying vec3 (triggers COW):\n";
    vec3[0] = 99; // Non-const access
    vec3.erase(1);
    
    std::cout << "Final state:\n";
    std::cout << "vec1: ";
    vec1.print();
    std::cout << "vec2: ";
    vec2.print();
    std::cout << "vec3: ";
    vec3.print();
}

void demonstrateCOWDocument() {
    using namespace COWDocument;
    
    std::cout << "\n=== Copy-on-Write Document ===\n";
    
    std::cout << "\nCreating original document:\n";
    Document doc1("Initial content");
    doc1.printInfo();
    
    std::cout << "\nCreating document snapshots:\n";
    Document snapshot1 = doc1;
    Document snapshot2 = doc1;
    
    std::cout << "All documents share same data:\n";
    std::cout << "doc1: ";
    doc1.printInfo();
    std::cout << "snapshot1: ";
    snapshot1.printInfo();
    std::cout << "snapshot2: ";
    snapshot2.printInfo();
    
    std::cout << "\nModifying original document:\n";
    doc1.append(" - First edit");
    doc1.append(" - Second edit");
    doc1.printInfo();
    
    std::cout << "\nSnapshots remain unchanged:\n";
    std::cout << "snapshot1: ";
    snapshot1.printInfo();
    std::cout << "snapshot2: ";
    snapshot2.printInfo();
    
    std::cout << "\nUndo operations on doc1:\n";
    doc1.undo();
    doc1.printInfo();
    doc1.undo();
    doc1.printInfo();
    
    std::cout << "\nModifying snapshot1 (triggers COW):\n";
    snapshot1.setText("Snapshot modified");
    snapshot1.printInfo();
}

void demonstrateSmartCOW() {
    using namespace SmartCOW;
    
    std::cout << "\n=== Smart Copy-on-Write Pointer ===\n";
    
    std::cout << "\nCreating COW pointer to complex object:\n";
    COWPtr<ComplexObject> cowPtr1("Object1");
    std::cout << "cowPtr1 use_count: " << cowPtr1.use_count() << "\n";
    cowPtr1->print();
    
    std::cout << "\nCreating copies (shallow):\n";
    COWPtr<ComplexObject> cowPtr2 = cowPtr1;
    COWPtr<ComplexObject> cowPtr3 = cowPtr1;
    
    std::cout << "After copying:\n";
    std::cout << "cowPtr1 use_count: " << cowPtr1.use_count() << "\n";
    std::cout << "cowPtr2 use_count: " << cowPtr2.use_count() << "\n";
    std::cout << "cowPtr3 use_count: " << cowPtr3.use_count() << "\n";
    
    std::cout << "\nRead-only access (no copy):\n";
    std::cout << "cowPtr1 name: " << cowPtr1->getName() << "\n";
    std::cout << "cowPtr2 data size: " << cowPtr2->getData().size() << "\n";
    
    std::cout << "\nModifying through cowPtr2 (triggers COW):\n";
    cowPtr2.mutable_get().addData(42);
    cowPtr2.mutable_get().setName("ModifiedObject");
    
    std::cout << "After modification:\n";
    std::cout << "cowPtr1 use_count: " << cowPtr1.use_count() << "\n";
    std::cout << "cowPtr2 use_count: " << cowPtr2.use_count() << "\n";
    std::cout << "cowPtr3 use_count: " << cowPtr3.use_count() << "\n";
    
    std::cout << "\nFinal state:\n";
    std::cout << "cowPtr1: ";
    cowPtr1->print();
    std::cout << "cowPtr2: ";
    cowPtr2->print();
    std::cout << "cowPtr3: ";
    cowPtr3->print();
}

int main() {
    std::cout << "=== Copy-on-Write Pattern Demo ===\n\n";
    
    demonstrateBasicCOW();
    demonstrateCOWVector();
    demonstrateCOWDocument();
    demonstrateSmartCOW();
    
    std::cout << "\n=== Copy-on-Write Benefits ===\n";
    std::cout << "1. Memory efficiency through sharing\n";
    std::cout << "2. Fast copy operations\n";
    std::cout << "3. Deferred expensive operations\n";
    std::cout << "4. Transparent to users\n";
    std::cout << "5. Thread-safe when implemented correctly\n";
    
    return 0;
}