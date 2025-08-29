// Proxy Pattern - Remote HPC Cluster Resource Manager
// Manages access to remote computational resources and large datasets
#include <iostream>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include <sstream>

// Subject interface - Computational resource
class ComputationalResource {
public:
    virtual ~ComputationalResource() = default;
    virtual void submitJob(const std::string& jobScript, int cores, double memory) = 0;
    virtual std::string getStatus() = 0;
    virtual double getUsage() = 0;
    virtual void retrieveResults(const std::string& jobId) = 0;
};

// Real subject - Remote HPC cluster
class RemoteHPCCluster : public ComputationalResource {
private:
    std::string clusterName_;
    std::string hostname_;
    int totalCores_;
    double totalMemory_; // in TB
    double currentLoad_;
    std::unordered_map<std::string, std::string> jobResults_;
    
    void establishConnection() {
        std::cout << "Establishing SSH connection to " << hostname_ << "...\n";
        std::cout << "  Authenticating with Kerberos...\n";
        std::cout << "  Loading environment modules...\n";
        
        // Simulate connection establishment
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        std::cout << "  Connected to " << clusterName_ << " (" 
                  << totalCores_ << " cores, " << totalMemory_ << " TB RAM)\n";
    }
    
    std::string generateJobId() {
        static int jobCounter = 1000;
        return "JOB_" + std::to_string(jobCounter++);
    }
    
public:
    RemoteHPCCluster(const std::string& name, const std::string& host, 
                     int cores, double memory)
        : clusterName_(name), hostname_(host), 
          totalCores_(cores), totalMemory_(memory), currentLoad_(0.3) {
        establishConnection();
    }
    
    void submitJob(const std::string& jobScript, int cores, double memory) override {
        std::string jobId = generateJobId();
        std::cout << "\nSubmitting job to " << clusterName_ << ":\n";
        std::cout << "  Job ID: " << jobId << "\n";
        std::cout << "  Requested: " << cores << " cores, " 
                  << std::fixed << std::setprecision(2) << memory << " GB RAM\n";
        std::cout << "  Script: " << jobScript << "\n";
        std::cout << "  Transferring input files...\n";
        std::cout << "  Job queued in partition 'gpu-v100'\n";
        
        // Simulate job execution
        currentLoad_ += cores / (double)totalCores_;
        jobResults_[jobId] = "Simulation completed: 1.2M timesteps, convergence achieved";
    }
    
    std::string getStatus() override {
        std::stringstream ss;
        ss << "Cluster: " << clusterName_ << " @ " << hostname_ << "\n";
        ss << "Total Resources: " << totalCores_ << " cores, " 
           << std::fixed << std::setprecision(2) << totalMemory_ << " TB\n";
        ss << "Current Load: " << std::fixed << std::setprecision(1) 
           << (currentLoad_ * 100) << "%\n";
        ss << "Queue: 42 jobs pending, 18 running";
        return ss.str();
    }
    
    double getUsage() override {
        return currentLoad_;
    }
    
    void retrieveResults(const std::string& jobId) override {
        std::cout << "\nRetrieving results for " << jobId << ":\n";
        std::cout << "  Downloading output files via GridFTP...\n";
        std::cout << "  Transferring 2.4 GB of simulation data...\n";
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        
        auto it = jobResults_.find(jobId);
        if (it != jobResults_.end()) {
            std::cout << "  Results: " << it->second << "\n";
            std::cout << "  Files saved to: /scratch/results/" << jobId << "/\n";
        }
    }
};

// Virtual proxy - Lazy connection to HPC resources
class HPCResourceProxy : public ComputationalResource {
private:
    std::string clusterName_;
    std::string hostname_;
    int totalCores_;
    double totalMemory_;
    mutable std::unique_ptr<RemoteHPCCluster> realCluster_;
    
    void ensureConnected() const {
        if (!realCluster_) {
            std::cout << "\nProxy: Establishing connection on first use...\n";
            realCluster_ = std::make_unique<RemoteHPCCluster>(
                clusterName_, hostname_, totalCores_, totalMemory_);
        }
    }
    
public:
    HPCResourceProxy(const std::string& name, const std::string& host,
                     int cores, double memory)
        : clusterName_(name), hostname_(host), 
          totalCores_(cores), totalMemory_(memory) {
        std::cout << "Resource proxy created for: " << clusterName_ 
                  << " (connection deferred)\n";
    }
    
