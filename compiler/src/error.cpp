#include "error.hpp"
#include "compiler.hpp"
#include <fstream>
#include <sstream>

// Initialize current location global variable
SourceLocation current_location;

// Store source code for error reporting
std::vector<std::string> source_lines;

void load_source_file(const std::string& filename) {
    source_lines.clear();
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            source_lines.push_back(line);
        }
    }
}

void load_source_from_string(const std::string& source) {
    source_lines.clear();
    std::istringstream stream(source);
    std::string line;
    while (std::getline(stream, line)) {
        source_lines.push_back(line);
    }
}

CompilerError createError(ErrorType type, const std::string& message, const SourceLocation& location) {
    return CompilerError(type, message, location);
}

void reportError(ErrorType type, const std::string& message, const SourceLocation& location) {
    CompilerError error(type, message, location);
    error.report();
    exit(1);  // Exit the program on error
}

void typeError(const std::string& message, const SourceLocation& location) {
    reportError(ErrorType::TYPE_ERROR, message, location);
}

void nameError(const std::string& message, const SourceLocation& location) {
    reportError(ErrorType::NAME_ERROR, message, location);
}

void syntaxError(const std::string& message, const SourceLocation& location) {
    reportError(ErrorType::SYNTAX_ERROR, message, location);
}

std::string typeToString(int type) {
    switch (static_cast<Type>(type)) {
        case Type::I32:
            return "i32";
        case Type::STR:
            return "str";
        case Type::UNKNOWN:
            return "unknown";
        default:
            return "invalid_type";
    }
}

// Update location for error reporting
void updateLocation(int line, int column, const std::string& filename) {
    current_location.line = line;
    current_location.column = column;
    current_location.filename = filename;
} 