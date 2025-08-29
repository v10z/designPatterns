// Minimal MVC (Model-View-Controller) Pattern Implementation
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

// Forward declarations
class IObserver;
class Model;
class View;
class Controller;

// Observer interface for MVC communication
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void update() = 0;
};

// Subject interface for Model
class ISubject {
public:
    virtual ~ISubject() = default;
    virtual void attach(IObserver* observer) = 0;
    virtual void detach(IObserver* observer) = 0;
    virtual void notify() = 0;
};

// Base Model class
class Model : public ISubject {
protected:
    std::vector<IObserver*> observers_;
    
public:
    void attach(IObserver* observer) override {
        observers_.push_back(observer);
    }
    
    void detach(IObserver* observer) override {
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }
    
    void notify() override {
        for (auto observer : observers_) {
            observer->update();
        }
    }
};

// Example 1: Simple Todo Application
namespace TodoApp {
    // Todo Model
    struct TodoItem {
        int id;
        std::string title;
        bool completed;
        std::time_t createdAt;
        
        TodoItem(int id, const std::string& title) 
            : id(id), title(title), completed(false), createdAt(std::time(nullptr)) {}
    };
    
    class TodoModel : public Model {
    private:
        std::vector<TodoItem> todos_;
        int nextId_ = 1;
        std::string filter_ = "all"; // all, active, completed
        
    public:
        void addTodo(const std::string& title) {
            todos_.emplace_back(nextId_++, title);
            notify();
        }
        
        void toggleTodo(int id) {
            auto it = std::find_if(todos_.begin(), todos_.end(),
                [id](const TodoItem& item) { return item.id == id; });
            
            if (it != todos_.end()) {
                it->completed = !it->completed;
                notify();
            }
        }
        
        void removeTodo(int id) {
            todos_.erase(
                std::remove_if(todos_.begin(), todos_.end(),
                    [id](const TodoItem& item) { return item.id == id; }),
                todos_.end()
            );
            notify();
        }
        
        void setFilter(const std::string& filter) {
            filter_ = filter;
            notify();
        }
        
        std::vector<TodoItem> getFilteredTodos() const {
            if (filter_ == "active") {
                std::vector<TodoItem> active;
                std::copy_if(todos_.begin(), todos_.end(), std::back_inserter(active),
                    [](const TodoItem& item) { return !item.completed; });
                return active;
            } else if (filter_ == "completed") {
                std::vector<TodoItem> completed;
                std::copy_if(todos_.begin(), todos_.end(), std::back_inserter(completed),
                    [](const TodoItem& item) { return item.completed; });
                return completed;
            }
            return todos_;
        }
        
        int getActiveCount() const {
            return std::count_if(todos_.begin(), todos_.end(),
                [](const TodoItem& item) { return !item.completed; });
        }
        
        std::string getFilter() const { return filter_; }
    };
    
    // Todo View
    class TodoView : public IObserver {
    private:
        TodoModel* model_;
        
    public:
        explicit TodoView(TodoModel* model) : model_(model) {
            model_->attach(this);
        }
        
        ~TodoView() {
            model_->detach(this);
        }
        
        void update() override {
            render();
        }
        
        void render() {
            std::cout << "\n=== Todo List (" << model_->getFilter() << ") ===\n";
            
            auto todos = model_->getFilteredTodos();
            if (todos.empty()) {
                std::cout << "No todos found.\n";
            } else {
                for (const auto& todo : todos) {
                    std::cout << "[" << (todo.completed ? "X" : " ") << "] "
                              << todo.id << ". " << todo.title;
                    
                    // Format date
                    auto tm = std::localtime(&todo.createdAt);
                    std::cout << " (created: " << std::put_time(tm, "%Y-%m-%d %H:%M") << ")\n";
                }
            }
            
            std::cout << "\nActive todos: " << model_->getActiveCount() << "\n";
        }
        
        void showCommands() {
            std::cout << "\nCommands:\n";
            std::cout << "  add <title>  - Add a new todo\n";
            std::cout << "  toggle <id>  - Toggle todo completion\n";
            std::cout << "  remove <id>  - Remove a todo\n";
            std::cout << "  filter <all|active|completed> - Set filter\n";
            std::cout << "  quit         - Exit application\n";
        }
    };
    