    void submitJob(const std::string& jobScript, int cores, double memory) override {
        ensureConnected();
        realCluster_->submitJob(jobScript, cores, memory);
    }
    
    std::string getStatus() override {
        ensureConnected();
        return realCluster_->getStatus();
    }
    
    double getUsage() override {
        ensureConnected();
        return realCluster_->getUsage();
    }
    
    void retrieveResults(const std::string& jobId) override {
        ensureConnected();
        realCluster_->retrieveResults(jobId);
    }
};

// Protection proxy - Access control and quota management
class SecureHPCProxy : public ComputationalResource {
private:
    std::unique_ptr<ComputationalResource> resource_;
    std::string currentUser_;
    
    struct UserQuota {
        int maxCores;
        double maxMemory;  // GB
        double cpuHoursUsed;
        double cpuHoursLimit;
        bool hasAccess;
    };
    
    std::unordered_map<std::string, UserQuota> userQuotas_;
    
    bool checkQuota(int requestedCores, double requestedMemory) const {
        auto it = userQuotas_.find(currentUser_);
        if (it == userQuotas_.end() || !it->second.hasAccess) {
            return false;
        }
        
        const auto& quota = it->second;
        return requestedCores <= quota.maxCores && 
               requestedMemory <= quota.maxMemory &&
               quota.cpuHoursUsed < quota.cpuHoursLimit;
    }
    
public:
    SecureHPCProxy(const std::string& name, const std::string& host,
                   int cores, double memory)
        : resource_(std::make_unique<HPCResourceProxy>(name, host, cores, memory)) {
        // Initialize user quotas
        userQuotas_["pi_smith"] = {512, 4096.0, 10000.0, 50000.0, true};
        userQuotas_["postdoc_jones"] = {256, 2048.0, 5000.0, 20000.0, true};
        userQuotas_["grad_student"] = {64, 512.0, 1000.0, 5000.0, true};
        userQuotas_["guest"] = {16, 128.0, 0.0, 100.0, false};
    }
    
    void setCurrentUser(const std::string& user) {
        currentUser_ = user;
        std::cout << "\nAuthenticated as: " << user << "\n";
        
        auto it = userQuotas_.find(user);
        if (it != userQuotas_.end()) {
            const auto& quota = it->second;
            std::cout << "Quota: " << quota.maxCores << " cores, "
                      << quota.maxMemory << " GB RAM\n";
            std::cout << "CPU hours: " << std::fixed << std::setprecision(1)
                      << quota.cpuHoursUsed << " / " << quota.cpuHoursLimit << " used\n";
        }
    }
    
    void submitJob(const std::string& jobScript, int cores, double memory) override {
        if (!checkQuota(cores, memory)) {
            std::cout << "\nAccess denied: " << currentUser_ 
                      << " - Insufficient quota or permissions\n";
            std::cout << "Requested: " << cores << " cores, " << memory << " GB\n";
            return;
        }
        
        resource_->submitJob(jobScript, cores, memory);
        
        // Update usage
        auto& quota = userQuotas_[currentUser_];
        quota.cpuHoursUsed += cores * 0.5; // Assume 30 min job
    }
    
    std::string getStatus() override {
        return resource_->getStatus();
    }
    
    double getUsage() override {
        return resource_->getUsage();
    }
    
    void retrieveResults(const std::string& jobId) override {
        if (userQuotas_[currentUser_].hasAccess) {
            resource_->retrieveResults(jobId);
        } else {
            std::cout << "Access denied for retrieving results\n";
        }
    }
};

// Caching proxy - Caches cluster status and job results
class CachingHPCProxy : public ComputationalResource {
private:
    std::unique_ptr<ComputationalResource> resource_;
    
    // Cached data
    mutable std::string cachedStatus_;
    mutable double cachedUsage_;
    mutable std::unordered_map<std::string, std::string> cachedResults_;
    
