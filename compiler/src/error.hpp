#ifndef ERROR_HPP
#define ERROR_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>

enum class ErrorType {
    SYNTAX_ERROR,
    TYPE_ERROR,
    NAME_ERROR,
    GENERAL_ERROR
};

struct SourceLocation {
    int line;
    int column;
    std::string filename;
    
    SourceLocation(int l = 0, int c = 0, const std::string& f = "")
        : line(l), column(c), filename(f) {}
};

extern SourceLocation current_location;

// Store the source code for displaying in error messages
extern std::vector<std::string> source_lines;

// Read source file into memory
void load_source_file(const std::string& filename);

// Load from string (for tests)
void load_source_from_string(const std::string& source);

class CompilerError {
private:
    ErrorType type;
    std::string message;
    SourceLocation primary_location;

public:
    CompilerError(ErrorType t, const std::string& msg, const SourceLocation& loc = current_location)
        : type(t), message(msg), primary_location(loc) {}
    
    std::string formatError() const {
        std::stringstream ss;
        
        switch (type) {
            case ErrorType::SYNTAX_ERROR:
                ss << "error[E01] syntax error";
                break;
            case ErrorType::TYPE_ERROR:
                ss << "error[E02] type mismatch";
                break;
            case ErrorType::NAME_ERROR:
                ss << "error[E03] name error";
                break;
            case ErrorType::GENERAL_ERROR:
                ss << "error[E00]";
                break;
        }
        
        // Add the error message
        ss << " " << message << std::endl;
        
        // Add primary location with code context
        if (primary_location.line > 0 && !source_lines.empty()) {
            // File location
            ss << " --> ";
            if (!primary_location.filename.empty()) {
                ss << primary_location.filename << ":";
            }
            ss << primary_location.line << ":" << primary_location.column << std::endl;
            
            // Show 2 lines before and after for context
            int context_start = std::max(1, primary_location.line - 2);
            int context_end = std::min(static_cast<int>(source_lines.size()), primary_location.line + 2);
            
            // Calculate width for line numbers
            int line_num_width = std::to_string(context_end).length();
            
            // Display context
            for (int i = context_start; i <= context_end; i++) {
                if (i - 1 < source_lines.size()) {
                    std::string padding(line_num_width - std::to_string(i).length(), ' ');
                    
                    // Mark the error line
                    if (i == primary_location.line) {
                        ss << " " << padding << i << " | " << source_lines[i - 1] << std::endl;
                        
                        // Add carets under the error position
                        ss << " " << std::string(line_num_width, ' ') << " | ";
                        
                        // Indent to the column position
                        for (int j = 1; j < primary_location.column; j++) {
                            ss << " ";
                        }
                        ss << "^";
                        
                        // For multi-character errors, add more carets
                        int error_length = 1; // Default to 1 if we don't know
                        for (int j = 1; j < error_length; j++) {
                            ss << "~";
                        }
                        ss << " ";
                        ss << "" << message << "" << std::endl;
                    } else {
                        ss << " " << padding << i << " | " << source_lines[i - 1] << std::endl;
                    }
                }
            }
        }
        
        return ss.str();
    }
    
    void report() const {
        std::cerr << formatError();
    }
};

// Global error functions
void reportError(ErrorType type, const std::string& message, const SourceLocation& location = current_location);
void typeError(const std::string& message, const SourceLocation& location = current_location);
void nameError(const std::string& message, const SourceLocation& location = current_location);
void syntaxError(const std::string& message, const SourceLocation& location = current_location);

CompilerError createError(ErrorType type, const std::string& message, const SourceLocation& location = current_location);

void updateLocation(int line, int column, const std::string& filename);

std::string typeToString(int type);

#endif // ERROR_HPP 