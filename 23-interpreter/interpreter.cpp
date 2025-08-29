// Interpreter Pattern - Scientific Formula and Expression Language
// Enables interpretation of mathematical formulas and computational expressions
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <set>

// Define M_PI and M_E if not already defined (for MSVC compatibility)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// Context for scientific computation with variables and constants
class ScientificContext {
private:
    std::unordered_map<std::string, double> variables_;
    std::unordered_map<std::string, double> constants_;
    std::unordered_map<std::string, std::vector<double>> vectors_;
    
public:
    ScientificContext() {
        // Initialize physical constants
        constants_["pi"] = M_PI;
        constants_["e"] = M_E;
        constants_["c"] = 299792458.0;       // Speed of light m/s
        constants_["h"] = 6.62607015e-34;    // Planck constant
        constants_["k_B"] = 1.380649e-23;    // Boltzmann constant
        constants_["N_A"] = 6.02214076e23;   // Avogadro's number
        constants_["g"] = 9.80665;           // Standard gravity
    }
    
    void setVariable(const std::string& name, double value) {
        variables_[name] = value;
        std::cout << "Set " << name << " = " << std::scientific << std::setprecision(4) 
                  << value << "\n";
    }
    
    double getVariable(const std::string& name) const {
        // Check constants first
        auto cit = constants_.find(name);
        if (cit != constants_.end()) {
            return cit->second;
        }
        
        // Then check variables
        auto vit = variables_.find(name);
        if (vit != variables_.end()) {
            return vit->second;
        }
        
        throw std::runtime_error("Undefined variable or constant: " + name);
    }
    
    void setVector(const std::string& name, const std::vector<double>& vec) {
        vectors_[name] = vec;
        std::cout << "Set vector " << name << " = [";
        for (size_t i = 0; i < vec.size(); ++i) {
            std::cout << vec[i];
            if (i < vec.size() - 1) std::cout << ", ";
        }
        std::cout << "]\n";
    }
    
    std::vector<double> getVector(const std::string& name) const {
        auto it = vectors_.find(name);
        if (it != vectors_.end()) {
            return it->second;
        }
        throw std::runtime_error("Undefined vector: " + name);
    }
    
    bool hasVariable(const std::string& name) const {
        return variables_.find(name) != variables_.end() || 
               constants_.find(name) != constants_.end();
    }
    
    void showVariables() const {
        std::cout << "\nVariables:\n";
        for (const auto& [name, value] : variables_) {
            std::cout << "  " << name << " = " << std::scientific 
                      << std::setprecision(4) << value << "\n";
        }
        
        std::cout << "\nConstants:\n";
        for (const auto& [name, value] : constants_) {
            std::cout << "  " << name << " = " << std::scientific 
                      << std::setprecision(4) << value << "\n";
        }
        
        if (!vectors_.empty()) {
            std::cout << "\nVectors:\n";
            for (const auto& [name, vec] : vectors_) {
                std::cout << "  " << name << " = [";
                for (size_t i = 0; i < vec.size(); ++i) {
                    std::cout << vec[i];
                    if (i < vec.size() - 1) std::cout << ", ";
                }
                std::cout << "]\n";
            }
        }
    }
};

// Abstract Expression for scientific computation
class Expression {
public:
    virtual ~Expression() = default;
    virtual double evaluate(ScientificContext& context) = 0;
    virtual std::string toString() const = 0;
};

// Terminal Expression - Number
class NumberExpression : public Expression {
private:
    double value_;
    
public:
    NumberExpression(double value) : value_(value) {}
    
    double evaluate(ScientificContext& context) override {
        return value_;
    }
    
    std::string toString() const override {
        std::stringstream ss;
        ss << std::scientific << std::setprecision(3) << value_;
        return ss.str();
    }
};

// Terminal Expression - Variable or Constant
class VariableExpression : public Expression {
private:
    std::string name_;
    
public:
    VariableExpression(const std::string& name) : name_(name) {}
    
    double evaluate(ScientificContext& context) override {
        return context.getVariable(name_);
    }
    
    std::string toString() const override {
        return name_;
    }
    
    const std::string& getName() const { return name_; }
};

