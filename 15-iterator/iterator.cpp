// Iterator Pattern - Scientific Dataset Traversal
// Provides uniform access to heterogeneous scientific data structures
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <iomanip>
#include <algorithm>

// Forward declarations
template<typename T> class DataIterator;
template<typename T> class ScientificDataset;

// Iterator interface for scientific data
template<typename T>
class DataIterator {
public:
    virtual ~DataIterator() = default;
    virtual bool hasNext() const = 0;
    virtual T next() = 0;
    virtual void reset() = 0;
    virtual size_t currentIndex() const = 0;
    virtual double getProgress() const = 0;  // For long-running iterations
};

// Dataset interface
template<typename T>
class ScientificDataset {
public:
    virtual ~ScientificDataset() = default;
    virtual std::unique_ptr<DataIterator<T>> createIterator() = 0;
    virtual size_t size() const = 0;
    virtual std::string getDatasetName() const = 0;
};

// Data point for time series
struct TimeSeriesPoint {
    double time;
    double value;
    double error;  // Measurement uncertainty
    
    TimeSeriesPoint(double t = 0, double v = 0, double e = 0) 
        : time(t), value(v), error(e) {}
};

// Concrete dataset - Time Series Data
class TimeSeriesDataset : public ScientificDataset<TimeSeriesPoint> {
private:
    std::vector<TimeSeriesPoint> data_;
    std::string name_;
    std::string units_;
    double samplingRate_;  // Hz
    
public:
    TimeSeriesDataset(const std::string& name, const std::string& units, double rate)
        : name_(name), units_(units), samplingRate_(rate) {}
    
    void addDataPoint(double time, double value, double error = 0.0) {
        data_.emplace_back(time, value, error);
    }
    
    void generateSyntheticData(size_t points, double frequency) {
        data_.clear();
        for (size_t i = 0; i < points; ++i) {
            double t = i / samplingRate_;
            double value = std::sin(2 * M_PI * frequency * t) + 
                          0.1 * std::sin(2 * M_PI * frequency * 3 * t);
            double error = 0.05 * (rand() / (double)RAND_MAX);
            addDataPoint(t, value, error);
        }
    }
    
    // Different iterator types
    enum class IterationType {
        SEQUENTIAL,      // All points in order
        WINDOWED,        // Moving window
        DECIMATED,       // Every nth point
        THRESHOLD_BASED  // Only points above threshold
    };
    
    // Sequential iterator - all points
    class SequentialIterator : public DataIterator<TimeSeriesPoint> {
    private:
        const std::vector<TimeSeriesPoint>& data_;
        size_t current_ = 0;
        
    public:
        SequentialIterator(const std::vector<TimeSeriesPoint>& data) 
            : data_(data) {}
        
        bool hasNext() const override {
            return current_ < data_.size();
        }
        
        TimeSeriesPoint next() override {
            if (!hasNext()) {
                throw std::out_of_range("No more data points");
            }
            return data_[current_++];
        }
        
        void reset() override {
            current_ = 0;
        }
        
        size_t currentIndex() const override {
            return current_;
        }
        
        double getProgress() const override {
            return data_.empty() ? 1.0 : 
                   static_cast<double>(current_) / data_.size();
        }
    };
    
    // Windowed iterator - sliding window
    class WindowedIterator : public DataIterator<std::vector<TimeSeriesPoint>> {
    private:
        const std::vector<TimeSeriesPoint>& data_;
        size_t windowSize_;
        size_t stride_;
        size_t current_ = 0;
        
    public:
        WindowedIterator(const std::vector<TimeSeriesPoint>& data, 
                        size_t windowSize, size_t stride)
            : data_(data), windowSize_(windowSize), stride_(stride) {}
        
        bool hasNext() const override {
            return current_ + windowSize_ <= data_.size();
        }
        
