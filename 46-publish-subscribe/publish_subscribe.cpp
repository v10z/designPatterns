// Minimal Publish-Subscribe Pattern Implementation
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <any>
#include <typeindex>
#include <mutex>
#include <queue>
#include <algorithm>

// Example 1: Basic Publish-Subscribe System
namespace BasicPubSub {
    template<typename EventType>
    class EventBus {
    private:
        using Handler = std::function<void(const EventType&)>;
        using HandlerId = size_t;
        
        struct Subscription {
            HandlerId id;
            Handler handler;
        };
        
        std::unordered_map<std::type_index, std::vector<Subscription>> handlers_;
        HandlerId nextId_ = 1;
        mutable std::mutex mutex_;
        
    public:
        // Subscribe to specific event type
        template<typename T>
        HandlerId subscribe(std::function<void(const T&)> handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            HandlerId id = nextId_++;
            auto typeIndex = std::type_index(typeid(T));
            
            handlers_[typeIndex].push_back({
                id,
                [handler](const EventType& event) {
                    handler(std::any_cast<const T&>(event));
                }
            });
            
            std::cout << "Subscriber " << id << " registered for " 
                      << typeIndex.name() << "\n";
            return id;
        }
        
        // Unsubscribe by ID
        void unsubscribe(HandlerId id) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            for (auto& [type, subs] : handlers_) {
                subs.erase(
                    std::remove_if(subs.begin(), subs.end(),
                        [id](const Subscription& sub) { return sub.id == id; }),
                    subs.end()
                );
            }
            
            std::cout << "Subscriber " << id << " unregistered\n";
        }
        
        // Publish event
        template<typename T>
        void publish(const T& event) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto typeIndex = std::type_index(typeid(T));
            auto it = handlers_.find(typeIndex);
            
            if (it != handlers_.end()) {
                std::cout << "Publishing " << typeIndex.name() 
                          << " to " << it->second.size() << " subscribers\n";
                
                EventType wrappedEvent = event;
                for (const auto& sub : it->second) {
                    sub.handler(wrappedEvent);
                }
            }
        }
    };
    
    // Event types
    struct UserLoginEvent {
        std::string username;
        std::string ipAddress;
        
        UserLoginEvent(const std::string& user, const std::string& ip)
            : username(user), ipAddress(ip) {}
    };
    
    struct UserLogoutEvent {
        std::string username;
        
        explicit UserLogoutEvent(const std::string& user)
            : username(user) {}
    };
    
    struct MessageEvent {
        std::string sender;
        std::string content;
        
        MessageEvent(const std::string& from, const std::string& msg)
            : sender(from), content(msg) {}
    };
}

// Example 2: Topic-Based Publish-Subscribe
namespace TopicBasedPubSub {
    class MessageBroker {
    private:
        using MessageHandler = std::function<void(const std::string&, const std::any&)>;
        
        struct Subscriber {
            std::string id;
            MessageHandler handler;
            bool active = true;
        };
        
        std::unordered_map<std::string, std::vector<std::shared_ptr<Subscriber>>> topics_;
        mutable std::mutex mutex_;
        
    public:
        // Subscribe to topic
        std::shared_ptr<Subscriber> subscribe(const std::string& topic, 
                                             const std::string& subscriberId,
                                             MessageHandler handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto subscriber = std::make_shared<Subscriber>();
            subscriber->id = subscriberId;
            subscriber->handler = handler;
            
            topics_[topic].push_back(subscriber);
            
            std::cout << "Subscriber '" << subscriberId 
                      << "' subscribed to topic '" << topic << "'\n";
            
            return subscriber;
        }
        
        // Unsubscribe from topic
        void unsubscribe(const std::string& topic, const std::string& subscriberId) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto it = topics_.find(topic);
            if (it != topics_.end()) {
                it->second.erase(
                    std::remove_if(it->second.begin(), it->second.end(),
                        [&subscriberId](const std::shared_ptr<Subscriber>& sub) {
                            return sub->id == subscriberId;
                        }),
                    it->second.end()
                );
                
                std::cout << "Subscriber '" << subscriberId 
                          << "' unsubscribed from topic '" << topic << "'\n";
            }
        }
        
