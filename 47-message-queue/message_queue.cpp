// Minimal Message Queue Pattern Implementation
#include <iostream>
#include <queue>
#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <functional>
#include <any>
#include <unordered_map>

// Example 1: Basic Message Queue
namespace BasicMessageQueue {
    template<typename T>
    class MessageQueue {
    private:
        std::queue<T> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        size_t maxSize_;
        std::atomic<bool> closed_{false};
        
    public:
        explicit MessageQueue(size_t maxSize = 1000) : maxSize_(maxSize) {}
        
        // Producer - push message
        bool push(const T& message) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Wait if queue is full
            cv_.wait(lock, [this] { 
                return queue_.size() < maxSize_ || closed_; 
            });
            
            if (closed_) {
                return false;
            }
            
            queue_.push(message);
            std::cout << "Message pushed. Queue size: " << queue_.size() << "\n";
            
            // Notify consumers
            cv_.notify_one();
            return true;
        }
        
        // Consumer - pop message
        bool pop(T& message) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Wait for message
            cv_.wait(lock, [this] { 
                return !queue_.empty() || closed_; 
            });
            
            if (queue_.empty()) {
                return false; // Queue closed
            }
            
            message = queue_.front();
            queue_.pop();
            std::cout << "Message popped. Queue size: " << queue_.size() << "\n";
            
            // Notify producers
            cv_.notify_one();
            return true;
        }
        
        // Try pop with timeout
        bool tryPop(T& message, std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            if (!cv_.wait_for(lock, timeout, [this] { 
                return !queue_.empty() || closed_; 
            })) {
                return false; // Timeout
            }
            
            if (queue_.empty()) {
                return false;
            }
            
            message = queue_.front();
            queue_.pop();
            cv_.notify_one();
            return true;
        }
        
        void close() {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
            cv_.notify_all();
        }
        
        size_t size() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }
        
        bool isClosed() const {
            return closed_;
        }
    };
    
    // Message type
    struct Message {
        int id;
        std::string content;
        std::chrono::system_clock::time_point timestamp;
        
        Message(int id, const std::string& content)
            : id(id), content(content), 
              timestamp(std::chrono::system_clock::now()) {}
    };
}

// Example 2: Priority Message Queue
namespace PriorityMessageQueue {
    template<typename T>
    class PriorityQueue {
    private:
        std::priority_queue<T> queue_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        size_t maxSize_;
        std::atomic<bool> closed_{false};
        
    public:
        explicit PriorityQueue(size_t maxSize = 1000) : maxSize_(maxSize) {}
        
        bool push(const T& message) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            cv_.wait(lock, [this] { 
                return queue_.size() < maxSize_ || closed_; 
            });
            
            if (closed_) return false;
            
            queue_.push(message);
            cv_.notify_one();
            return true;
        }
        
        bool pop(T& message) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            cv_.wait(lock, [this] { 
                return !queue_.empty() || closed_; 
            });
            
            if (queue_.empty()) return false;
            
            message = queue_.top();
            queue_.pop();
            cv_.notify_one();
            return true;
        }
        
        void close() {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
            cv_.notify_all();
        }
        
        size_t size() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }
    };
    
    // Priority message
    struct PriorityMessage {
        enum Priority { LOW = 0, NORMAL = 1, HIGH = 2, URGENT = 3 };
        
        Priority priority;
        std::string content;
        int sequenceNumber;
        
        PriorityMessage(Priority p, const std::string& c, int seq)
            : priority(p), content(c), sequenceNumber(seq) {}
        
        // Higher priority first, then by sequence
        bool operator<(const PriorityMessage& other) const {
            if (priority != other.priority) {
                return priority < other.priority;
            }
            return sequenceNumber > other.sequenceNumber;
        }
    };
}

// Example 3: Topic-Based Message Queue
namespace TopicMessageQueue {
    class MessageBroker {
    private:
        struct TopicQueue {
            std::queue<std::any> messages;
            std::vector<std::function<void(const std::any&)>> subscribers;
        };
        
        std::unordered_map<std::string, TopicQueue> topics_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic<bool> running_{true};
        std::thread dispatcherThread_;
        
