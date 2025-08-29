// Minimal Data Mapper Pattern Implementation
#include <iostream>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <sstream>
#include <iomanip>
#include <ctime>

// Domain Model - completely independent of persistence
namespace Domain {
    class Address {
    private:
        std::string street_;
        std::string city_;
        std::string zipCode_;
        std::string country_;
        
    public:
        Address() = default;
        
        Address(const std::string& street, const std::string& city,
                const std::string& zipCode, const std::string& country)
            : street_(street), city_(city), zipCode_(zipCode), country_(country) {}
        
        // Business logic
        bool isInternational() const {
            return country_ != "USA";
        }
        
        std::string getFullAddress() const {
            return street_ + ", " + city_ + ", " + zipCode_ + ", " + country_;
        }
        
        // Getters/Setters
        std::string getStreet() const { return street_; }
        void setStreet(const std::string& street) { street_ = street; }
        
        std::string getCity() const { return city_; }
        void setCity(const std::string& city) { city_ = city; }
        
        std::string getZipCode() const { return zipCode_; }
        void setZipCode(const std::string& zipCode) { zipCode_ = zipCode; }
        
        std::string getCountry() const { return country_; }
        void setCountry(const std::string& country) { country_ = country; }
    };
    
    class Customer {
    private:
        int id_;
        std::string firstName_;
        std::string lastName_;
        std::string email_;
        Address shippingAddress_;
        Address billingAddress_;
        double totalPurchases_;
        bool isVip_;
        
    public:
        Customer() : id_(0), totalPurchases_(0), isVip_(false) {}
        
        Customer(int id, const std::string& firstName, const std::string& lastName,
                const std::string& email)
            : id_(id), firstName_(firstName), lastName_(lastName), 
              email_(email), totalPurchases_(0), isVip_(false) {}
        
        // Business logic
        std::string getFullName() const {
            return firstName_ + " " + lastName_;
        }
        
        void recordPurchase(double amount) {
            totalPurchases_ += amount;
            if (totalPurchases_ > 10000) {
                isVip_ = true;
            }
        }
        
        double getDiscount() const {
            return isVip_ ? 0.15 : 0.0;
        }
        
        // Getters/Setters
        int getId() const { return id_; }
        void setId(int id) { id_ = id; }
        
        std::string getFirstName() const { return firstName_; }
        void setFirstName(const std::string& name) { firstName_ = name; }
        
        std::string getLastName() const { return lastName_; }
        void setLastName(const std::string& name) { lastName_ = name; }
        
        std::string getEmail() const { return email_; }
        void setEmail(const std::string& email) { email_ = email; }
        
        const Address& getShippingAddress() const { return shippingAddress_; }
        void setShippingAddress(const Address& address) { shippingAddress_ = address; }
        
        const Address& getBillingAddress() const { return billingAddress_; }
        void setBillingAddress(const Address& address) { billingAddress_ = address; }
        
        double getTotalPurchases() const { return totalPurchases_; }
        void setTotalPurchases(double amount) { totalPurchases_ = amount; }
        
        bool getIsVip() const { return isVip_; }
        void setIsVip(bool vip) { isVip_ = vip; }
    };
    
    class Product {
    private:
        int id_;
        std::string sku_;
        std::string name_;
        std::string description_;
        double price_;
        int stockQuantity_;
        std::string category_;
        
    public:
        Product() : id_(0), price_(0), stockQuantity_(0) {}
        
        Product(int id, const std::string& sku, const std::string& name,
                double price, int stock)
            : id_(id), sku_(sku), name_(name), price_(price), stockQuantity_(stock) {}
        
        // Business logic
        bool isInStock() const {
            return stockQuantity_ > 0;
        }
        
        bool canFulfillOrder(int quantity) const {
            return stockQuantity_ >= quantity;
        }
        
        void reduceStock(int quantity) {
            if (canFulfillOrder(quantity)) {
                stockQuantity_ -= quantity;
            } else {
                throw std::runtime_error("Insufficient stock");
            }
        }
        
        // Getters/Setters
        int getId() const { return id_; }
        void setId(int id) { id_ = id; }
        
        std::string getSku() const { return sku_; }
        void setSku(const std::string& sku) { sku_ = sku; }
        
        std::string getName() const { return name_; }
        void setName(const std::string& name) { name_ = name; }
        
        std::string getDescription() const { return description_; }
        void setDescription(const std::string& desc) { description_ = desc; }
        
        double getPrice() const { return price_; }
        void setPrice(double price) { price_ = price; }
        