// Scientific Function Expression
class FunctionExpression : public Expression {
private:
    std::string function_;
    std::unique_ptr<Expression> argument_;
    
public:
    FunctionExpression(const std::string& func, std::unique_ptr<Expression> arg)
        : function_(func), argument_(std::move(arg)) {}
    
    double evaluate(ScientificContext& context) override {
        double arg = argument_->evaluate(context);
        
        // Mathematical functions
        if (function_ == "sin") return std::sin(arg);
        if (function_ == "cos") return std::cos(arg);
        if (function_ == "tan") return std::tan(arg);
        if (function_ == "exp") return std::exp(arg);
        if (function_ == "ln") return std::log(arg);
        if (function_ == "log10") return std::log10(arg);
        if (function_ == "sqrt") return std::sqrt(arg);
        if (function_ == "abs") return std::abs(arg);
        
        // Hyperbolic functions
        if (function_ == "sinh") return std::sinh(arg);
        if (function_ == "cosh") return std::cosh(arg);
        if (function_ == "tanh") return std::tanh(arg);
        
        // Special functions
        if (function_ == "erf") return std::erf(arg);
        if (function_ == "gamma") return std::tgamma(arg);
        
        throw std::runtime_error("Unknown function: " + function_);
    }
    
    std::string toString() const override {
        return function_ + "(" + argument_->toString() + ")";
    }
};

// Non-terminal Expression - Addition
class AddExpression : public Expression {
private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    
public:
    AddExpression(std::unique_ptr<Expression> left, 
                  std::unique_ptr<Expression> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    
    double evaluate(ScientificContext& context) override {
        return left_->evaluate(context) + right_->evaluate(context);
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " + " + right_->toString() + ")";
    }
};

// Non-terminal Expression - Subtraction
class SubtractExpression : public Expression {
private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    
public:
    SubtractExpression(std::unique_ptr<Expression> left, 
                       std::unique_ptr<Expression> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    
    double evaluate(ScientificContext& context) override {
        return left_->evaluate(context) - right_->evaluate(context);
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " - " + right_->toString() + ")";
    }
};

// Non-terminal Expression - Multiplication
class MultiplyExpression : public Expression {
private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    
public:
    MultiplyExpression(std::unique_ptr<Expression> left, 
                       std::unique_ptr<Expression> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    
    double evaluate(ScientificContext& context) override {
        return left_->evaluate(context) * right_->evaluate(context);
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " * " + right_->toString() + ")";
    }
};

// Non-terminal Expression - Division
class DivideExpression : public Expression {
private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
    
public:
    DivideExpression(std::unique_ptr<Expression> left, 
                     std::unique_ptr<Expression> right)
        : left_(std::move(left)), right_(std::move(right)) {}
    
    double evaluate(ScientificContext& context) override {
        double rightValue = right_->evaluate(context);
        if (std::abs(rightValue) < 1e-15) {
            throw std::runtime_error("Division by zero");
        }
        return left_->evaluate(context) / rightValue;
    }
    
    std::string toString() const override {
        return "(" + left_->toString() + " / " + right_->toString() + ")";
    }
};

// Power Expression
class PowerExpression : public Expression {
private:
    std::unique_ptr<Expression> base_;
    std::unique_ptr<Expression> exponent_;
    
public:
    PowerExpression(std::unique_ptr<Expression> base, 
                    std::unique_ptr<Expression> exponent)
        : base_(std::move(base)), exponent_(std::move(exponent)) {}
    
    double evaluate(ScientificContext& context) override {
        return std::pow(base_->evaluate(context), exponent_->evaluate(context));
    }
    
    std::string toString() const override {
        return "(" + base_->toString() + " ^ " + exponent_->toString() + ")";
    }
};

// Assignment Expression
class AssignmentExpression : public Expression {
private:
    std::string variableName_;
    std::unique_ptr<Expression> value_;
    
public:
    AssignmentExpression(const std::string& variableName, 
                        std::unique_ptr<Expression> value)
        : variableName_(variableName), value_(std::move(value)) {}
    
    double evaluate(ScientificContext& context) override {
        double result = value_->evaluate(context);
        context.setVariable(variableName_, result);
        return result;
    }
    