    // Cache timestamps
    mutable std::chrono::steady_clock::time_point lastStatusUpdate_;
    mutable std::chrono::steady_clock::time_point lastUsageUpdate_;
    
    // Cache durations
    static constexpr auto STATUS_CACHE_DURATION = std::chrono::seconds(30);
    static constexpr auto USAGE_CACHE_DURATION = std::chrono::seconds(10);
    
    bool isStatusCacheValid() const {
        auto now = std::chrono::steady_clock::now();
        return !cachedStatus_.empty() && 
               (now - lastStatusUpdate_) < STATUS_CACHE_DURATION;
    }
    
    bool isUsageCacheValid() const {
        auto now = std::chrono::steady_clock::now();
        return (now - lastUsageUpdate_) < USAGE_CACHE_DURATION;
    }
    
public:
    CachingHPCProxy(const std::string& name, const std::string& host,
                    int cores, double memory)
        : resource_(std::make_unique<SecureHPCProxy>(name, host, cores, memory)),
          cachedUsage_(0.0) {
        std::cout << "Caching proxy enabled for HPC resource\n";
    }
    
    void setCurrentUser(const std::string& user) {
        // Forward to secure proxy
        if (auto* secureProxy = dynamic_cast<SecureHPCProxy*>(resource_.get())) {
            secureProxy->setCurrentUser(user);
        }
    }
    
    void submitJob(const std::string& jobScript, int cores, double memory) override {
        resource_->submitJob(jobScript, cores, memory);
        // Invalidate usage cache after job submission
        lastUsageUpdate_ = std::chrono::steady_clock::time_point{};
    }
    
    std::string getStatus() override {
        if (isStatusCacheValid()) {
            std::cout << "[Cache hit] Returning cached cluster status\n";
            return cachedStatus_;
        }
        
        std::cout << "[Cache miss] Fetching fresh cluster status\n";
        cachedStatus_ = resource_->getStatus();
        lastStatusUpdate_ = std::chrono::steady_clock::now();
        return cachedStatus_;
    }
    
    double getUsage() override {
        if (isUsageCacheValid()) {
            std::cout << "[Cache hit] Returning cached usage: " 
                      << std::fixed << std::setprecision(1) 
                      << (cachedUsage_ * 100) << "%\n";
            return cachedUsage_;
        }
        
        std::cout << "[Cache miss] Querying current cluster usage\n";
        cachedUsage_ = resource_->getUsage();
        lastUsageUpdate_ = std::chrono::steady_clock::now();
        return cachedUsage_;
    }
    
    void retrieveResults(const std::string& jobId) override {
        // Check if results are cached
        auto it = cachedResults_.find(jobId);
        if (it != cachedResults_.end()) {
            std::cout << "[Cache hit] Results already downloaded for " << jobId << "\n";
            std::cout << "  Using local copy from cache\n";
            return;
        }
        
        // Retrieve and cache results
        resource_->retrieveResults(jobId);
        cachedResults_[jobId] = "cached";
    }
};

// Distributed data proxy for large scientific datasets
class DistributedDataProxy : public ComputationalResource {
private:
    std::string datasetName_;
    double totalSize_; // TB
    std::vector<std::string> dataNodes_;
    mutable std::unordered_map<std::string, bool> localChunks_;
    
public:
    DistributedDataProxy(const std::string& dataset, double size)
        : datasetName_(dataset), totalSize_(size) {
        dataNodes_ = {"node1.hpc.edu", "node2.hpc.edu", "node3.hpc.edu"};
        std::cout << "\nDistributed data proxy for: " << datasetName_ 
                  << " (" << totalSize_ << " TB across " 
                  << dataNodes_.size() << " nodes)\n";
    }
    
    void submitJob(const std::string& jobScript, int cores, double memory) override {
        std::cout << "\nScheduling data-local computation:\n";
        std::cout << "  Dataset: " << datasetName_ << "\n";
        std::cout << "  Analyzing data locality...\n";
        std::cout << "  Job scheduled on nodes with data chunks\n";
    }
    
