// Minimal Repository Pattern Implementation
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <algorithm>
#include <functional>
#include <sstream>
#include <chrono>
#include <iomanip>

// Domain Entity
class User {
private:
    int id_;
    std::string username_;
    std::string email_;
    std::chrono::system_clock::time_point created_at_;
    bool active_;
    
public:
    User() : id_(0), active_(true), created_at_(std::chrono::system_clock::now()) {}
    
    User(int id, const std::string& username, const std::string& email)
        : id_(id), username_(username), email_(email), 
          active_(true), created_at_(std::chrono::system_clock::now()) {}
    
    // Getters
    int getId() const { return id_; }
    std::string getUsername() const { return username_; }
    std::string getEmail() const { return email_; }
    bool isActive() const { return active_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return created_at_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setActive(bool active) { active_ = active; }
    
    std::string toString() const {
        std::stringstream ss;
        auto time_t = std::chrono::system_clock::to_time_t(created_at_);
        ss << "User{id=" << id_ 
           << ", username='" << username_ 
           << "', email='" << email_
           << "', active=" << (active_ ? "true" : "false")
           << ", created=" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
           << "}";
        return ss.str();
    }
};

// Specification pattern for queries
template<typename T>
class Specification {
public:
    virtual ~Specification() = default;
    virtual bool isSatisfiedBy(const T& entity) const = 0;
    
    // Composite specifications
    std::shared_ptr<Specification<T>> and_(std::shared_ptr<Specification<T>> other) {
        return std::make_shared<AndSpecification<T>>(
            std::shared_ptr<Specification<T>>(this), other);
    }
    
    std::shared_ptr<Specification<T>> or_(std::shared_ptr<Specification<T>> other) {
        return std::make_shared<OrSpecification<T>>(
            std::shared_ptr<Specification<T>>(this), other);
    }
    
    std::shared_ptr<Specification<T>> not_() {
        return std::make_shared<NotSpecification<T>>(
            std::shared_ptr<Specification<T>>(this));
    }
};

template<typename T>
class AndSpecification : public Specification<T> {
private:
    std::shared_ptr<Specification<T>> left_;
    std::shared_ptr<Specification<T>> right_;
    
public:
    AndSpecification(std::shared_ptr<Specification<T>> left, 
                    std::shared_ptr<Specification<T>> right)
        : left_(left), right_(right) {}
    
    bool isSatisfiedBy(const T& entity) const override {
        return left_->isSatisfiedBy(entity) && right_->isSatisfiedBy(entity);
    }
};

template<typename T>
class OrSpecification : public Specification<T> {
private:
    std::shared_ptr<Specification<T>> left_;
    std::shared_ptr<Specification<T>> right_;
    
public:
    OrSpecification(std::shared_ptr<Specification<T>> left, 
                   std::shared_ptr<Specification<T>> right)
        : left_(left), right_(right) {}
    
    bool isSatisfiedBy(const T& entity) const override {
        return left_->isSatisfiedBy(entity) || right_->isSatisfiedBy(entity);
    }
};

template<typename T>
class NotSpecification : public Specification<T> {
private:
    std::shared_ptr<Specification<T>> spec_;
    
public:
    explicit NotSpecification(std::shared_ptr<Specification<T>> spec)
        : spec_(spec) {}
    
    bool isSatisfiedBy(const T& entity) const override {
        return !spec_->isSatisfiedBy(entity);
    }
};

// Concrete specifications for User
class ActiveUserSpecification : public Specification<User> {
public:
    bool isSatisfiedBy(const User& user) const override {
        return user.isActive();
    }
};

class UserByUsernameSpecification : public Specification<User> {
private:
    std::string username_;
    
public:
    explicit UserByUsernameSpecification(const std::string& username)
        : username_(username) {}
    
    bool isSatisfiedBy(const User& user) const override {
        return user.getUsername() == username_;
    }
};

class UserByEmailDomainSpecification : public Specification<User> {
private:
    std::string domain_;
    
public:
    explicit UserByEmailDomainSpecification(const std::string& domain)
        : domain_(domain) {}
    
    bool isSatisfiedBy(const User& user) const override {
        size_t atPos = user.getEmail().find('@');
        if (atPos != std::string::npos) {
            return user.getEmail().substr(atPos + 1) == domain_;
        }
        return false;
    }
};

// Repository Interface
template<typename T, typename Id>
class IRepository {
public:
    virtual ~IRepository() = default;
    
    // CRUD operations
    virtual void add(const T& entity) = 0;
    virtual void update(const T& entity) = 0;
    virtual void remove(Id id) = 0;
    virtual std::optional<T> findById(Id id) const = 0;
    virtual std::vector<T> findAll() const = 0;
    