        void dispatcherLoop() {
            while (running_) {
                std::unique_lock<std::mutex> lock(mutex_);
                
                cv_.wait(lock, [this] {
                    for (const auto& [topic, queue] : topics_) {
                        if (!queue.messages.empty()) return true;
                    }
                    return !running_;
                });
                
                if (!running_) break;
                
                // Process all topics
                for (auto& [topic, topicQueue] : topics_) {
                    while (!topicQueue.messages.empty()) {
                        auto message = topicQueue.messages.front();
                        topicQueue.messages.pop();
                        
                        // Deliver to all subscribers
                        for (const auto& subscriber : topicQueue.subscribers) {
                            subscriber(message);
                        }
                    }
                }
            }
        }
        
    public:
        MessageBroker() : dispatcherThread_(&MessageBroker::dispatcherLoop, this) {}
        
        ~MessageBroker() {
            stop();
            if (dispatcherThread_.joinable()) {
                dispatcherThread_.join();
            }
        }
        
        // Publish to topic
        template<typename T>
        void publish(const std::string& topic, const T& message) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            topics_[topic].messages.push(message);
            std::cout << "Published to topic '" << topic 
                      << "'. Queue size: " << topics_[topic].messages.size() << "\n";
            
            cv_.notify_one();
        }
        
        // Subscribe to topic
        template<typename T>
        void subscribe(const std::string& topic, 
                      std::function<void(const T&)> handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            topics_[topic].subscribers.push_back(
                [handler](const std::any& msg) {
                    try {
                        handler(std::any_cast<T>(msg));
                    } catch (const std::bad_any_cast&) {
                        // Type mismatch - ignore
                    }
                }
            );
            
            std::cout << "Subscriber added to topic '" << topic << "'\n";
        }
        
        void stop() {
            running_ = false;
            cv_.notify_all();
        }
    };
}

// Example 4: Dead Letter Queue
namespace DeadLetterQueue {
    template<typename T>
    class ReliableQueue {
    private:
        struct EnvelopedMessage {
            T message;
            int retryCount = 0;
            std::chrono::system_clock::time_point firstAttempt;
        };
        
        std::queue<EnvelopedMessage> mainQueue_;
        std::queue<T> deadLetterQueue_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        int maxRetries_;
        std::atomic<bool> closed_{false};
        
    public:
        explicit ReliableQueue(int maxRetries = 3) : maxRetries_(maxRetries) {}
        
        void push(const T& message) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            mainQueue_.push({message, 0, std::chrono::system_clock::now()});
            cv_.notify_one();
        }
        
        // Process with acknowledgment
        bool process(std::function<bool(const T&)> processor) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            cv_.wait(lock, [this] { 
                return !mainQueue_.empty() || closed_; 
            });
            
            if (mainQueue_.empty()) return false;
            
            auto envelope = mainQueue_.front();
            mainQueue_.pop();
            
            lock.unlock();
            
            // Try to process
            bool success = false;
            try {
                success = processor(envelope.message);
            } catch (const std::exception& e) {
                std::cout << "Processing failed: " << e.what() << "\n";
            }
            
            lock.lock();
            
            if (!success) {
                envelope.retryCount++;
                
                if (envelope.retryCount >= maxRetries_) {
                    // Move to dead letter queue
                    deadLetterQueue_.push(envelope.message);
                    std::cout << "Message moved to dead letter queue after " 
                              << envelope.retryCount << " retries\n";
                } else {
                    // Retry
                    mainQueue_.push(envelope);
                    std::cout << "Message requeued. Retry " 
                              << envelope.retryCount << "/" << maxRetries_ << "\n";
                }
            }
            
            return true;
        }
        
        std::vector<T> getDeadLetters() {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<T> result;
            
            while (!deadLetterQueue_.empty()) {
                result.push_back(deadLetterQueue_.front());
                deadLetterQueue_.pop();
            }
            
            return result;
        }
        
        void close() {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
            cv_.notify_all();
        }
    };
}

// Example 5: Pub-Sub Queue with Acknowledgment
namespace PubSubQueue {
    class MessageQueue {
    private:
        struct Message {
            std::string id;
            std::any data;
            std::chrono::system_clock::time_point timestamp;
            std::unordered_map<std::string, bool> acknowledgments;
        };
        