        std::vector<TimeSeriesPoint> next() override {
            if (!hasNext()) {
                throw std::out_of_range("No more windows");
            }
            
            std::vector<TimeSeriesPoint> window;
            for (size_t i = 0; i < windowSize_; ++i) {
                window.push_back(data_[current_ + i]);
            }
            current_ += stride_;
            return window;
        }
        
        void reset() override {
            current_ = 0;
        }
        
        size_t currentIndex() const override {
            return current_ / stride_;
        }
        
        double getProgress() const override {
            size_t totalWindows = (data_.size() - windowSize_) / stride_ + 1;
            return totalWindows == 0 ? 1.0 : 
                   static_cast<double>(currentIndex()) / totalWindows;
        }
    };
    
    // Decimated iterator - for downsampling
    class DecimatedIterator : public DataIterator<TimeSeriesPoint> {
    private:
        const std::vector<TimeSeriesPoint>& data_;
        size_t decimationFactor_;
        size_t current_ = 0;
        
    public:
        DecimatedIterator(const std::vector<TimeSeriesPoint>& data, 
                         size_t factor)
            : data_(data), decimationFactor_(factor) {}
        
        bool hasNext() const override {
            return current_ < data_.size();
        }
        
        TimeSeriesPoint next() override {
            if (!hasNext()) {
                throw std::out_of_range("No more data points");
            }
            TimeSeriesPoint point = data_[current_];
            current_ += decimationFactor_;
            return point;
        }
        
        void reset() override {
            current_ = 0;
        }
        
        size_t currentIndex() const override {
            return current_ / decimationFactor_;
        }
        
        double getProgress() const override {
            return data_.empty() ? 1.0 : 
                   std::min(1.0, static_cast<double>(current_) / data_.size());
        }
    };
    
    std::unique_ptr<DataIterator<TimeSeriesPoint>> createIterator() override {
        return std::make_unique<SequentialIterator>(data_);
    }
    
    std::unique_ptr<DataIterator<std::vector<TimeSeriesPoint>>> 
    createWindowedIterator(size_t windowSize, size_t stride) {
        return std::make_unique<WindowedIterator>(data_, windowSize, stride);
    }
    
    std::unique_ptr<DataIterator<TimeSeriesPoint>> 
    createDecimatedIterator(size_t factor) {
        return std::make_unique<DecimatedIterator>(data_, factor);
    }
    
    size_t size() const override { return data_.size(); }
    std::string getDatasetName() const override { return name_; }
    std::string getUnits() const { return units_; }
    double getSamplingRate() const { return samplingRate_; }
};

// Grid data point
struct GridPoint {
    size_t i, j, k;    // Grid indices
    double value;      // Field value
    double x, y, z;    // Physical coordinates
    
    GridPoint(size_t ii = 0, size_t jj = 0, size_t kk = 0, 
              double v = 0, double xx = 0, double yy = 0, double zz = 0)
        : i(ii), j(jj), k(kk), value(v), x(xx), y(yy), z(zz) {}
};

// 3D Grid Dataset for CFD/FEM simulations
class GridDataset : public ScientificDataset<GridPoint> {
private:
    std::vector<std::vector<std::vector<double>>> grid_;
    size_t nx_, ny_, nz_;
    double dx_, dy_, dz_;
    std::string name_;
    std::string fieldName_;
    
public:
    GridDataset(const std::string& name, const std::string& field,
                size_t nx, size_t ny, size_t nz,
                double dx, double dy, double dz)
        : name_(name), fieldName_(field), 
          nx_(nx), ny_(ny), nz_(nz),
          dx_(dx), dy_(dy), dz_(dz) {
        // Initialize grid
        grid_.resize(nx_);
        for (size_t i = 0; i < nx_; ++i) {
            grid_[i].resize(ny_);
            for (size_t j = 0; j < ny_; ++j) {
                grid_[i][j].resize(nz_, 0.0);
            }
        }
    }
    
    void setGridValue(size_t i, size_t j, size_t k, double value) {
        if (i < nx_ && j < ny_ && k < nz_) {
            grid_[i][j][k] = value;
        }
    }
    
