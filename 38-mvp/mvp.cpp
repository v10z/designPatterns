// Minimal MVP (Model-View-Presenter) Pattern Implementation
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>

// MVP interfaces
template<typename T>
class IView {
public:
    virtual ~IView() = default;
    virtual void showData(const T& data) = 0;
    virtual void showMessage(const std::string& message) = 0;
    virtual void showError(const std::string& error) = 0;
};

// Example 1: Login System
namespace LoginSystem {
    // Model
    struct User {
        std::string username;
        std::string email;
        std::string role;
        bool isActive;
        
        User(const std::string& user, const std::string& mail, const std::string& r = "user")
            : username(user), email(mail), role(r), isActive(true) {}
    };
    
    class UserModel {
    private:
        std::vector<User> users_;
        User* currentUser_ = nullptr;
        
    public:
        UserModel() {
            // Add some test users
            users_.emplace_back("admin", "admin@example.com", "admin");
            users_.emplace_back("john", "john@example.com", "user");
            users_.emplace_back("jane", "jane@example.com", "moderator");
        }
        
        bool authenticate(const std::string& username, const std::string& password) {
            // Simple authentication (in real app, check hashed password)
            auto it = std::find_if(users_.begin(), users_.end(),
                [&username](const User& u) { return u.username == username; });
            
            if (it != users_.end() && password == "password123") {
                currentUser_ = &(*it);
                return true;
            }
            return false;
        }
        
        User* getCurrentUser() { return currentUser_; }
        
        void logout() {
            currentUser_ = nullptr;
        }
    };
    
    // View Interface
    class ILoginView : public IView<User> {
    public:
        virtual void showLoginForm() = 0;
        virtual void showDashboard(const User& user) = 0;
        virtual void clearForm() = 0;
        virtual std::string getUsername() = 0;
        virtual std::string getPassword() = 0;
        
        // From IView
        void showData(const User& user) override {
            showDashboard(user);
        }
    };
    
    // Concrete View
    class ConsoleLoginView : public ILoginView {
    private:
        std::string inputUsername_;
        std::string inputPassword_;
        
    public:
        void showLoginForm() override {
            std::cout << "\n=== Login Form ===\n";
            std::cout << "Username: ";
            std::getline(std::cin, inputUsername_);
            std::cout << "Password: ";
            std::getline(std::cin, inputPassword_);
        }
        
        void showDashboard(const User& user) override {
            std::cout << "\n=== Dashboard ===\n";
            std::cout << "Welcome, " << user.username << "!\n";
            std::cout << "Email: " << user.email << "\n";
            std::cout << "Role: " << user.role << "\n";
            std::cout << "Status: " << (user.isActive ? "Active" : "Inactive") << "\n";
        }
        
        void showMessage(const std::string& message) override {
            std::cout << "[Info] " << message << "\n";
        }
        
        void showError(const std::string& error) override {
            std::cout << "[Error] " << error << "\n";
        }
        
        void clearForm() override {
            inputUsername_.clear();
            inputPassword_.clear();
        }
        
        std::string getUsername() override { return inputUsername_; }
        std::string getPassword() override { return inputPassword_; }
    };
    
    // Presenter
    class LoginPresenter {
    private:
        UserModel* model_;
        ILoginView* view_;
        
    public:
        LoginPresenter(UserModel* model, ILoginView* view)
            : model_(model), view_(view) {}
        
        void showLogin() {
            view_->showLoginForm();
        }
        
        void attemptLogin() {
            std::string username = view_->getUsername();
            std::string password = view_->getPassword();
            
            if (username.empty() || password.empty()) {
                view_->showError("Username and password are required");
                return;
            }
            
            if (model_->authenticate(username, password)) {
                view_->showMessage("Login successful!");
                User* user = model_->getCurrentUser();
                if (user) {
                    view_->showDashboard(*user);
                }
                view_->clearForm();
            } else {
                view_->showError("Invalid username or password");
                view_->clearForm();
            }
        }
        
        void logout() {
            model_->logout();
            view_->showMessage("Logged out successfully");
        }
    };
}

// Example 2: Task Management System
namespace TaskManagement {
    // Model
    struct Task {
        int id;
        std::string title;
        std::string description;
        std::string priority;
        bool completed;
        std::time_t dueDate;
        
