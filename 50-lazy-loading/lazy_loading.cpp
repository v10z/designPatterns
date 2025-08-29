// Minimal Lazy Loading Pattern Implementation
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <mutex>

// Example 1: Basic Lazy Initialization
namespace BasicLazyLoading {
    // Lazy-loaded expensive resource
    class ExpensiveResource {
    private:
        std::string data_;
        
    public:
        explicit ExpensiveResource(const std::string& id) {
            std::cout << "Creating expensive resource: " << id << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
            data_ = "Loaded data for " + id;
        }
        
        const std::string& getData() const {
            return data_;
        }
    };
    
    // Lazy initialization wrapper
    template<typename T>
    class Lazy {
    private:
        mutable std::optional<T> value_;
        std::function<T()> factory_;
        mutable std::mutex mutex_;
        
    public:
        explicit Lazy(std::function<T()> factory) : factory_(std::move(factory)) {}
        
        const T& get() const {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!value_) {
                std::cout << "Lazy evaluation triggered\n";
                value_ = factory_();
            }
            return *value_;
        }
        
        bool isInitialized() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return value_.has_value();
        }
        
        void reset() {
            std::lock_guard<std::mutex> lock(mutex_);
            value_.reset();
        }
    };
    
    // Usage example
    class ServiceWithLazyResource {
    private:
        std::string id_;
        Lazy<ExpensiveResource> resource_;
        
    public:
        explicit ServiceWithLazyResource(const std::string& id)
            : id_(id), resource_([id]() { return ExpensiveResource(id); }) {}
        
        const std::string& getResourceData() {
            return resource_.get().getData();
        }
        
        bool isResourceLoaded() const {
            return resource_.isInitialized();
        }
    };
}

// Example 2: Lazy Collection Loading
namespace LazyCollection {
    // Simulate database record
    struct User {
        int id;
        std::string name;
        std::string email;
        
        User(int id, const std::string& name, const std::string& email)
            : id(id), name(name), email(email) {}
    };
    
    struct Order {
        int id;
        int userId;
        double amount;
        std::string product;
        
        Order(int id, int userId, double amount, const std::string& product)
            : id(id), userId(userId), amount(amount), product(product) {}
    };
    
    // Simulate database operations
    class Database {
    public:
        static std::vector<Order> loadOrdersForUser(int userId) {
            std::cout << "Loading orders for user " << userId << " from database...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Simulate DB query
            
            // Return mock orders
            return {
                Order(1, userId, 99.99, "Laptop"),
                Order(2, userId, 29.99, "Mouse"),
                Order(3, userId, 159.99, "Monitor")
            };
        }
        
        static User loadUser(int userId) {
            std::cout << "Loading user " << userId << " from database...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return User(userId, "User" + std::to_string(userId), 
                       "user" + std::to_string(userId) + "@example.com");
        }
    };
    
    // Lazy-loaded collection
    template<typename T>
    class LazyCollection {
    private:
        mutable std::optional<std::vector<T>> items_;
        std::function<std::vector<T>()> loader_;
        mutable std::mutex mutex_;
        
    public:
        explicit LazyCollection(std::function<std::vector<T>()> loader)
            : loader_(std::move(loader)) {}
        
        const std::vector<T>& getItems() const {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!items_) {
                std::cout << "Lazy loading collection...\n";
                items_ = loader_();
            }
            return *items_;
        }
        
        size_t size() const {
            return getItems().size();
        }
        
        bool empty() const {
            return getItems().empty();
        }
        
        auto begin() const {
            return getItems().begin();
        }
        
        auto end() const {
            return getItems().end();
        }
        
        bool isLoaded() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return items_.has_value();
        }
    };
    
    // User entity with lazy-loaded orders
    class UserEntity {
    private:
        User user_;
        LazyCollection<Order> orders_;
        
    public:
        explicit UserEntity(int userId)
            : user_(Database::loadUser(userId)),
              orders_([userId]() { return Database::loadOrdersForUser(userId); }) {}
        
        const User& getUser() const {
            return user_;
        }
        
        const LazyCollection<Order>& getOrders() const {
            return orders_;
        }
        
        double getTotalSpent() const {
            double total = 0.0;
            for (const auto& order : orders_.getItems()) {
                total += order.amount;
            }
            return total;
        }
    };
}

