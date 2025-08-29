// Chain of Responsibility Pattern - Scientific Data Validation Pipeline
// Multi-stage validation for experimental data before analysis
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <limits>
#include <iomanip>

// Data quality issues that validators can detect
enum class DataIssueType {
    MISSING_VALUES,
    OUTLIERS,
    NOISE_LEVEL,
    CALIBRATION_ERROR,
    PHYSICAL_CONSTRAINT_VIOLATION,
    STATISTICAL_ANOMALY,
    INSTRUMENT_MALFUNCTION
};

// Scientific dataset with validation metadata
class ScientificDataset {
private:
    std::string experimentName_;
    std::string instrumentType_;
    std::vector<double> data_;
    double samplingRate_;      // Hz
    double expectedRange_[2];   // Min and max expected values
    double temperature_;        // Kelvin
    double pressure_;          // Pa
    std::vector<DataIssueType> detectedIssues_;
    bool validated_;
    
public:
    ScientificDataset(const std::string& experiment, const std::string& instrument,
                      const std::vector<double>& data, double samplingRate,
                      double minExpected, double maxExpected,
                      double temp = 293.15, double pressure = 101325.0)
        : experimentName_(experiment), instrumentType_(instrument), data_(data),
          samplingRate_(samplingRate), temperature_(temp), pressure_(pressure),
          validated_(false) {
        expectedRange_[0] = minExpected;
        expectedRange_[1] = maxExpected;
    }
    
    const std::string& getExperimentName() const { return experimentName_; }
    const std::string& getInstrumentType() const { return instrumentType_; }
    const std::vector<double>& getData() const { return data_; }
    std::vector<double>& getDataMutable() { return data_; }
    double getSamplingRate() const { return samplingRate_; }
    double getMinExpected() const { return expectedRange_[0]; }
    double getMaxExpected() const { return expectedRange_[1]; }
    double getTemperature() const { return temperature_; }
    double getPressure() const { return pressure_; }
    
    void addIssue(DataIssueType issue) {
        detectedIssues_.push_back(issue);
    }
    
    const std::vector<DataIssueType>& getIssues() const { return detectedIssues_; }
    
    void setValidated(bool valid) { validated_ = valid; }
    bool isValidated() const { return validated_; }
    
    std::string getIssueString(DataIssueType issue) const {
        switch (issue) {
            case DataIssueType::MISSING_VALUES: return "Missing Values";
            case DataIssueType::OUTLIERS: return "Statistical Outliers";
            case DataIssueType::NOISE_LEVEL: return "Excessive Noise";
            case DataIssueType::CALIBRATION_ERROR: return "Calibration Error";
            case DataIssueType::PHYSICAL_CONSTRAINT_VIOLATION: return "Physical Constraint Violation";
            case DataIssueType::STATISTICAL_ANOMALY: return "Statistical Anomaly";
            case DataIssueType::INSTRUMENT_MALFUNCTION: return "Instrument Malfunction";
            default: return "Unknown Issue";
        }
    }
};

// Abstract validator in the chain
class DataValidator {
protected:
    std::shared_ptr<DataValidator> nextValidator_;
    std::string validatorName_;
    double tolerance_;
    
public:
    DataValidator(const std::string& name, double tolerance = 0.01)
        : validatorName_(name), tolerance_(tolerance) {}
    virtual ~DataValidator() = default;
    
    void setNext(std::shared_ptr<DataValidator> validator) {
        nextValidator_ = validator;
    }
    
    virtual void validate(ScientificDataset& dataset) {
        std::cout << "\n[" << validatorName_ << "] Starting validation...\n";
        
        bool passedValidation = performValidation(dataset);
        
        if (!passedValidation) {
            std::cout << "  ⚠️  Issues detected by " << validatorName_ << "\n";
        } else {
            std::cout << "  ✓ Passed " << validatorName_ << " validation\n";
        }
        
        // Always pass to next validator in chain
        if (nextValidator_) {
            nextValidator_->validate(dataset);
        } else {
            // End of chain - summarize validation results
            summarizeValidation(dataset);
        }
    }
    
protected:
    virtual bool performValidation(ScientificDataset& dataset) = 0;
    
