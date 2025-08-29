// Composite Pattern - Hierarchical Simulation Domain Decomposition
// Represents computational domains as tree structures for multi-scale physics
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>

// Component interface - Computational domain element
class ComputationalDomain {
public:
    virtual ~ComputationalDomain() = default;
    virtual void computePhysics(double timeStep) = 0;
    virtual void displayHierarchy(int depth = 0) const = 0;
    virtual double getTotalDOFs() const = 0;  // Degrees of freedom
    virtual double getComputationalCost() const = 0;
    virtual bool isLeaf() const { return false; }
    
    // Composite operations (default: do nothing for leaf nodes)
    virtual void addSubdomain(std::shared_ptr<ComputationalDomain> subdomain) {}
    virtual void removeSubdomain(std::shared_ptr<ComputationalDomain> subdomain) {}
    virtual std::shared_ptr<ComputationalDomain> getSubdomain(int index) { return nullptr; }
};

// Leaf node - Single physics element (cannot be subdivided)
class ElementDomain : public ComputationalDomain {
private:
    std::string elementType_;
    int elementId_;
    double volume_;
    int nodesPerElement_;
    std::string physicsModel_;
    
public:
    ElementDomain(const std::string& type, int id, double volume, 
                  int nodes, const std::string& physics)
        : elementType_(type), elementId_(id), volume_(volume), 
          nodesPerElement_(nodes), physicsModel_(physics) {}
    
    void computePhysics(double timeStep) override {
        std::cout << "Element " << elementId_ << ": Computing " << physicsModel_ 
                  << " (dt=" << timeStep << "s)\n";
        std::cout << "  Solving on " << nodesPerElement_ << " nodes\n";
        std::cout << "  Volume: " << volume_ << " m³\n";
    }
    
    void displayHierarchy(int depth) const override {
        std::cout << std::string(depth * 2, ' ') 
                  << "└─ " << elementType_ << "[" << elementId_ << "]: "
                  << physicsModel_ << " (" << nodesPerElement_ << " DOFs)\n";
    }
    
    double getTotalDOFs() const override {
        return nodesPerElement_ * 3;  // 3 DOFs per node (x,y,z)
    }
    
    double getComputationalCost() const override {
        // Cost proportional to DOFs squared (matrix operations)
        double dofs = getTotalDOFs();
        return dofs * dofs;
    }
    
    bool isLeaf() const override { return true; }
};

// Composite node - Domain that can contain subdomains
class RegionDomain : public ComputationalDomain {
private:
    std::string regionName_;
    std::string decompositionMethod_;
    std::vector<std::shared_ptr<ComputationalDomain>> subdomains_;
    int mpiRank_;  // MPI process assignment
    
public:
    RegionDomain(const std::string& name, const std::string& method, int rank)
        : regionName_(name), decompositionMethod_(method), mpiRank_(rank) {}
    
    void addSubdomain(std::shared_ptr<ComputationalDomain> subdomain) override {
        subdomains_.push_back(subdomain);
    }
    
    void removeSubdomain(std::shared_ptr<ComputationalDomain> subdomain) override {
        subdomains_.erase(
            std::remove(subdomains_.begin(), subdomains_.end(), subdomain),
            subdomains_.end()
        );
    }
    
    std::shared_ptr<ComputationalDomain> getSubdomain(int index) override {
        if (index >= 0 && index < subdomains_.size()) {
            return subdomains_[index];
        }
        return nullptr;
    }
    
    void computePhysics(double timeStep) override {
        std::cout << "\nRegion '" << regionName_ << "' (MPI Rank " << mpiRank_ << "):\n";
        std::cout << "Decomposition: " << decompositionMethod_ << "\n";
        
        // Compute all subdomains
        for (const auto& subdomain : subdomains_) {
            subdomain->computePhysics(timeStep);
        }
        
        // Exchange boundary data between subdomains
        std::cout << "Exchanging boundary data between " << subdomains_.size() 
                  << " subdomains\n";
    }
    
    void displayHierarchy(int depth) const override {
        std::cout << std::string(depth * 2, ' ') 
                  << "[" << regionName_ << "] "
                  << "(" << decompositionMethod_ << ", Rank " << mpiRank_ << ")\n";
        for (const auto& subdomain : subdomains_) {
            subdomain->displayHierarchy(depth + 1);
        }
    }
    
