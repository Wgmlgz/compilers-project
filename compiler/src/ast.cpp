#include "ast.hpp"
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


// // Check if a variable exists in the current or parent scope
// bool hasVar(const string &name) {
//     for (auto &scope : std::views::reverse(scopes)) {
//         if (scope.count(name)) return true;
//     }
//     return false;
// }

// // Get variable with type checking
// Value getVar(const string &name) {
//     for (auto &scope : std::views::reverse(scopes)) {
//         if (scope.count(name)) return scope[name];
//     }
    
//     // Create detailed error for undefined variable
//     auto error = createError(ErrorType::NAME_ERROR, 
//                              "Cannot find variable '" + name + "' in this scope", 
//                              current_location);
    
//     // Suggest looking for typos or declaring the variable
//     error.suggest_fix("make sure the variable is declared before use", 
//                       "let " + name + " = value;");
    
//     error.report();
//     exit(1);
// }

// // Get variable type info
// VariableInfo getVarInfo(const string &name) {
//     for (auto &scope : std::views::reverse(varTypes)) {
//         if (scope.count(name)) return scope[name];
//     }
//     nameError("Undefined variable '" + name + "'");
//     exit(1); // Redundant as nameError will exit, but keeps compiler happy
// }

// // Create a new variable with type information
// void createVar(const string &name, Value value, bool mutable_, Type type) { 
//     if (scopes.back().count(name)) {
//         nameError("Variable '" + name + "' already exists in this scope");
//         exit(1);
//     }
//     scopes.back()[name] = value;
//     varTypes.back()[name] = VariableInfo(type, mutable_);
// }

// // Update an existing variable with type checking
// void setVar(const string &name, Value value) {
//     for (int i = scopes.size() - 1; i >= 0; i--) {
//         if (scopes[i].count(name)) {
//             // Check mutability
//             VariableInfo info = varTypes[i][name];
//             if (!info.mutable_) {
//                 immutableError("Cannot assign to immutable variable '" + name + "'");
//                 exit(1);
//             }
//             // Type checking would occur here
//             scopes[i][name] = value;
//             return;
//         }
//     }
//     nameError("Undefined variable '" + name + "'");
//     exit(1);
// }

// // Debug function to print the contents of variables
// void debugPrintVars() {
//     std::cout << "Variables in current scope:" << std::endl;
//     for (const auto& scope : scopes) {
//         for (const auto& [name, value] : scope) {
//             std::cout << name << " = ";
//             if (std::holds_alternative<int>(value)) {
//                 std::cout << std::get<int>(value);
//             } else if (std::holds_alternative<bool>(value)) {
//                 std::cout << (std::get<bool>(value) ? "true" : "false");
//             } else if (std::holds_alternative<string>(value)) {
//                 std::cout << "\"" << std::get<string>(value) << "\"";
//             } else {
//                 std::cout << "()";
//             }
//             std::cout << std::endl;
//         }
//     }
// }