        // Publish to topic
        template<typename T>
        void publish(const std::string& topic, const T& message) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto it = topics_.find(topic);
            if (it != topics_.end()) {
                std::cout << "Publishing to topic '" << topic 
                          << "' (" << it->second.size() << " subscribers)\n";
                
                std::any wrappedMessage = message;
                for (const auto& subscriber : it->second) {
                    if (subscriber->active) {
                        subscriber->handler(topic, wrappedMessage);
                    }
                }
            } else {
                std::cout << "No subscribers for topic '" << topic << "'\n";
            }
        }
        
        // Get topic statistics
        void printStats() const {
            std::lock_guard<std::mutex> lock(mutex_);
            
            std::cout << "\nBroker Statistics:\n";
            for (const auto& [topic, subscribers] : topics_) {
                std::cout << "  Topic '" << topic << "': " 
                          << subscribers.size() << " subscribers\n";
            }
        }
    };
    
    // Message types
    struct StockPrice {
        std::string symbol;
        double price;
        
        StockPrice(const std::string& sym, double p) 
            : symbol(sym), price(p) {}
    };
    
    struct NewsAlert {
        std::string headline;
        std::string category;
        
        NewsAlert(const std::string& head, const std::string& cat)
            : headline(head), category(cat) {}
    };
}

// Example 3: Filtered Publish-Subscribe
namespace FilteredPubSub {
    template<typename EventType>
    class FilteredEventBus {
    private:
        using Filter = std::function<bool(const EventType&)>;
        using Handler = std::function<void(const EventType&)>;
        
        struct Subscription {
            std::string id;
            Filter filter;
            Handler handler;
            int priority;
        };
        
        std::vector<Subscription> subscriptions_;
        mutable std::mutex mutex_;
        
    public:
        // Subscribe with filter
        void subscribe(const std::string& id, 
                      Filter filter, 
                      Handler handler,
                      int priority = 0) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            subscriptions_.push_back({id, filter, handler, priority});
            
            // Sort by priority
            std::sort(subscriptions_.begin(), subscriptions_.end(),
                [](const Subscription& a, const Subscription& b) {
                    return a.priority > b.priority;
                });
            
            std::cout << "Subscriber '" << id << "' registered with priority " 
                      << priority << "\n";
        }
        
        // Publish event
        void publish(const EventType& event) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            int matchCount = 0;
            for (const auto& sub : subscriptions_) {
                if (sub.filter(event)) {
                    sub.handler(event);
                    matchCount++;
                }
            }
            
            std::cout << "Event delivered to " << matchCount 
                      << " matching subscribers\n";
        }
        
        // Remove subscriber
        void unsubscribe(const std::string& id) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            subscriptions_.erase(
                std::remove_if(subscriptions_.begin(), subscriptions_.end(),
                    [&id](const Subscription& sub) { return sub.id == id; }),
                subscriptions_.end()
            );
        }
    };
    
    // Event type
    struct LogEvent {
        enum Level { DEBUG, INFO, WARNING, ERROR };
        
        Level level;
        std::string source;
        std::string message;
        
        LogEvent(Level lvl, const std::string& src, const std::string& msg)
            : level(lvl), source(src), message(msg) {}
    };
}

// Example 4: Weak Reference Publish-Subscribe
namespace WeakPubSub {
    class Publisher {
    private:
        using Handler = std::function<void(const std::string&)>;
        
        struct Subscriber {
            std::weak_ptr<void> owner;
            Handler handler;
        };
        
        std::vector<Subscriber> subscribers_;
        mutable std::mutex mutex_;
        
        void cleanupExpired() {
            subscribers_.erase(
                std::remove_if(subscribers_.begin(), subscribers_.end(),
                    [](const Subscriber& sub) { return sub.owner.expired(); }),
                subscribers_.end()
            );
        }
        
    public:
        // Subscribe with weak reference to owner
        void subscribe(std::weak_ptr<void> owner, Handler handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            cleanupExpired();
            subscribers_.push_back({owner, handler});
            
            std::cout << "New subscriber added (total: " 
                      << subscribers_.size() << ")\n";
        }
        
        // Publish message
        void publish(const std::string& message) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            cleanupExpired();
            
            std::cout << "Publishing: " << message << "\n";
            for (const auto& sub : subscribers_) {
                if (auto locked = sub.owner.lock()) {
                    sub.handler(message);
                }
            }
        }
        