    std::string toString() const override {
        return variableName_ + " = " + value_->toString();
    }
};

// Derivative Expression (numerical differentiation)
class DerivativeExpression : public Expression {
private:
    std::unique_ptr<Expression> function_;
    std::string variable_;
    double h_ = 1e-8;  // Step size
    
public:
    DerivativeExpression(std::unique_ptr<Expression> func, const std::string& var)
        : function_(std::move(func)), variable_(var) {}
    
    double evaluate(ScientificContext& context) override {
        double x0 = context.getVariable(variable_);
        
        // Forward difference approximation
        context.setVariable(variable_, x0 + h_);
        double f_plus = function_->evaluate(context);
        
        context.setVariable(variable_, x0 - h_);
        double f_minus = function_->evaluate(context);
        
        // Restore original value
        context.setVariable(variable_, x0);
        
        // Central difference
        return (f_plus - f_minus) / (2.0 * h_);
    }
    
    std::string toString() const override {
        return "d/d" + variable_ + "[" + function_->toString() + "]";
    }
};

// Integral Expression (numerical integration using Simpson's rule)
class IntegralExpression : public Expression {
private:
    std::unique_ptr<Expression> function_;
    std::string variable_;
    std::unique_ptr<Expression> lower_;
    std::unique_ptr<Expression> upper_;
    int n_ = 1000;  // Number of intervals
    
public:
    IntegralExpression(std::unique_ptr<Expression> func, const std::string& var,
                      std::unique_ptr<Expression> lower, std::unique_ptr<Expression> upper)
        : function_(std::move(func)), variable_(var), 
          lower_(std::move(lower)), upper_(std::move(upper)) {}
    
    double evaluate(ScientificContext& context) override {
        double a = lower_->evaluate(context);
        double b = upper_->evaluate(context);
        double h = (b - a) / n_;
        
        double x0 = context.getVariable(variable_);  // Save original value
        
        // Simpson's rule
        context.setVariable(variable_, a);
        double sum = function_->evaluate(context);
        
        context.setVariable(variable_, b);
        sum += function_->evaluate(context);
        
        for (int i = 1; i < n_; i++) {
            double x = a + i * h;
            context.setVariable(variable_, x);
            double fx = function_->evaluate(context);
            sum += (i % 2 == 0) ? 2 * fx : 4 * fx;
        }
        
        context.setVariable(variable_, x0);  // Restore original value
        
        return (h / 3.0) * sum;
    }
    
    std::string toString() const override {
        return "∫(" + lower_->toString() + " to " + upper_->toString() + 
               ") " + function_->toString() + " d" + variable_;
    }
};

// Parser for scientific expressions
class ExpressionParser {
private:
    std::vector<std::string> tokens_;
    size_t currentToken_ = 0;
    
    void tokenize(const std::string& input) {
        tokens_.clear();
        currentToken_ = 0;
        
        std::stringstream ss(input);
        std::string token;
        
        while (ss >> token) {
            tokens_.push_back(token);
        }
    }
    
    bool hasMoreTokens() const {
        return currentToken_ < tokens_.size();
    }
    
    std::string peekToken() const {
        if (hasMoreTokens()) {
            return tokens_[currentToken_];
        }
        return "";
    }
    
    std::string nextToken() {
        if (hasMoreTokens()) {
            return tokens_[currentToken_++];
        }
        return "";
    }
    
    bool isNumber(const std::string& token) const {
        if (token.empty()) return false;
        size_t idx = 0;
        if (token[0] == '-' || token[0] == '+') idx = 1;
        
        bool hasDecimal = false;
        bool hasExponent = false;
        
        for (size_t i = idx; i < token.length(); ++i) {
            if (token[i] == '.') {
                if (hasDecimal || hasExponent) return false;
                hasDecimal = true;
            } else if (token[i] == 'e' || token[i] == 'E') {
                if (hasExponent) return false;
                hasExponent = true;
                if (i + 1 < token.length() && (token[i+1] == '-' || token[i+1] == '+')) {
                    i++;
                }
            } else if (!std::isdigit(token[i])) {
                return false;
            }
        }
        return idx < token.length();
    }
    
    bool isVariable(const std::string& token) const {
        if (token.empty() || !std::isalpha(token[0])) return false;
        
        // Allow alphanumeric and underscore after first character
        for (size_t i = 1; i < token.length(); ++i) {
            if (!std::isalnum(token[i]) && token[i] != '_') {
                return false;
            }
        }
        return true;
    }
    