    void generateTestField() {
        // Generate a test scalar field (e.g., temperature distribution)
        for (size_t i = 0; i < nx_; ++i) {
            for (size_t j = 0; j < ny_; ++j) {
                for (size_t k = 0; k < nz_; ++k) {
                    double x = i * dx_;
                    double y = j * dy_;
                    double z = k * dz_;
                    // Gaussian blob
                    double r2 = (x-nx_*dx_/2)*(x-nx_*dx_/2) + 
                               (y-ny_*dy_/2)*(y-ny_*dy_/2) + 
                               (z-nz_*dz_/2)*(z-nz_*dz_/2);
                    grid_[i][j][k] = 100.0 * std::exp(-r2 / (nx_*dx_*nx_*dx_/16));
                }
            }
        }
    }
    
    // Iterator traversal patterns
    enum class TraversalPattern {
        ROW_MAJOR,      // i varies fastest (cache-friendly for C arrays)
        COLUMN_MAJOR,   // k varies fastest
        MORTON_ORDER,   // Z-order curve for better cache locality
        BOUNDARY_ONLY   // Only boundary cells
    };
    
    // Row-major iterator (most common for scientific computing)
    class RowMajorIterator : public DataIterator<GridPoint> {
    private:
        const GridDataset& dataset_;
        size_t i_ = 0, j_ = 0, k_ = 0;
        
    public:
        RowMajorIterator(const GridDataset& dataset) : dataset_(dataset) {}
        
        bool hasNext() const override {
            return i_ < dataset_.nx_;
        }
        
        GridPoint next() override {
            if (!hasNext()) {
                throw std::out_of_range("No more grid points");
            }
            
            GridPoint point(i_, j_, k_, 
                           dataset_.grid_[i_][j_][k_],
                           i_ * dataset_.dx_,
                           j_ * dataset_.dy_,
                           k_ * dataset_.dz_);
            
            // Advance indices
            k_++;
            if (k_ >= dataset_.nz_) {
                k_ = 0;
                j_++;
                if (j_ >= dataset_.ny_) {
                    j_ = 0;
                    i_++;
                }
            }
            
            return point;
        }
        
        void reset() override {
            i_ = j_ = k_ = 0;
        }
        
        size_t currentIndex() const override {
            return i_ * dataset_.ny_ * dataset_.nz_ + 
                   j_ * dataset_.nz_ + k_;
        }
        
        double getProgress() const override {
            return static_cast<double>(currentIndex()) / 
                   (dataset_.nx_ * dataset_.ny_ * dataset_.nz_);
        }
    };
    
    // Boundary-only iterator (for boundary conditions)
    class BoundaryIterator : public DataIterator<GridPoint> {
    private:
        const GridDataset& dataset_;
        std::vector<GridPoint> boundaryPoints_;
        size_t current_ = 0;
        
        void collectBoundaryPoints() {
            // Collect all boundary points
            for (size_t i = 0; i < dataset_.nx_; ++i) {
                for (size_t j = 0; j < dataset_.ny_; ++j) {
                    for (size_t k = 0; k < dataset_.nz_; ++k) {
                        if (i == 0 || i == dataset_.nx_-1 ||
                            j == 0 || j == dataset_.ny_-1 ||
                            k == 0 || k == dataset_.nz_-1) {
                            boundaryPoints_.emplace_back(
                                i, j, k, dataset_.grid_[i][j][k],
                                i * dataset_.dx_, j * dataset_.dy_, k * dataset_.dz_
                            );
                        }
                    }
                }
            }
        }
        
    public:
        BoundaryIterator(const GridDataset& dataset) : dataset_(dataset) {
            collectBoundaryPoints();
        }
        
        bool hasNext() const override {
            return current_ < boundaryPoints_.size();
        }
        
