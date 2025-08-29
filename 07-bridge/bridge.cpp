// Bridge Pattern - Computational Mesh Framework
// Separates mesh abstraction from storage implementation (structured/unstructured)
#include <iostream>
#include <memory>
#include <vector>
#include <string>

// Implementation interface - Storage backend for computational meshes
class MeshStorageBackend {
public:
    virtual ~MeshStorageBackend() = default;
    virtual void storeNodes(const std::vector<std::vector<double>>& nodes) = 0;
    virtual void storeElements(const std::vector<std::vector<int>>& elements) = 0;
    virtual void storeBoundaryConditions(const std::string& type, const std::vector<int>& nodeIds) = 0;
    virtual size_t getMemoryUsage() const = 0;
};

// Concrete implementation - Array-based storage for structured meshes
class StructuredMeshStorage : public MeshStorageBackend {
private:
    size_t nx, ny, nz;
    std::vector<double> nodeCoordinates;  // Implicit ordering i + nx*(j + ny*k)
    
public:
    void storeNodes(const std::vector<std::vector<double>>& nodes) override {
        std::cout << "Structured Storage: Storing nodes in 3D array format\n";
        std::cout << "  Using implicit indexing (i,j,k) -> linear index\n";
        std::cout << "  Memory layout: Contiguous for cache efficiency\n";
        nodeCoordinates.reserve(nodes.size() * 3);
        for (const auto& node : nodes) {
            nodeCoordinates.insert(nodeCoordinates.end(), node.begin(), node.end());
        }
    }
    
    void storeElements(const std::vector<std::vector<int>>& elements) override {
        std::cout << "Structured Storage: Elements implicitly defined by grid\n";
        std::cout << "  Hexahedral elements from regular connectivity\n";
        std::cout << "  No explicit storage needed - computed on demand\n";
    }
    
    void storeBoundaryConditions(const std::string& type, const std::vector<int>& nodeIds) override {
        std::cout << "Structured Storage: Boundary condition '" << type << "'\n";
        std::cout << "  Storing as face indices (6 faces per hex)\n";
        std::cout << "  Nodes: " << nodeIds.size() << " boundary points\n";
    }
    
    size_t getMemoryUsage() const override {
        return nodeCoordinates.size() * sizeof(double) + sizeof(*this);
    }
};

// Concrete implementation - Graph-based storage for unstructured meshes
class UnstructuredMeshStorage : public MeshStorageBackend {
private:
    std::vector<std::vector<double>> nodeList;
    std::vector<std::vector<int>> elementConnectivity;
    std::vector<std::vector<int>> nodeToElementMap;
    
public:
    void storeNodes(const std::vector<std::vector<double>>& nodes) override {
        std::cout << "Unstructured Storage: Storing nodes in coordinate list\n";
        std::cout << "  Flexible positioning for complex geometries\n";
        std::cout << "  Building spatial hash for fast neighbor queries\n";
        nodeList = nodes;
        // Build reverse connectivity
        nodeToElementMap.resize(nodes.size());
    }
    
    void storeElements(const std::vector<std::vector<int>>& elements) override {
        std::cout << "Unstructured Storage: Explicit element connectivity\n";
        std::cout << "  Supporting mixed element types (tet, hex, prism)\n";
        std::cout << "  Building element adjacency graph\n";
        elementConnectivity = elements;
    }
    
    void storeBoundaryConditions(const std::string& type, const std::vector<int>& nodeIds) override {
        std::cout << "Unstructured Storage: Boundary condition '" << type << "'\n";
        std::cout << "  Marking nodes with boundary flags\n";
        std::cout << "  Creating boundary element list\n";
    }
    
    size_t getMemoryUsage() const override {
        size_t mem = nodeList.size() * 3 * sizeof(double);
        for (const auto& elem : elementConnectivity) {
            mem += elem.size() * sizeof(int);
        }
        return mem + sizeof(*this);
    }
};

// Abstraction - Computational mesh interface
class ComputationalMesh {
protected:
    std::shared_ptr<MeshStorageBackend> storage;
    std::string meshType;
    int dimension;
    
public:
    ComputationalMesh(std::shared_ptr<MeshStorageBackend> backend, 
                      const std::string& type, int dim)
        : storage(backend), meshType(type), dimension(dim) {}
    
    virtual ~ComputationalMesh() = default;
    virtual void generateMesh() = 0;
    virtual void applyBoundaryConditions() = 0;
    virtual void refineMesh(double factor) = 0;
    
    void reportMemoryUsage() {
        std::cout << "\nMemory usage: " << storage->getMemoryUsage() 
                  << " bytes\n\n";
    }
};

// Refined abstraction - Finite Element Mesh for structural analysis
class FEMStructuralMesh : public ComputationalMesh {
private:
    double length, width, height;
    int elementsPerDim;
    
public:
    FEMStructuralMesh(double l, double w, double h, int elements,
                      std::shared_ptr<MeshStorageBackend> backend)
        : ComputationalMesh(backend, "FEM_Structural", 3),
          length(l), width(w), height(h), elementsPerDim(elements) {}
    