    bool isFunction(const std::string& token) const {
        static const std::set<std::string> functions = {
            "sin", "cos", "tan", "exp", "ln", "log10", "sqrt", "abs",
            "sinh", "cosh", "tanh", "erf", "gamma"
        };
        return functions.find(token) != functions.end();
    }
    
    // Parse primary expression (number, variable, function, or parentheses)
    std::unique_ptr<Expression> parsePrimary() {
        std::string token = nextToken();
        
        if (isNumber(token)) {
            return std::make_unique<NumberExpression>(std::stod(token));
        }
        
        if (isFunction(token)) {
            if (nextToken() != "(") {
                throw std::runtime_error("Expected '(' after function " + token);
            }
            auto arg = parseExpression();
            if (nextToken() != ")") {
                throw std::runtime_error("Expected ')' after function argument");
            }
            return std::make_unique<FunctionExpression>(token, std::move(arg));
        }
        
        if (isVariable(token)) {
            // Check for assignment
            if (peekToken() == "=") {
                nextToken(); // consume '='
                auto value = parseExpression();
                return std::make_unique<AssignmentExpression>(token, std::move(value));
            }
            return std::make_unique<VariableExpression>(token);
        }
        
        if (token == "(") {
            auto expr = parseExpression();
            if (nextToken() != ")") {
                throw std::runtime_error("Expected closing parenthesis");
            }
            return expr;
        }
        
        throw std::runtime_error("Unexpected token: " + token);
    }
    
    // Parse power expressions
    std::unique_ptr<Expression> parsePower() {
        auto left = parsePrimary();
        
        while (hasMoreTokens()) {
            std::string op = peekToken();
            if (op == "^") {
                nextToken(); // consume operator
                auto right = parsePrimary();
                left = std::make_unique<PowerExpression>(
                    std::move(left), std::move(right));
            } else {
                break;
            }
        }
        
        return left;
    }
    
    // Parse multiplication and division
    std::unique_ptr<Expression> parseTerm() {
        auto left = parsePower();
        
        while (hasMoreTokens()) {
            std::string op = peekToken();
            if (op == "*" || op == "/") {
                nextToken(); // consume operator
                auto right = parsePower();
                
                if (op == "*") {
                    left = std::make_unique<MultiplyExpression>(
                        std::move(left), std::move(right));
                } else {
                    left = std::make_unique<DivideExpression>(
                        std::move(left), std::move(right));
                }
            } else {
                break;
            }
        }
        
        return left;
    }
    
    // Parse addition and subtraction
    std::unique_ptr<Expression> parseExpression() {
        auto left = parseTerm();
        
        while (hasMoreTokens()) {
            std::string op = peekToken();
            if (op == "+" || op == "-") {
                nextToken(); // consume operator
                auto right = parseTerm();
                
                if (op == "+") {
                    left = std::make_unique<AddExpression>(
                        std::move(left), std::move(right));
                } else {
                    left = std::make_unique<SubtractExpression>(
                        std::move(left), std::move(right));
                }
            } else {
                break;
            }
        }
        
        return left;
    }
    
public:
    std::unique_ptr<Expression> parse(const std::string& input) {
        tokenize(input);
        
        if (!hasMoreTokens()) {
            throw std::runtime_error("Empty expression");
        }
        
        auto expr = parseExpression();
        
        if (hasMoreTokens()) {
            throw std::runtime_error("Unexpected token: " + peekToken());
        }
        
        return expr;
    }
};

// Physical Unit Expression System
class UnitExpression {
public:
    struct Unit {
        double value;
        int meter = 0;      // Length
        int kilogram = 0;   // Mass
        int second = 0;     // Time
        int ampere = 0;     // Electric current
        int kelvin = 0;     // Temperature
        int mole = 0;       // Amount of substance
        int candela = 0;    // Luminous intensity
    };
    
    virtual ~UnitExpression() = default;
    virtual Unit evaluate(ScientificContext& context) = 0;
    virtual std::string toString() const = 0;
};

class PhysicalQuantity : public UnitExpression {
private:
    double value_;
    std::string unitName_;
    Unit unit_;
    