// Example 3: Lazy Property Loading
namespace LazyProperty {
    // Lazy property implementation
    template<typename T>
    class LazyProperty {
    private:
        mutable std::optional<T> value_;
        std::function<T()> getter_;
        mutable std::mutex mutex_;
        std::string name_;
        
    public:
        LazyProperty(const std::string& name, std::function<T()> getter)
            : name_(name), getter_(std::move(getter)) {}
        
        const T& operator()() const {
            return getValue();
        }
        
        const T& getValue() const {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!value_) {
                std::cout << "Loading lazy property: " << name_ << "\n";
                value_ = getter_();
            }
            return *value_;
        }
        
        bool isLoaded() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return value_.has_value();
        }
        
        void invalidate() {
            std::lock_guard<std::mutex> lock(mutex_);
            value_.reset();
        }
    };
    
    // Document with lazy-loaded properties
    class Document {
    private:
        std::string filename_;
        LazyProperty<std::string> content_;
        LazyProperty<size_t> wordCount_;
        LazyProperty<std::vector<std::string>> keywords_;
        
        std::string loadContent() const {
            std::cout << "Reading file: " << filename_ << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            return "This is the content of " + filename_ + 
                   ". It contains important information about design patterns.";
        }
        
        size_t calculateWordCount() const {
            std::cout << "Calculating word count...\n";
            const auto& content = content_.getValue();
            size_t count = 1; // Simple word counting
            for (char c : content) {
                if (c == ' ') count++;
            }
            return count;
        }
        
        std::vector<std::string> extractKeywords() const {
            std::cout << "Extracting keywords...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return {"design", "patterns", "lazy", "loading"};
        }
        
    public:
        explicit Document(const std::string& filename)
            : filename_(filename),
              content_("content", [this]() { return loadContent(); }),
              wordCount_("wordCount", [this]() { return calculateWordCount(); }),
              keywords_("keywords", [this]() { return extractKeywords(); }) {}
        
        const std::string& getContent() const {
            return content_();
        }
        
        size_t getWordCount() const {
            return wordCount_();
        }
        
        const std::vector<std::string>& getKeywords() const {
            return keywords_();
        }
        
        void printLoadedProperties() const {
            std::cout << "Loaded properties for " << filename_ << ":\n";
            std::cout << "  Content: " << (content_.isLoaded() ? "Yes" : "No") << "\n";
            std::cout << "  Word Count: " << (wordCount_.isLoaded() ? "Yes" : "No") << "\n";
            std::cout << "  Keywords: " << (keywords_.isLoaded() ? "Yes" : "No") << "\n";
        }
    };
}

// Example 4: Lazy Singleton
namespace LazySingleton {
    class DatabaseConnection {
    private:
        std::string connectionString_;
        
        explicit DatabaseConnection(const std::string& connStr) 
            : connectionString_(connStr) {
            std::cout << "Establishing database connection: " << connStr << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        
    public:
        static DatabaseConnection& getInstance() {
            static DatabaseConnection instance("postgresql://localhost:5432/mydb");
            return instance;
        }
        
        void executeQuery(const std::string& query) {
            std::cout << "Executing query: " << query << "\n";
        }
        
        const std::string& getConnectionString() const {
            return connectionString_;
        }
        
        // Delete copy constructor and assignment
        DatabaseConnection(const DatabaseConnection&) = delete;
        DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    };
    