        std::queue<std::shared_ptr<Message>> queue_;
        std::unordered_map<std::string, std::shared_ptr<Message>> inFlight_;
        std::vector<std::string> subscribers_;
        mutable std::mutex mutex_;
        std::condition_variable cv_;
        int messageIdCounter_ = 1;
        
    public:
        std::string registerSubscriber(const std::string& name) {
            std::lock_guard<std::mutex> lock(mutex_);
            subscribers_.push_back(name);
            std::cout << "Subscriber '" << name << "' registered\n";
            return name;
        }
        
        template<typename T>
        std::string publish(const T& data) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto message = std::make_shared<Message>();
            message->id = "MSG" + std::to_string(messageIdCounter_++);
            message->data = data;
            message->timestamp = std::chrono::system_clock::now();
            
            // Initialize acknowledgments
            for (const auto& subscriber : subscribers_) {
                message->acknowledgments[subscriber] = false;
            }
            
            queue_.push(message);
            std::cout << "Published message " << message->id << "\n";
            
            cv_.notify_all();
            return message->id;
        }
        
        std::pair<std::string, std::any> consume(const std::string& subscriber) {
            std::unique_lock<std::mutex> lock(mutex_);
            
            cv_.wait(lock, [this] { return !queue_.empty(); });
            
            auto message = queue_.front();
            
            // Move to in-flight if first consumer
            if (inFlight_.find(message->id) == inFlight_.end()) {
                queue_.pop();
                inFlight_[message->id] = message;
            }
            
            return {message->id, message->data};
        }
        
        void acknowledge(const std::string& messageId, const std::string& subscriber) {
            std::lock_guard<std::mutex> lock(mutex_);
            
            auto it = inFlight_.find(messageId);
            if (it != inFlight_.end()) {
                it->second->acknowledgments[subscriber] = true;
                
                // Check if all acknowledged
                bool allAcked = true;
                for (const auto& [sub, acked] : it->second->acknowledgments) {
                    if (!acked) {
                        allAcked = false;
                        break;
                    }
                }
                
                if (allAcked) {
                    std::cout << "Message " << messageId 
                              << " fully acknowledged and removed\n";
                    inFlight_.erase(it);
                } else {
                    std::cout << "Message " << messageId 
                              << " acknowledged by " << subscriber << "\n";
                }
            }
        }
        
        void printStatus() const {
            std::lock_guard<std::mutex> lock(mutex_);
            std::cout << "\nQueue Status:\n";
            std::cout << "  Pending: " << queue_.size() << "\n";
            std::cout << "  In-flight: " << inFlight_.size() << "\n";
            std::cout << "  Subscribers: " << subscribers_.size() << "\n";
        }
    };
}