    // Todo Controller
    class TodoController {
    private:
        TodoModel* model_;
        TodoView* view_;
        
    public:
        TodoController(TodoModel* model, TodoView* view) 
            : model_(model), view_(view) {}
        
        void run() {
            view_->render();
            view_->showCommands();
            
            std::string command;
            while (std::getline(std::cin, command)) {
                if (command == "quit") break;
                
                processCommand(command);
            }
        }
        
        void processCommand(const std::string& command) {
            std::istringstream iss(command);
            std::string action;
            iss >> action;
            
            if (action == "add") {
                std::string title;
                std::getline(iss, title);
                if (!title.empty() && title[0] == ' ') {
                    title = title.substr(1);
                }
                if (!title.empty()) {
                    model_->addTodo(title);
                }
            } else if (action == "toggle") {
                int id;
                if (iss >> id) {
                    model_->toggleTodo(id);
                }
            } else if (action == "remove") {
                int id;
                if (iss >> id) {
                    model_->removeTodo(id);
                }
            } else if (action == "filter") {
                std::string filter;
                if (iss >> filter) {
                    model_->setFilter(filter);
                }
            } else {
                std::cout << "Unknown command: " << action << "\n";
                view_->showCommands();
            }
        }
    };
}

// Example 2: User Management System
namespace UserManagement {
    // User Model
    struct User {
        int id;
        std::string username;
        std::string email;
        std::string role;
        bool active;
        
        User(int id, const std::string& username, const std::string& email, 
             const std::string& role = "user")
            : id(id), username(username), email(email), role(role), active(true) {}
    };
    
    class UserModel : public Model {
    private:
        std::vector<User> users_;
        int nextId_ = 1;
        User* currentUser_ = nullptr;
        
    public:
        void addUser(const std::string& username, const std::string& email, 
                    const std::string& role = "user") {
            users_.emplace_back(nextId_++, username, email, role);
            notify();
        }
        
        void updateUser(int id, const std::string& email, const std::string& role) {
            auto it = std::find_if(users_.begin(), users_.end(),
                [id](const User& u) { return u.id == id; });
            
            if (it != users_.end()) {
                it->email = email;
                it->role = role;
                notify();
            }
        }
        
        void deleteUser(int id) {
            users_.erase(
                std::remove_if(users_.begin(), users_.end(),
                    [id](const User& u) { return u.id == id; }),
                users_.end()
            );
            notify();
        }
        
        void toggleUserStatus(int id) {
            auto it = std::find_if(users_.begin(), users_.end(),
                [id](const User& u) { return u.id == id; });
            
            if (it != users_.end()) {
                it->active = !it->active;
                notify();
            }
        }
        
        void selectUser(int id) {
            auto it = std::find_if(users_.begin(), users_.end(),
                [id](User& u) { return u.id == id; });
            
            currentUser_ = (it != users_.end()) ? &(*it) : nullptr;
            notify();
        }
        
        const std::vector<User>& getUsers() const { return users_; }
        User* getCurrentUser() { return currentUser_; }
    };
    
    // Multiple Views
    class UserListView : public IObserver {
    private:
        UserModel* model_;
        
    public:
        explicit UserListView(UserModel* model) : model_(model) {
            model_->attach(this);
        }
        
        ~UserListView() {
            model_->detach(this);
        }
        
        void update() override {
            render();
        }
        
        void render() {
            std::cout << "\n=== User List ===\n";
            std::cout << std::setw(5) << "ID" << std::setw(15) << "Username" 
                      << std::setw(25) << "Email" << std::setw(10) << "Role" 
                      << std::setw(10) << "Status" << "\n";
            std::cout << std::string(65, '-') << "\n";
            
            for (const auto& user : model_->getUsers()) {
                std::cout << std::setw(5) << user.id 
                          << std::setw(15) << user.username
                          << std::setw(25) << user.email
                          << std::setw(10) << user.role
                          << std::setw(10) << (user.active ? "Active" : "Inactive")
                          << "\n";
            }
        }
    };
    
    class UserDetailView : public IObserver {
    private:
        UserModel* model_;
        