        int getStockQuantity() const { return stockQuantity_; }
        void setStockQuantity(int quantity) { stockQuantity_ = quantity; }
        
        std::string getCategory() const { return category_; }
        void setCategory(const std::string& category) { category_ = category; }
    };
}

// Data Transfer Objects (DTOs) - represent database structure
namespace Data {
    struct CustomerRecord {
        int id;
        std::string first_name;
        std::string last_name;
        std::string email;
        std::string shipping_street;
        std::string shipping_city;
        std::string shipping_zip;
        std::string shipping_country;
        std::string billing_street;
        std::string billing_city;
        std::string billing_zip;
        std::string billing_country;
        double total_purchases;
        bool is_vip;
    };
    
    struct ProductRecord {
        int id;
        std::string sku;
        std::string name;
        std::string description;
        double price;
        int stock_quantity;
        std::string category;
    };
}

// Data Mapper interfaces
template<typename TDomain, typename TData>
class IDataMapper {
public:
    virtual ~IDataMapper() = default;
    
    // Map from domain to data
    virtual TData toData(const TDomain& domain) const = 0;
    
    // Map from data to domain
    virtual TDomain toDomain(const TData& data) const = 0;
    
    // CRUD operations
    virtual std::optional<TDomain> findById(int id) = 0;
    virtual std::vector<TDomain> findAll() = 0;
    virtual void insert(TDomain& entity) = 0;
    virtual void update(const TDomain& entity) = 0;
    virtual void remove(int id) = 0;
};

// Customer Data Mapper
class CustomerDataMapper : public IDataMapper<Domain::Customer, Data::CustomerRecord> {
private:
    // Simulate database with in-memory storage
    std::unordered_map<int, Data::CustomerRecord> database_;
    int nextId_ = 1;
    
public:
    Data::CustomerRecord toData(const Domain::Customer& customer) const override {
        Data::CustomerRecord record;
        record.id = customer.getId();
        record.first_name = customer.getFirstName();
        record.last_name = customer.getLastName();
        record.email = customer.getEmail();
        
        // Map shipping address
        record.shipping_street = customer.getShippingAddress().getStreet();
        record.shipping_city = customer.getShippingAddress().getCity();
        record.shipping_zip = customer.getShippingAddress().getZipCode();
        record.shipping_country = customer.getShippingAddress().getCountry();
        
        // Map billing address
        record.billing_street = customer.getBillingAddress().getStreet();
        record.billing_city = customer.getBillingAddress().getCity();
        record.billing_zip = customer.getBillingAddress().getZipCode();
        record.billing_country = customer.getBillingAddress().getCountry();
        
        record.total_purchases = customer.getTotalPurchases();
        record.is_vip = customer.getIsVip();
        
        return record;
    }
    
    Domain::Customer toDomain(const Data::CustomerRecord& record) const override {
        Domain::Customer customer(record.id, record.first_name, 
                                 record.last_name, record.email);
        
        // Map shipping address
        Domain::Address shippingAddr(record.shipping_street, record.shipping_city,
                                    record.shipping_zip, record.shipping_country);
        customer.setShippingAddress(shippingAddr);
        
        // Map billing address
        Domain::Address billingAddr(record.billing_street, record.billing_city,
                                   record.billing_zip, record.billing_country);
        customer.setBillingAddress(billingAddr);
        
        customer.setTotalPurchases(record.total_purchases);
        customer.setIsVip(record.is_vip);
        
        return customer;
    }
    
    std::optional<Domain::Customer> findById(int id) override {
        std::cout << "SQL: SELECT * FROM customers WHERE id = " << id << "\n";
        
        auto it = database_.find(id);
        if (it != database_.end()) {
            return toDomain(it->second);
        }
        return std::nullopt;
    }
    
    std::vector<Domain::Customer> findAll() override {
        std::cout << "SQL: SELECT * FROM customers\n";
        
        std::vector<Domain::Customer> result;
        for (const auto& [id, record] : database_) {
            result.push_back(toDomain(record));
        }
        return result;
    }
    
    void insert(Domain::Customer& entity) override {
        if (entity.getId() == 0) {
            entity.setId(nextId_++);
        }
        
        auto record = toData(entity);
        database_[record.id] = record;
        
        std::cout << "SQL: INSERT INTO customers (id, first_name, last_name, ...) "
                  << "VALUES (" << record.id << ", '" << record.first_name 
                  << "', '" << record.last_name << "', ...)\n";
    }
    