    double getTotalDOFs() const override {
        double totalDOFs = 0;
        for (const auto& subdomain : subdomains_) {
            totalDOFs += subdomain->getTotalDOFs();
        }
        return totalDOFs;
    }
    
    double getComputationalCost() const override {
        double totalCost = 0;
        for (const auto& subdomain : subdomains_) {
            totalCost += subdomain->getComputationalCost();
        }
        // Add communication overhead
        totalCost += subdomains_.size() * 100;  // Inter-subdomain communication
        return totalCost;
    }
};

int main() {
    std::cout << "=== Multi-Scale Domain Decomposition Demo ===\n\n";
    
    // Create hierarchical computational domain
    auto globalDomain = std::make_shared<RegionDomain>(
        "Global Ocean Model", "Recursive Coordinate Bisection", 0);
    
    // Regional subdomains
    auto atlanticRegion = std::make_shared<RegionDomain>(
        "Atlantic Basin", "Graph Partitioning", 1);
    auto pacificRegion = std::make_shared<RegionDomain>(
        "Pacific Basin", "Graph Partitioning", 2);
    
    // Sub-regional domains
    auto gulfStream = std::make_shared<RegionDomain>(
        "Gulf Stream Region", "Adaptive Refinement", 1);
    auto sargassoSea = std::make_shared<RegionDomain>(
        "Sargasso Sea", "Uniform Decomposition", 1);
    
    // Leaf elements - actual computation happens here
    // Gulf Stream high-resolution elements
    auto elem1 = std::make_shared<ElementDomain>(
        "Hexahedral", 1001, 1e6, 27, "Navier-Stokes + Heat Transport");
    auto elem2 = std::make_shared<ElementDomain>(
        "Hexahedral", 1002, 1e6, 27, "Navier-Stokes + Heat Transport");
    auto elem3 = std::make_shared<ElementDomain>(
        "Tetrahedral", 1003, 0.5e6, 10, "Turbulence Model (LES)");
    
    // Sargasso Sea coarse elements
    auto elem4 = std::make_shared<ElementDomain>(
        "Hexahedral", 2001, 8e6, 27, "Hydrostatic Approximation");
    auto elem5 = std::make_shared<ElementDomain>(
        "Hexahedral", 2002, 8e6, 27, "Hydrostatic Approximation");
    
    // Pacific elements
    auto elem6 = std::make_shared<ElementDomain>(
        "Prism", 3001, 4e6, 18, "Shallow Water Equations");
    auto elem7 = std::make_shared<ElementDomain>(
        "Prism", 3002, 4e6, 18, "Shallow Water Equations");
    
    // Build hierarchical structure
    globalDomain->addSubdomain(atlanticRegion);
    globalDomain->addSubdomain(pacificRegion);
    
    atlanticRegion->addSubdomain(gulfStream);
    atlanticRegion->addSubdomain(sargassoSea);
    
    gulfStream->addSubdomain(elem1);
    gulfStream->addSubdomain(elem2);
    gulfStream->addSubdomain(elem3);
    
    sargassoSea->addSubdomain(elem4);
    sargassoSea->addSubdomain(elem5);
    
    pacificRegion->addSubdomain(elem6);
    pacificRegion->addSubdomain(elem7);
    
    // Display domain hierarchy
    std::cout << "Domain Decomposition Hierarchy:\n";
    std::cout << "===============================\n";
    globalDomain->displayHierarchy(0);
    
    // Computational statistics
    std::cout << "\nComputational Statistics:\n";
    std::cout << "========================\n";
    std::cout << "Total DOFs in system: " << globalDomain->getTotalDOFs() << "\n";
    std::cout << "Atlantic region DOFs: " << atlanticRegion->getTotalDOFs() << "\n";
    std::cout << "Gulf Stream DOFs: " << gulfStream->getTotalDOFs() << "\n";
    std::cout << "\nEstimated computational cost: " 
              << globalDomain->getComputationalCost() << " FLOPS\n";
    
    // Run simulation
    std::cout << "\n=== Running Multi-Scale Ocean Simulation ===\n";
    double timeStep = 300.0;  // 5 minutes
    globalDomain->computePhysics(timeStep);
    
    std::cout << "\nComposite pattern enables hierarchical domain decomposition\n";
    std::cout << "for efficient parallel computation in Earth system models!\n";
    
    return 0;
}