        Task(int id, const std::string& title, const std::string& desc,
             const std::string& pri = "medium")
            : id(id), title(title), description(desc), priority(pri),
              completed(false), dueDate(std::time(nullptr) + 86400) {} // Due tomorrow
    };
    
    class TaskModel {
    private:
        std::vector<Task> tasks_;
        int nextId_ = 1;
        
    public:
        void addTask(const std::string& title, const std::string& description,
                    const std::string& priority) {
            tasks_.emplace_back(nextId_++, title, description, priority);
        }
        
        void updateTask(int id, const std::string& title, const std::string& description,
                       const std::string& priority) {
            auto it = std::find_if(tasks_.begin(), tasks_.end(),
                [id](const Task& t) { return t.id == id; });
            
            if (it != tasks_.end()) {
                it->title = title;
                it->description = description;
                it->priority = priority;
            }
        }
        
        void completeTask(int id) {
            auto it = std::find_if(tasks_.begin(), tasks_.end(),
                [id](const Task& t) { return t.id == id; });
            
            if (it != tasks_.end()) {
                it->completed = true;
            }
        }
        
        void deleteTask(int id) {
            tasks_.erase(
                std::remove_if(tasks_.begin(), tasks_.end(),
                    [id](const Task& t) { return t.id == id; }),
                tasks_.end()
            );
        }
        
        std::vector<Task> getTasks() const { return tasks_; }
        
        std::vector<Task> getTasksByPriority(const std::string& priority) const {
            std::vector<Task> filtered;
            std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(filtered),
                [&priority](const Task& t) { return t.priority == priority; });
            return filtered;
        }
        
        std::vector<Task> getPendingTasks() const {
            std::vector<Task> pending;
            std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(pending),
                [](const Task& t) { return !t.completed; });
            return pending;
        }
    };
    
    // View Interface
    class ITaskView : public IView<std::vector<Task>> {
    public:
        virtual void showTaskList(const std::vector<Task>& tasks) = 0;
        virtual void showTaskForm() = 0;
        virtual void showTaskDetails(const Task& task) = 0;
        virtual Task getTaskInput() = 0;
        virtual int getSelectedTaskId() = 0;
        
        // From IView
        void showData(const std::vector<Task>& tasks) override {
            showTaskList(tasks);
        }
    };
    
    // Concrete View
    class ConsoleTaskView : public ITaskView {
    private:
        Task inputTask_{0, "", "", "medium"};
        int selectedId_ = 0;
        
    public:
        void showTaskList(const std::vector<Task>& tasks) override {
            std::cout << "\n=== Task List ===\n";
            std::cout << std::setw(5) << "ID" 
                      << std::setw(30) << "Title"
                      << std::setw(10) << "Priority"
                      << std::setw(12) << "Status" << "\n";
            std::cout << std::string(57, '-') << "\n";
            
            for (const auto& task : tasks) {
                std::cout << std::setw(5) << task.id
                          << std::setw(30) << task.title
                          << std::setw(10) << task.priority
                          << std::setw(12) << (task.completed ? "Completed" : "Pending")
                          << "\n";
            }
        }
        
        void showTaskForm() override {
            std::cout << "\n=== New Task ===\n";
            std::cout << "Title: ";
            std::getline(std::cin, inputTask_.title);
            std::cout << "Description: ";
            std::getline(std::cin, inputTask_.description);
            std::cout << "Priority (high/medium/low): ";
            std::getline(std::cin, inputTask_.priority);
        }
        
        void showTaskDetails(const Task& task) override {
            std::cout << "\n=== Task Details ===\n";
            std::cout << "ID: " << task.id << "\n";
            std::cout << "Title: " << task.title << "\n";
            std::cout << "Description: " << task.description << "\n";
            std::cout << "Priority: " << task.priority << "\n";
            std::cout << "Status: " << (task.completed ? "Completed" : "Pending") << "\n";
            
            auto tm = std::localtime(&task.dueDate);
            std::cout << "Due Date: " << std::put_time(tm, "%Y-%m-%d") << "\n";
        }
        
        void showMessage(const std::string& message) override {
            std::cout << "[Info] " << message << "\n";
        }
        
        void showError(const std::string& error) override {
            std::cout << "[Error] " << error << "\n";
        }
        
        Task getTaskInput() override { return inputTask_; }
        
        int getSelectedTaskId() override { 
            std::cout << "Enter task ID: ";
            std::cin >> selectedId_;
            std::cin.ignore(); // Clear newline
            return selectedId_;
        }
    };
    
    // Presenter
    class TaskPresenter {
    private:
        TaskModel* model_;
        ITaskView* view_;
        
    public:
        TaskPresenter(TaskModel* model, ITaskView* view)
            : model_(model), view_(view) {}
        
        void showAllTasks() {
            auto tasks = model_->getTasks();
            view_->showTaskList(tasks);
        }
        
        void showPendingTasks() {
            auto tasks = model_->getPendingTasks();
            view_->showMessage("Showing pending tasks");
            view_->showTaskList(tasks);
        }
        
        void showTasksByPriority(const std::string& priority) {
            auto tasks = model_->getTasksByPriority(priority);
            view_->showMessage("Showing " + priority + " priority tasks");
            view_->showTaskList(tasks);
        }
        
        void createTask() {
            view_->showTaskForm();
            Task task = view_->getTaskInput();
            
            if (task.title.empty()) {
                view_->showError("Task title is required");
                return;
            }
            
            model_->addTask(task.title, task.description, task.priority);
            view_->showMessage("Task created successfully");
            showAllTasks();
        }
        
        void completeTask() {
            int id = view_->getSelectedTaskId();
            model_->completeTask(id);
            view_->showMessage("Task marked as completed");
            showAllTasks();
        }
        
        void deleteTask() {
            int id = view_->getSelectedTaskId();
            model_->deleteTask(id);
            view_->showMessage("Task deleted");
            showAllTasks();
        }
    };
}

