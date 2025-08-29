// Decorator Pattern - Scientific Data Filter Pipeline
// Adds preprocessing filters to raw sensor data streams
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <numeric>

// Component interface - Data stream from scientific instruments
class DataStream {
public:
    virtual ~DataStream() = default;
    virtual std::vector<double> getData() const = 0;
    virtual std::string getDescription() const = 0;
    virtual double getNoiseLevel() const = 0;
    virtual int getSampleRate() const = 0;
};

// Concrete component - Raw sensor data
class RawSensorData : public DataStream {
private:
    std::vector<double> rawData_;
    std::string sensorType_;
    double baseNoise_;
    int sampleRate_;
    
public:
    RawSensorData(const std::string& type, int samples, double noise, int rate)
        : sensorType_(type), baseNoise_(noise), sampleRate_(rate) {
        // Generate simulated raw data with noise
        rawData_.reserve(samples);
        for (int i = 0; i < samples; ++i) {
            double signal = 10.0 * sin(2.0 * M_PI * i / 100.0);  // Base signal
            double noise = baseNoise_ * (rand() / double(RAND_MAX) - 0.5);
            rawData_.push_back(signal + noise);
        }
    }
    
    std::vector<double> getData() const override {
        return rawData_;
    }
    
    std::string getDescription() const override {
        return "Raw " + sensorType_ + " Data";
    }
    
    double getNoiseLevel() const override {
        return baseNoise_;
    }
    
    int getSampleRate() const override {
        return sampleRate_;
    }
};

// Base decorator for data filters
class DataFilter : public DataStream {
protected:
    std::unique_ptr<DataStream> dataStream_;
    
public:
    DataFilter(std::unique_ptr<DataStream> stream) 
        : dataStream_(std::move(stream)) {}
    
    std::vector<double> getData() const override {
        return dataStream_->getData();
    }
    
    std::string getDescription() const override {
        return dataStream_->getDescription();
    }
    
    double getNoiseLevel() const override {
        return dataStream_->getNoiseLevel();
    }
    
    int getSampleRate() const override {
        return dataStream_->getSampleRate();
    }
};

// Kalman filter decorator
class KalmanFilter : public DataFilter {
private:
    mutable double estimate_ = 0.0;
    mutable double errorCovariance_ = 1.0;
    double processNoise_ = 0.1;
    double measurementNoise_ = 0.5;
    
public:
    KalmanFilter(std::unique_ptr<DataStream> stream) 
        : DataFilter(std::move(stream)) {}
    
    std::vector<double> getData() const override {
        std::vector<double> input = dataStream_->getData();
        std::vector<double> filtered;
        filtered.reserve(input.size());
        
        for (double measurement : input) {
            // Prediction step
            double predictedEstimate = estimate_;
            double predictedError = errorCovariance_ + processNoise_;
            
            // Update step
            double kalmanGain = predictedError / (predictedError + measurementNoise_);
            estimate_ = predictedEstimate + kalmanGain * (measurement - predictedEstimate);
            errorCovariance_ = (1 - kalmanGain) * predictedError;
            
            filtered.push_back(estimate_);
        }
        
        return filtered;
    }
    
    std::string getDescription() const override {
        return dataStream_->getDescription() + " -> Kalman Filter";
    }
    
    double getNoiseLevel() const override {
        return dataStream_->getNoiseLevel() * 0.3;  // Kalman reduces noise
    }
};

// Moving average filter decorator
class MovingAverageFilter : public DataFilter {
private:
    int windowSize_;
    
public:
    MovingAverageFilter(std::unique_ptr<DataStream> stream, int window) 
        : DataFilter(std::move(stream)), windowSize_(window) {}
    
    std::vector<double> getData() const override {
        std::vector<double> input = dataStream_->getData();
        std::vector<double> filtered;
        filtered.reserve(input.size());
        
        for (size_t i = 0; i < input.size(); ++i) {
            double sum = 0.0;
            int count = 0;
            
            for (int j = std::max(0, (int)i - windowSize_ + 1); 
                 j <= (int)i && j < input.size(); ++j) {
                sum += input[j];
                count++;
            }
            
            filtered.push_back(sum / count);
        }
        
        return filtered;
    }
    
    std::string getDescription() const override {
        return dataStream_->getDescription() + " -> Moving Average(" + 
               std::to_string(windowSize_) + ")";
    }
    
    double getNoiseLevel() const override {
        return dataStream_->getNoiseLevel() / sqrt(windowSize_);
    }
};

// Fourier transform filter decorator (low-pass)
class FourierLowPassFilter : public DataFilter {
private:
    double cutoffFrequency_;
    
public:
    FourierLowPassFilter(std::unique_ptr<DataStream> stream, double cutoff) 
        : DataFilter(std::move(stream)), cutoffFrequency_(cutoff) {}
    