    // Query operations
    virtual std::vector<T> findBySpecification(const Specification<T>& spec) const = 0;
    virtual size_t count() const = 0;
    virtual bool exists(Id id) const = 0;
};

// User Repository Interface
class IUserRepository : public IRepository<User, int> {
public:
    virtual std::optional<User> findByUsername(const std::string& username) const = 0;
    virtual std::optional<User> findByEmail(const std::string& email) const = 0;
    virtual std::vector<User> findActiveUsers() const = 0;
};

// In-Memory Repository Implementation
class InMemoryUserRepository : public IUserRepository {
private:
    mutable std::unordered_map<int, User> users_;
    mutable int nextId_ = 1;
    
public:
    void add(const User& user) override {
        User newUser = user;
        if (newUser.getId() == 0) {
            newUser.setId(nextId_++);
        }
        users_[newUser.getId()] = newUser;
        std::cout << "Added: " << newUser.toString() << "\n";
    }
    
    void update(const User& user) override {
        if (users_.find(user.getId()) != users_.end()) {
            users_[user.getId()] = user;
            std::cout << "Updated: " << user.toString() << "\n";
        } else {
            throw std::runtime_error("User not found for update");
        }
    }
    
    void remove(int id) override {
        auto it = users_.find(id);
        if (it != users_.end()) {
            std::cout << "Removed: " << it->second.toString() << "\n";
            users_.erase(it);
        }
    }
    