        size_t getSubscriberCount() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return subscribers_.size();
        }
    };
    
    // Subscriber class
    class Observer : public std::enable_shared_from_this<Observer> {
    private:
        std::string name_;
        
    public:
        explicit Observer(const std::string& name) : name_(name) {}
        
        void subscribeToPublisher(Publisher& publisher) {
            publisher.subscribe(
                shared_from_this(),
                [this](const std::string& message) {
                    handleMessage(message);
                }
            );
        }
        
        void handleMessage(const std::string& message) {
            std::cout << "  " << name_ << " received: " << message << "\n";
        }
    };
}

// Example 5: Priority Queue Publish-Subscribe
namespace PriorityPubSub {
    class PriorityEventQueue {
    private:
        struct Event {
            int priority;
            std::any data;
            std::chrono::steady_clock::time_point timestamp;
            
            bool operator<(const Event& other) const {
                return priority < other.priority; // Higher priority first
            }
        };
        
        using Handler = std::function<void(const std::any&)>;
        
        std::priority_queue<Event> eventQueue_;
        std::unordered_map<std::type_index, std::vector<Handler>> handlers_;
        std::mutex queueMutex_;
        std::mutex handlerMutex_;
        bool processing_ = false;
        
    public:
        // Subscribe to event type
        template<typename T>
        void subscribe(std::function<void(const T&)> handler) {
            std::lock_guard<std::mutex> lock(handlerMutex_);
            
            auto typeIndex = std::type_index(typeid(T));
            handlers_[typeIndex].push_back(
                [handler](const std::any& data) {
                    handler(std::any_cast<const T&>(data));
                }
            );
        }
        
        // Publish with priority
        template<typename T>
        void publish(const T& data, int priority = 0) {
            {
                std::lock_guard<std::mutex> lock(queueMutex_);
                eventQueue_.push({
                    priority, 
                    data, 
                    std::chrono::steady_clock::now()
                });
            }
            
            if (!processing_) {
                processEvents();
            }
        }
        
        // Process queued events
        void processEvents() {
            processing_ = true;
            
            while (true) {
                Event event;
                
                {
                    std::lock_guard<std::mutex> lock(queueMutex_);
                    if (eventQueue_.empty()) {
                        break;
                    }
                    event = eventQueue_.top();
                    eventQueue_.pop();
                }
                
                // Dispatch event
                std::lock_guard<std::mutex> lock(handlerMutex_);
                auto typeIndex = std::type_index(event.data.type());
                auto it = handlers_.find(typeIndex);
                
                if (it != handlers_.end()) {
                    for (const auto& handler : it->second) {
                        handler(event.data);
                    }
                }
            }
            
            processing_ = false;
        }
    };
}

// Demo functions
void demonstrateBasicPubSub() {
    using namespace BasicPubSub;
    
    std::cout << "=== Basic Publish-Subscribe ===\n";
    
    EventBus<std::any> eventBus;
    
    // Subscribe to different event types
    auto loginHandler = eventBus.subscribe<UserLoginEvent>(
        [](const UserLoginEvent& event) {
            std::cout << "  Login handler: User '" << event.username 
                      << "' logged in from " << event.ipAddress << "\n";
        }
    );
    
    auto securityHandler = eventBus.subscribe<UserLoginEvent>(
        [](const UserLoginEvent& event) {
            std::cout << "  Security handler: Checking IP " << event.ipAddress << "\n";
        }
    );
    
    auto logoutHandler = eventBus.subscribe<UserLogoutEvent>(
        [](const UserLogoutEvent& event) {
            std::cout << "  Logout handler: User '" << event.username 
                      << "' logged out\n";
        }
    );
    
    auto messageHandler = eventBus.subscribe<MessageEvent>(
        [](const MessageEvent& event) {
            std::cout << "  Message from " << event.sender 
                      << ": " << event.content << "\n";
        }
    );
    
    // Publish events
    eventBus.publish(UserLoginEvent("alice", "192.168.1.100"));
    eventBus.publish(MessageEvent("alice", "Hello everyone!"));
    eventBus.publish(UserLogoutEvent("alice"));
    
    // Unsubscribe
    eventBus.unsubscribe(securityHandler);
    
    std::cout << "\nAfter unsubscribing security handler:\n";
    eventBus.publish(UserLoginEvent("bob", "192.168.1.101"));
}