    void update(const Domain::Customer& entity) override {
        auto record = toData(entity);
        database_[record.id] = record;
        
        std::cout << "SQL: UPDATE customers SET first_name = '" << record.first_name
                  << "', last_name = '" << record.last_name 
                  << "', ... WHERE id = " << record.id << "\n";
    }
    
    void remove(int id) override {
        database_.erase(id);
        std::cout << "SQL: DELETE FROM customers WHERE id = " << id << "\n";
    }
    
    // Additional query methods
    std::optional<Domain::Customer> findByEmail(const std::string& email) {
        std::cout << "SQL: SELECT * FROM customers WHERE email = '" << email << "'\n";
        
        for (const auto& [id, record] : database_) {
            if (record.email == email) {
                return toDomain(record);
            }
        }
        return std::nullopt;
    }
    
    std::vector<Domain::Customer> findVipCustomers() {
        std::cout << "SQL: SELECT * FROM customers WHERE is_vip = true\n";
        
        std::vector<Domain::Customer> result;
        for (const auto& [id, record] : database_) {
            if (record.is_vip) {
                result.push_back(toDomain(record));
            }
        }
        return result;
    }
};

// Product Data Mapper
class ProductDataMapper : public IDataMapper<Domain::Product, Data::ProductRecord> {
private:
    std::unordered_map<int, Data::ProductRecord> database_;
    int nextId_ = 1;
    
public:
    Data::ProductRecord toData(const Domain::Product& product) const override {
        Data::ProductRecord record;
        record.id = product.getId();
        record.sku = product.getSku();
        record.name = product.getName();
        record.description = product.getDescription();
        record.price = product.getPrice();
        record.stock_quantity = product.getStockQuantity();
        record.category = product.getCategory();
        return record;
    }
    
    Domain::Product toDomain(const Data::ProductRecord& record) const override {
        Domain::Product product(record.id, record.sku, record.name,
                               record.price, record.stock_quantity);
        product.setDescription(record.description);
        product.setCategory(record.category);
        return product;
    }
    
    std::optional<Domain::Product> findById(int id) override {
        auto it = database_.find(id);
        if (it != database_.end()) {
            return toDomain(it->second);
        }
        return std::nullopt;
    }
    
    std::vector<Domain::Product> findAll() override {
        std::vector<Domain::Product> result;
        for (const auto& [id, record] : database_) {
            result.push_back(toDomain(record));
        }
        return result;
    }
    
    void insert(Domain::Product& entity) override {
        if (entity.getId() == 0) {
            entity.setId(nextId_++);
        }
        
        auto record = toData(entity);
        database_[record.id] = record;
        
        std::cout << "SQL: INSERT INTO products (id, sku, name, price, ...) "
                  << "VALUES (" << record.id << ", '" << record.sku 
                  << "', '" << record.name << "', " << record.price << ", ...)\n";
    }
    
    void update(const Domain::Product& entity) override {
        auto record = toData(entity);
        database_[record.id] = record;
        
        std::cout << "SQL: UPDATE products SET name = '" << record.name
                  << "', price = " << record.price 
                  << ", stock_quantity = " << record.stock_quantity
                  << " WHERE id = " << record.id << "\n";
    }
    
    void remove(int id) override {
        database_.erase(id);
        std::cout << "SQL: DELETE FROM products WHERE id = " << id << "\n";
    }
    
    // Custom query methods
    std::vector<Domain::Product> findByCategory(const std::string& category) {
        std::cout << "SQL: SELECT * FROM products WHERE category = '" << category << "'\n";
        
        std::vector<Domain::Product> result;
        for (const auto& [id, record] : database_) {
            if (record.category == category) {
                result.push_back(toDomain(record));
            }
        }
        return result;
    }
    
    std::vector<Domain::Product> findInStock() {
        std::cout << "SQL: SELECT * FROM products WHERE stock_quantity > 0\n";
        
        std::vector<Domain::Product> result;
        for (const auto& [id, record] : database_) {
            if (record.stock_quantity > 0) {
                result.push_back(toDomain(record));
            }
        }
        return result;
    }
};

// Service layer using mappers
class CustomerService {
private:
    std::shared_ptr<CustomerDataMapper> mapper_;
    
public:
    explicit CustomerService(std::shared_ptr<CustomerDataMapper> mapper)
        : mapper_(mapper) {}
    