// Example 3: Shopping Cart
namespace ShoppingCart {
    // Model
    struct Product {
        int id;
        std::string name;
        double price;
        int stock;
        
        Product(int id, const std::string& name, double price, int stock)
            : id(id), name(name), price(price), stock(stock) {}
    };
    
    struct CartItem {
        Product product;
        int quantity;
        
        CartItem(const Product& p, int qty) : product(p), quantity(qty) {}
        
        double getSubtotal() const {
            return product.price * quantity;
        }
    };
    
    class ShoppingCartModel {
    private:
        std::vector<Product> catalog_;
        std::vector<CartItem> cart_;
        
    public:
        ShoppingCartModel() {
            // Initialize catalog
            catalog_.emplace_back(1, "Laptop", 999.99, 10);
            catalog_.emplace_back(2, "Mouse", 29.99, 50);
            catalog_.emplace_back(3, "Keyboard", 79.99, 30);
            catalog_.emplace_back(4, "Monitor", 299.99, 15);
            catalog_.emplace_back(5, "Headphones", 89.99, 25);
        }
        
        std::vector<Product> getCatalog() const { return catalog_; }
        std::vector<CartItem> getCart() const { return cart_; }
        
        bool addToCart(int productId, int quantity) {
            auto it = std::find_if(catalog_.begin(), catalog_.end(),
                [productId](const Product& p) { return p.id == productId; });
            
            if (it != catalog_.end() && it->stock >= quantity) {
                // Check if already in cart
                auto cartIt = std::find_if(cart_.begin(), cart_.end(),
                    [productId](const CartItem& item) { 
                        return item.product.id == productId; 
                    });
                
                if (cartIt != cart_.end()) {
                    cartIt->quantity += quantity;
                } else {
                    cart_.emplace_back(*it, quantity);
                }
                
                it->stock -= quantity;
                return true;
            }
            return false;
        }
        
        void removeFromCart(int productId) {
            auto it = std::find_if(cart_.begin(), cart_.end(),
                [productId](const CartItem& item) { 
                    return item.product.id == productId; 
                });
            
            if (it != cart_.end()) {
                // Return stock
                auto catalogIt = std::find_if(catalog_.begin(), catalog_.end(),
                    [productId](Product& p) { return p.id == productId; });
                
                if (catalogIt != catalog_.end()) {
                    catalogIt->stock += it->quantity;
                }
                
                cart_.erase(it);
            }
        }
        