    std::vector<double> getData() const override {
        std::vector<double> input = dataStream_->getData();
        std::vector<double> filtered = input;  // Simplified - would use FFT
        
        std::cout << "  Applying FFT low-pass filter (cutoff: " 
                  << cutoffFrequency_ << " Hz)\n";
        
        // Simplified low-pass simulation
        for (size_t i = 1; i < filtered.size() - 1; ++i) {
            filtered[i] = 0.25 * input[i-1] + 0.5 * input[i] + 0.25 * input[i+1];
        }
        
        return filtered;
    }
    
    std::string getDescription() const override {
        return dataStream_->getDescription() + " -> FFT Low-Pass(" + 
               std::to_string(cutoffFrequency_) + "Hz)";
    }
    
    double getNoiseLevel() const override {
        return dataStream_->getNoiseLevel() * 0.2;  // Significant noise reduction
    }
};

// Outlier removal filter decorator
class OutlierRemovalFilter : public DataFilter {
private:
    double threshold_;
    
public:
    OutlierRemovalFilter(std::unique_ptr<DataStream> stream, double zscore) 
        : DataFilter(std::move(stream)), threshold_(zscore) {}
    
    std::vector<double> getData() const override {
        std::vector<double> input = dataStream_->getData();
        
        // Calculate mean and standard deviation
        double mean = std::accumulate(input.begin(), input.end(), 0.0) / input.size();
        double sq_sum = std::inner_product(input.begin(), input.end(), 
                                          input.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / input.size() - mean * mean);
        
        // Remove outliers
        std::vector<double> filtered;
        for (double value : input) {
            if (std::abs(value - mean) <= threshold_ * stdev) {
                filtered.push_back(value);
            } else {
                filtered.push_back(mean);  // Replace with mean
                std::cout << "  Outlier detected and removed: " << value << "\n";
            }
        }
        
        return filtered;
    }
    
    std::string getDescription() const override {
        return dataStream_->getDescription() + " -> Outlier Removal(" + 
               std::to_string(threshold_) + "σ)";
    }
    
    double getNoiseLevel() const override {
        return dataStream_->getNoiseLevel() * 0.8;
    }
};

// Helper function to analyze data stream
void analyzeDataStream(const DataStream& stream) {
    std::cout << "\nPipeline: " << stream.getDescription() << "\n";
    std::cout << "Sample Rate: " << stream.getSampleRate() << " Hz\n";
    std::cout << "Noise Level: " << stream.getNoiseLevel() << "\n";
    
    std::vector<double> data = stream.getData();
    if (!data.empty()) {
        double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
        std::cout << "Data samples: " << data.size() << "\n";
        std::cout << "Mean value: " << mean << "\n";
        std::cout << "First 5 values: ";
        for (int i = 0; i < 5 && i < data.size(); ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    std::cout << "=== Scientific Data Filter Pipeline Demo ===\n";
    
    // Create raw sensor data streams
    auto seismicData = std::make_unique<RawSensorData>(
        "Seismometer", 200, 2.0, 100);
    
    auto magnetometerData = std::make_unique<RawSensorData>(
        "Magnetometer", 200, 5.0, 50);
    
    // Pipeline 1: Raw data
    std::cout << "\n--- Pipeline 1: Raw Data ---";
    analyzeDataStream(*seismicData);
    
    // Pipeline 2: Simple moving average
    std::cout << "\n--- Pipeline 2: Moving Average Filter ---";
    auto pipeline2 = std::make_unique<MovingAverageFilter>(
        std::make_unique<RawSensorData>("Seismometer", 200, 2.0, 100), 5
    );
    analyzeDataStream(*pipeline2);
    
    // Pipeline 3: Kalman filter
    std::cout << "\n--- Pipeline 3: Kalman Filter ---";
    auto pipeline3 = std::make_unique<KalmanFilter>(
        std::make_unique<RawSensorData>("Seismometer", 200, 2.0, 100)
    );
    analyzeDataStream(*pipeline3);
    
    // Pipeline 4: Complex multi-stage filtering
    std::cout << "\n--- Pipeline 4: Multi-Stage Filtering ---";
    auto pipeline4 = std::make_unique<FourierLowPassFilter>(
        std::make_unique<KalmanFilter>(
            std::make_unique<OutlierRemovalFilter>(
                std::make_unique<MovingAverageFilter>(
                    std::make_unique<RawSensorData>("Magnetometer", 200, 5.0, 50),
                    3
                ),
                2.5
            )
        ),
        10.0
    );
    analyzeDataStream(*pipeline4);
    
    // Pipeline 5: Double Kalman filtering for ultra-low noise
    std::cout << "\n--- Pipeline 5: Double Kalman Filter ---";
    auto pipeline5 = std::make_unique<KalmanFilter>(
        std::make_unique<KalmanFilter>(
            std::make_unique<RawSensorData>("Gravitational Wave Detector", 200, 10.0, 1000)
        )
    );
    analyzeDataStream(*pipeline5);
    
    std::cout << "\nDecorator pattern enables flexible composition of\n";
    std::cout << "data processing filters for scientific instruments!\n";
    
    return 0;
}