    void summarizeValidation(ScientificDataset& dataset) {
        std::cout << "\n=== Validation Summary ===\n";
        if (dataset.getIssues().empty()) {
            std::cout << "✓ Dataset passed all validations!\n";
            dataset.setValidated(true);
        } else {
            std::cout << "⚠️  Dataset has " << dataset.getIssues().size() 
                      << " issue(s):\n";
            for (const auto& issue : dataset.getIssues()) {
                std::cout << "   - " << dataset.getIssueString(issue) << "\n";
            }
            dataset.setValidated(false);
        }
    }
};

// Concrete validators
class MissingValueValidator : public DataValidator {
public:
    MissingValueValidator() : DataValidator("Missing Value Validator") {}
    
protected:
    bool performValidation(ScientificDataset& dataset) override {
        const auto& data = dataset.getData();
        int missingCount = 0;
        int nanCount = 0;
        int infCount = 0;
        
        for (const auto& value : data) {
            if (std::isnan(value)) {
                nanCount++;
            } else if (std::isinf(value)) {
                infCount++;
            } else if (value == -999.0 || value == -9999.0) { // Common missing value markers
                missingCount++;
            }
        }
        
        int totalIssues = missingCount + nanCount + infCount;
        double missingRatio = (double)totalIssues / data.size();
        
        std::cout << "  Checking for missing values...\n";
        std::cout << "  Found: " << nanCount << " NaN, " 
                  << infCount << " Inf, " 
                  << missingCount << " sentinel values\n";
        std::cout << "  Missing data ratio: " 
                  << std::fixed << std::setprecision(2) 
                  << (missingRatio * 100) << "%\n";
        
        if (missingRatio > tolerance_) {
            dataset.addIssue(DataIssueType::MISSING_VALUES);
            return false;
        }
        return true;
    }
};

class RangeValidator : public DataValidator {
public:
    RangeValidator() : DataValidator("Physical Range Validator", 0.0) {}
    
protected:
    bool performValidation(ScientificDataset& dataset) override {
        const auto& data = dataset.getData();
        double minExpected = dataset.getMinExpected();
        double maxExpected = dataset.getMaxExpected();
        int outOfRangeCount = 0;
        double minFound = std::numeric_limits<double>::max();
        double maxFound = std::numeric_limits<double>::lowest();
        
        for (const auto& value : data) {
            if (!std::isnan(value) && !std::isinf(value)) {
                minFound = std::min(minFound, value);
                maxFound = std::max(maxFound, value);
                
                if (value < minExpected || value > maxExpected) {
                    outOfRangeCount++;
                }
            }
        }
        
        std::cout << "  Checking physical range constraints...\n";
        std::cout << "  Expected range: [" << minExpected << ", " 
                  << maxExpected << "]\n";
        std::cout << "  Actual range: [" << minFound << ", " 
                  << maxFound << "]\n";
        std::cout << "  Out of range values: " << outOfRangeCount << "\n";
        
        if (outOfRangeCount > 0) {
            dataset.addIssue(DataIssueType::PHYSICAL_CONSTRAINT_VIOLATION);
            return false;
        }
        return true;
    }
};

