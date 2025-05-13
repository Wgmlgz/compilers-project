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
    IMMUTABLE_ERROR,
    FORMAT_ERROR,
    FUNCTION_ERROR,
    GENERAL_ERROR
};

struct SourceSpan {
    int start_line;
    int start_column;
    int end_line;
    int end_column;
    std::string filename;
    
    SourceSpan(int sl = 0, int sc = 0, int el = 0, int ec = 0, const std::string& f = "")
        : start_line(sl), start_column(sc), end_line(el), end_column(ec), filename(f) {}
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
    std::vector<std::pair<SourceLocation, std::string>> additional_notes;
    std::string suggested_fix;
    std::string fix_code;

public:
    CompilerError(ErrorType t, const std::string& msg, const SourceLocation& loc = current_location)
        : type(t), message(msg), primary_location(loc) {}
    
    void add_note(const SourceLocation& loc, const std::string& note) {
        additional_notes.emplace_back(loc, note);
    }
    
    void suggest_fix(const std::string& suggestion, const std::string& code = "") {
        suggested_fix = suggestion;
        fix_code = code;
    }
    
    std::string formatError() const {
        std::stringstream ss;
        
        // Start with colorized error type (if terminal supports it)
        switch (type) {
            case ErrorType::SYNTAX_ERROR:
                ss << "\033[1;31merror[E01]:\033[0m syntax error";
                break;
            case ErrorType::TYPE_ERROR:
                ss << "\033[1;31merror[E02]:\033[0m type mismatch";
                break;
            case ErrorType::NAME_ERROR:
                ss << "\033[1;31merror[E03]:\033[0m name error";
                break;
            case ErrorType::IMMUTABLE_ERROR:
                ss << "\033[1;31merror[E04]:\033[0m immutability violation";
                break;
            case ErrorType::FORMAT_ERROR:
                ss << "\033[1;31merror[E05]:\033[0m format string error";
                break;
            case ErrorType::FUNCTION_ERROR:
                ss << "\033[1;31merror[E06]:\033[0m function error";
                break;
            case ErrorType::GENERAL_ERROR:
                ss << "\033[1;31merror[E00]:\033[0m";
                break;
        }
        
        // Add the error message
        ss << " " << message << std::endl;
        
        // Add primary location with code context
        if (primary_location.line > 0 && !source_lines.empty()) {
            // File location
            ss << " \033[1;34m-->\033[0m ";
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
                        ss << " \033[1;34m" << padding << i << " |\033[0m " << source_lines[i - 1] << std::endl;
                        
                        // Add carets under the error position
                        ss << " \033[1;34m" << std::string(line_num_width, ' ') << " | \033[0m";
                        
                        // Indent to the column position
                        for (int j = 1; j < primary_location.column; j++) {
                            ss << " ";
                        }
                        ss << "\033[1;31m^";
                        
                        // For multi-character errors, add more carets
                        int error_length = 1; // Default to 1 if we don't know
                        for (int j = 1; j < error_length; j++) {
                            ss << "~";
                        }
                        ss << "\033[0m ";
                        ss << "\033[1;31m" << message << "\033[0m" << std::endl;
                    } else {
                        ss << " \033[1;34m" << padding << i << " |\033[0m " << source_lines[i - 1] << std::endl;
                    }
                }
            }
        }
        
        // Add additional notes
        for (const auto& [loc, note] : additional_notes) {
            if (loc.line > 0 && loc.line <= static_cast<int>(source_lines.size())) {
                ss << " \033[1;34m-->\033[0m ";
                if (!loc.filename.empty()) {
                    ss << loc.filename << ":";
                }
                ss << loc.line << ":" << loc.column << std::endl;
                
                // Show the line with the note
                int line_num_width = std::to_string(loc.line).length();
                std::string padding(line_num_width - std::to_string(loc.line).length(), ' ');
                
                ss << " \033[1;34m" << padding << loc.line << " |\033[0m " << source_lines[loc.line - 1] << std::endl;
                
                // Add marker under the column position
                ss << " \033[1;34m" << std::string(line_num_width, ' ') << " | \033[0m";
                for (int j = 1; j < loc.column; j++) {
                    ss << " ";
                }
                ss << "\033[1;33m^ " << note << "\033[0m" << std::endl;
            } else {
                // If we don't have source for this location, just show the note
                ss << " \033[1;33mnote:\033[0m " << note << std::endl;
            }
        }
        
        // Add help message with potential fix
        if (!suggested_fix.empty()) {
            ss << " \033[1;32mhelp:\033[0m " << suggested_fix << std::endl;
            
            // Show example code for the fix if provided
            if (!fix_code.empty()) {
                ss << " \033[1;34m|\033[0m" << std::endl;
                
                std::stringstream fix_ss(fix_code);
                std::string line;
                int line_num = 1;
                
                while (std::getline(fix_ss, line)) {
                    ss << " \033[1;34m|\033[0m " << line << std::endl;
                    line_num++;
                }
            }
        } else {
            // Generic help message if no specific suggestion
            switch (type) {
                case ErrorType::SYNTAX_ERROR:
                    ss << " \033[1;32mhelp:\033[0m check for missing semicolons, mismatched brackets, or invalid token sequences" << std::endl;
                    break;
                case ErrorType::TYPE_ERROR:
                    ss << " \033[1;32mhelp:\033[0m make sure variable types match their assigned values and operation arguments" << std::endl;
                    break;
                case ErrorType::NAME_ERROR:
                    ss << " \033[1;32mhelp:\033[0m ensure all variables are defined before use and are in the current scope" << std::endl;
                    break;
                case ErrorType::IMMUTABLE_ERROR:
                    ss << " \033[1;32mhelp:\033[0m use 'let mut' for variables that need to be reassigned" << std::endl;
                    break;
                case ErrorType::FORMAT_ERROR:
                    ss << " \033[1;32mhelp:\033[0m check that format strings have matching arguments and valid format specifiers" << std::endl;
                    break;
                case ErrorType::FUNCTION_ERROR:
                    ss << " \033[1;32mhelp:\033[0m verify function arguments count, types, and that all code paths return a value" << std::endl;
                    break;
                default:
                    break;
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
void immutableError(const std::string& message, const SourceLocation& location = current_location);
void formatError(const std::string& message, const SourceLocation& location = current_location);
void functionError(const std::string& message, const SourceLocation& location = current_location);
void syntaxError(const std::string& message, const SourceLocation& location = current_location);

// Enhanced error functions with notes and suggestions
CompilerError createError(ErrorType type, const std::string& message, const SourceLocation& location = current_location);

// Location tracking
void updateLocation(int line, int column, const std::string& filename);

std::string typeToString(int type);

#endif // ERROR_HPP 