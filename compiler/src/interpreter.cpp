#include "interpreter.hpp"
#include "error.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <ranges>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <vector>
#include <algorithm>

// Variable scope management with type information
std::vector<std::unordered_map<string, VariableInfo>> varTypes = {{}};
std::vector<std::unordered_map<string, Value>> scopes = {{}};
std::unordered_map<string, FunctionNode*> functions;

void enterScope() { 
    scopes.push_back({}); 
    varTypes.push_back({});
}

void exitScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
        varTypes.pop_back();
    }
}

// Check if a variable exists in the current or parent scope
bool hasVar(const string &name) {
    for (auto &scope : std::views::reverse(scopes)) {
        if (scope.count(name)) return true;
    }
    return false;
}

// Get variable with type checking
Value getVar(const string &name) {
    for (auto &scope : std::views::reverse(scopes)) {
        if (scope.count(name)) return scope[name];
    }
    
    // Create detailed error for undefined variable
    auto error = createError(ErrorType::NAME_ERROR, 
                             "Cannot find variable '" + name + "' in this scope", 
                             current_location);
    
    // Suggest looking for typos or declaring the variable
    error.suggest_fix("make sure the variable is declared before use", 
                      "let " + name + " = value;");
    
    error.report();
    exit(1);
}

// Get variable type info
VariableInfo getVarInfo(const string &name) {
    for (auto &scope : std::views::reverse(varTypes)) {
        if (scope.count(name)) return scope[name];
    }
    nameError("Undefined variable '" + name + "'");
    exit(1); // Redundant as nameError will exit, but keeps compiler happy
}

// Create a new variable with type information
void createVar(const string &name, Value value, bool mutable_, Type type) { 
    if (scopes.back().count(name)) {
        nameError("Variable '" + name + "' already exists in this scope");
        exit(1);
    }
    scopes.back()[name] = value;
    varTypes.back()[name] = VariableInfo(type, mutable_);
}

// Update an existing variable with type checking
void setVar(const string &name, Value value) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        if (scopes[i].count(name)) {
            // Check mutability
            VariableInfo info = varTypes[i][name];
            if (!info.mutable_) {
                immutableError("Cannot assign to immutable variable '" + name + "'");
                exit(1);
            }
            // Type checking would occur here
            scopes[i][name] = value;
            return;
        }
    }
    nameError("Undefined variable '" + name + "'");
    exit(1);
}

// Convert a value to int
int toInt(const Value &v) {
    if (auto p = std::get_if<int>(&v)) return *p;
    if (auto p = std::get_if<bool>(&v)) return *p ? 1 : 0;
    
    typeError("Cannot convert value to integer");
    exit(1);
}

// Convert a value to string representation
string toString(const Value &v) {
    if (auto p = std::get_if<int>(&v)) {
        return std::to_string(*p);
    }
    if (auto p = std::get_if<bool>(&v)) {
        return *p ? "true" : "false";
    }
    if (auto p = std::get_if<string>(&v)) {
        return *p;
    }
    return "()"; // Unit type
}

// Determine if a value is truthy
bool isTruthy(const Value &val) {
    if (auto p = std::get_if<bool>(&val)) return *p;
    if (std::holds_alternative<UndefinedType>(val)) return false;
    if (auto p = std::get_if<int>(&val)) return *p != 0;
    if (auto p = std::get_if<string>(&val)) return !p->empty();
    return false;
}

// Evaluate a variable node
Value VariableNode::evaluate() const { 
    return getVar(name); 
}

Type VariableNode::typeCheck() const {
    return getVarInfo(name).type;
}