// Demo functions
void demonstrateBasicMessageQueue() {
    using namespace BasicMessageQueue;
    
    std::cout << "=== Basic Message Queue ===\n";
    
    MessageQueue<Message> queue(5); // Max 5 messages
    
    // Producer thread
    std::thread producer([&queue]() {
        for (int i = 1; i <= 7; ++i) {
            Message msg(i, "Message " + std::to_string(i));
            if (queue.push(msg)) {
                std::cout << "Producer: Sent message " << i << "\n";
            } else {
                std::cout << "Producer: Queue closed\n";
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // Consumer thread
    std::thread consumer([&queue]() {
        Message msg(0, "");
        while (queue.pop(msg)) {
            std::cout << "Consumer: Received message " << msg.id 
                      << ": " << msg.content << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::cout << "Consumer: Queue closed\n";
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    queue.close();
    
    producer.join();
    consumer.join();
}

void demonstratePriorityMessageQueue() {
    using namespace PriorityMessageQueue;
    
    std::cout << "\n=== Priority Message Queue ===\n";
    
    PriorityQueue<PriorityMessage> queue;
    
    // Push messages with different priorities
    int seq = 0;
    queue.push(PriorityMessage(PriorityMessage::LOW, "Low priority task", seq++));
    queue.push(PriorityMessage(PriorityMessage::URGENT, "Urgent task", seq++));
    queue.push(PriorityMessage(PriorityMessage::NORMAL, "Normal task", seq++));
    queue.push(PriorityMessage(PriorityMessage::HIGH, "High priority task", seq++));
    queue.push(PriorityMessage(PriorityMessage::URGENT, "Another urgent task", seq++));
    
    // Process in priority order
    PriorityMessage msg(PriorityMessage::LOW, "", 0);
    while (queue.pop(msg)) {
        std::cout << "Processing [Priority " << msg.priority << "]: " 
                  << msg.content << "\n";
    }
}

void demonstrateTopicMessageQueue() {
    using namespace TopicMessageQueue;
    
    std::cout << "\n=== Topic-Based Message Queue ===\n";
    
    MessageBroker broker;
    
    // Subscribe to topics
    broker.subscribe<std::string>("logs", 
        [](const std::string& log) {
            std::cout << "  Log handler: " << log << "\n";
        });
    
    broker.subscribe<int>("metrics", 
        [](const int& value) {
            std::cout << "  Metric handler: " << value << "\n";
        });
    
    broker.subscribe<std::string>("alerts", 
        [](const std::string& alert) {
            std::cout << "  Alert handler: " << alert << "\n";
        });
    
    // Publish messages
    broker.publish("logs", std::string("Application started"));
    broker.publish("metrics", 42);
    broker.publish("alerts", std::string("High CPU usage"));
    broker.publish("logs", std::string("User logged in"));
    broker.publish("metrics", 85);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void demonstrateDeadLetterQueue() {
    using namespace DeadLetterQueue;
    
    std::cout << "\n=== Dead Letter Queue ===\n";
    
    ReliableQueue<std::string> queue(3); // Max 3 retries
    
    // Push some messages
    queue.push("Good message");
    queue.push("Bad message");
    queue.push("Another good message");
    
    // Process with simulated failures
    int processCount = 0;
    auto processor = [&processCount](const std::string& msg) -> bool {
        processCount++;
        std::cout << "Processing: " << msg << "\n";
        
        if (msg == "Bad message") {
            std::cout << "  Failed to process!\n";
            return false;
        }
        
        std::cout << "  Successfully processed\n";
        return true;
    };
    
    // Process messages
    for (int i = 0; i < 10; ++i) {
        if (!queue.process(processor)) {
            break;
        }
    }
    
    // Check dead letters
    auto deadLetters = queue.getDeadLetters();
    std::cout << "\nDead Letter Queue contains:\n";
    for (const auto& letter : deadLetters) {
        std::cout << "  " << letter << "\n";
    }
}

void demonstratePubSubQueue() {
    using namespace PubSubQueue;
    
    std::cout << "\n=== Pub-Sub Queue with Acknowledgment ===\n";
    
    MessageQueue queue;
    
    // Register subscribers
    auto sub1 = queue.registerSubscriber("Worker1");
    auto sub2 = queue.registerSubscriber("Worker2");
    
    // Publish messages
    auto msgId1 = queue.publish(std::string("Task 1"));
    auto msgId2 = queue.publish(std::string("Task 2"));
    
    // Simulate consumption
    auto [id1, data1] = queue.consume(sub1);
    std::cout << sub1 << " consumed: " << std::any_cast<std::string>(data1) << "\n";
    
    auto [id2, data2] = queue.consume(sub2);
    std::cout << sub2 << " consumed: " << std::any_cast<std::string>(data2) << "\n";
    
    // Acknowledge
    queue.acknowledge(id1, sub1);
    queue.acknowledge(id1, sub2);
    
    queue.printStatus();
}

int main() {
    std::cout << "=== Message Queue Pattern Demo ===\n\n";
    
    demonstrateBasicMessageQueue();
    demonstratePriorityMessageQueue();
    demonstrateTopicMessageQueue();
    demonstrateDeadLetterQueue();
    demonstratePubSubQueue();
    
    std::cout << "\n=== Message Queue Benefits ===\n";
    std::cout << "1. Decouples producers and consumers\n";
    std::cout << "2. Handles load spikes\n";
    std::cout << "3. Enables async processing\n";
    std::cout << "4. Provides reliability\n";
    std::cout << "5. Supports various messaging patterns\n";
    
    return 0;
}