    public:
        explicit UserDetailView(UserModel* model) : model_(model) {
            model_->attach(this);
        }
        
        ~UserDetailView() {
            model_->detach(this);
        }
        
        void update() override {
            render();
        }
        
        void render() {
            auto user = model_->getCurrentUser();
            if (user) {
                std::cout << "\n=== User Details ===\n";
                std::cout << "ID: " << user->id << "\n";
                std::cout << "Username: " << user->username << "\n";
                std::cout << "Email: " << user->email << "\n";
                std::cout << "Role: " << user->role << "\n";
                std::cout << "Status: " << (user->active ? "Active" : "Inactive") << "\n";
            }
        }
    };
    
    // User Controller
    class UserController {
    private:
        UserModel* model_;
        
    public:
        explicit UserController(UserModel* model) : model_(model) {}
        
        void createUser(const std::string& username, const std::string& email, 
                       const std::string& role = "user") {
            model_->addUser(username, email, role);
            std::cout << "User created successfully.\n";
        }
        
        void updateUser(int id, const std::string& email, const std::string& role) {
            model_->updateUser(id, email, role);
            std::cout << "User updated successfully.\n";
        }
        
        void deleteUser(int id) {
            model_->deleteUser(id);
            std::cout << "User deleted successfully.\n";
        }
        
        void toggleUserStatus(int id) {
            model_->toggleUserStatus(id);
            std::cout << "User status toggled.\n";
        }
        
        void viewUser(int id) {
            model_->selectUser(id);
        }
    };
}

// Example 3: Calculator MVC
namespace Calculator {
    class CalculatorModel : public Model {
    private:
        double currentValue_ = 0.0;
        double storedValue_ = 0.0;
        std::string operation_ = "";
        std::string display_ = "0";
        std::vector<std::string> history_;
        
    public:
        void inputNumber(const std::string& num) {
            if (display_ == "0" || operation_ == "=") {
                display_ = num;
            } else {
                display_ += num;
            }
            currentValue_ = std::stod(display_);
            notify();
        }
        
        void setOperation(const std::string& op) {
            if (operation_ != "" && operation_ != "=") {
                calculate();
            } else {
                storedValue_ = currentValue_;
            }
            operation_ = op;
            display_ = "0";
            notify();
        }
        
        void calculate() {
            double result = storedValue_;
            
            if (operation_ == "+") {
                result += currentValue_;
            } else if (operation_ == "-") {
                result -= currentValue_;
            } else if (operation_ == "*") {
                result *= currentValue_;
            } else if (operation_ == "/") {
                if (currentValue_ != 0) {
                    result /= currentValue_;
                } else {
                    display_ = "Error";
                    notify();
                    return;
                }
            }
            
            std::ostringstream oss;
            oss << storedValue_ << " " << operation_ << " " << currentValue_ 
                << " = " << result;
            history_.push_back(oss.str());
            
            currentValue_ = result;
            storedValue_ = result;
            display_ = std::to_string(result);
            operation_ = "=";
            notify();
        }
        
        void clear() {
            currentValue_ = 0.0;
            storedValue_ = 0.0;
            operation_ = "";
            display_ = "0";
            notify();
        }
        
        double getCurrentValue() const { return currentValue_; }
        std::string getDisplay() const { return display_; }
        const std::vector<std::string>& getHistory() const { return history_; }
    };
    
    class CalculatorView : public IObserver {
    private:
        CalculatorModel* model_;
        
    public:
        explicit CalculatorView(CalculatorModel* model) : model_(model) {
            model_->attach(this);
        }
        
        ~CalculatorView() {
            model_->detach(this);
        }
        
        void update() override {
            render();
        }
        
        void render() {
            std::cout << "\n┌─────────────────┐\n";
            std::cout << "│ " << std::setw(15) << model_->getDisplay() << " │\n";
            std::cout << "├─────────────────┤\n";
            std::cout << "│ 7 │ 8 │ 9 │ / │\n";
            std::cout << "│ 4 │ 5 │ 6 │ * │\n";
            std::cout << "│ 1 │ 2 │ 3 │ - │\n";
            std::cout << "│ 0 │ . │ = │ + │\n";
            std::cout << "│   C (clear)   │\n";
            std::cout << "└─────────────────┘\n";
        }
        
