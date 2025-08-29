// Flyweight Pattern - Molecular Dynamics Force Field Library
// Shares molecular parameters across millions of atoms in MD simulations
#include <iostream>
#include <unordered_map>
#include <memory>
#include <vector>
#include <random>
#include <cmath>
#include <iomanip>

// Flyweight interface - Shared molecular parameters
class AtomType {
private:
    std::string element_;
    double mass_;           // Atomic mass (amu)
    double radius_;         // Van der Waals radius (Angstroms)
    double charge_;         // Partial charge for electrostatics
    double epsilon_;        // Lennard-Jones well depth (kcal/mol)
    double sigma_;          // Lennard-Jones distance (Angstroms)
    std::vector<std::pair<std::string, double>> bondParams_; // Bond parameters
    
public:
    AtomType(const std::string& element, double mass, double radius, 
             double charge, double epsilon, double sigma)
        : element_(element), mass_(mass), radius_(radius), 
          charge_(charge), epsilon_(epsilon), sigma_(sigma) {
        // Initialize common bond parameters
        if (element == "C") {
            bondParams_ = {{"C-C", 1.54}, {"C-H", 1.09}, {"C-O", 1.43}, {"C-N", 1.47}};
        } else if (element == "H") {
            bondParams_ = {{"H-O", 0.96}, {"H-N", 1.01}};
        } else if (element == "O") {
            bondParams_ = {{"O-H", 0.96}, {"O=C", 1.23}};
        } else if (element == "N") {
            bondParams_ = {{"N-H", 1.01}, {"N-C", 1.47}};
        }
    }
    
    // Compute non-bonded interaction energy (intrinsic calculation)
    double computeLennardJones(double distance, const AtomType& other) const {
        double sigma_ij = (sigma_ + other.sigma_) / 2.0;
        double epsilon_ij = std::sqrt(epsilon_ * other.epsilon_);
        double r6 = std::pow(sigma_ij / distance, 6);
        double r12 = r6 * r6;
        return 4.0 * epsilon_ij * (r12 - r6);
    }
    
    double computeCoulomb(double distance, const AtomType& other) const {
        const double ke = 332.0637; // Coulomb constant in kcal*Angstrom/(mol*e^2)
        return ke * charge_ * other.charge_ / distance;
    }
    
    void displayInfo(int atomId, double x, double y, double z, double velocity) const {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Atom " << std::setw(6) << atomId << ": " 
                  << std::setw(2) << element_ 
                  << " at (" << std::setw(7) << x 
                  << ", " << std::setw(7) << y 
                  << ", " << std::setw(7) << z << ") Å"
                  << ", |v| = " << std::setw(6) << velocity << " Å/ps"
                  << " [m=" << mass_ << " amu, q=" << charge_ << " e]\n";
    }
    
    const std::string& getElement() const { return element_; }
    double getMass() const { return mass_; }
    double getCharge() const { return charge_; }
    double getEpsilon() const { return epsilon_; }
    double getSigma() const { return sigma_; }
    
    size_t getMemorySize() const {
        // Actual memory footprint of force field parameters
        return sizeof(*this) + bondParams_.size() * sizeof(std::pair<std::string, double>);
    }
};

// Flyweight factory - Force field parameter database
class ForceFieldLibrary {
private:
    static std::unordered_map<std::string, std::shared_ptr<AtomType>> atomTypes_;
    static std::string forceFieldName_;
    
public:
    static void loadForceField(const std::string& ffName) {
        forceFieldName_ = ffName;
        std::cout << "Loading " << ffName << " force field parameters...\n";
    }
    