        GridPoint next() override {
            if (!hasNext()) {
                throw std::out_of_range("No more boundary points");
            }
            return boundaryPoints_[current_++];
        }
        
        void reset() override {
            current_ = 0;
        }
        
        size_t currentIndex() const override {
            return current_;
        }
        
        double getProgress() const override {
            return boundaryPoints_.empty() ? 1.0 :
                   static_cast<double>(current_) / boundaryPoints_.size();
        }
    };
    
    std::unique_ptr<DataIterator<GridPoint>> createIterator() override {
        return std::make_unique<RowMajorIterator>(*this);
    }
    
    std::unique_ptr<DataIterator<GridPoint>> 
    createBoundaryIterator() {
        return std::make_unique<BoundaryIterator>(*this);
    }
    
    size_t size() const override { return nx_ * ny_ * nz_; }
    std::string getDatasetName() const override { return name_; }
    std::string getFieldName() const { return fieldName_; }
    size_t getDimensions(size_t& nx, size_t& ny, size_t& nz) const {
        nx = nx_; ny = ny_; nz = nz_;
        return nx_ * ny_ * nz_;
    }
};

// Analysis functions using iterators
void analyzeTimeSeries(TimeSeriesDataset& dataset) {
    std::cout << "\nAnalyzing: " << dataset.getDatasetName() 
              << " (" << dataset.size() << " points)\n";
    
    // Sequential analysis
    auto it = dataset.createIterator();
    double sum = 0, sumSq = 0;
    double minVal = INFINITY, maxVal = -INFINITY;
    
    while (it->hasNext()) {
        auto point = it->next();
        sum += point.value;
        sumSq += point.value * point.value;
        minVal = std::min(minVal, point.value);
        maxVal = std::max(maxVal, point.value);
        
        if (it->currentIndex() % 100 == 0) {
            std::cout << "  Progress: " << std::fixed << std::setprecision(1) 
                      << (it->getProgress() * 100) << "%\r" << std::flush;
        }
    }
    
    double mean = sum / dataset.size();
    double variance = sumSq / dataset.size() - mean * mean;
    double stdDev = std::sqrt(variance);
    
    std::cout << "\n  Statistics:\n";
    std::cout << "    Mean: " << std::fixed << std::setprecision(4) << mean << "\n";
    std::cout << "    Std Dev: " << stdDev << "\n";
    std::cout << "    Range: [" << minVal << ", " << maxVal << "]\n";
}

void analyzeWindowed(TimeSeriesDataset& dataset, size_t windowSize) {
    std::cout << "\n  Windowed analysis (window=" << windowSize << "):\n";
    auto winIt = dataset.createWindowedIterator(windowSize, windowSize/2);
    
    int windowNum = 0;
    while (winIt->hasNext() && windowNum < 5) { // Show first 5 windows
        auto window = winIt->next();
        double winMean = 0;
        for (const auto& point : window) {
            winMean += point.value;
        }
        winMean /= window.size();
        
        std::cout << "    Window " << windowNum++ << ": "
                  << "t=[" << std::fixed << std::setprecision(3) 
                  << window.front().time << ", " << window.back().time 
                  << "], mean=" << winMean << "\n";
    }
}

