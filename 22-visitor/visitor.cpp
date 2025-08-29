// Visitor Pattern - Scientific Data Structure Analysis and Transformation
// Enables operations on complex scientific data structures without modifying their classes
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <complex>
#include <algorithm>
#include <map>

// Define M_PI for MSVC
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations for molecular structures
class Atom;
class Bond;
class Molecule;
class Protein;

// Molecular structure visitor interface
class MolecularVisitor {
public:
    virtual ~MolecularVisitor() = default;
    virtual void visit(Atom& atom) = 0;
    virtual void visit(Bond& bond) = 0;
    virtual void visit(Molecule& molecule) = 0;
    virtual void visit(Protein& protein) = 0;
};

// Base molecular structure interface
class MolecularStructure {
public:
    virtual ~MolecularStructure() = default;
    virtual void accept(MolecularVisitor& visitor) = 0;
    virtual std::string getName() const = 0;
};

// Concrete molecular structures
class Atom : public MolecularStructure {
private:
    std::string element_;
    int atomicNumber_;
    double x_, y_, z_;  // 3D coordinates
    double charge_;
    
public:
    Atom(const std::string& element, int atomicNumber, 
         double x, double y, double z, double charge = 0.0)
        : element_(element), atomicNumber_(atomicNumber), 
          x_(x), y_(y), z_(z), charge_(charge) {}
    