    std::optional<User> findById(int id) const override {
        auto it = users_.find(id);
        if (it != users_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::vector<User> findAll() const override {
        std::vector<User> result;
        for (const auto& [id, user] : users_) {
            result.push_back(user);
        }
        return result;
    }
    
    std::vector<User> findBySpecification(const Specification<User>& spec) const override {
        std::vector<User> result;
        for (const auto& [id, user] : users_) {
            if (spec.isSatisfiedBy(user)) {
                result.push_back(user);
            }
        }
        return result;
    }
    
    size_t count() const override {
        return users_.size();
    }
    
    bool exists(int id) const override {
        return users_.find(id) != users_.end();
    }
    
    std::optional<User> findByUsername(const std::string& username) const override {
        auto it = std::find_if(users_.begin(), users_.end(),
            [&username](const auto& pair) {
                return pair.second.getUsername() == username;
            });
        
        if (it != users_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::optional<User> findByEmail(const std::string& email) const override {
        auto it = std::find_if(users_.begin(), users_.end(),
            [&email](const auto& pair) {
                return pair.second.getEmail() == email;
            });
        
        if (it != users_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    std::vector<User> findActiveUsers() const override {
        ActiveUserSpecification spec;
        return findBySpecification(spec);
    }
};

// Generic Repository with caching
template<typename T, typename Id>
class CachedRepository : public IRepository<T, Id> {
private:
    std::unique_ptr<IRepository<T, Id>> repository_;
    mutable std::unordered_map<Id, T> cache_;
    mutable std::vector<T> allCache_;
    mutable bool allCacheValid_ = false;
    
public:
    explicit CachedRepository(std::unique_ptr<IRepository<T, Id>> repository)
        : repository_(std::move(repository)) {}
    
    void add(const T& entity) override {
        repository_->add(entity);
        invalidateCache();
    }
    
    void update(const T& entity) override {
        repository_->update(entity);
        invalidateCache();
    }
    
    void remove(Id id) override {
        repository_->remove(id);
        cache_.erase(id);
        allCacheValid_ = false;
    }
    
    std::optional<T> findById(Id id) const override {
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            std::cout << "Cache hit for id: " << id << "\n";
            return it->second;
        }
        
        std::cout << "Cache miss for id: " << id << "\n";
        auto result = repository_->findById(id);
        if (result.has_value()) {
            cache_[id] = result.value();
        }
        return result;
    }
    
    std::vector<T> findAll() const override {
        if (allCacheValid_) {
            std::cout << "Returning cached all results\n";
            return allCache_;
        }
        
        std::cout << "Cache miss for findAll\n";
        allCache_ = repository_->findAll();
        allCacheValid_ = true;
        return allCache_;
    }
    
    std::vector<T> findBySpecification(const Specification<T>& spec) const override {
        return repository_->findBySpecification(spec);
    }
    
    size_t count() const override {
        return repository_->count();
    }
    
    bool exists(Id id) const override {
        return cache_.find(id) != cache_.end() || repository_->exists(id);
    }
    
private:
    void invalidateCache() {
        cache_.clear();
        allCacheValid_ = false;
    }
};

// Product entity for another example
class Product {
private:
    int id_;
    std::string name_;
    double price_;
    int stock_;
    
public:
    Product(int id, const std::string& name, double price, int stock)
        : id_(id), name_(name), price_(price), stock_(stock) {}
    
    int getId() const { return id_; }
    std::string getName() const { return name_; }
    double getPrice() const { return price_; }
    int getStock() const { return stock_; }
    
    void setStock(int stock) { stock_ = stock; }
    
    std::string toString() const {
        std::stringstream ss;
        ss << "Product{id=" << id_
           << ", name='" << name_
           << "', price=$" << std::fixed << std::setprecision(2) << price_
           << ", stock=" << stock_ << "}";
        return ss.str();
    }
};

// Product specifications
class InStockSpecification : public Specification<Product> {
public:
    bool isSatisfiedBy(const Product& product) const override {
        return product.getStock() > 0;
    }
};

class PriceRangeSpecification : public Specification<Product> {
private:
    double minPrice_;
    double maxPrice_;
    
public:
    PriceRangeSpecification(double minPrice, double maxPrice)
        : minPrice_(minPrice), maxPrice_(maxPrice) {}
    
    bool isSatisfiedBy(const Product& product) const override {
        return product.getPrice() >= minPrice_ && product.getPrice() <= maxPrice_;
    }
};

// Service layer using repository
class UserService {
private:
    std::shared_ptr<IUserRepository> repository_;
    
public:
    explicit UserService(std::shared_ptr<IUserRepository> repository)
        : repository_(repository) {}
    
    void registerUser(const std::string& username, const std::string& email) {
        // Check if username already exists
        if (repository_->findByUsername(username).has_value()) {
            throw std::runtime_error("Username already exists");
        }
        
        // Check if email already exists
        if (repository_->findByEmail(email).has_value()) {
            throw std::runtime_error("Email already exists");
        }
        
        // Create and save new user
        User newUser(0, username, email);
        repository_->add(newUser);
    }
    
    void deactivateUser(int userId) {
        auto userOpt = repository_->findById(userId);
        if (!userOpt.has_value()) {
            throw std::runtime_error("User not found");
        }
        
        User user = userOpt.value();
        user.setActive(false);
        repository_->update(user);
    }
    
    std::vector<User> getActiveUsersFromDomain(const std::string& domain) {
        auto activeSpec = std::make_shared<ActiveUserSpecification>();
        auto domainSpec = std::make_shared<UserByEmailDomainSpecification>(domain);
        
        // Combine specifications
        auto combinedSpec = AndSpecification<User>(activeSpec, domainSpec);
        
        return repository_->findBySpecification(combinedSpec);
    }
};

int main() {
    std::cout << "=== Repository Pattern Demo ===\n\n";
    
    // Basic repository usage
    std::cout << "=== Basic Repository Usage ===\n";
    auto userRepo = std::make_shared<InMemoryUserRepository>();
    
    // Add users
    userRepo->add(User(0, "alice", "alice@example.com"));
    userRepo->add(User(0, "bob", "bob@example.com"));
    userRepo->add(User(0, "charlie", "charlie@test.com"));
    
    std::cout << "\nTotal users: " << userRepo->count() << "\n";
    
    // Find by ID
    std::cout << "\n=== Find by ID ===\n";
    auto user = userRepo->findById(2);
    if (user.has_value()) {
        std::cout << "Found: " << user.value().toString() << "\n";
    }
    
    // Find by username
    std::cout << "\n=== Find by Username ===\n";
    auto alice = userRepo->findByUsername("alice");
    if (alice.has_value()) {
        std::cout << "Found: " << alice.value().toString() << "\n";
    }
    
    // Update user
    std::cout << "\n=== Update User ===\n";
    if (alice.has_value()) {
        User updatedAlice = alice.value();
        updatedAlice.setEmail("alice@newdomain.com");
        userRepo->update(updatedAlice);
    }
    
    // Specification pattern
    std::cout << "\n=== Specification Pattern ===\n";
    ActiveUserSpecification activeSpec;
    auto activeUsers = userRepo->findBySpecification(activeSpec);
    std::cout << "Active users: " << activeUsers.size() << "\n";
    
    UserByEmailDomainSpecification domainSpec("example.com");
    auto exampleUsers = userRepo->findBySpecification(domainSpec);
    std::cout << "Users from example.com: " << exampleUsers.size() << "\n";
    
    // Service layer
    std::cout << "\n=== Service Layer ===\n";
    UserService userService(userRepo);
    
    try {
        userService.registerUser("david", "david@example.com");
        userService.registerUser("alice", "alice2@example.com"); // Should fail
    } catch (const std::exception& e) {
        std::cout << "Registration failed: " << e.what() << "\n";
    }
    
    // Deactivate user
    userService.deactivateUser(1);
    
    // Get active users from domain
    auto activeExampleUsers = userService.getActiveUsersFromDomain("example.com");
    std::cout << "\nActive users from example.com:\n";
    for (const auto& user : activeExampleUsers) {
        std::cout << "  " << user.toString() << "\n";
    }
    
    // Cached repository
    std::cout << "\n=== Cached Repository ===\n";
    auto baseRepo = std::make_unique<InMemoryUserRepository>();
    baseRepo->add(User(0, "test1", "test1@example.com"));
    baseRepo->add(User(0, "test2", "test2@example.com"));
    
    CachedRepository<User, int> cachedRepo(std::move(baseRepo));
    
    // First access - cache miss
    cachedRepo.findById(1);
    // Second access - cache hit
    cachedRepo.findById(1);
    
    // Find all - cache miss
    cachedRepo.findAll();
    // Find all again - cache hit
    cachedRepo.findAll();
    
    return 0;
}