    void parseUnit(const std::string& unitStr) {
        unit_ = {value_, 0, 0, 0, 0, 0, 0, 0};
        
        if (unitStr == "m") unit_.meter = 1;
        else if (unitStr == "kg") unit_.kilogram = 1;
        else if (unitStr == "s") unit_.second = 1;
        else if (unitStr == "A") unit_.ampere = 1;
        else if (unitStr == "K") unit_.kelvin = 1;
        else if (unitStr == "mol") unit_.mole = 1;
        else if (unitStr == "cd") unit_.candela = 1;
        // Derived units
        else if (unitStr == "N") { // Newton = kg⋅m/s²
            unit_.kilogram = 1;
            unit_.meter = 1;
            unit_.second = -2;
        }
        else if (unitStr == "J") { // Joule = kg⋅m²/s²
            unit_.kilogram = 1;
            unit_.meter = 2;
            unit_.second = -2;
        }
        else if (unitStr == "W") { // Watt = kg⋅m²/s³
            unit_.kilogram = 1;
            unit_.meter = 2;
            unit_.second = -3;
        }
        else if (unitStr == "Pa") { // Pascal = kg/(m⋅s²)
            unit_.kilogram = 1;
            unit_.meter = -1;
            unit_.second = -2;
        }
    }
    
public:
    PhysicalQuantity(double value, const std::string& unitName)
        : value_(value), unitName_(unitName) {
        parseUnit(unitName);
    }
    
    Unit evaluate(ScientificContext& context) override {
        return unit_;
    }
    