void analyzeGrid(GridDataset& dataset) {
    size_t nx, ny, nz;
    dataset.getDimensions(nx, ny, nz);
    
    std::cout << "\nAnalyzing: " << dataset.getDatasetName() 
              << " - " << dataset.getFieldName()
              << " (" << nx << "×" << ny << "×" << nz << " grid)\n";
    
    // Analyze full grid
    auto it = dataset.createIterator();
    double sum = 0, maxVal = -INFINITY;
    size_t maxI = 0, maxJ = 0, maxK = 0;
    
    while (it->hasNext()) {
        auto point = it->next();
        sum += point.value;
        if (point.value > maxVal) {
            maxVal = point.value;
            maxI = point.i;
            maxJ = point.j;
            maxK = point.k;
        }
    }
    
    std::cout << "  Grid statistics:\n";
    std::cout << "    Total cells: " << dataset.size() << "\n";
    std::cout << "    Average value: " << sum / dataset.size() << "\n";
    std::cout << "    Maximum value: " << maxVal 
              << " at (" << maxI << ", " << maxJ << ", " << maxK << ")\n";
    
    // Analyze boundary
    auto boundIt = dataset.createBoundaryIterator();
    size_t boundaryCount = 0;
    double boundarySum = 0;
    
    while (boundIt->hasNext()) {
        auto point = boundIt->next();
        boundarySum += point.value;
        boundaryCount++;
    }
    
    std::cout << "  Boundary statistics:\n";
    std::cout << "    Boundary cells: " << boundaryCount << "\n";
    std::cout << "    Average boundary value: " 
              << boundarySum / boundaryCount << "\n";
}

int main() {
    std::cout << "=== Scientific Dataset Iterator Pattern Demo ===\n\n";
    
    // Time Series Dataset
    std::cout << "1. Time Series Dataset\n";
    std::cout << "======================\n";
    TimeSeriesDataset tempData("Temperature Measurements", "Celsius", 100.0);
    tempData.generateSyntheticData(1000, 0.5); // 1000 points at 0.5 Hz signal
    
    analyzeTimeSeries(tempData);
    analyzeWindowed(tempData, 50);
    
    // Decimated iteration for visualization
    std::cout << "\n  Decimated data (factor=100):\n";
    auto decIt = tempData.createDecimatedIterator(100);
    int count = 0;
    while (decIt->hasNext() && count < 5) {
        auto point = decIt->next();
        std::cout << "    t=" << std::fixed << std::setprecision(3) 
                  << point.time << "s, value=" << std::setprecision(4) 
                  << point.value << " ± " << point.error << "\n";
        count++;
    }
    
    // 3D Grid Dataset
    std::cout << "\n\n2. 3D Grid Dataset\n";
    std::cout << "==================\n";
    GridDataset tempField("Heat Transfer Simulation", "Temperature", 
                         20, 20, 20, 0.1, 0.1, 0.1);
    tempField.generateTestField();
    
    analyzeGrid(tempField);
    
    // Show iteration patterns
    std::cout << "\n  Sample grid points (row-major order):\n";
    auto gridIt = tempField.createIterator();
    count = 0;
    while (gridIt->hasNext() && count < 5) {
        auto point = gridIt->next();
        std::cout << "    (" << point.i << "," << point.j << "," << point.k 
                  << ") at (" << std::fixed << std::setprecision(1)
                  << point.x << "," << point.y << "," << point.z
                  << ") = " << std::setprecision(3) << point.value << "\n";
        count++;
    }
    
    // Performance comparison
    std::cout << "\n\n3. Iterator Performance Comparison\n";
    std::cout << "==================================\n";
    
    // Large dataset
    TimeSeriesDataset largeData("Large Dataset", "Units", 1000.0);
    largeData.generateSyntheticData(100000, 1.0);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto seqIt = largeData.createIterator();
    double sum = 0;
    while (seqIt->hasNext()) {
        sum += seqIt->next().value;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto seqTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "  Sequential iteration (100k points): " 
              << seqTime.count() << " μs\n";
    
    start = std::chrono::high_resolution_clock::now();
    auto decIt2 = largeData.createDecimatedIterator(10);
    sum = 0;
    while (decIt2->hasNext()) {
        sum += decIt2->next().value;
    }
    end = std::chrono::high_resolution_clock::now();
    auto decTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "  Decimated iteration (10k points): " 
              << decTime.count() << " μs\n";
    std::cout << "  Speedup: " << std::fixed << std::setprecision(2)
              << (double)seqTime.count() / decTime.count() << "x\n";
    
    std::cout << "\nIterator pattern provides flexible traversal\n";
    std::cout << "of scientific datasets with different access patterns!\n";
    
    return 0;
}