    void accept(MolecularVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    std::string getName() const override {
        return element_ + " (Z=" + std::to_string(atomicNumber_) + ")";
    }
    
    const std::string& getElement() const { return element_; }
    int getAtomicNumber() const { return atomicNumber_; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    double getCharge() const { return charge_; }
    
    double distanceTo(const Atom& other) const {
        double dx = x_ - other.x_;
        double dy = y_ - other.y_;
        double dz = z_ - other.z_;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

class Bond : public MolecularStructure {
private:
    std::shared_ptr<Atom> atom1_;
    std::shared_ptr<Atom> atom2_;
    int order_;  // 1=single, 2=double, 3=triple
    double length_;
    
public:
    Bond(std::shared_ptr<Atom> atom1, std::shared_ptr<Atom> atom2, int order)
        : atom1_(atom1), atom2_(atom2), order_(order) {
        length_ = atom1->distanceTo(*atom2);
    }
    
    void accept(MolecularVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    std::string getName() const override {
        return atom1_->getElement() + "-" + atom2_->getElement() + 
               " (order=" + std::to_string(order_) + ")";
    }
    
    std::shared_ptr<Atom> getAtom1() const { return atom1_; }
    std::shared_ptr<Atom> getAtom2() const { return atom2_; }
    int getOrder() const { return order_; }
    double getLength() const { return length_; }
};

class Molecule : public MolecularStructure {
private:
    std::string name_;
    std::vector<std::shared_ptr<Atom>> atoms_;
    std::vector<std::shared_ptr<Bond>> bonds_;
    
public:
    Molecule(const std::string& name) : name_(name) {}
    
    void addAtom(std::shared_ptr<Atom> atom) {
        atoms_.push_back(atom);
    }
    
    void addBond(std::shared_ptr<Atom> atom1, std::shared_ptr<Atom> atom2, int order) {
        bonds_.push_back(std::make_shared<Bond>(atom1, atom2, order));
    }
    
    void accept(MolecularVisitor& visitor) override {
        visitor.visit(*this);
        // Visit all atoms and bonds
        for (auto& atom : atoms_) {
            atom->accept(visitor);
        }
        for (auto& bond : bonds_) {
            bond->accept(visitor);
        }
    }
    
    std::string getName() const override { return name_; }
    const std::vector<std::shared_ptr<Atom>>& getAtoms() const { return atoms_; }
    const std::vector<std::shared_ptr<Bond>>& getBonds() const { return bonds_; }
};

class Protein : public MolecularStructure {
private:
    std::string name_;
    std::string sequence_;
    std::vector<std::shared_ptr<Molecule>> residues_;
    
public:
    Protein(const std::string& name, const std::string& sequence)
        : name_(name), sequence_(sequence) {}
    
    void addResidue(std::shared_ptr<Molecule> residue) {
        residues_.push_back(residue);
    }
    
    void accept(MolecularVisitor& visitor) override {
        visitor.visit(*this);
        // Visit all residues
        for (auto& residue : residues_) {
            residue->accept(visitor);
        }
    }
    
    std::string getName() const override { return name_; }
    const std::string& getSequence() const { return sequence_; }
    const std::vector<std::shared_ptr<Molecule>>& getResidues() const { return residues_; }
};

// Concrete molecular visitors
class MolecularMassCalculator : public MolecularVisitor {
private:
    double totalMass_ = 0.0;
    std::map<std::string, double> atomicMasses_ = {
        {"H", 1.008}, {"C", 12.011}, {"N", 14.007}, {"O", 15.999},
        {"P", 30.974}, {"S", 32.065}, {"F", 18.998}, {"Cl", 35.453}
    };
    
public:
    void visit(Atom& atom) override {
        auto it = atomicMasses_.find(atom.getElement());
        if (it != atomicMasses_.end()) {
            totalMass_ += it->second;
            std::cout << "  " << atom.getElement() << " atom: " 
                      << std::fixed << std::setprecision(3) 
                      << it->second << " Da\n";
        }
    }
    
    void visit(Bond& bond) override {
        // Bonds don't contribute to mass
    }
    
    void visit(Molecule& molecule) override {
        std::cout << "Calculating mass for molecule: " << molecule.getName() << "\n";
    }
    
    void visit(Protein& protein) override {
        std::cout << "\nCalculating mass for protein: " << protein.getName() << "\n";
        std::cout << "Sequence: " << protein.getSequence() << "\n";
    }
    
    double getTotalMass() const { return totalMass_; }
};

class ChargeCalculator : public MolecularVisitor {
private:
    double totalCharge_ = 0.0;
    int chargedAtoms_ = 0;
    
public:
    void visit(Atom& atom) override {
        if (std::abs(atom.getCharge()) > 1e-6) {
            totalCharge_ += atom.getCharge();
            chargedAtoms_++;
            std::cout << "  " << atom.getElement() << " at (" 
                      << atom.getX() << ", " << atom.getY() << ", " << atom.getZ() 
                      << "): charge = " << std::showpos << atom.getCharge() 
                      << std::noshowpos << "e\n";
        }
    }
    
    void visit(Bond& bond) override {
        // Calculate bond dipole moment
        double q1 = bond.getAtom1()->getCharge();
        double q2 = bond.getAtom2()->getCharge();
        if (std::abs(q1 - q2) > 1e-6) {
            double dipole = std::abs(q1 - q2) * bond.getLength();
            std::cout << "  Bond " << bond.getName() 
                      << " dipole moment: " << dipole << " Debye\n";
        }
    }
    
    void visit(Molecule& molecule) override {
        std::cout << "\nAnalyzing charges in molecule: " << molecule.getName() << "\n";
    }
    
    void visit(Protein& protein) override {
        std::cout << "\nAnalyzing charges in protein: " << protein.getName() << "\n";
    }
    
    double getTotalCharge() const { return totalCharge_; }
    int getChargedAtomCount() const { return chargedAtoms_; }
};

class StructureAnalyzer : public MolecularVisitor {
private:
    int atomCount_ = 0;
    int bondCount_ = 0;
    int moleculeCount_ = 0;
    double minBondLength_ = std::numeric_limits<double>::max();
    double maxBondLength_ = 0.0;
    std::map<std::string, int> elementCounts_;
    
public:
    void visit(Atom& atom) override {
        atomCount_++;
        elementCounts_[atom.getElement()]++;
    }
    
    void visit(Bond& bond) override {
        bondCount_++;
        double length = bond.getLength();
        minBondLength_ = std::min(minBondLength_, length);
        maxBondLength_ = std::max(maxBondLength_, length);
        
        std::cout << "  Bond " << bond.getName() 
                  << ": length = " << std::fixed << std::setprecision(3) 
                  << length << " Å, order = " << bond.getOrder() << "\n";
    }
    
    void visit(Molecule& molecule) override {
        moleculeCount_++;
        std::cout << "\nAnalyzing structure of: " << molecule.getName() << "\n";
    }
    
    void visit(Protein& protein) override {
        std::cout << "\nAnalyzing protein structure: " << protein.getName() << "\n";
        std::cout << "Number of residues: " << protein.getResidues().size() << "\n";
    }
    
    void printSummary() const {
        std::cout << "\n=== Structure Analysis Summary ===\n";
        std::cout << "Total atoms: " << atomCount_ << "\n";
        std::cout << "Total bonds: " << bondCount_ << "\n";
        std::cout << "Total molecules/residues: " << moleculeCount_ << "\n";
        
        if (bondCount_ > 0) {
            std::cout << "Bond length range: " << std::fixed << std::setprecision(3)
                      << minBondLength_ << " - " << maxBondLength_ << " Å\n";
        }
        
        std::cout << "\nElement composition:\n";
        for (const auto& [element, count] : elementCounts_) {
            std::cout << "  " << element << ": " << count << " atoms\n";
        }
    }
};

// Computational mesh/grid structure visitor example
class ComputationalMeshVisitor;

class MeshElement {
public:
    virtual ~MeshElement() = default;
    virtual void accept(ComputationalMeshVisitor& visitor) = 0;
    virtual std::string getType() const = 0;
};

class GridPoint;
class FiniteElement;
class MeshRegion;

class ComputationalMeshVisitor {
public:
    virtual ~ComputationalMeshVisitor() = default;
    virtual void visit(GridPoint& point) = 0;
    virtual void visit(FiniteElement& element) = 0;
    virtual void visit(MeshRegion& region) = 0;
};

class GridPoint : public MeshElement {
private:
    int id_;
    double x_, y_, z_;
    std::vector<double> values_;  // Field values at this point
    
public:
    GridPoint(int id, double x, double y, double z)
        : id_(id), x_(x), y_(y), z_(z) {}
    
    void accept(ComputationalMeshVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    void setFieldValue(double value) {
        values_.push_back(value);
    }
    
    std::string getType() const override { return "GridPoint"; }
    int getId() const { return id_; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    const std::vector<double>& getValues() const { return values_; }
};

class FiniteElement : public MeshElement {
private:
    int elementId_;
    std::string elementType_;  // "Tet", "Hex", "Prism"
    std::vector<std::shared_ptr<GridPoint>> nodes_;
    double volume_;
    
public:
    FiniteElement(int id, const std::string& type)
        : elementId_(id), elementType_(type), volume_(0.0) {}
    
    void addNode(std::shared_ptr<GridPoint> node) {
        nodes_.push_back(node);
    }
    
    void accept(ComputationalMeshVisitor& visitor) override {
        visitor.visit(*this);
        for (auto& node : nodes_) {
            node->accept(visitor);
        }
    }
    
    void calculateVolume() {
        // Simplified volume calculation
        if (elementType_ == "Tet" && nodes_.size() == 4) {
            // Tetrahedron volume calculation
            volume_ = 0.1;  // Placeholder
        }
    }
    
    std::string getType() const override { return elementType_; }
    int getId() const { return elementId_; }
    double getVolume() const { return volume_; }
    const std::vector<std::shared_ptr<GridPoint>>& getNodes() const { return nodes_; }
};

class MeshRegion : public MeshElement {
private:
    std::string name_;
    std::vector<std::shared_ptr<MeshElement>> elements_;
    std::string material_;
    
public:
    MeshRegion(const std::string& name, const std::string& material)
        : name_(name), material_(material) {}
    
    void addElement(std::shared_ptr<MeshElement> element) {
        elements_.push_back(element);
    }
    
    void accept(ComputationalMeshVisitor& visitor) override {
        visitor.visit(*this);
        for (auto& element : elements_) {
            element->accept(visitor);
        }
    }
    
    std::string getType() const override { return "MeshRegion"; }
    const std::string& getName() const { return name_; }
    const std::string& getMaterial() const { return material_; }
};

class MeshStatisticsVisitor : public ComputationalMeshVisitor {
private:
    int nodeCount_ = 0;
    int elementCount_ = 0;
    int regionCount_ = 0;
    double minX_ = std::numeric_limits<double>::max();
    double maxX_ = std::numeric_limits<double>::lowest();
    double minY_ = std::numeric_limits<double>::max();
    double maxY_ = std::numeric_limits<double>::lowest();
    double minZ_ = std::numeric_limits<double>::max();
    double maxZ_ = std::numeric_limits<double>::lowest();
    
public:
    void visit(GridPoint& point) override {
        nodeCount_++;
        minX_ = std::min(minX_, point.getX());
        maxX_ = std::max(maxX_, point.getX());
        minY_ = std::min(minY_, point.getY());
        maxY_ = std::max(maxY_, point.getY());
        minZ_ = std::min(minZ_, point.getZ());
        maxZ_ = std::max(maxZ_, point.getZ());
    }
    
    void visit(FiniteElement& element) override {
        elementCount_++;
        std::cout << "  Element " << element.getId() << " (" << element.getType() 
                  << "): " << element.getNodes().size() << " nodes\n";
    }
    
    void visit(MeshRegion& region) override {
        regionCount_++;
        std::cout << "\nAnalyzing mesh region: " << region.getName() 
                  << " (Material: " << region.getMaterial() << ")\n";
    }
    
    void printStatistics() const {
        std::cout << "\n=== Mesh Statistics ===\n";
        std::cout << "Total nodes: " << nodeCount_ << "\n";
        std::cout << "Total elements: " << elementCount_ << "\n";
        std::cout << "Total regions: " << regionCount_ << "\n";
        std::cout << "Bounding box:\n";
        std::cout << "  X: [" << minX_ << ", " << maxX_ << "]\n";
        std::cout << "  Y: [" << minY_ << ", " << maxY_ << "]\n";
        std::cout << "  Z: [" << minZ_ << ", " << maxZ_ << "]\n";
    }
};

class FieldValueVisitor : public ComputationalMeshVisitor {
private:
    double minValue_ = std::numeric_limits<double>::max();
    double maxValue_ = std::numeric_limits<double>::lowest();
    double sumValue_ = 0.0;
    int valueCount_ = 0;
    
public:
    void visit(GridPoint& point) override {
        const auto& values = point.getValues();
        if (!values.empty()) {
            double value = values.front();  // Use first field value
            minValue_ = std::min(minValue_, value);
            maxValue_ = std::max(maxValue_, value);
            sumValue_ += value;
            valueCount_++;
            
            if (std::abs(value) > 100.0) {  // High value threshold
                std::cout << "  High value at node " << point.getId() 
                          << " (" << point.getX() << ", " << point.getY() 
                          << ", " << point.getZ() << "): " << value << "\n";
            }
        }
    }
    
    void visit(FiniteElement& element) override {
        // Elements don't have direct field values
    }
    
    void visit(MeshRegion& region) override {
        std::cout << "\nAnalyzing field values in region: " << region.getName() << "\n";
    }
    
    void printFieldStatistics() const {
        if (valueCount_ > 0) {
            std::cout << "\n=== Field Value Statistics ===\n";
            std::cout << "Min value: " << minValue_ << "\n";
            std::cout << "Max value: " << maxValue_ << "\n";
            std::cout << "Average value: " << (sumValue_ / valueCount_) << "\n";
            std::cout << "Total points with values: " << valueCount_ << "\n";
        }
    }
};

// Mathematical expression tree for scientific computing
class MathExpressionVisitor;

class MathExpression {
public:
    virtual ~MathExpression() = default;
    virtual void accept(MathExpressionVisitor& visitor) = 0;
};

class ScalarExpression;
class VectorExpression;
class MatrixExpression;
class DotProductExpression;
class CrossProductExpression;
class GradientExpression;

class MathExpressionVisitor {
public:
    virtual ~MathExpressionVisitor() = default;
    virtual void visit(ScalarExpression& expr) = 0;
    virtual void visit(VectorExpression& expr) = 0;
    virtual void visit(MatrixExpression& expr) = 0;
    virtual void visit(DotProductExpression& expr) = 0;
    virtual void visit(CrossProductExpression& expr) = 0;
    virtual void visit(GradientExpression& expr) = 0;
};

class ScalarExpression : public MathExpression {
private:
    double value_;
    std::string name_;
    
public:
    ScalarExpression(double value, const std::string& name = "")
        : value_(value), name_(name) {}
    
    void accept(MathExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    double getValue() const { return value_; }
    const std::string& getName() const { return name_; }
};

class VectorExpression : public MathExpression {
private:
    std::vector<double> components_;
    std::string name_;
    
public:
    VectorExpression(const std::vector<double>& components, const std::string& name = "")
        : components_(components), name_(name) {}
    
    void accept(MathExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    const std::vector<double>& getComponents() const { return components_; }
    const std::string& getName() const { return name_; }
    size_t getDimension() const { return components_.size(); }
};

class MatrixExpression : public MathExpression {
private:
    std::vector<std::vector<double>> elements_;
    std::string name_;
    
public:
    MatrixExpression(const std::vector<std::vector<double>>& elements, 
                     const std::string& name = "")
        : elements_(elements), name_(name) {}
    
    void accept(MathExpressionVisitor& visitor) override {
        visitor.visit(*this);
    }
    
    const std::vector<std::vector<double>>& getElements() const { return elements_; }
    const std::string& getName() const { return name_; }
    size_t getRows() const { return elements_.size(); }
    size_t getCols() const { return elements_.empty() ? 0 : elements_[0].size(); }
};

class DotProductExpression : public MathExpression {
private:
    std::shared_ptr<VectorExpression> vec1_;
    std::shared_ptr<VectorExpression> vec2_;
    
public:
    DotProductExpression(std::shared_ptr<VectorExpression> vec1,
                        std::shared_ptr<VectorExpression> vec2)
        : vec1_(vec1), vec2_(vec2) {}
    
    void accept(MathExpressionVisitor& visitor) override {
        vec1_->accept(visitor);
        vec2_->accept(visitor);
        visitor.visit(*this);
    }
    
    std::shared_ptr<VectorExpression> getVector1() const { return vec1_; }
    std::shared_ptr<VectorExpression> getVector2() const { return vec2_; }
};

class CrossProductExpression : public MathExpression {
private:
    std::shared_ptr<VectorExpression> vec1_;
    std::shared_ptr<VectorExpression> vec2_;
    
public:
    CrossProductExpression(std::shared_ptr<VectorExpression> vec1,
                          std::shared_ptr<VectorExpression> vec2)
        : vec1_(vec1), vec2_(vec2) {}
    
    void accept(MathExpressionVisitor& visitor) override {
        vec1_->accept(visitor);
        vec2_->accept(visitor);
        visitor.visit(*this);
    }
    
    std::shared_ptr<VectorExpression> getVector1() const { return vec1_; }
    std::shared_ptr<VectorExpression> getVector2() const { return vec2_; }
};

class GradientExpression : public MathExpression {
private:
    std::shared_ptr<ScalarExpression> scalarField_;
    std::vector<std::string> variables_;
    
public:
    GradientExpression(std::shared_ptr<ScalarExpression> scalarField,
                      const std::vector<std::string>& variables)
        : scalarField_(scalarField), variables_(variables) {}
    
    void accept(MathExpressionVisitor& visitor) override {
        scalarField_->accept(visitor);
        visitor.visit(*this);
    }
    
    std::shared_ptr<ScalarExpression> getScalarField() const { return scalarField_; }
    const std::vector<std::string>& getVariables() const { return variables_; }
};

class MathEvaluatorVisitor : public MathExpressionVisitor {
private:
    std::vector<double> scalarStack_;
    std::vector<std::vector<double>> vectorStack_;
    
public:
    void visit(ScalarExpression& expr) override {
        scalarStack_.push_back(expr.getValue());
        std::cout << "Scalar " << expr.getName() << " = " << expr.getValue() << "\n";
    }
    
    void visit(VectorExpression& expr) override {
        vectorStack_.push_back(expr.getComponents());
        std::cout << "Vector " << expr.getName() << " = [";
        const auto& comp = expr.getComponents();
        for (size_t i = 0; i < comp.size(); ++i) {
            std::cout << comp[i];
            if (i < comp.size() - 1) std::cout << ", ";
        }
        std::cout << "]\n";
    }
    
    void visit(MatrixExpression& expr) override {
        std::cout << "Matrix " << expr.getName() << " (" 
                  << expr.getRows() << "x" << expr.getCols() << "):\n";
        for (const auto& row : expr.getElements()) {
            std::cout << "  [";
            for (size_t i = 0; i < row.size(); ++i) {
                std::cout << std::setw(8) << std::setprecision(3) << row[i];
                if (i < row.size() - 1) std::cout << ", ";
            }
            std::cout << "]\n";
        }
    }
    
    void visit(DotProductExpression& expr) override {
        auto vec2 = vectorStack_.back(); vectorStack_.pop_back();
        auto vec1 = vectorStack_.back(); vectorStack_.pop_back();
        
        double result = 0.0;
        for (size_t i = 0; i < vec1.size() && i < vec2.size(); ++i) {
            result += vec1[i] * vec2[i];
        }
        
        scalarStack_.push_back(result);
        std::cout << "Dot product result: " << result << "\n";
    }
    
    void visit(CrossProductExpression& expr) override {
        auto vec2 = vectorStack_.back(); vectorStack_.pop_back();
        auto vec1 = vectorStack_.back(); vectorStack_.pop_back();
        
        if (vec1.size() == 3 && vec2.size() == 3) {
            std::vector<double> result = {
                vec1[1]*vec2[2] - vec1[2]*vec2[1],
                vec1[2]*vec2[0] - vec1[0]*vec2[2],
                vec1[0]*vec2[1] - vec1[1]*vec2[0]
            };
            
            vectorStack_.push_back(result);
            std::cout << "Cross product result: [" 
                      << result[0] << ", " << result[1] << ", " << result[2] << "]\n";
        }
    }
    
    void visit(GradientExpression& expr) override {
        std::cout << "Gradient of scalar field with respect to ";
        for (const auto& var : expr.getVariables()) {
            std::cout << var << " ";
        }
        std::cout << "\n";
    }
};

class LaTeXVisitor : public MathExpressionVisitor {
private:
    std::stringstream latex_;
    
public:
    void visit(ScalarExpression& expr) override {
        if (!expr.getName().empty()) {
            latex_ << expr.getName();
        } else {
            latex_ << expr.getValue();
        }
    }
    
    void visit(VectorExpression& expr) override {
        latex_ << "\\vec{" << expr.getName() << "}";
    }
    
    void visit(MatrixExpression& expr) override {
        latex_ << "\\mathbf{" << expr.getName() << "}";
    }
    
    void visit(DotProductExpression& expr) override {
        latex_ << "\\vec{a} \\cdot \\vec{b}";
    }
    
    void visit(CrossProductExpression& expr) override {
        latex_ << "\\vec{a} \\times \\vec{b}";
    }
    
    void visit(GradientExpression& expr) override {
        latex_ << "\\nabla f";
    }
    
    std::string getLatex() const { return latex_.str(); }
};

int main() {
    std::cout << "=== Scientific Data Structure Analysis with Visitor Pattern ===\n\n";
    
    // Molecular Structure Analysis
    std::cout << "=== Molecular Structure Analysis ===\n";
    
    // Create water molecule (H2O)
    auto water = std::make_shared<Molecule>("Water (H2O)");
    
    auto O = std::make_shared<Atom>("O", 8, 0.0, 0.0, 0.0, -0.82);
    auto H1 = std::make_shared<Atom>("H", 1, 0.0, 0.757, 0.587, 0.41);
    auto H2 = std::make_shared<Atom>("H", 1, 0.0, -0.757, 0.587, 0.41);
    
    water->addAtom(O);
    water->addAtom(H1);
    water->addAtom(H2);
    water->addBond(O, H1, 1);  // O-H single bond
    water->addBond(O, H2, 1);  // O-H single bond
    
    // Create methane molecule (CH4)
    auto methane = std::make_shared<Molecule>("Methane (CH4)");
    
    auto C = std::make_shared<Atom>("C", 6, 0.0, 0.0, 0.0, -0.4);
    auto H3 = std::make_shared<Atom>("H", 1, 0.631, 0.631, 0.631, 0.1);
    auto H4 = std::make_shared<Atom>("H", 1, -0.631, -0.631, 0.631, 0.1);
    auto H5 = std::make_shared<Atom>("H", 1, -0.631, 0.631, -0.631, 0.1);
    auto H6 = std::make_shared<Atom>("H", 1, 0.631, -0.631, -0.631, 0.1);
    
    methane->addAtom(C);
    methane->addAtom(H3);
    methane->addAtom(H4);
    methane->addAtom(H5);
    methane->addAtom(H6);
    methane->addBond(C, H3, 1);
    methane->addBond(C, H4, 1);
    methane->addBond(C, H5, 1);
    methane->addBond(C, H6, 1);
    
    // Calculate molecular masses
    std::cout << "\n--- Molecular Mass Calculation ---\n";
    MolecularMassCalculator massCalc;
    water->accept(massCalc);
    std::cout << "Total mass of water: " << std::fixed << std::setprecision(3) 
              << massCalc.getTotalMass() << " Da\n";
    
    massCalc = MolecularMassCalculator();  // Reset
    methane->accept(massCalc);
    std::cout << "Total mass of methane: " << std::fixed << std::setprecision(3) 
              << massCalc.getTotalMass() << " Da\n";
    
    // Analyze charges
    std::cout << "\n--- Charge Analysis ---\n";
    ChargeCalculator chargeCalc;
    water->accept(chargeCalc);
    std::cout << "Net charge of water: " << chargeCalc.getTotalCharge() << "e\n";
    
    // Structure analysis
    std::cout << "\n--- Structure Analysis ---\n";
    StructureAnalyzer structAnalyzer;
    water->accept(structAnalyzer);
    methane->accept(structAnalyzer);
    structAnalyzer.printSummary();
    
    // Computational Mesh Analysis
    std::cout << "\n\n=== Computational Mesh Analysis ===\n";
    
    // Create a simple mesh region
    auto fluidRegion = std::make_shared<MeshRegion>("Fluid Domain", "Water");
    
    // Add grid points
    auto p1 = std::make_shared<GridPoint>(1, 0.0, 0.0, 0.0);
    auto p2 = std::make_shared<GridPoint>(2, 1.0, 0.0, 0.0);
    auto p3 = std::make_shared<GridPoint>(3, 0.0, 1.0, 0.0);
    auto p4 = std::make_shared<GridPoint>(4, 0.0, 0.0, 1.0);
    
    // Set field values (e.g., pressure)
    p1->setFieldValue(101325.0);  // 1 atm
    p2->setFieldValue(101500.0);
    p3->setFieldValue(101200.0);
    p4->setFieldValue(101400.0);
    
    // Create tetrahedral element
    auto tet1 = std::make_shared<FiniteElement>(1, "Tet");
    tet1->addNode(p1);
    tet1->addNode(p2);
    tet1->addNode(p3);
    tet1->addNode(p4);
    
    fluidRegion->addElement(tet1);
    
    // Analyze mesh
    std::cout << "--- Mesh Statistics ---\n";
    MeshStatisticsVisitor meshStats;
    fluidRegion->accept(meshStats);
    meshStats.printStatistics();
    
    // Analyze field values
    std::cout << "\n--- Field Value Analysis ---\n";
    FieldValueVisitor fieldAnalyzer;
    fluidRegion->accept(fieldAnalyzer);
    fieldAnalyzer.printFieldStatistics();
    
    // Mathematical Expression Analysis
    std::cout << "\n\n=== Mathematical Expression Analysis ===\n";
    
    // Create vectors and matrices
    auto vec1 = std::make_shared<VectorExpression>(
        std::vector<double>{1.0, 2.0, 3.0}, "v1"
    );
    auto vec2 = std::make_shared<VectorExpression>(
        std::vector<double>{4.0, 5.0, 6.0}, "v2"
    );
    
    auto matrix = std::make_shared<MatrixExpression>(
        std::vector<std::vector<double>>{
            {1.0, 2.0, 3.0},
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 9.0}
        }, "A"
    );
    
    // Create expressions
    auto dotProd = std::make_shared<DotProductExpression>(vec1, vec2);
    auto crossProd = std::make_shared<CrossProductExpression>(vec1, vec2);
    
    // Evaluate expressions
    std::cout << "--- Mathematical Evaluations ---\n";
    MathEvaluatorVisitor mathEval;
    
    std::cout << "\nVector and Matrix definitions:\n";
    vec1->accept(mathEval);
    vec2->accept(mathEval);
    matrix->accept(mathEval);
    
    std::cout << "\nDot Product v1 · v2:\n";
    dotProd->accept(mathEval);
    
    std::cout << "\nCross Product v1 × v2:\n";
    crossProd->accept(mathEval);
    
    // Generate LaTeX
    std::cout << "\n--- LaTeX Generation ---\n";
    LaTeXVisitor latexGen;
    
    std::cout << "v1: ";
    vec1->accept(latexGen);
    std::cout << latexGen.getLatex() << "\n";
    
    latexGen = LaTeXVisitor();  // Reset
    std::cout << "Matrix A: ";
    matrix->accept(latexGen);
    std::cout << latexGen.getLatex() << "\n";
    
    std::cout << "\n=== Visitor Pattern Summary ===\n";
    std::cout << "The Visitor pattern enables powerful operations on complex scientific data:\n";
    std::cout << "• Molecular structure analysis without modifying chemical classes\n";
    std::cout << "• Computational mesh statistics and field analysis\n";
    std::cout << "• Mathematical expression evaluation and formatting\n";
    std::cout << "\nThis pattern is essential for scientific computing where we need\n";
    std::cout << "multiple algorithms to operate on complex hierarchical data structures.\n";
    
    return 0;
}