    void registerCustomer(const std::string& firstName, const std::string& lastName,
                         const std::string& email, const Domain::Address& address) {
        // Check if email already exists
        auto existing = mapper_->findByEmail(email);
        if (existing.has_value()) {
            throw std::runtime_error("Email already registered");
        }
        
        // Create new customer
        Domain::Customer customer(0, firstName, lastName, email);
        customer.setShippingAddress(address);
        customer.setBillingAddress(address); // Same as shipping initially
        
        mapper_->insert(customer);
        
        std::cout << "Customer registered with ID: " << customer.getId() << "\n";
    }
    
    void recordPurchase(int customerId, double amount) {
        auto customer = mapper_->findById(customerId);
        if (!customer.has_value()) {
            throw std::runtime_error("Customer not found");
        }
        
        customer->recordPurchase(amount);
        mapper_->update(customer.value());
        
        if (customer->getIsVip()) {
            std::cout << "Customer " << customer->getFullName() 
                      << " is now a VIP!\n";
        }
    }
    
    void printCustomerReport() {
        auto customers = mapper_->findAll();
        
        std::cout << "\n=== Customer Report ===\n";
        for (const auto& customer : customers) {
            std::cout << "ID: " << customer.getId() << "\n";
            std::cout << "Name: " << customer.getFullName() << "\n";
            std::cout << "Email: " << customer.getEmail() << "\n";
            std::cout << "Total Purchases: $" << std::fixed << std::setprecision(2) 
                      << customer.getTotalPurchases() << "\n";
            std::cout << "VIP Status: " << (customer.getIsVip() ? "Yes" : "No") << "\n";
            std::cout << "Discount: " << (customer.getDiscount() * 100) << "%\n";
            std::cout << "Shipping: " << customer.getShippingAddress().getFullAddress() << "\n";
            std::cout << "---\n";
        }
    }
};

int main() {
    std::cout << "=== Data Mapper Pattern Demo ===\n\n";
    
    // Create mappers
    auto customerMapper = std::make_shared<CustomerDataMapper>();
    auto productMapper = std::make_shared<ProductDataMapper>();
    
    // Create service
    CustomerService customerService(customerMapper);
    
    // Register customers
    std::cout << "=== Registering Customers ===\n";
    Domain::Address addr1("123 Main St", "New York", "10001", "USA");
    customerService.registerCustomer("John", "Doe", "john@example.com", addr1);
    
    Domain::Address addr2("456 Oak Ave", "Los Angeles", "90001", "USA");
    customerService.registerCustomer("Jane", "Smith", "jane@example.com", addr2);
    
    Domain::Address addr3("789 Elm St", "London", "SW1A 1AA", "UK");
    customerService.registerCustomer("Bob", "Johnson", "bob@example.com", addr3);
    
    // Record purchases
    std::cout << "\n=== Recording Purchases ===\n";
    customerService.recordPurchase(1, 5000);
    customerService.recordPurchase(1, 3000);
    customerService.recordPurchase(1, 4000); // Should become VIP
    
    customerService.recordPurchase(2, 2000);
    
    // Product operations
    std::cout << "\n=== Product Operations ===\n";
    Domain::Product laptop(0, "LAPTOP-001", "High-end Laptop", 1500.00, 10);
    laptop.setDescription("Professional laptop with 16GB RAM");
    laptop.setCategory("Electronics");
    productMapper->insert(laptop);
    
    Domain::Product mouse(0, "MOUSE-001", "Wireless Mouse", 50.00, 100);
    mouse.setDescription("Ergonomic wireless mouse");
    mouse.setCategory("Electronics");
    productMapper->insert(mouse);
    
    Domain::Product book(0, "BOOK-001", "Design Patterns", 45.00, 0);
    book.setDescription("Gang of Four Design Patterns");
    book.setCategory("Books");
    productMapper->insert(book);
    
    // Find products in stock
    std::cout << "\n=== Products In Stock ===\n";
    auto inStock = productMapper->findInStock();
    for (const auto& product : inStock) {
        std::cout << product.getName() << " - $" << product.getPrice() 
                  << " (" << product.getStockQuantity() << " in stock)\n";
    }
    
    // Update product stock
    std::cout << "\n=== Updating Product Stock ===\n";
    auto product = productMapper->findById(1);
    if (product.has_value()) {
        product->reduceStock(2);
        productMapper->update(product.value());
    }
    
    // Print customer report
    customerService.printCustomerReport();
    
    // Find VIP customers
    std::cout << "\n=== VIP Customers ===\n";
    auto vips = customerMapper->findVipCustomers();
    for (const auto& vip : vips) {
        std::cout << vip.getFullName() << " - " << vip.getEmail() << "\n";
    }
    
    return 0;
}