// Utils.h - Exceptions, string helpers, validation rules and console input helpers
#ifndef UTILS_H
#define UTILS_H

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------- exceptions
class AppException : public std::runtime_error {
public:
    explicit AppException(const std::string& msg) : std::runtime_error(msg) {}
};
class ValidationException : public AppException {   // bad user/file data
public:
    explicit ValidationException(const std::string& msg) : AppException(msg) {}
};
class NotFoundException : public AppException {     // ID / record does not exist
public:
    explicit NotFoundException(const std::string& msg) : AppException(msg) {}
};
class OperationException : public AppException {    // business-rule violation
public:
    explicit OperationException(const std::string& msg) : AppException(msg) {}
};
class FileException : public AppException {         // file open/read/write problem
public:
    explicit FileException(const std::string& msg) : AppException(msg) {}
};
class InputEnded : public std::exception {          // stdin closed (Ctrl+Z / Ctrl+D)
public:
    const char* what() const noexcept override { return "Input stream closed"; }
};

// ------------------------------------------------------------ string helpers
namespace Utils {
    std::vector<std::string> split(const std::string& s, char delim);
    std::string trim(const std::string& s);
    std::string toLower(const std::string& s);
    bool containsIgnoreCase(const std::string& hay, const std::string& needle);
    std::string fit(const std::string& s, size_t width);   // truncate with ".."
    std::string pad(const std::string& s, size_t width);   // truncate + right-pad
    std::string toStr(long long v);                         // integer -> text (portable std::to_string)
    std::string num(double v, int precision = 2);           // fixed-point number
    std::string money(double v);                            // "Rs.123.45"
    std::string now();                                      // "YYYY-MM-DD HH:MM"
    int    toInt(const std::string& s, const std::string& field);
    double toDouble(const std::string& s, const std::string& field);
    void   field(const std::string& label, const std::string& value);  // "  Label : value"
    void   line(char ch = '-', int n = 78);
}

// ----------------------------------------------------------- validation rules
namespace Validate {
    void name(const std::string& v, const std::string& field = "Name");
    void phone(const std::string& v);
    void text(const std::string& v, const std::string& field, size_t maxLen = 60);
    void range(double v, double lo, double hi, const std::string& field);
}

// -------------------------------------------------------------- console input
namespace Input {
    extern bool echo;   // echo typed lines (useful for scripted demos / screenshots)

    std::string line(const std::string& prompt, bool allowEmpty = false);
    int         integer(const std::string& prompt, int lo, int hi);
    double      number(const std::string& prompt, double lo, double hi);
    std::string name(const std::string& prompt);
    std::string phone(const std::string& prompt);
    std::string text(const std::string& prompt, size_t maxLen = 60);
    bool        yesNo(const std::string& prompt);
}

#endif