    static std::shared_ptr<AtomType> getAtomType(const std::string& element,
                                                  const std::string& hybridization = "") {
        std::string key = element + (hybridization.empty() ? "" : "_" + hybridization);
        
        auto it = atomTypes_.find(key);
        if (it == atomTypes_.end()) {
            std::cout << "  Loading parameters for " << key << "...\n";
            
            // AMBER/CHARMM-like parameters (simplified)
            if (element == "C") {
                if (hybridization == "sp3") {
                    auto atomType = std::make_shared<AtomType>("C", 12.011, 1.908, -0.18, 0.1094, 3.3997);
                    atomTypes_[key] = atomType;
                    return atomType;
                } else if (hybridization == "sp2") {
                    auto atomType = std::make_shared<AtomType>("C", 12.011, 1.908, 0.0, 0.0860, 3.3997);
                    atomTypes_[key] = atomType;
                    return atomType;
                }
            } else if (element == "H") {
                auto atomType = std::make_shared<AtomType>("H", 1.008, 1.487, 0.06, 0.0157, 2.6495);
                atomTypes_[key] = atomType;
                return atomType;
            } else if (element == "O") {
                auto atomType = std::make_shared<AtomType>("O", 15.999, 1.661, -0.50, 0.2100, 2.9599);
                atomTypes_[key] = atomType;
                return atomType;
            } else if (element == "N") {
                auto atomType = std::make_shared<AtomType>("N", 14.007, 1.824, -0.47, 0.1700, 3.2500);
                atomTypes_[key] = atomType;
                return atomType;
            } else if (element == "S") {
                auto atomType = std::make_shared<AtomType>("S", 32.065, 2.000, -0.40, 0.2500, 3.5636);
                atomTypes_[key] = atomType;
                return atomType;
            }
        }
        
        return it->second;
    }
    
    static size_t getAtomTypeCount() {
        return atomTypes_.size();
    }
    
    static size_t getTotalMemory() {
        size_t total = 0;
        for (const auto& [key, type] : atomTypes_) {
            total += type->getMemorySize();
        }
        return total;
    }
    
    static void displayLoadedTypes() {
        std::cout << "\nLoaded atom types in " << forceFieldName_ << ":\n";
        for (const auto& [key, type] : atomTypes_) {
            std::cout << "  - " << key << ": ε=" << type->getEpsilon() 
                      << " kcal/mol, σ=" << type->getSigma() << " Å\n";
        }
    }
};

// Context class containing extrinsic state - Individual atom in simulation
class Atom {
private:
    int atomId_;                          // Unique atom identifier
    double x_, y_, z_;                    // Position (extrinsic)
    double vx_, vy_, vz_;                 // Velocity (extrinsic)
    double fx_, fy_, fz_;                 // Force (extrinsic)
    std::shared_ptr<AtomType> type_;     // Shared flyweight parameters
    std::vector<int> bondedAtoms_;       // Connectivity (extrinsic)
    
public:
    Atom(int id, double x, double y, double z, std::shared_ptr<AtomType> type)
        : atomId_(id), x_(x), y_(y), z_(z), 
          vx_(0), vy_(0), vz_(0), 
          fx_(0), fy_(0), fz_(0),
          type_(type) {}
    
    void setVelocity(double vx, double vy, double vz) {
        vx_ = vx; vy_ = vy; vz_ = vz;
    }
    
    void addBond(int atomId) {
        bondedAtoms_.push_back(atomId);
    }
    