    void generateMesh() override {
        std::cout << "=== Generating FEM Structural Mesh ===\n";
        std::cout << "Domain: " << length << "m x " << width << "m x " 
                  << height << "m\n";
        std::cout << "Elements: " << elementsPerDim << "^3 = " 
                  << elementsPerDim * elementsPerDim * elementsPerDim << "\n\n";
        
        // Generate nodes
        std::vector<std::vector<double>> nodes;
        for (int k = 0; k <= elementsPerDim; ++k) {
            for (int j = 0; j <= elementsPerDim; ++j) {
                for (int i = 0; i <= elementsPerDim; ++i) {
                    nodes.push_back({
                        i * length / elementsPerDim,
                        j * width / elementsPerDim,
                        k * height / elementsPerDim
                    });
                }
            }
        }
        storage->storeNodes(nodes);
        
        // Generate elements (if unstructured)
        std::vector<std::vector<int>> elements;
        storage->storeElements(elements);
    }
    
    void applyBoundaryConditions() override {
        std::cout << "\nApplying structural boundary conditions:\n";
        std::vector<int> fixedNodes = {0, 1, 2, 3};  // Fixed base
        storage->storeBoundaryConditions("Fixed_Displacement", fixedNodes);
        
        std::vector<int> loadedNodes = {100, 101, 102, 103};  // Top surface
        storage->storeBoundaryConditions("Applied_Force", loadedNodes);
    }
    
    void refineMesh(double factor) override {
        elementsPerDim = static_cast<int>(elementsPerDim * factor);
        std::cout << "\nMesh refined by factor " << factor << "\n";
        std::cout << "New resolution: " << elementsPerDim << " elements per dimension\n";
    }
};

// Refined abstraction - CFD mesh for fluid dynamics
class CFDFluidMesh : public ComputationalMesh {
private:
    double diameter;
    double pipeLength;
    int radialElements;
    int axialElements;
    
public:
    CFDFluidMesh(double d, double l, int radial, int axial,
                 std::shared_ptr<MeshStorageBackend> backend)
        : ComputationalMesh(backend, "CFD_Pipe_Flow", 3),
          diameter(d), pipeLength(l), 
          radialElements(radial), axialElements(axial) {}
    
    void generateMesh() override {
        std::cout << "=== Generating CFD Pipe Flow Mesh ===\n";
        std::cout << "Pipe geometry: D=" << diameter << "m, L=" << pipeLength << "m\n";
        std::cout << "Mesh density: " << radialElements << " radial x " 
                  << axialElements << " axial\n\n";
        
        // Generate cylindrical mesh nodes
        std::vector<std::vector<double>> nodes;
        double radius = diameter / 2.0;
        
        for (int i = 0; i <= axialElements; ++i) {
            double z = i * pipeLength / axialElements;
            for (int j = 0; j <= radialElements; ++j) {
                double r = j * radius / radialElements;
                for (int k = 0; k < 8; ++k) {  // Circumferential
                    double theta = k * 2.0 * 3.14159 / 8;
                    nodes.push_back({
                        r * cos(theta),
                        r * sin(theta),
                        z
                    });
                }
            }
        }
        storage->storeNodes(nodes);
        
        std::vector<std::vector<int>> elements;
        storage->storeElements(elements);
    }
    
    void applyBoundaryConditions() override {
        std::cout << "\nApplying CFD boundary conditions:\n";
        std::vector<int> inletNodes = {0, 1, 2, 3, 4, 5, 6, 7};
        storage->storeBoundaryConditions("Velocity_Inlet", inletNodes);
        
        std::vector<int> wallNodes;
        for (int i = 0; i < 100; ++i) wallNodes.push_back(i * 8);
        storage->storeBoundaryConditions("No_Slip_Wall", wallNodes);
        
        std::vector<int> outletNodes = {800, 801, 802, 803, 804, 805, 806, 807};
        storage->storeBoundaryConditions("Pressure_Outlet", outletNodes);
    }
    
    void refineMesh(double factor) override {
        radialElements = static_cast<int>(radialElements * factor);
        axialElements = static_cast<int>(axialElements * factor);
        std::cout << "\nCFD mesh refined by factor " << factor << "\n";
        std::cout << "New density: " << radialElements << " x " << axialElements << "\n";
    }
};

int main() {
    std::cout << "=== Computational Mesh Bridge Pattern Demo ===\n\n";
    
    // Create storage backends
    auto structuredStorage = std::make_shared<StructuredMeshStorage>();
    auto unstructuredStorage = std::make_shared<UnstructuredMeshStorage>();
    
    // Create meshes with different storage implementations
    std::cout << "Creating computational meshes with different storage backends:\n\n";
    
    // Structural analysis with structured mesh
    FEMStructuralMesh beamMesh(10.0, 1.0, 1.0, 20, structuredStorage);
    beamMesh.generateMesh();
    beamMesh.applyBoundaryConditions();
    beamMesh.reportMemoryUsage();
    
    // CFD analysis with unstructured mesh
    CFDFluidMesh pipeMesh(0.1, 2.0, 10, 50, unstructuredStorage);
    pipeMesh.generateMesh();
    pipeMesh.applyBoundaryConditions();
    pipeMesh.reportMemoryUsage();
    
    // Demonstrate mesh refinement
    std::cout << "=== Adaptive Mesh Refinement ===\n";
    beamMesh.refineMesh(1.5);
    pipeMesh.refineMesh(2.0);
    
    // Switch storage backend for comparison
    std::cout << "\n=== Storage Backend Comparison ===\n";
    std::cout << "Creating same mesh with different storage:\n\n";
    
    FEMStructuralMesh beamUnstructured(10.0, 1.0, 1.0, 20, unstructuredStorage);
    beamUnstructured.generateMesh();
    beamUnstructured.reportMemoryUsage();
    
    std::cout << "Bridge pattern allows switching between structured and\n";
    std::cout << "unstructured storage without changing mesh algorithms!\n";
    
    return 0;
}