// Evaluate a binary operation node
Value BinaryNode::evaluate() const {
    Value leftVal = left->evaluate();
    Value rightVal = right->evaluate();

    // String concatenation
    if (op == "+" && 
        (std::holds_alternative<string>(leftVal) || 
         std::holds_alternative<string>(rightVal))) {
        return toString(leftVal) + toString(rightVal);
    }

    // Arithmetic operations
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        int l = toInt(leftVal);
        int r = toInt(rightVal);

        if (op == "+") return l + r;
        if (op == "-") return l - r;
        if (op == "*") return l * r;
        if (op == "/") {
            if (r == 0) {
                typeError("Division by zero");
                exit(1);
            }
            return l / r;
        }
        if (op == "%") {
            if (r == 0) {
                typeError("Modulo by zero");
                exit(1);
            }
            return l % r;
        }
    }

    // Comparison operations
    if (op == "==" || op == "!=") {
        if (std::holds_alternative<int>(leftVal) && 
            std::holds_alternative<int>(rightVal)) {
            int l = std::get<int>(leftVal);
            int r = std::get<int>(rightVal);
            return op == "==" ? (l == r) : (l != r);
        } else if (std::holds_alternative<bool>(leftVal) && 
                  std::holds_alternative<bool>(rightVal)) {
            bool l = std::get<bool>(leftVal);
            bool r = std::get<bool>(rightVal);
            return op == "==" ? (l == r) : (l != r);
        } else if (std::holds_alternative<string>(leftVal) && 
                  std::holds_alternative<string>(rightVal)) {
            string l = std::get<string>(leftVal);
            string r = std::get<string>(rightVal);
            return op == "==" ? (l == r) : (l != r);
        } else {
            // Different types are always not equal
            return op == "==" ? false : true;
        }
    }

    if (op == ">" || op == "<" || op == ">=" || op == "<=") {
        int l = toInt(leftVal);
        int r = toInt(rightVal);
        if (op == ">") return l > r;
        if (op == "<") return l < r;
        if (op == ">=") return l >= r;
        if (op == "<=") return l <= r;
    }

    // Logical operations
    if (op == "&&" || op == "||") {
        bool l = isTruthy(leftVal);
        if (op == "&&") return l ? isTruthy(rightVal) : false;
        if (op == "||") return l ? true : isTruthy(rightVal);
    }

    std::cerr << "Error: Invalid operator '" << op << "'" << std::endl;
    exit(1);
}

Type BinaryNode::typeCheck() const {
    Type leftType = left->typeCheck();
    Type rightType = right->typeCheck();
    
    // String concatenation
    if (op == "+") {
        if (leftType == Type::STR || rightType == Type::STR) {
            return Type::STR;
        }
        
        // Numeric addition
        if (leftType == Type::I32 && rightType == Type::I32) {
            return Type::I32;
        }
        
        // Create detailed error for type mismatch
        auto error = createError(ErrorType::TYPE_ERROR, 
                                "Mismatched types for operator '+': cannot add " + 
                                typeToString(static_cast<int>(leftType)) + " and " + 
                                typeToString(static_cast<int>(rightType)), 
                                current_location);
        
        std::string fixExample = "// Example for numbers:\nlet a: i32 = 5;\nlet b: i32 = 10;\nlet sum = a + b;\n\n";
        fixExample += "// Example for strings:\nlet s1 = \"hello\";\nlet s2 = \"world\";\nlet combined = s1 + s2;";
        
        error.suggest_fix("ensure both operands have compatible types", fixExample);
        
        error.report();
        exit(1);
    }
    
    // Arithmetic operations
    if (op == "-" || op == "*" || op == "/" || op == "%") {
        if (leftType == Type::I32 && rightType == Type::I32) {
            return Type::I32;
        }
        
        typeError("Cannot apply operator '" + op + "' to types " + 
                  typeToString(static_cast<int>(leftType)) + " and " + 
                  typeToString(static_cast<int>(rightType)));
        return Type::UNKNOWN;
    }
    
    // Comparison operations
    if (op == "==" || op == "!=" || op == ">" || op == "<" || op == ">=" || op == "<=") {
        // Type compatibility check
        if (leftType != rightType && 
            !(leftType == Type::I32 && rightType == Type::I32)) {
            typeError("Cannot compare values of different types: " + 
                      typeToString(static_cast<int>(leftType)) + " and " + 
                      typeToString(static_cast<int>(rightType)));
        }
        return Type::BOOL;
    }
    
    // Logical operations
    if (op == "&&" || op == "||") {
        if (leftType != Type::BOOL || rightType != Type::BOOL) {
            typeError("Logical operators require boolean operands");
        }
        return Type::BOOL;
    }
    
    return Type::UNKNOWN;
}

// Evaluate a unary operation node
Value UnaryNode::evaluate() const {
    Value result = right->evaluate();
    
    if (op == "-") {
        int num = toInt(result);
        return -num;
    } else if (op == "!") {
        return !isTruthy(result);
    }
    
    std::cerr << "Error: Invalid unary operator '" << op << "'" << std::endl;
    exit(1);
}

Type UnaryNode::typeCheck() const {
    Type rightType = right->typeCheck();
    
    if (op == "-") {
        if (rightType != Type::I32) {
            std::cerr << "Error: Unary '-' requires i32 operand" << std::endl;
            exit(1);
        }
        return Type::I32;
    } else if (op == "!") {
        if (rightType != Type::BOOL) {
            std::cerr << "Error: Unary '!' requires bool operand" << std::endl;
            exit(1);
        }
        return Type::BOOL;
    }
    
    std::cerr << "Error: Unknown unary operator '" << op << "'" << std::endl;
    exit(1);
}