    // Service that uses lazy singleton
    class UserService {
    public:
        void createUser(const std::string& name) {
            // Database connection is created only when first accessed
            auto& db = DatabaseConnection::getInstance();
            db.executeQuery("INSERT INTO users (name) VALUES ('" + name + "')");
        }
        
        void deleteUser(int id) {
            auto& db = DatabaseConnection::getInstance();
            db.executeQuery("DELETE FROM users WHERE id = " + std::to_string(id));
        }
    };
}

// Example 5: Virtual Proxy with Lazy Loading
namespace VirtualProxy {
    // Expensive image resource
    class Image {
    private:
        std::string filename_;
        std::vector<uint8_t> imageData_;
        int width_, height_;
        
    public:
        explicit Image(const std::string& filename) : filename_(filename) {
            std::cout << "Loading image: " << filename << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            // Simulate loading image data
            width_ = 800;
            height_ = 600;
            imageData_.resize(width_ * height_ * 3); // RGB
            std::fill(imageData_.begin(), imageData_.end(), 128); // Gray
            
            std::cout << "Image loaded: " << width_ << "x" << height_ << "\n";
        }
        
        void display() const {
            std::cout << "Displaying image: " << filename_ 
                      << " (" << width_ << "x" << height_ << ")\n";
        }
        
        int getWidth() const { return width_; }
        int getHeight() const { return height_; }
        const std::string& getFilename() const { return filename_; }
    };
    
    // Virtual proxy for lazy image loading
    class ImageProxy {
    private:
        std::string filename_;
        mutable std::unique_ptr<Image> realImage_;
        mutable std::mutex mutex_;
        
        const Image& getRealImage() const {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!realImage_) {
                std::cout << "Proxy: Loading real image on demand\n";
                realImage_ = std::make_unique<Image>(filename_);
            }
            return *realImage_;
        }
        
    public:
        explicit ImageProxy(const std::string& filename) : filename_(filename) {
            std::cout << "ImageProxy created for: " << filename << "\n";
        }
        
        void display() const {
            getRealImage().display();
        }
        
        int getWidth() const {
            return getRealImage().getWidth();
        }
        
        int getHeight() const {
            return getRealImage().getHeight();
        }
        
        const std::string& getFilename() const {
            return filename_;
        }
        
        bool isLoaded() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return realImage_ != nullptr;
        }
    };
    
    // Gallery with lazy-loaded images
    class ImageGallery {
    private:
        std::vector<std::unique_ptr<ImageProxy>> images_;
        
    public:
        void addImage(const std::string& filename) {
            images_.push_back(std::make_unique<ImageProxy>(filename));
        }
        
        void displayImage(size_t index) {
            if (index < images_.size()) {
                images_[index]->display();
            }
        }
        
        void showGalleryInfo() {
            std::cout << "\nGallery contains " << images_.size() << " images:\n";
            for (size_t i = 0; i < images_.size(); ++i) {
                std::cout << "  [" << i << "] " << images_[i]->getFilename()
                          << " (loaded: " << (images_[i]->isLoaded() ? "Yes" : "No") 
                          << ")\n";
            }
        }
    };
}

// Demo functions
void demonstrateBasicLazyLoading() {
    using namespace BasicLazyLoading;
    
    std::cout << "=== Basic Lazy Loading ===\n";
    
    std::cout << "\nCreating service (resource not loaded yet):\n";
    ServiceWithLazyResource service("DB-001");
    
    std::cout << "Resource loaded: " << service.isResourceLoaded() << "\n";
    
    std::cout << "\nFirst access (triggers loading):\n";
    std::cout << "Data: " << service.getResourceData() << "\n";
    
    std::cout << "Resource loaded: " << service.isResourceLoaded() << "\n";
    
    std::cout << "\nSecond access (uses cached value):\n";
    std::cout << "Data: " << service.getResourceData() << "\n";
}