        void updateQuantity(int productId, int newQuantity) {
            auto it = std::find_if(cart_.begin(), cart_.end(),
                [productId](CartItem& item) { 
                    return item.product.id == productId; 
                });
            
            if (it != cart_.end()) {
                int diff = newQuantity - it->quantity;
                
                auto catalogIt = std::find_if(catalog_.begin(), catalog_.end(),
                    [productId](Product& p) { return p.id == productId; });
                
                if (catalogIt != catalog_.end() && catalogIt->stock >= diff) {
                    it->quantity = newQuantity;
                    catalogIt->stock -= diff;
                }
            }
        }
        
        double getTotal() const {
            double total = 0.0;
            for (const auto& item : cart_) {
                total += item.getSubtotal();
            }
            return total;
        }
        
        void clearCart() {
            // Return all stock
            for (const auto& item : cart_) {
                auto catalogIt = std::find_if(catalog_.begin(), catalog_.end(),
                    [&item](Product& p) { return p.id == item.product.id; });
                
                if (catalogIt != catalog_.end()) {
                    catalogIt->stock += item.quantity;
                }
            }
            cart_.clear();
        }
    };
    
    // View Interface
    class IShoppingView : public IView<std::vector<CartItem>> {
    public:
        virtual void showCatalog(const std::vector<Product>& products) = 0;
        virtual void showCart(const std::vector<CartItem>& items, double total) = 0;
        virtual std::pair<int, int> getProductSelection() = 0;
        virtual int getProductId() = 0;
        
        // From IView
        void showData(const std::vector<CartItem>& items) override {
            double total = 0.0;
            for (const auto& item : items) {
                total += item.getSubtotal();
            }
            showCart(items, total);
        }
    };
    
    // Concrete View
    class ConsoleShoppingView : public IShoppingView {
    public:
        void showCatalog(const std::vector<Product>& products) override {
            std::cout << "\n=== Product Catalog ===\n";
            std::cout << std::setw(5) << "ID"
                      << std::setw(20) << "Product"
                      << std::setw(10) << "Price"
                      << std::setw(10) << "Stock" << "\n";
            std::cout << std::string(45, '-') << "\n";
            
            for (const auto& product : products) {
                std::cout << std::setw(5) << product.id
                          << std::setw(20) << product.name
                          << std::setw(10) << std::fixed << std::setprecision(2) 
                          << product.price
                          << std::setw(10) << product.stock << "\n";
            }
        }
        
        void showCart(const std::vector<CartItem>& items, double total) override {
            std::cout << "\n=== Shopping Cart ===\n";
            if (items.empty()) {
                std::cout << "Your cart is empty.\n";
                return;
            }
            
            std::cout << std::setw(20) << "Product"
                      << std::setw(10) << "Price"
                      << std::setw(10) << "Quantity"
                      << std::setw(12) << "Subtotal" << "\n";
            std::cout << std::string(52, '-') << "\n";
            
            for (const auto& item : items) {
                std::cout << std::setw(20) << item.product.name
                          << std::setw(10) << std::fixed << std::setprecision(2) 
                          << item.product.price
                          << std::setw(10) << item.quantity
                          << std::setw(12) << item.getSubtotal() << "\n";
            }
            
            std::cout << std::string(52, '-') << "\n";
            std::cout << std::setw(42) << "Total: $"
                      << std::setw(10) << total << "\n";
        }
        
        void showMessage(const std::string& message) override {
            std::cout << "[Info] " << message << "\n";
        }
        
        void showError(const std::string& error) override {
            std::cout << "[Error] " << error << "\n";
        }
        
        std::pair<int, int> getProductSelection() override {
            int id, quantity;
            std::cout << "Enter product ID: ";
            std::cin >> id;
            std::cout << "Enter quantity: ";
            std::cin >> quantity;
            std::cin.ignore();
            return {id, quantity};
        }
        
        int getProductId() override {
            int id;
            std::cout << "Enter product ID: ";
            std::cin >> id;
            std::cin.ignore();
            return id;
        }
    };
    