// Assign a value to a variable
Value AssignNode::evaluate() const {
    // First, check if the variable exists
    if (!hasVar(name)) {
        nameError("Undefined variable '" + name + "'");
        exit(1);
    }
    
    // Check if the variable is mutable
    VariableInfo info = getVarInfo(name);
    if (!info.mutable_) {
        // Create a detailed error with context and fix suggestion
        auto error = createError(ErrorType::IMMUTABLE_ERROR, 
                                "Cannot assign to immutable variable '" + name + "'", 
                                current_location);
        
        // Add a specific note and suggestion for the fix
        error.suggest_fix("consider declaring the variable with 'mut' to make it mutable", 
                          "let mut " + name + " = value;\n" +
                          name + " = new_value;");
        
        error.report();
        exit(1);
    }
    
    // If we get here, assignment is allowed
    Value result = expression->evaluate();
    setVar(name, result);
    return result;
}

Type AssignNode::typeCheck() const {
    Type exprType = expression->typeCheck();
    Type varType = getVarInfo(name).type;
    
    if (varType != exprType && varType != Type::UNKNOWN) {
        std::cerr << "Error: Cannot assign " << static_cast<int>(exprType) 
                  << " to variable '" << name << "' of type " 
                  << static_cast<int>(varType) << std::endl;
        exit(1);
    }
    
    return exprType;
}

// Declare a new variable
Value VarDeclNode::evaluate() const {
    Value result = expression->evaluate();
    createVar(name, result, mutable_, declaredType == Type::UNKNOWN ? expression->typeCheck() : declaredType);
    return result;
}

Type VarDeclNode::typeCheck() const {
    Type exprType = expression->typeCheck();
    
    if (declaredType != Type::UNKNOWN && declaredType != exprType) {
        std::cerr << "Error: Cannot initialize " << static_cast<int>(declaredType) 
                  << " variable '" << name << "' with " 
                  << static_cast<int>(exprType) << " value" << std::endl;
        exit(1);
    }
    
    return declaredType == Type::UNKNOWN ? exprType : declaredType;
}

// Evaluate an if statement
Value IfNode::evaluate() const {
    if (isTruthy(condition->evaluate())) {
        return thenBlock->evaluate();
    } else if (elseBlock) {
        return elseBlock->evaluate();
    }
    return UndefinedType();
}

Type IfNode::typeCheck() const {
    Type condType = condition->typeCheck();
    if (condType != Type::BOOL) {
        std::cerr << "Error: If condition must be a bool" << std::endl;
        exit(1);
    }
    
    Type thenType = thenBlock->typeCheck();
    
    if (elseBlock) {
        Type elseType = elseBlock->typeCheck();
        if (thenType != elseType) {
            std::cerr << "Error: If and else blocks must have the same type" << std::endl;
            exit(1);
        }
    }
    
    return thenType;
}

// Evaluate a block of statements
Value BlockNode::evaluate() const {
    enterScope();
    Value lastValue = UndefinedType();
    
    for (size_t i = 0; i < statements.size(); i++) {
        lastValue = statements[i]->evaluate();
        
        // If this is the last statement and returnsValue is true, return its value
        if (i == statements.size() - 1 && returnsValue) {
            exitScope();
            return lastValue;
        }
    }
    
    exitScope();
    return lastValue;
}

Type BlockNode::typeCheck() const {
    enterScope();
    Type lastType = Type::UNIT;
    
    for (size_t i = 0; i < statements.size(); i++) {
        lastType = statements[i]->typeCheck();
    }
    
    exitScope();
    return lastType;
}

// Evaluate a while loop
Value WhileNode::evaluate() const {
    while (isTruthy(condition->evaluate())) {
        block->evaluate();
    }
    return UndefinedType();
}

Type WhileNode::typeCheck() const {
    Type condType = condition->typeCheck();
    if (condType != Type::BOOL) {
        std::cerr << "Error: While condition must be a bool" << std::endl;
        exit(1);
    }
    
    block->typeCheck();
    return Type::UNIT;
}

// Function declaration
Value FunctionNode::evaluate() const {
    functions[name] = const_cast<FunctionNode*>(this);
    return UndefinedType();
}

Type FunctionNode::typeCheck() const {
    // Check return type against function body
    enterScope();
    
    // Add parameters to scope
    for (const auto& param : parameters) {
        createVar(param.name, UndefinedType(), false, param.type);
    }
    
    Type bodyType = body->typeCheck();
    
    // Check return type
    if (returnType != bodyType && returnType != Type::UNIT) {
        std::cerr << "Error: Function '" << name 
                  << "' returns " << static_cast<int>(bodyType)
                  << " but declared return type is " << static_cast<int>(returnType) << std::endl;
        exit(1);
    }
    
    exitScope();
    return returnType;
}