void demonstrateLazyCollection() {
    using namespace LazyCollection;
    
    std::cout << "\n=== Lazy Collection Loading ===\n";
    
    std::cout << "\nCreating user entity (orders not loaded yet):\n";
    UserEntity user(123);
    
    std::cout << "User: " << user.getUser().name << " (" << user.getUser().email << ")\n";
    std::cout << "Orders loaded: " << user.getOrders().isLoaded() << "\n";
    
    std::cout << "\nAccessing orders (triggers loading):\n";
    std::cout << "Total orders: " << user.getOrders().size() << "\n";
    
    std::cout << "Orders loaded: " << user.getOrders().isLoaded() << "\n";
    
    std::cout << "\nCalculating total spent (uses cached orders):\n";
    std::cout << "Total spent: $" << user.getTotalSpent() << "\n";
    
    std::cout << "\nOrder details:\n";
    for (const auto& order : user.getOrders()) {
        std::cout << "  Order " << order.id << ": " << order.product 
                  << " - $" << order.amount << "\n";
    }
}

void demonstrateLazyProperty() {
    using namespace LazyProperty;
    
    std::cout << "\n=== Lazy Property Loading ===\n";
    
    std::cout << "\nCreating document (properties not loaded yet):\n";
    Document doc("design_patterns.txt");
    doc.printLoadedProperties();
    
    std::cout << "\nAccessing content (triggers loading):\n";
    const auto& content = doc.getContent();
    std::cout << "Content length: " << content.length() << " characters\n";
    doc.printLoadedProperties();
    
    std::cout << "\nAccessing word count (triggers loading):\n";
    std::cout << "Word count: " << doc.getWordCount() << " words\n";
    doc.printLoadedProperties();
    
    std::cout << "\nAccessing keywords (triggers loading):\n";
    const auto& keywords = doc.getKeywords();
    std::cout << "Keywords: ";
    for (const auto& keyword : keywords) {
        std::cout << keyword << " ";
    }
    std::cout << "\n";
    doc.printLoadedProperties();
}

void demonstrateLazySingleton() {
    using namespace LazySingleton;
    
    std::cout << "\n=== Lazy Singleton ===\n";
    
    std::cout << "\nCreating user service (DB connection not created yet):\n";
    UserService userService;
    
    std::cout << "\nFirst database operation (triggers connection creation):\n";
    userService.createUser("Alice");
    
    std::cout << "\nSecond database operation (reuses connection):\n";
    userService.deleteUser(1);
    
    std::cout << "\nThird database operation (reuses connection):\n";
    userService.createUser("Bob");
}

void demonstrateVirtualProxy() {
    using namespace VirtualProxy;
    
    std::cout << "\n=== Virtual Proxy with Lazy Loading ===\n";
    
    std::cout << "\nCreating image gallery (images not loaded yet):\n";
    ImageGallery gallery;
    gallery.addImage("photo1.jpg");
    gallery.addImage("photo2.jpg");
    gallery.addImage("photo3.jpg");
    
    gallery.showGalleryInfo();
    
    std::cout << "\nDisplaying first image (triggers loading):\n";
    gallery.displayImage(0);
    
    gallery.showGalleryInfo();
    
    std::cout << "\nDisplaying third image (triggers loading):\n";
    gallery.displayImage(2);
    
    gallery.showGalleryInfo();
    
    std::cout << "\nDisplaying first image again (uses cached):\n";
    gallery.displayImage(0);
}

int main() {
    std::cout << "=== Lazy Loading Pattern Demo ===\n\n";
    
    demonstrateBasicLazyLoading();
    demonstrateLazyCollection();
    demonstrateLazyProperty();
    demonstrateLazySingleton();
    demonstrateVirtualProxy();
    
    std::cout << "\n=== Lazy Loading Benefits ===\n";
    std::cout << "1. Defers expensive operations\n";
    std::cout << "2. Improves initial load times\n";
    std::cout << "3. Saves memory and resources\n";
    std::cout << "4. Load-on-demand behavior\n";
    std::cout << "5. Better user experience\n";
    
    return 0;
}