#include "Utils.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Utils {

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::string cur;
    for (char ch : s) {
        if (ch == delim) { parts.push_back(cur); cur.clear(); }
        else cur += ch;
    }
    parts.push_back(cur);
    return parts;
}

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string toLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return r;
}

bool containsIgnoreCase(const std::string& hay, const std::string& needle) {
    return toLower(hay).find(toLower(needle)) != std::string::npos;
}

std::string fit(const std::string& s, size_t width) {
    if (s.size() <= width) return s;
    if (width <= 2) return s.substr(0, width);
    return s.substr(0, width - 2) + "..";
}

std::string pad(const std::string& s, size_t width) {
    std::string r = fit(s, width);
    if (r.size() < width) r.append(width - r.size(), ' ');
    return r;
}

std::string toStr(long long v) {
    std::ostringstream o;
    o << v;
    return o.str();
}

std::string num(double v, int precision) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(precision) << v;
    return o.str();
}

std::string money(double v) { return "Rs." + num(v, 2); }

std::string now() {
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", lt);
    return buf;
}

int toInt(const std::string& s, const std::string& field) {
    if (s.empty()) throw ValidationException(field + " is not a valid whole number");
    char* end = nullptr;
    errno = 0;
    long v = std::strtol(s.c_str(), &end, 10);
    if (*end != '\0') throw ValidationException(field + " is not a valid whole number");
    if (errno == ERANGE || v > INT_MAX || v < INT_MIN) throw ValidationException(field + " is out of range");
    return static_cast<int>(v);
}

double toDouble(const std::string& s, const std::string& field) {
    if (s.empty()) throw ValidationException(field + " is not a valid number");
    for (size_t i = 0; i < s.size(); ++i) {             // plain decimal notation only (no hex / inf / nan)
        char c = s[i];
        if (!(std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E'))
            throw ValidationException(field + " is not a valid number");
    }
    char* end = nullptr;
    errno = 0;
    double v = std::strtod(s.c_str(), &end);
    if (*end != '\0') throw ValidationException(field + " is not a valid number");
    if (errno == ERANGE || !std::isfinite(v)) throw ValidationException(field + " is out of range");
    return v;
}

void field(const std::string& label, const std::string& value) {
    std::cout << "  " << std::left << std::setw(14) << label << ": " << value << "\n";
}

void line(char ch, int n) { std::cout << std::string(n, ch) << "\n"; }

} // namespace Utils

// ------------------------------------------------------------------ Validate
namespace Validate {

void name(const std::string& v, const std::string& field) {
    if (v.size() < 2 || v.size() > 40)
        throw ValidationException(field + " must be 2 to 40 characters long");
    for (char ch : v) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (!(std::isalpha(c) || ch == ' ' || ch == '.' || ch == '\'' || ch == '-'))
            throw ValidationException(field + " may contain only letters, spaces, . ' and -");
    }
}

void phone(const std::string& v) {
    if (v.size() != 10)
        throw ValidationException("Phone must be exactly 10 digits");
    for (char ch : v)
        if (!std::isdigit(static_cast<unsigned char>(ch)))
            throw ValidationException("Phone must contain digits only");
    if (v[0] < '6' || v[0] > '9')
        throw ValidationException("Phone must start with 6, 7, 8 or 9");
}

void text(const std::string& v, const std::string& field, size_t maxLen) {
    if (v.empty()) throw ValidationException(field + " cannot be empty");
    if (v.size() > maxLen)
        throw ValidationException(field + " is too long (max " + Utils::toStr(maxLen) + " characters)");
    for (char ch : v)
        if (ch == '|' || static_cast<unsigned char>(ch) < 32)
            throw ValidationException(field + " contains an invalid character ('|' is reserved)");
}

void range(double v, double lo, double hi, const std::string& field) {
    if (v < lo || v > hi)
        throw ValidationException(field + " must be between " + Utils::num(lo, lo == (int)lo ? 0 : 1) +
                                  " and " + Utils::num(hi, hi == (int)hi ? 0 : 1));
}

} // namespace Validate

// --------------------------------------------------------------------- Input
namespace Input {

bool echo = false;

std::string line(const std::string& prompt, bool allowEmpty) {
    while (true) {
        std::cout << prompt << std::flush;
        std::string s;
        if (!std::getline(std::cin, s)) throw InputEnded();
        s = Utils::trim(s);
        if (echo) std::cout << s << "\n";
        if (s.empty() && !allowEmpty) {
            std::cout << "  Input cannot be empty.\n";
            continue;
        }
        return s;
    }
}

int integer(const std::string& prompt, int lo, int hi) {
    while (true) {
        std::string s = line(prompt);
        try {
            int v = Utils::toInt(s, "Value");
            Validate::range(v, lo, hi, "Value");
            return v;
        } catch (const AppException& e) {
            std::cout << "  Invalid input: " << e.what() << ".\n";
        }
    }
}

double number(const std::string& prompt, double lo, double hi) {
    while (true) {
        std::string s = line(prompt);
        try {
            double v = Utils::toDouble(s, "Value");
            Validate::range(v, lo, hi, "Value");
            return v;
        } catch (const AppException& e) {
            std::cout << "  Invalid input: " << e.what() << ".\n";
        }
    }
}

static std::string validated(const std::string& prompt,
                             const std::function<void(const std::string&)>& rule) {
    while (true) {
        std::string s = line(prompt);
        try {
            rule(s);
            return s;
        } catch (const AppException& e) {
            std::cout << "  Invalid input: " << e.what() << ".\n";
        }
    }
}

std::string name(const std::string& prompt) {
    return validated(prompt, [](const std::string& s) { Validate::name(s); });
}
std::string phone(const std::string& prompt) {
    return validated(prompt, [](const std::string& s) { Validate::phone(s); });
}
std::string text(const std::string& prompt, size_t maxLen) {
    return validated(prompt, [maxLen](const std::string& s) { Validate::text(s, "Text", maxLen); });
}

bool yesNo(const std::string& prompt) {
    while (true) {
        std::string s = Utils::toLower(line(prompt + " (y/n): "));
        if (s == "y" || s == "yes") return true;
        if (s == "n" || s == "no")  return false;
        std::cout << "  Please answer y or n.\n";
    }
}

} // namespace Input
