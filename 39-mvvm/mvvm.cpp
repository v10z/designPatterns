// Minimal MVVM (Model-View-ViewModel) Pattern Implementation
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <any>

// Property Changed Notification
class INotifyPropertyChanged {
public:
    using PropertyChangedHandler = std::function<void(const std::string&)>;
    
    virtual ~INotifyPropertyChanged() = default;
    
    void addPropertyChangedHandler(PropertyChangedHandler handler) {
        handlers_.push_back(handler);
    }
    
protected:
    template<typename T> friend class ObservableProperty;
    
    void notifyPropertyChanged(const std::string& propertyName) {
        for (const auto& handler : handlers_) {
            handler(propertyName);
        }
    }
    
private:
    std::vector<PropertyChangedHandler> handlers_;
};

// Observable Property Template
template<typename T>
class ObservableProperty {
private:
    T value_;
    std::string name_;
    INotifyPropertyChanged* owner_;
    
public:
    ObservableProperty(const std::string& name, INotifyPropertyChanged* owner, const T& initial = T())
        : name_(name), owner_(owner), value_(initial) {}
    
    const T& get() const { return value_; }
    
    void set(const T& newValue) {
        if (value_ != newValue) {
            value_ = newValue;
            if (owner_) {
                owner_->notifyPropertyChanged(name_);
            }
        }
    }
    
    operator const T&() const { return value_; }
    
    ObservableProperty& operator=(const T& newValue) {
        set(newValue);
        return *this;
    }
};

// Command Interface
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual bool canExecute() const = 0;
};

// Relay Command
class RelayCommand : public ICommand {
private:
    std::function<void()> executeFunc_;
    std::function<bool()> canExecuteFunc_;
    
public:
    RelayCommand(std::function<void()> execute, 
                std::function<bool()> canExecute = []() { return true; })
        : executeFunc_(execute), canExecuteFunc_(canExecute) {}
    
    void execute() override {
        if (canExecute()) {
            executeFunc_();
        }
    }
    
    bool canExecute() const override {
        return canExecuteFunc_();
    }
};

// Example 1: Simple Counter Application
namespace CounterApp {
    // Model
    class CounterModel {
    private:
        int value_ = 0;
        
    public:
        int getValue() const { return value_; }
        void setValue(int value) { value_ = value; }
        void increment() { value_++; }
        void decrement() { value_--; }
        void reset() { value_ = 0; }
    };
    
    // ViewModel
    class CounterViewModel : public INotifyPropertyChanged {
    private:
        CounterModel model_;
        ObservableProperty<int> counterValue_;
        ObservableProperty<std::string> displayText_;
        ObservableProperty<bool> canDecrement_;
        
        std::unique_ptr<ICommand> incrementCommand_;
        std::unique_ptr<ICommand> decrementCommand_;
        std::unique_ptr<ICommand> resetCommand_;
        
        void updateDisplayText() {
            displayText_.set("Counter: " + std::to_string(counterValue_.get()));
        }
        
        void updateCanDecrement() {
            canDecrement_.set(counterValue_.get() > 0);
        }
        
    public:
        CounterViewModel() 
            : counterValue_("CounterValue", this, 0),
              displayText_("DisplayText", this, "Counter: 0"),
              canDecrement_("CanDecrement", this, false) {
            
            // Initialize commands
            incrementCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    model_.increment();
                    counterValue_.set(model_.getValue());
                    updateDisplayText();
                    updateCanDecrement();
                }
            );
            
            decrementCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    model_.decrement();
                    counterValue_.set(model_.getValue());
                    updateDisplayText();
                    updateCanDecrement();
                },
                [this]() { return canDecrement_.get(); }
            );
            
            resetCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    model_.reset();
                    counterValue_.set(model_.getValue());
                    updateDisplayText();
                    updateCanDecrement();
                }
            );
        }
        
        // Properties
        const ObservableProperty<int>& getCounterValue() const { return counterValue_; }
        const ObservableProperty<std::string>& getDisplayText() const { return displayText_; }
        const ObservableProperty<bool>& getCanDecrement() const { return canDecrement_; }
        
        // Commands
        ICommand* getIncrementCommand() { return incrementCommand_.get(); }
        ICommand* getDecrementCommand() { return decrementCommand_.get(); }
        ICommand* getResetCommand() { return resetCommand_.get(); }
    };
    
    // View
    class CounterView {
    private:
        CounterViewModel* viewModel_;
        
    public:
        explicit CounterView(CounterViewModel* vm) : viewModel_(vm) {
            // Bind to property changes
            viewModel_->addPropertyChangedHandler(
                [this](const std::string& propertyName) {
                    onPropertyChanged(propertyName);
                }
            );
        }
        
        void render() {
            std::cout << "\n=== Counter Application ===\n";
            std::cout << viewModel_->getDisplayText().get() << "\n";
            std::cout << "Can Decrement: " 
                      << (viewModel_->getCanDecrement().get() ? "Yes" : "No") << "\n";
            std::cout << "\nCommands:\n";
            std::cout << "[+] Increment\n";
            std::cout << "[-] Decrement" 
                      << (viewModel_->getCanDecrement().get() ? "" : " (disabled)") << "\n";
            std::cout << "[R] Reset\n";
        }
        
        void onPropertyChanged(const std::string& propertyName) {
            std::cout << "Property changed: " << propertyName << "\n";
            render();
        }
        
        void simulateUserInput(char command) {
            switch (command) {
                case '+':
                    viewModel_->getIncrementCommand()->execute();
                    break;
                case '-':
                    viewModel_->getDecrementCommand()->execute();
                    break;
                case 'R':
                case 'r':
                    viewModel_->getResetCommand()->execute();
                    break;
            }
        }
    };
}