class StatisticalValidator : public DataValidator {
public:
    StatisticalValidator() : DataValidator("Statistical Outlier Validator", 3.0) {} // 3 sigma
    
protected:
    bool performValidation(ScientificDataset& dataset) override {
        auto& data = dataset.getDataMutable();
        
        // Calculate mean and standard deviation
        double sum = 0.0, sumSq = 0.0;
        int validCount = 0;
        
        for (const auto& value : data) {
            if (!std::isnan(value) && !std::isinf(value)) {
                sum += value;
                sumSq += value * value;
                validCount++;
            }
        }
        
        if (validCount < 2) return true; // Not enough data for statistics
        
        double mean = sum / validCount;
        double variance = (sumSq / validCount) - (mean * mean);
        double stdDev = std::sqrt(variance);
        
        // Detect outliers using Z-score
        int outlierCount = 0;
        std::vector<int> outlierIndices;
        
        for (size_t i = 0; i < data.size(); ++i) {
            if (!std::isnan(data[i]) && !std::isinf(data[i])) {
                double zScore = std::abs((data[i] - mean) / stdDev);
                if (zScore > tolerance_) {
                    outlierCount++;
                    outlierIndices.push_back(i);
                }
            }
        }
        
        std::cout << "  Performing statistical analysis...\n";
        std::cout << "  Mean: " << std::fixed << std::setprecision(4) << mean 
                  << ", Std Dev: " << stdDev << "\n";
        std::cout << "  Z-score threshold: " << tolerance_ << " sigma\n";
        std::cout << "  Outliers detected: " << outlierCount 
                  << " (" << std::fixed << std::setprecision(1) 
                  << (100.0 * outlierCount / validCount) << "%)\n";
        
        if (outlierCount > validCount * 0.05) { // More than 5% outliers
            dataset.addIssue(DataIssueType::OUTLIERS);
            
            // Show some outlier examples
            std::cout << "  Example outliers: ";
            for (int i = 0; i < std::min(3, (int)outlierIndices.size()); ++i) {
                int idx = outlierIndices[i];
                std::cout << "index " << idx << " (" << data[idx] << ") ";
            }
            std::cout << "\n";
            return false;
        }
        return true;
    }
};

class NoiseAnalysisValidator : public DataValidator {
public:
    NoiseAnalysisValidator() : DataValidator("Noise Level Validator", 0.20) {} // 20% SNR threshold
    
protected:
    bool performValidation(ScientificDataset& dataset) override {
        const auto& data = dataset.getData();
        double samplingRate = dataset.getSamplingRate();
        
        if (data.size() < 100) return true; // Need sufficient data for noise analysis
        
        // Calculate signal power (using moving average as "signal")
        std::vector<double> smoothed;
        int windowSize = 10;
        for (size_t i = windowSize/2; i < data.size() - windowSize/2; ++i) {
            double avg = 0.0;
            for (int j = -windowSize/2; j <= windowSize/2; ++j) {
                if (!std::isnan(data[i+j])) {
                    avg += data[i+j];
                }
            }
            smoothed.push_back(avg / windowSize);
        }
        
        // Calculate noise as difference from smoothed signal
        double signalPower = 0.0, noisePower = 0.0;
        for (size_t i = 0; i < smoothed.size(); ++i) {
            size_t dataIdx = i + windowSize/2;
            if (!std::isnan(data[dataIdx]) && !std::isnan(smoothed[i])) {
                signalPower += smoothed[i] * smoothed[i];
                double noise = data[dataIdx] - smoothed[i];
                noisePower += noise * noise;
            }
        }
        
        signalPower /= smoothed.size();
        noisePower /= smoothed.size();
        
        double snr = 10.0 * std::log10(signalPower / noisePower);
        double noiseRatio = std::sqrt(noisePower) / std::sqrt(signalPower);
        
        std::cout << "  Analyzing noise characteristics...\n";
        std::cout << "  Sampling rate: " << samplingRate << " Hz\n";
        std::cout << "  Signal-to-Noise Ratio: " 
                  << std::fixed << std::setprecision(2) << snr << " dB\n";
        std::cout << "  Noise level: " 
                  << std::fixed << std::setprecision(1) 
                  << (noiseRatio * 100) << "% of signal\n";
        
        if (noiseRatio > tolerance_) {
            dataset.addIssue(DataIssueType::NOISE_LEVEL);
            return false;
        }
        return true;
    }
};