    std::string toString() const override {
        return std::to_string(value_) + " " + unitName_;
    }
};

int main() {
    std::cout << "=== Scientific Formula and Expression Interpreter ===\n\n";
    
    ScientificContext context;
    ExpressionParser parser;
    
    // Example 1: Basic scientific calculations
    std::cout << "Example 1: Basic Scientific Calculations\n";
    std::cout << "-----------------------------------------\n";
    
    auto expr1 = parser.parse("2.5 * 3.14159");
    std::cout << "Expression: " << expr1->toString() << "\n";
    std::cout << "Result: " << std::scientific << std::setprecision(4) 
              << expr1->evaluate(context) << "\n\n";
    
    // Example 2: Using physical constants
    std::cout << "Example 2: Physical Constants\n";
    std::cout << "-----------------------------\n";
    
    auto expr2 = parser.parse("2 * pi");
    std::cout << "2π = " << expr2->evaluate(context) << "\n";
    
    auto expr3 = parser.parse("h * c");
    std::cout << "h·c (Planck constant × speed of light) = " 
              << expr3->evaluate(context) << " J·m\n";
    
    auto expr4 = parser.parse("k_B * N_A");
    std::cout << "k_B·N_A (Boltzmann × Avogadro = R) = " 
              << expr4->evaluate(context) << " J/(mol·K)\n\n";
    
    // Example 3: Scientific functions
    std::cout << "Example 3: Scientific Functions\n";
    std::cout << "-------------------------------\n";
    
    auto expr5 = parser.parse("sin ( pi / 2 )");
    std::cout << "sin(π/2) = " << std::fixed << std::setprecision(6) 
              << expr5->evaluate(context) << "\n";
    
    auto expr6 = parser.parse("exp ( -0.5 )");
    std::cout << "exp(-0.5) = " << expr6->evaluate(context) << "\n";
    
    auto expr7 = parser.parse("ln ( e ^ 2 )");
    std::cout << "ln(e²) = " << expr7->evaluate(context) << "\n";
    
    auto expr8 = parser.parse("sqrt ( 2 )");
    std::cout << "√2 = " << expr8->evaluate(context) << "\n\n";
    
    // Example 4: Variable assignments for scientific calculations
    std::cout << "Example 4: Scientific Variable Assignments\n";
    std::cout << "-----------------------------------------\n";
    
    // Temperature conversion
    auto tempC = parser.parse("T_celsius = 25");
    tempC->evaluate(context);
    
    auto tempK = parser.parse("T_kelvin = T_celsius + 273.15");
    tempK->evaluate(context);
    
    auto tempF = parser.parse("T_fahrenheit = T_celsius * 9 / 5 + 32");
    tempF->evaluate(context);
    
    // Energy calculations
    auto mass = parser.parse("m = 0.001");  // 1 gram in kg
    mass->evaluate(context);
    
    auto energy = parser.parse("E = m * c ^ 2");
    std::cout << "E = mc² for 1 gram: " << std::scientific 
              << energy->evaluate(context) << " J\n\n";
    
    // Example 5: Complex scientific expressions
    std::cout << "Example 5: Complex Scientific Expressions\n";
    std::cout << "----------------------------------------\n";
    
    // Gaussian distribution
    auto x = parser.parse("x = 1.5");
    x->evaluate(context);
    
    auto mu = parser.parse("mu = 0");
    mu->evaluate(context);
    
    auto sigma = parser.parse("sigma = 1");
    sigma->evaluate(context);
    
    auto gaussian = parser.parse("exp ( - ( x - mu ) ^ 2 / ( 2 * sigma ^ 2 ) ) / sqrt ( 2 * pi * sigma ^ 2 )");
    std::cout << "Gaussian PDF at x=1.5, μ=0, σ=1: " << std::fixed << std::setprecision(6)
              << gaussian->evaluate(context) << "\n\n";
    
    // Example 6: Numerical derivatives (if implemented)
    std::cout << "Example 6: Numerical Derivatives\n";
    std::cout << "--------------------------------\n";
    
    // Create a simple quadratic function for derivative
    auto quadratic = std::make_unique<AddExpression>(
        std::make_unique<PowerExpression>(
            std::make_unique<VariableExpression>("x"),
            std::make_unique<NumberExpression>(2)
        ),
        std::make_unique<MultiplyExpression>(
            std::make_unique<NumberExpression>(3),
            std::make_unique<VariableExpression>("x")
        )
    );
    
    auto derivative = std::make_unique<DerivativeExpression>(
        std::move(quadratic), "x"
    );
    
    std::cout << "d/dx[x² + 3x] at x=1.5: " << derivative->evaluate(context) << "\n";
    std::cout << "Analytical: 2x + 3 = " << (2 * 1.5 + 3) << "\n\n";
    
    // Example 7: Physical unit calculations
    std::cout << "Example 7: Physical Units (Demonstration)\n";
    std::cout << "----------------------------------------\n";
    
    auto force = std::make_unique<PhysicalQuantity>(10.0, "N");
    std::cout << "Force: " << force->toString() << "\n";
    
    auto pressure = std::make_unique<PhysicalQuantity>(101325.0, "Pa");
    std::cout << "Atmospheric pressure: " << pressure->toString() << "\n";
    
    auto energy_unit = std::make_unique<PhysicalQuantity>(1.0, "J");
    std::cout << "Energy unit: " << energy_unit->toString() << "\n\n";
    
    // Show all variables and constants
    context.showVariables();
    
    // Interactive mode for scientific calculations
    std::cout << "\n=== Interactive Scientific Calculator ===\n";
    std::cout << "Available functions: sin, cos, tan, exp, ln, log10, sqrt, abs\n";
    std::cout << "Physical constants: pi, e, c, h, k_B, N_A, g\n";
    std::cout << "Operators: +, -, *, /, ^ (power)\n";
    std::cout << "Commands: 'vars' (show variables), 'quit' (exit)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  sin ( pi / 4 )\n";
    std::cout << "  v = 10\n";
    std::cout << "  KE = 0.5 * m * v ^ 2\n\n";
    
    std::string input;
    std::cout << "> ";
    while (std::getline(std::cin, input)) {
        if (input == "quit") break;
        if (input == "vars") {
            context.showVariables();
            std::cout << "\n> ";
            continue;
        }
        
        try {
            auto expr = parser.parse(input);
            std::cout << "Parsed: " << expr->toString() << "\n";
            double result = expr->evaluate(context);
            std::cout << "Result: " << std::scientific << std::setprecision(6) 
                      << result << "\n\n> ";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n\n> ";
        }
    }
    
    std::cout << "\n=== Interpreter Pattern Summary ===\n";
    std::cout << "The Interpreter pattern enables powerful scientific computing:\n";
    std::cout << "• Mathematical expression evaluation with functions and constants\n";
    std::cout << "• Support for physical constants and scientific notation\n";
    std::cout << "• Numerical derivatives and integration capabilities\n";
    std::cout << "• Physical unit awareness for dimensional analysis\n";
    std::cout << "• Extensible framework for domain-specific languages\n";
    
    return 0;
}