// Example 2: Todo List Application
namespace TodoApp {
    // Model
    struct TodoItem {
        int id;
        std::string title;
        bool completed;
        
        TodoItem(int id, const std::string& title)
            : id(id), title(title), completed(false) {}
            
        bool operator==(const TodoItem& other) const {
            return id == other.id && title == other.title && completed == other.completed;
        }
        
        bool operator!=(const TodoItem& other) const {
            return !(*this == other);
        }
    };
    
    class TodoModel {
    private:
        std::vector<TodoItem> items_;
        int nextId_ = 1;
        
    public:
        std::vector<TodoItem> getItems() const { return items_; }
        
        void addItem(const std::string& title) {
            items_.emplace_back(nextId_++, title);
        }
        
        void toggleItem(int id) {
            auto it = std::find_if(items_.begin(), items_.end(),
                [id](TodoItem& item) { return item.id == id; });
            if (it != items_.end()) {
                it->completed = !it->completed;
            }
        }
        
        void removeItem(int id) {
            items_.erase(
                std::remove_if(items_.begin(), items_.end(),
                    [id](const TodoItem& item) { return item.id == id; }),
                items_.end()
            );
        }
        
        int getActiveCount() const {
            return std::count_if(items_.begin(), items_.end(),
                [](const TodoItem& item) { return !item.completed; });
        }
    };
    
    // ViewModel
    class TodoViewModel : public INotifyPropertyChanged {
    private:
        TodoModel model_;
        ObservableProperty<std::vector<TodoItem>> items_;
        ObservableProperty<std::string> newItemTitle_;
        ObservableProperty<int> activeCount_;
        ObservableProperty<std::string> filter_;
        
        std::unique_ptr<ICommand> addCommand_;
        std::unique_ptr<ICommand> toggleCommand_;
        std::unique_ptr<ICommand> removeCommand_;
        std::unique_ptr<ICommand> setFilterCommand_;
        
        void refreshItems() {
            auto allItems = model_.getItems();
            
            if (filter_.get() == "active") {
                std::vector<TodoItem> filtered;
                std::copy_if(allItems.begin(), allItems.end(), 
                           std::back_inserter(filtered),
                           [](const TodoItem& item) { return !item.completed; });
                items_.set(filtered);
            } else if (filter_.get() == "completed") {
                std::vector<TodoItem> filtered;
                std::copy_if(allItems.begin(), allItems.end(), 
                           std::back_inserter(filtered),
                           [](const TodoItem& item) { return item.completed; });
                items_.set(filtered);
            } else {
                items_.set(allItems);
            }
            
            activeCount_.set(model_.getActiveCount());
        }
        
    public:
        TodoViewModel()
            : items_("Items", this),
              newItemTitle_("NewItemTitle", this, ""),
              activeCount_("ActiveCount", this, 0),
              filter_("Filter", this, "all") {
            
            addCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    if (!newItemTitle_.get().empty()) {
                        model_.addItem(newItemTitle_.get());
                        newItemTitle_.set("");
                        refreshItems();
                    }
                },
                [this]() { return !newItemTitle_.get().empty(); }
            );
            
            toggleCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    // In real app, would get selected item ID
                    // For demo, toggle first item
                    auto items = model_.getItems();
                    if (!items.empty()) {
                        model_.toggleItem(items[0].id);
                        refreshItems();
                    }
                }
            );
            
            removeCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    // In real app, would get selected item ID
                    // For demo, remove first item
                    auto items = model_.getItems();
                    if (!items.empty()) {
                        model_.removeItem(items[0].id);
                        refreshItems();
                    }
                }
            );
            
            setFilterCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    // Cycle through filters
                    if (filter_.get() == "all") {
                        filter_.set("active");
                    } else if (filter_.get() == "active") {
                        filter_.set("completed");
                    } else {
                        filter_.set("all");
                    }
                    refreshItems();
                }
            );
        }
        
        // Properties
        const ObservableProperty<std::vector<TodoItem>>& getItems() const { return items_; }
        ObservableProperty<std::string>& getNewItemTitle() { return newItemTitle_; }
        const ObservableProperty<int>& getActiveCount() const { return activeCount_; }
        const ObservableProperty<std::string>& getFilter() const { return filter_; }
        
        // Commands
        ICommand* getAddCommand() { return addCommand_.get(); }
        ICommand* getToggleCommand() { return toggleCommand_.get(); }
        ICommand* getRemoveCommand() { return removeCommand_.get(); }
        ICommand* getSetFilterCommand() { return setFilterCommand_.get(); }
        
        // Methods
        void toggleItem(int id) {
            model_.toggleItem(id);
            refreshItems();
        }
        
        void removeItem(int id) {
            model_.removeItem(id);
            refreshItems();
        }
    };
    
    // View
    class TodoView {
    private:
        TodoViewModel* viewModel_;
        
    public:
        explicit TodoView(TodoViewModel* vm) : viewModel_(vm) {
            viewModel_->addPropertyChangedHandler(
                [this](const std::string& propertyName) {
                    if (propertyName == "Items" || propertyName == "Filter") {
                        render();
                    }
                }
            );
        }
        
        void render() {
            std::cout << "\n=== Todo List (" << viewModel_->getFilter().get() << ") ===\n";
            
            const auto& items = viewModel_->getItems().get();
            if (items.empty()) {
                std::cout << "No items to display.\n";
            } else {
                for (const auto& item : items) {
                    std::cout << "[" << (item.completed ? "X" : " ") << "] "
                              << item.id << ". " << item.title << "\n";
                }
            }
            
            std::cout << "\nActive items: " << viewModel_->getActiveCount().get() << "\n";
            std::cout << "New item title: " << viewModel_->getNewItemTitle().get() << "\n";
        }
        
        void simulateAddItem(const std::string& title) {
            viewModel_->getNewItemTitle().set(title);
            viewModel_->getAddCommand()->execute();
        }
    };
}

// Example 3: Form Validation
namespace FormValidation {
    // Model
    struct UserRegistration {
        std::string username;
        std::string email;
        std::string password;
        
        bool isValid() const {
            return !username.empty() && 
                   !email.empty() && 
                   email.find('@') != std::string::npos &&
                   password.length() >= 8;
        }
    };
    
    // ViewModel
    class RegistrationViewModel : public INotifyPropertyChanged {
    private:
        UserRegistration model_;
        
        ObservableProperty<std::string> username_;
        ObservableProperty<std::string> email_;
        ObservableProperty<std::string> password_;
        ObservableProperty<std::string> validationMessage_;
        ObservableProperty<bool> isValid_;
        ObservableProperty<bool> isRegistered_;
        
        std::unique_ptr<ICommand> registerCommand_;
        
        void validate() {
            std::string message;
            bool valid = true;
            
            if (username_.get().empty()) {
                message = "Username is required";
                valid = false;
            } else if (email_.get().empty()) {
                message = "Email is required";
                valid = false;
            } else if (email_.get().find('@') == std::string::npos) {
                message = "Invalid email format";
                valid = false;
            } else if (password_.get().length() < 8) {
                message = "Password must be at least 8 characters";
                valid = false;
            } else {
                message = "All fields valid";
            }
            
            validationMessage_.set(message);
            isValid_.set(valid);
        }
        
    public:
        RegistrationViewModel()
            : username_("Username", this, ""),
              email_("Email", this, ""),
              password_("Password", this, ""),
              validationMessage_("ValidationMessage", this, ""),
              isValid_("IsValid", this, false),
              isRegistered_("IsRegistered", this, false) {
            
            // Set up property change handlers for validation
            addPropertyChangedHandler(
                [this](const std::string& propertyName) {
                    if (propertyName == "Username" || 
                        propertyName == "Email" || 
                        propertyName == "Password") {
                        validate();
                    }
                }
            );
            
            registerCommand_ = std::make_unique<RelayCommand>(
                [this]() {
                    if (isValid_.get()) {
                        model_.username = username_.get();
                        model_.email = email_.get();
                        model_.password = password_.get();
                        isRegistered_.set(true);
                        validationMessage_.set("Registration successful!");
                    }
                },
                [this]() { return isValid_.get(); }
            );
        }
        