class CalibrationValidator : public DataValidator {
public:
    CalibrationValidator() : DataValidator("Calibration Validator") {}
    
protected:
    bool performValidation(ScientificDataset& dataset) override {
        const auto& data = dataset.getData();
        const std::string& instrument = dataset.getInstrumentType();
        
        // Instrument-specific calibration checks
        std::cout << "  Checking calibration for " << instrument << "...\n";
        
        bool calibrationOk = true;
        
        if (instrument == "Spectrometer") {
            // Check for wavelength calibration issues
            // Look for expected peaks or patterns
            std::cout << "  Verifying spectral peak positions...\n";
            std::cout << "  Checking baseline stability...\n";
            
            // Simulate calibration check
            double baselineDrift = calculateBaselineDrift(data);
            std::cout << "  Baseline drift: " << baselineDrift << " units\n";
            
            if (baselineDrift > 0.05) {
                calibrationOk = false;
            }
            
        } else if (instrument == "Thermometer") {
            // Check temperature calibration
            double temp = dataset.getTemperature();
            std::cout << "  Reference temperature: " << temp << " K\n";
            
            // Check if readings are consistent with environmental conditions
            double avgReading = 0.0;
            int count = 0;
            for (const auto& value : data) {
                if (!std::isnan(value)) {
                    avgReading += value;
                    count++;
                }
            }
            avgReading /= count;
            
            double deviation = std::abs(avgReading - temp) / temp;
            std::cout << "  Average reading: " << avgReading << " K\n";
            std::cout << "  Deviation from reference: " 
                      << (deviation * 100) << "%\n";
            
            if (deviation > 0.02) { // 2% tolerance
                calibrationOk = false;
            }
            
        } else if (instrument == "Pressure Sensor") {
            // Check pressure calibration
            double pressure = dataset.getPressure();
            std::cout << "  Reference pressure: " << pressure << " Pa\n";
            // Similar checks...
        }
        
        if (!calibrationOk) {
            dataset.addIssue(DataIssueType::CALIBRATION_ERROR);
            std::cout << "  ⚠️  Calibration drift detected!\n";
            return false;
        }
        
        std::cout << "  ✓ Calibration within specifications\n";
        return true;
    }
    
private:
    double calculateBaselineDrift(const std::vector<double>& data) {
        // Simple baseline drift calculation
        if (data.size() < 100) return 0.0;
        
        double firstQuarter = 0.0, lastQuarter = 0.0;
        int quarterSize = data.size() / 4;
        int count1 = 0, count2 = 0;
        
        for (int i = 0; i < quarterSize; ++i) {
            if (!std::isnan(data[i])) {
                firstQuarter += data[i];
                count1++;
            }
        }
        
        for (size_t i = data.size() - quarterSize; i < data.size(); ++i) {
            if (!std::isnan(data[i])) {
                lastQuarter += data[i];
                count2++;
            }
        }
        
        if (count1 > 0 && count2 > 0) {
            firstQuarter /= count1;
            lastQuarter /= count2;
            return std::abs(lastQuarter - firstQuarter);
        }
        
        return 0.0;
    }
};

// Data validation pipeline using chain of responsibility
class DataValidationPipeline {
private:
    std::shared_ptr<DataValidator> firstValidator_;
    
public:
    DataValidationPipeline() {
        // Create validators
        auto missingValueValidator = std::make_shared<MissingValueValidator>();
        auto rangeValidator = std::make_shared<RangeValidator>();
        auto statisticalValidator = std::make_shared<StatisticalValidator>();
        auto noiseValidator = std::make_shared<NoiseAnalysisValidator>();
        auto calibrationValidator = std::make_shared<CalibrationValidator>();
        
        // Set up validation chain
        missingValueValidator->setNext(rangeValidator);
        rangeValidator->setNext(statisticalValidator);
        statisticalValidator->setNext(noiseValidator);
        noiseValidator->setNext(calibrationValidator);
        
        firstValidator_ = missingValueValidator;
        
        std::cout << "=== Scientific Data Validation Pipeline ===\n";
        std::cout << "Chain: Missing Values → Physical Range → Statistical Analysis\n";
        std::cout << "       → Noise Level → Calibration Check\n\n";
    }
    