// Return statement
Value ReturnNode::evaluate() const {
    Value val = expression ? expression->evaluate() : UndefinedType();
    throw ReturnValue(val);
}

Type ReturnNode::typeCheck() const {
    if (!expression) {
        return Type::UNIT;
    }
    return expression->typeCheck();
}

// Function call
Value FunctionCallNode::evaluate() const {
    // Look up function by name
    if (functions.find(functionName) == functions.end()) {
        functionError("Undefined function '" + functionName + "'");
        exit(1);
    }
    
    auto func = functions[functionName];
    
    // Validate argument count
    if (arguments.size() != func->parameters.size()) {
        functionError("Function '" + functionName + "' expects " + 
                     std::to_string(func->parameters.size()) + " arguments, but got " + 
                     std::to_string(arguments.size()));
        exit(1);
    }
    
    // Evaluate arguments
    std::vector<Value> argValues;
    for (int i = 0; i < arguments.size(); i++) {
        argValues.push_back(arguments[i]->evaluate());
    }
    
    // Create function scope with parameter bindings
    enterScope();
    for (int i = 0; i < func->parameters.size(); i++) {
        createVar(func->parameters[i].name, argValues[i], false, func->parameters[i].type);
    }
    
    // Execute function body
    Value result;
    try {
        result = func->body->evaluate();
    } catch (const ReturnValue& rv) {
        result = rv.value;
    }
    
    // Clean up function scope
    exitScope();
    
    return result;
}

Type FunctionCallNode::typeCheck() const {
    // Look up function by name
    if (functions.find(functionName) == functions.end()) {
        functionError("Undefined function '" + functionName + "'");
        return Type::UNKNOWN;
    }
    
    auto func = functions[functionName];
    
    // Validate argument count
    if (arguments.size() != func->parameters.size()) {
        functionError("Function '" + functionName + "' expects " + 
                     std::to_string(func->parameters.size()) + " arguments, but got " + 
                     std::to_string(arguments.size()));
        return Type::UNKNOWN;
    }
    
    // Type check arguments against parameter types
    for (int i = 0; i < arguments.size(); i++) {
        Type argType = arguments[i]->typeCheck();
        Type paramType = func->parameters[i].type;
        
        if (argType != paramType && paramType != Type::UNKNOWN) {
            functionError("Argument " + std::to_string(i) + " to function '" + functionName + 
                         "' is " + typeToString(static_cast<int>(argType)) + 
                         " but expected " + typeToString(static_cast<int>(paramType)));
        }
    }
    
    return func->returnType;
}

// Print formatting
Value PrintNode::evaluate() const {
    std::string formatted = format;
    
    // Replace {} placeholders with argument values
    size_t pos = 0;
    for (size_t argIndex = 0; argIndex < arguments.size(); argIndex++) {
        pos = formatted.find("{}", pos);
        if (pos == std::string::npos) {
            break; // No more placeholders
        }
        
        Value argValue = arguments[argIndex]->evaluate();
        std::string valueStr = toString(argValue);
        
        formatted.replace(pos, 2, valueStr);
        pos += valueStr.length();
    }
    
    std::cout << formatted << std::endl;
    return UndefinedType();
}

// Program node evaluation
Value ProgramNode::evaluate() const {
    // First evaluate all function definitions
    for (const auto& func : functions) {
        func->evaluate();
    }
    
    // Then evaluate the main block if it exists
    if (mainBlock) {
        try {
            std::cout << "\nOutput:" << std::endl;
            Value result = mainBlock->evaluate();
            return result;
        } catch (const std::exception& e) {
            std::cerr << "Runtime error: " << e.what() << std::endl;
            return UndefinedType();
        } catch (...) {
            std::cerr << "Unknown runtime error during evaluation" << std::endl;
            return UndefinedType();
        }
    }
    
    return UndefinedType();
}

// Debug function to print the contents of variables
void debugPrintVars() {
    std::cout << "Variables in current scope:" << std::endl;
    for (const auto& scope : scopes) {
        for (const auto& [name, value] : scope) {
            std::cout << name << " = ";
            if (std::holds_alternative<int>(value)) {
                std::cout << std::get<int>(value);
            } else if (std::holds_alternative<bool>(value)) {
                std::cout << (std::get<bool>(value) ? "true" : "false");
            } else if (std::holds_alternative<string>(value)) {
                std::cout << "\"" << std::get<string>(value) << "\"";
            } else {
                std::cout << "()";
            }
            std::cout << std::endl;
        }
    }
}