void demonstrateTopicBasedPubSub() {
    using namespace TopicBasedPubSub;
    
    std::cout << "\n=== Topic-Based Publish-Subscribe ===\n";
    
    MessageBroker broker;
    
    // Stock price subscriber
    auto stockSub = broker.subscribe("stocks", "trader1",
        [](const std::string& topic, const std::any& data) {
            try {
                auto price = std::any_cast<StockPrice>(data);
                std::cout << "  Trader1: " << price.symbol 
                          << " = $" << price.price << "\n";
            } catch (...) {}
        }
    );
    
    // News subscriber
    broker.subscribe("news", "newsReader",
        [](const std::string& topic, const std::any& data) {
            try {
                auto alert = std::any_cast<NewsAlert>(data);
                std::cout << "  NewsReader: [" << alert.category << "] " 
                          << alert.headline << "\n";
            } catch (...) {}
        }
    );
    
    // Multi-topic subscriber
    broker.subscribe("stocks", "analyst",
        [](const std::string& topic, const std::any& data) {
            std::cout << "  Analyst received data on topic: " << topic << "\n";
        }
    );
    
    broker.subscribe("news", "analyst",
        [](const std::string& topic, const std::any& data) {
            std::cout << "  Analyst received data on topic: " << topic << "\n";
        }
    );
    
    // Publish to topics
    broker.publish("stocks", StockPrice("AAPL", 150.25));
    broker.publish("stocks", StockPrice("GOOGL", 2750.80));
    broker.publish("news", NewsAlert("Tech stocks rally", "Finance"));
    broker.publish("weather", "Sunny"); // No subscribers
    
    broker.printStats();
}

void demonstrateFilteredPubSub() {
    using namespace FilteredPubSub;
    
    std::cout << "\n=== Filtered Publish-Subscribe ===\n";
    
    FilteredEventBus<LogEvent> logBus;
    
    // Subscribe to errors only
    logBus.subscribe("errorHandler",
        [](const LogEvent& e) { return e.level == LogEvent::ERROR; },
        [](const LogEvent& e) {
            std::cout << "  ERROR Handler: " << e.message << "\n";
        },
        10  // High priority
    );
    
    // Subscribe to warnings and errors
    logBus.subscribe("warningHandler",
        [](const LogEvent& e) { 
            return e.level >= LogEvent::WARNING; 
        },
        [](const LogEvent& e) {
            std::cout << "  Warning+ Handler: " << e.message << "\n";
        },
        5
    );
    
    // Subscribe to specific source
    logBus.subscribe("databaseLogger",
        [](const LogEvent& e) { return e.source == "Database"; },
        [](const LogEvent& e) {
            std::cout << "  DB Logger: [" << e.level << "] " << e.message << "\n";
        }
    );
    
    // Publish events
    logBus.publish(LogEvent(LogEvent::INFO, "App", "Application started"));
    logBus.publish(LogEvent(LogEvent::WARNING, "Network", "High latency detected"));
    logBus.publish(LogEvent(LogEvent::ERROR, "Database", "Connection failed"));
    logBus.publish(LogEvent(LogEvent::DEBUG, "Database", "Query executed"));
}

void demonstrateWeakPubSub() {
    using namespace WeakPubSub;
    
    std::cout << "\n=== Weak Reference Publish-Subscribe ===\n";
    
    Publisher publisher;
    
    // Create subscribers
    {
        auto observer1 = std::make_shared<Observer>("Observer1");
        auto observer2 = std::make_shared<Observer>("Observer2");
        
        observer1->subscribeToPublisher(publisher);
        observer2->subscribeToPublisher(publisher);
        
        std::cout << "Active subscribers: " << publisher.getSubscriberCount() << "\n";
        
        publisher.publish("First message");
        
        // observer2 goes out of scope
    }
    
    std::cout << "\nAfter observer2 destroyed:\n";
    std::cout << "Active subscribers: " << publisher.getSubscriberCount() << "\n";
    publisher.publish("Second message");
}

int main() {
    std::cout << "=== Publish-Subscribe Pattern Demo ===\n\n";
    
    demonstrateBasicPubSub();
    demonstrateTopicBasedPubSub();
    demonstrateFilteredPubSub();
    demonstrateWeakPubSub();
    
    std::cout << "\n=== Publish-Subscribe Benefits ===\n";
    std::cout << "1. Loose coupling between publishers and subscribers\n";
    std::cout << "2. Dynamic subscription/unsubscription\n";
    std::cout << "3. Multiple subscribers per event\n";
    std::cout << "4. Filtered/selective subscription\n";
    std::cout << "5. Asynchronous communication\n";
    
    return 0;
}