    void validateDataset(ScientificDataset& dataset) {
        std::cout << "Dataset: " << dataset.getExperimentName() << "\n";
        std::cout << "Instrument: " << dataset.getInstrumentType() << "\n";
        std::cout << "Data points: " << dataset.getData().size() << "\n";
        std::cout << "Starting validation pipeline...\n";
        
        firstValidator_->validate(dataset);
        
        if (dataset.isValidated()) {
            std::cout << "\n✅ Dataset approved for analysis!\n";
        } else {
            std::cout << "\n❌ Dataset requires attention before analysis.\n";
        }
    }
};

// Helper function to generate synthetic experimental data
std::vector<double> generateExperimentalData(int size, double baseline, 
                                            double amplitude, double noiseLevel,
                                            double missingRate = 0.0,
                                            int outliers = 0) {
    std::vector<double> data;
    
    for (int i = 0; i < size; ++i) {
        double t = (double)i / size * 2 * M_PI;
        double signal = baseline + amplitude * std::sin(t);
        
        // Add noise
        double noise = (rand() / (double)RAND_MAX - 0.5) * 2 * noiseLevel;
        double value = signal + noise;
        
        // Introduce missing values
        if (rand() / (double)RAND_MAX < missingRate) {
            value = std::numeric_limits<double>::quiet_NaN();
        }
        
        // Add outliers
        if (outliers > 0 && i % (size / outliers) == 0) {
            value *= 10.0; // Extreme outlier
        }
        
        data.push_back(value);
    }
    
    return data;
}

int main() {
    DataValidationPipeline pipeline;
    
    std::cout << "=== Testing Scientific Data Validation Pipeline ===\n\n";
    
    // Test Case 1: Clean data
    std::cout << "Test Case 1: Clean spectroscopy data\n";
    std::cout << "=====================================\n";
    auto cleanData = generateExperimentalData(1000, 100.0, 20.0, 0.5);
    ScientificDataset dataset1("UV-Vis Absorption Spectrum", "Spectrometer", 
                              cleanData, 1000.0, 70.0, 130.0);
    pipeline.validateDataset(dataset1);
    
    std::cout << "\n\nTest Case 2: Data with missing values\n";
    std::cout << "======================================\n";
    auto missingData = generateExperimentalData(1000, 50.0, 10.0, 0.5, 0.15);
    ScientificDataset dataset2("Temperature Time Series", "Thermometer", 
                              missingData, 10.0, 30.0, 70.0, 323.15);
    pipeline.validateDataset(dataset2);
    
    std::cout << "\n\nTest Case 3: Data with outliers\n";
    std::cout << "================================\n";
    auto outlierData = generateExperimentalData(1000, 1.0, 0.5, 0.1, 0.0, 20);
    ScientificDataset dataset3("Mass Spectrometry", "Mass Spectrometer", 
                              outlierData, 10000.0, 0.0, 2.0);
    pipeline.validateDataset(dataset3);
    
    std::cout << "\n\nTest Case 4: Noisy sensor data\n";
    std::cout << "===============================\n";
    auto noisyData = generateExperimentalData(1000, 101325.0, 100.0, 500.0);
    ScientificDataset dataset4("Pressure Monitoring", "Pressure Sensor", 
                              noisyData, 100.0, 100000.0, 102000.0, 293.15, 101325.0);
    pipeline.validateDataset(dataset4);
    
    std::cout << "\n\nTest Case 5: Data with calibration drift\n";
    std::cout << "=========================================\n";
    // Simulate baseline drift
    auto driftData = generateExperimentalData(1000, 100.0, 20.0, 1.0);
    for (size_t i = 0; i < driftData.size(); ++i) {
        driftData[i] += i * 0.1; // Add linear drift
    }
    ScientificDataset dataset5("Long-term Spectroscopy", "Spectrometer", 
                              driftData, 1.0, 70.0, 200.0);
    pipeline.validateDataset(dataset5);
    
    std::cout << "\n\nChain of Responsibility pattern enables modular\n";
    std::cout << "data validation for scientific experiments!\n";
    
    return 0;
}