    // Presenter
    class ShoppingPresenter {
    private:
        ShoppingCartModel* model_;
        IShoppingView* view_;
        
    public:
        ShoppingPresenter(ShoppingCartModel* model, IShoppingView* view)
            : model_(model), view_(view) {}
        
        void showCatalog() {
            auto products = model_->getCatalog();
            view_->showCatalog(products);
        }
        
        void showCart() {
            auto items = model_->getCart();
            double total = model_->getTotal();
            view_->showCart(items, total);
        }
        
        void addToCart() {
            auto [productId, quantity] = view_->getProductSelection();
            
            if (quantity <= 0) {
                view_->showError("Quantity must be positive");
                return;
            }
            
            if (model_->addToCart(productId, quantity)) {
                view_->showMessage("Added to cart successfully");
                showCart();
            } else {
                view_->showError("Unable to add to cart (insufficient stock)");
            }
        }
        
        void removeFromCart() {
            int productId = view_->getProductId();
            model_->removeFromCart(productId);
            view_->showMessage("Removed from cart");
            showCart();
        }
        
        void checkout() {
            double total = model_->getTotal();
            if (total > 0) {
                view_->showMessage("Order placed! Total: $" + 
                                 std::to_string(total));
                model_->clearCart();
                showCart();
            } else {
                view_->showError("Cart is empty");
            }
        }
    };
}

// Demo
void demonstrateLoginSystem() {
    std::cout << "=== Login System Demo ===\n";
    
    LoginSystem::UserModel model;
    LoginSystem::ConsoleLoginView view;
    LoginSystem::LoginPresenter presenter(&model, &view);
    
    // Simulate login attempts
    std::cout << "Simulating login with empty fields...\n";
    presenter.attemptLogin();
    
    std::cout << "\nSimulating failed login...\n";
    // In real app, would get from view
    presenter.attemptLogin();
    
    std::cout << "\nSimulating successful login (use 'admin' and 'password123')...\n";
    // In real app, would show form and get input
}

void demonstrateTaskManagement() {
    std::cout << "\n=== Task Management Demo ===\n";
    
    TaskManagement::TaskModel model;
    TaskManagement::ConsoleTaskView view;
    TaskManagement::TaskPresenter presenter(&model, &view);
    
    // Add some tasks
    model.addTask("Complete MVP pattern", "Implement Model-View-Presenter", "high");
    model.addTask("Review code", "Review all pattern implementations", "medium");
    model.addTask("Write tests", "Add unit tests for patterns", "low");
    model.addTask("Update documentation", "Update README files", "high");
    
    // Show all tasks
    presenter.showAllTasks();
    
    // Show filtered views
    presenter.showTasksByPriority("high");
    
    // Complete a task
    model.completeTask(1);
    presenter.showPendingTasks();
}

void demonstrateShoppingCart() {
    std::cout << "\n=== Shopping Cart Demo ===\n";
    
    ShoppingCart::ShoppingCartModel model;
    ShoppingCart::ConsoleShoppingView view;
    ShoppingCart::ShoppingPresenter presenter(&model, &view);
    
    // Show catalog
    presenter.showCatalog();
    
    // Add items to cart
    model.addToCart(1, 1);  // 1 Laptop
    model.addToCart(2, 2);  // 2 Mice
    model.addToCart(4, 1);  // 1 Monitor
    
    // Show cart
    presenter.showCart();
    
    // Update quantity
    model.updateQuantity(2, 3);  // Change mice quantity to 3
    
    // Show updated cart
    presenter.showCart();
}

int main() {
    std::cout << "=== MVP (Model-View-Presenter) Pattern Demo ===\n\n";
    
    demonstrateLoginSystem();
    demonstrateTaskManagement();
    demonstrateShoppingCart();
    
    std::cout << "\n=== MVP vs MVC ===\n";
    std::cout << "MVP Differences:\n";
    std::cout << "- View is passive (no direct Model access)\n";
    std::cout << "- Presenter handles all UI logic\n";
    std::cout << "- Better testability (mock views)\n";
    std::cout << "- Clear separation of concerns\n";
    std::cout << "- One-to-one View-Presenter relationship\n";
    
    return 0;
}