    std::string getStatus() override {
        std::stringstream ss;
        ss << "Dataset: " << datasetName_ << " (" << totalSize_ << " TB)\n";
        ss << "Distributed across: ";
        for (const auto& node : dataNodes_) {
            ss << node << " ";
        }
        return ss.str();
    }
    
    double getUsage() override {
        return 0.0; // Not applicable for data proxy
    }
    
    void retrieveResults(const std::string& chunkId) override {
        if (localChunks_[chunkId]) {
            std::cout << "  Chunk " << chunkId << " already cached locally\n";
        } else {
            std::cout << "  Streaming chunk " << chunkId << " from remote node...\n";
            std::cout << "  Using parallel GridFTP for " 
                      << (totalSize_ / dataNodes_.size()) << " TB transfer\n";
            localChunks_[chunkId] = true;
        }
    }
};

int main() {
    std::cout << "=== HPC Resource Proxy Pattern Demo ===\n\n";
    
    // Virtual Proxy - Lazy connection
    std::cout << "1. Virtual Proxy (Lazy Connection) Demo:\n";
    std::cout << "=========================================\n";
    HPCResourceProxy cluster1("Frontera", "frontera.tacc.edu", 448000, 2300.0);
    HPCResourceProxy cluster2("Summit", "summit.olcf.ornl.gov", 202000, 2800.0);
    
    std::cout << "\nNote: No connections established yet\n";
    
    std::cout << "\nFirst job submission to Frontera:\n";
    cluster1.submitJob("cfd_simulation.slurm", 512, 1024.0);
    
    std::cout << "\nSecond job to Frontera (already connected):\n";
    cluster1.submitJob("molecular_dynamics.slurm", 256, 512.0);
    
    std::cout << "\nNote: Summit still not connected (lazy loading)\n";
    
    // Protection Proxy - Access control and quotas
    std::cout << "\n\n2. Protection Proxy (Access Control) Demo:\n";
    std::cout << "==========================================\n";
    SecureHPCProxy secureCluster("Perlmutter", "perlmutter.nersc.gov", 760000, 3500.0);
    
    secureCluster.setCurrentUser("guest");
    secureCluster.submitJob("test_job.sh", 64, 256.0);
    
    secureCluster.setCurrentUser("grad_student");
    secureCluster.submitJob("quantum_espresso.slurm", 64, 256.0);
    
    secureCluster.setCurrentUser("pi_smith");
    secureCluster.submitJob("climate_model.slurm", 512, 2048.0);
    
    // Caching Proxy - Performance optimization
    std::cout << "\n\n3. Caching Proxy (Performance) Demo:\n";
    std::cout << "====================================\n";
    CachingHPCProxy cachedCluster("Stampede3", "stampede3.tacc.edu", 560000, 2900.0);
    cachedCluster.setCurrentUser("postdoc_jones");
    
    std::cout << "\nFirst status request:\n";
    std::cout << cachedCluster.getStatus() << "\n";
    
    std::cout << "\nSecond status request (should be cached):\n";
    std::cout << cachedCluster.getStatus() << "\n";
    
    std::cout << "\nMultiple usage queries:\n";
    for (int i = 0; i < 3; ++i) {
        cachedCluster.getUsage();
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    
    std::cout << "\nJob submission (invalidates cache):\n";
    cachedCluster.submitJob("large_scale_fem.slurm", 1024, 4096.0);
    cachedCluster.getUsage();
    
    std::cout << "\nRetrieving results (with caching):\n";
    cachedCluster.retrieveResults("JOB_1002");
    cachedCluster.retrieveResults("JOB_1002"); // Should use cache
    
    // Distributed Data Proxy
    std::cout << "\n\n4. Distributed Data Proxy Demo:\n";
    std::cout << "===============================\n";
    DistributedDataProxy climateData("CMIP6_Historical_Data", 850.0);
    std::cout << climateData.getStatus() << "\n";
    
    std::cout << "\nAccessing data chunks:\n";
    climateData.retrieveResults("chunk_001");
    climateData.retrieveResults("chunk_002");
    climateData.retrieveResults("chunk_001"); // Cached
    
    std::cout << "\nProxy pattern enables efficient management of\n";
    std::cout << "remote HPC resources and large scientific datasets!\n";
    
    return 0;
}