    double computeDistance(const Atom& other) const {
        double dx = x_ - other.x_;
        double dy = y_ - other.y_;
        double dz = z_ - other.z_;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
    
    double computeKineticEnergy() const {
        double v2 = vx_*vx_ + vy_*vy_ + vz_*vz_;
        return 0.5 * type_->getMass() * v2 * 0.01; // Convert to kcal/mol
    }
    
    void display() const {
        double velocity = std::sqrt(vx_*vx_ + vy_*vy_ + vz_*vz_);
        type_->displayInfo(atomId_, x_, y_, z_, velocity);
    }
    
    size_t getMemorySize() const {
        // Only extrinsic state size
        return sizeof(*this) + bondedAtoms_.size() * sizeof(int);
    }
    
    std::shared_ptr<AtomType> getType() const { return type_; }
    int getId() const { return atomId_; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
};

// Molecular system managing millions of atoms
class MolecularSystem {
private:
    std::vector<Atom> atoms_;
    std::string systemName_;
    double boxX_, boxY_, boxZ_;  // Simulation box dimensions
    double temperature_;         // System temperature (K)
    double totalEnergy_;
    
public:
    MolecularSystem(const std::string& name, double boxSize, double temp)
        : systemName_(name), boxX_(boxSize), boxY_(boxSize), boxZ_(boxSize),
          temperature_(temp), totalEnergy_(0.0) {}
    
    void addAtom(double x, double y, double z, 
                 const std::string& element,
                 const std::string& hybridization = "") {
        auto type = ForceFieldLibrary::getAtomType(element, hybridization);
        int atomId = atoms_.size() + 1;
        atoms_.emplace_back(atomId, x, y, z, type);
    }
    
    void addProteinBackbone(double startX, double startY, double startZ, int length) {
        // Simplified protein backbone (N-Ca-C pattern)
        for (int i = 0; i < length; ++i) {
            double offset = i * 3.8; // Approximate residue spacing
            
            // Nitrogen
            addAtom(startX + offset, startY, startZ, "N");
            if (i > 0) atoms_.back().addBond(atoms_.size() - 3);
            
            // Alpha carbon
            addAtom(startX + offset + 1.5, startY, startZ, "C", "sp3");
            atoms_.back().addBond(atoms_.size() - 1);
            
            // Carbonyl carbon
            addAtom(startX + offset + 2.5, startY, startZ + 0.5, "C", "sp2");
            atoms_.back().addBond(atoms_.size() - 1);
            
            // Carbonyl oxygen
            addAtom(startX + offset + 2.5, startY, startZ + 1.7, "O");
            atoms_.back().addBond(atoms_.size() - 1);
        }
    }
    
    void initializeVelocities() {
        // Maxwell-Boltzmann distribution
        std::random_device rd;
        std::mt19937 gen(rd());
        
        for (auto& atom : atoms_) {
            double kT = 0.001987 * temperature_; // Boltzmann constant * T
            double sigma = std::sqrt(kT / atom.getType()->getMass());
            std::normal_distribution<> vel_dist(0.0, sigma * 100); // Convert to Å/ps
            
            atom.setVelocity(vel_dist(gen), vel_dist(gen), vel_dist(gen));
        }
    }
    
    double computeTotalEnergy() {
        double kineticEnergy = 0.0;
        double potentialEnergy = 0.0;
        
        // Kinetic energy
        for (const auto& atom : atoms_) {
            kineticEnergy += atom.computeKineticEnergy();
        }
        
        // Non-bonded interactions (simplified - only nearest neighbors)
        for (size_t i = 0; i < atoms_.size(); ++i) {
            for (size_t j = i + 1; j < atoms_.size(); ++j) {
                double dist = atoms_[i].computeDistance(atoms_[j]);
                if (dist < 12.0) { // Cutoff distance
                    potentialEnergy += atoms_[i].getType()->computeLennardJones(
                        dist, *atoms_[j].getType());
                    potentialEnergy += atoms_[i].getType()->computeCoulomb(
                        dist, *atoms_[j].getType());
                }
            }
        }
        
        totalEnergy_ = kineticEnergy + potentialEnergy;
        return totalEnergy_;
    }
    
    void displaySystem() const {
        std::cout << "\nMolecular System: " << systemName_ << "\n";
        std::cout << "Box dimensions: " << boxX_ << " × " << boxY_ 
                  << " × " << boxZ_ << " Å³\n";
        std::cout << "Temperature: " << temperature_ << " K\n";
        std::cout << "Total atoms: " << atoms_.size() << "\n";
    }
    
    void displaySample(int count) const {
        std::cout << "\nSample atoms:\n";
        for (int i = 0; i < count && i < atoms_.size(); ++i) {
            atoms_[i].display();
        }
    }
    
    size_t getAtomCount() const {
        return atoms_.size();
    }
    
    size_t getTotalMemory() const {
        size_t extrinsicMemory = 0;
        for (const auto& atom : atoms_) {
            extrinsicMemory += atom.getMemorySize();
        }
        
        size_t intrinsicMemory = ForceFieldLibrary::getTotalMemory();
        
        return extrinsicMemory + intrinsicMemory;
    }
    
    size_t getMemoryWithoutFlyweight() const {
        // If each atom stored its own force field parameters
        size_t paramSize = 200; // Estimated bytes for all parameters
        return atoms_.size() * (sizeof(Atom) + paramSize);
    }
};

int main() {
    std::cout << "=== Molecular Dynamics Simulation with Flyweight Pattern ===\n";
    std::cout << "Sharing force field parameters across millions of atoms\n\n";
    
    // Load force field
    ForceFieldLibrary::loadForceField("AMBER ff14SB");
    
    // Create molecular system
    MolecularSystem proteinSystem("Protein in Water", 100.0, 300.0);
    
    // Build a small protein (10 residues)
    std::cout << "\nBuilding protein structure...\n";
    proteinSystem.addProteinBackbone(10.0, 10.0, 10.0, 10);
    
    // Add water molecules around the protein
    std::cout << "\nAdding solvent molecules...\n";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> pos_dist(5.0, 95.0);
    
    // Add 10000 water molecules (30000 atoms)
    for (int i = 0; i < 10000; ++i) {
        double x = pos_dist(gen);
        double y = pos_dist(gen);
        double z = pos_dist(gen);
        
        // Water molecule (O-H-H)
        proteinSystem.addAtom(x, y, z, "O");
        proteinSystem.addAtom(x + 0.8, y + 0.6, z, "H");
        proteinSystem.addAtom(x - 0.8, y + 0.6, z, "H");
    }
    
    // Initialize velocities
    proteinSystem.initializeVelocities();
    
    // Display force field information
    ForceFieldLibrary::displayLoadedTypes();
    
    // Display system information
    proteinSystem.displaySystem();
    
    // Memory statistics
    std::cout << "\nMemory Usage Statistics:\n";
    std::cout << "========================\n";
    std::cout << "Total atoms: " << proteinSystem.getAtomCount() << "\n";
    std::cout << "Unique atom types: " << ForceFieldLibrary::getAtomTypeCount() << "\n";
    std::cout << "Memory with flyweight: " << proteinSystem.getTotalMemory() / 1024.0 
              << " KB\n";
    std::cout << "Memory without flyweight: " << proteinSystem.getMemoryWithoutFlyweight() / 1024.0 
              << " KB\n";
    std::cout << "Memory saved: " 
              << (proteinSystem.getMemoryWithoutFlyweight() - proteinSystem.getTotalMemory()) / 1024.0 
              << " KB\n";
    std::cout << "Savings percentage: " 
              << (1.0 - (double)proteinSystem.getTotalMemory() / proteinSystem.getMemoryWithoutFlyweight()) * 100 
              << "%\n";
    
    // Display sample atoms
    proteinSystem.displaySample(10);
    
    // Compute initial energy
    std::cout << "\nComputing system energy...\n";
    double energy = proteinSystem.computeTotalEnergy();
    std::cout << "Total energy: " << std::fixed << std::setprecision(2) 
              << energy << " kcal/mol\n";
    
    // Small test system to show parameter sharing
    std::cout << "\n=== Small Test System ===\n";
    MolecularSystem testSystem("Methane in vacuum", 20.0, 300.0);
    
    // Methane molecule (CH4)
    testSystem.addAtom(10.0, 10.0, 10.0, "C", "sp3");
    testSystem.addAtom(10.0, 11.09, 10.0, "H");
    testSystem.addAtom(10.0, 9.45, 10.89, "H");
    testSystem.addAtom(10.89, 9.45, 9.45, "H");
    testSystem.addAtom(9.11, 9.45, 9.45, "H");
    
    testSystem.displaySystem();
    testSystem.displaySample(5);
    
    std::cout << "\nFlyweight pattern enables efficient molecular dynamics\n";
    std::cout << "simulations by sharing force field parameters!\n";
    
    return 0;
}

// Static member initialization
std::unordered_map<std::string, std::shared_ptr<AtomType>> ForceFieldLibrary::atomTypes_;
std::string ForceFieldLibrary::forceFieldName_;