        // Properties
        ObservableProperty<std::string>& getUsername() { return username_; }
        ObservableProperty<std::string>& getEmail() { return email_; }
        ObservableProperty<std::string>& getPassword() { return password_; }
        const ObservableProperty<std::string>& getValidationMessage() const { 
            return validationMessage_; 
        }
        const ObservableProperty<bool>& getIsValid() const { return isValid_; }
        const ObservableProperty<bool>& getIsRegistered() const { return isRegistered_; }
        
        // Commands
        ICommand* getRegisterCommand() { return registerCommand_.get(); }
    };
    
    // View
    class RegistrationView {
    private:
        RegistrationViewModel* viewModel_;
        
    public:
        explicit RegistrationView(RegistrationViewModel* vm) : viewModel_(vm) {
            viewModel_->addPropertyChangedHandler(
                [this](const std::string& propertyName) {
                    if (propertyName == "ValidationMessage" || 
                        propertyName == "IsRegistered") {
                        showValidation();
                    }
                }
            );
        }
        
        void render() {
            std::cout << "\n=== Registration Form ===\n";
            std::cout << "Username: " << viewModel_->getUsername().get() << "\n";
            std::cout << "Email: " << viewModel_->getEmail().get() << "\n";
            std::cout << "Password: " << std::string(viewModel_->getPassword().get().length(), '*') << "\n";
            showValidation();
            std::cout << "\n[Register] " 
                      << (viewModel_->getIsValid().get() ? "(enabled)" : "(disabled)") << "\n";
        }
        
        void showValidation() {
            const auto& message = viewModel_->getValidationMessage().get();
            if (!message.empty()) {
                std::cout << "\nValidation: " << message << "\n";
            }
            
            if (viewModel_->getIsRegistered().get()) {
                std::cout << "\n*** REGISTRATION COMPLETE ***\n";
            }
        }
        
        void simulateInput(const std::string& username, 
                          const std::string& email, 
                          const std::string& password) {
            viewModel_->getUsername().set(username);
            viewModel_->getEmail().set(email);
            viewModel_->getPassword().set(password);
            render();
        }
    };
}

// Demo
void demonstrateCounter() {
    std::cout << "=== Counter Application Demo ===\n";
    
    CounterApp::CounterViewModel viewModel;
    CounterApp::CounterView view(&viewModel);
    
    view.render();
    
    // Simulate user interactions
    std::cout << "\nIncrementing...\n";
    view.simulateUserInput('+');
    
    std::cout << "\nIncrementing again...\n";
    view.simulateUserInput('+');
    
    std::cout << "\nDecrementing...\n";
    view.simulateUserInput('-');
    
    std::cout << "\nResetting...\n";
    view.simulateUserInput('R');
}

void demonstrateTodoList() {
    std::cout << "\n=== Todo List Demo ===\n";
    
    TodoApp::TodoViewModel viewModel;
    TodoApp::TodoView view(&viewModel);
    
    // Add some items
    view.simulateAddItem("Learn MVVM pattern");
    view.simulateAddItem("Implement data binding");
    view.simulateAddItem("Write unit tests");
    
    // Toggle first item
    std::cout << "\nToggling first item...\n";
    viewModel.toggleItem(1);
    
    // Change filter
    std::cout << "\nChanging filter to 'active'...\n";
    viewModel.getSetFilterCommand()->execute();
}

void demonstrateFormValidation() {
    std::cout << "\n=== Form Validation Demo ===\n";
    
    FormValidation::RegistrationViewModel viewModel;
    FormValidation::RegistrationView view(&viewModel);
    
    // Try invalid inputs
    std::cout << "Testing empty form:\n";
    view.render();
    
    std::cout << "\nTesting partial input:\n";
    view.simulateInput("john", "", "");
    
    std::cout << "\nTesting invalid email:\n";
    view.simulateInput("john", "invalid-email", "pass");
    
    std::cout << "\nTesting short password:\n";
    view.simulateInput("john", "john@example.com", "short");
    
    std::cout << "\nTesting valid input:\n";
    view.simulateInput("john", "john@example.com", "password123");
    
    // Try to register
    if (viewModel.getRegisterCommand()->canExecute()) {
        std::cout << "\nRegistering...\n";
        viewModel.getRegisterCommand()->execute();
        view.render();
    }
}

int main() {
    std::cout << "=== MVVM (Model-View-ViewModel) Pattern Demo ===\n\n";
    
    demonstrateCounter();
    demonstrateTodoList();
    demonstrateFormValidation();
    
    std::cout << "\n=== MVVM Pattern Characteristics ===\n";
    std::cout << "1. Data Binding between View and ViewModel\n";
    std::cout << "2. ViewModel exposes Observable Properties\n";
    std::cout << "3. Commands encapsulate actions\n";
    std::cout << "4. View has no direct Model access\n";
    std::cout << "5. ViewModel is View-agnostic (testable)\n";
    
    return 0;
}