        void showHistory() {
            std::cout << "\n=== Calculation History ===\n";
            for (const auto& entry : model_->getHistory()) {
                std::cout << entry << "\n";
            }
        }
    };
    
    class CalculatorController {
    private:
        CalculatorModel* model_;
        CalculatorView* view_;
        
    public:
        CalculatorController(CalculatorModel* model, CalculatorView* view)
            : model_(model), view_(view) {}
        
        void processInput(const std::string& input) {
            if (input >= "0" && input <= "9") {
                model_->inputNumber(input);
            } else if (input == "+" || input == "-" || input == "*" || input == "/") {
                model_->setOperation(input);
            } else if (input == "=") {
                model_->calculate();
            } else if (input == "C" || input == "c") {
                model_->clear();
            } else if (input == "H" || input == "h") {
                view_->showHistory();
            }
        }
    };
}

// MVC Framework Base Classes
namespace MVCFramework {
    template<typename TModel>
    class View : public IObserver {
    protected:
        TModel* model_;
        
    public:
        explicit View(TModel* model) : model_(model) {
            model_->attach(this);
        }
        
        virtual ~View() {
            model_->detach(this);
        }
        
        void update() override {
            onModelChanged();
        }
        
        virtual void onModelChanged() = 0;
    };
    
    template<typename TModel, typename TView>
    class Controller {
    protected:
        TModel* model_;
        TView* view_;
        
    public:
        Controller(TModel* model, TView* view) 
            : model_(model), view_(view) {}
        
        virtual ~Controller() = default;
    };
}

// Demo application
void demonstrateTodoApp() {
    std::cout << "=== Todo Application Demo ===\n";
    
    TodoApp::TodoModel model;
    TodoApp::TodoView view(&model);
    TodoApp::TodoController controller(&model, &view);
    
    // Simulate some commands
    controller.processCommand("add Learn MVC Pattern");
    controller.processCommand("add Implement Todo App");
    controller.processCommand("add Write Documentation");
    controller.processCommand("toggle 2");
    controller.processCommand("filter active");
    controller.processCommand("filter all");
}

void demonstrateUserManagement() {
    std::cout << "\n=== User Management System Demo ===\n";
    
    UserManagement::UserModel model;
    UserManagement::UserListView listView(&model);
    UserManagement::UserDetailView detailView(&model);
    UserManagement::UserController controller(&model);
    
    // Create some users
    controller.createUser("alice", "alice@example.com", "admin");
    controller.createUser("bob", "bob@example.com", "user");
    controller.createUser("charlie", "charlie@example.com", "moderator");
    
    // View specific user
    controller.viewUser(2);
    
    // Update user
    controller.updateUser(2, "bob@newdomain.com", "moderator");
    
    // Toggle user status
    controller.toggleUserStatus(3);
}

void demonstrateCalculator() {
    std::cout << "\n=== Calculator Demo ===\n";
    
    Calculator::CalculatorModel model;
    Calculator::CalculatorView view(&model);
    Calculator::CalculatorController controller(&model, &view);
    
    // Initial display
    view.render();
    
    // Perform calculation: 15 + 7 = 22
    controller.processInput("1");
    controller.processInput("5");
    controller.processInput("+");
    controller.processInput("7");
    controller.processInput("=");
    
    // Another calculation: 22 * 3 = 66
    controller.processInput("*");
    controller.processInput("3");
    controller.processInput("=");
    
    // Show history
    view.showHistory();
}

int main() {
    std::cout << "=== MVC (Model-View-Controller) Pattern Demo ===\n\n";
    
    demonstrateTodoApp();
    demonstrateUserManagement();
    demonstrateCalculator();
    
    std::cout << "\n=== MVC Pattern Benefits ===\n";
    std::cout << "1. Separation of Concerns\n";
    std::cout << "2. Multiple Views of Same Model\n";
    std::cout << "3. Reusable Components\n";
    std::cout << "4. Easier Testing\n";
    std::cout << "5. Parallel Development\n";
    
    return 0;
}