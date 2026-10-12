#include "FileManager.h"
#include "Utils.h"

#include <cstdio>
#include <fstream>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

// ---- tiny portable file-system helpers (no <filesystem>, works with old MinGW / C++11)
static bool pathExists(const std::string& p) {
    struct stat st;
    return stat(p.c_str(), &st) == 0;
}
static bool isDirectory(const std::string& p) {
    struct stat st;
    return stat(p.c_str(), &st) == 0 && (st.st_mode & S_IFDIR) != 0;
}
static bool makeDirectory(const std::string& p) {
#ifdef _WIN32
    return _mkdir(p.c_str()) == 0;
#else
    return mkdir(p.c_str(), 0755) == 0;
#endif
}
// Creates the folder and any missing parent folders.
static void ensureDirectory(const std::string& path) {
    if (path.empty()) return;
    std::string cur;
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        bool isSep = (c == '/' || c == '\\');
        if (!isSep) cur += c;
        if ((isSep || i + 1 == path.size()) && !cur.empty()) {
            bool driveOnly = (cur.size() == 2 && cur[1] == ':');
            if (cur != "." && cur != ".." && !driveOnly && !isDirectory(cur) && !makeDirectory(cur) && !isDirectory(cur))
                throw FileException("Cannot create folder '" + cur + "'");
        }
        if (isSep) cur += '/';
    }
}

const char* FileManager::CUSTOMERS_FILE = "customers.txt";
const char* FileManager::AGENTS_FILE    = "agents.txt";
const char* FileManager::ROUTES_FILE    = "routes.txt";
const char* FileManager::ORDERS_FILE    = "orders.txt";
const char* FileManager::REPORT_FILE    = "report.txt";

FileManager::FileManager(const std::string& dir) : dataDir(dir) {
    ensureDirectory(dataDir);
}

std::string FileManager::locateDataDir() {
    if (isDirectory("Data"))    return "Data";
    if (isDirectory("../Data")) return "../Data";
    return "Data";                                   // will be created on first use
}

std::string FileManager::pathOf(const std::string& file) const {
    if (dataDir.empty()) return file;
    char last = dataDir[dataDir.size() - 1];
    return (last == '/' || last == '\\') ? dataDir + file : dataDir + "/" + file;
}

// Write to <file>.tmp first and then rename, so a crash never leaves a half-written data file.
void FileManager::writeLines(const std::string& file, const std::string& header,
                             const std::vector<std::string>& lines) const {
    const std::string finalPath = pathOf(file);
    const std::string tmpPath = finalPath + ".tmp";
    {
        std::ofstream out(tmpPath.c_str(), std::ios::trunc);
        if (!out) throw FileException("Cannot open '" + tmpPath + "' for writing");
        out << "# " << header << "\n";
        for (const std::string& l : lines) out << l << "\n";
        out.flush();
        if (!out) throw FileException("Write failed for '" + tmpPath + "'");
    }
#ifdef _WIN32
    std::remove(finalPath.c_str());                  // Windows rename() refuses to overwrite an existing file
#endif
    if (std::rename(tmpPath.c_str(), finalPath.c_str()) != 0)
        throw FileException("Cannot replace '" + finalPath + "' (the new data is kept in '" + tmpPath + "')");
}

bool FileManager::readLines(const std::string& file, std::vector<std::pair<int, std::string> >& out) const {
    const std::string path = pathOf(file);
    if (!pathExists(path)) return false;
    std::ifstream in(path.c_str());
    if (!in) throw FileException("Cannot open '" + path + "' for reading");
    std::string line;
    int lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        line = Utils::trim(line);
        if (line.empty() || line[0] == '#') continue;
        out.push_back(std::make_pair(lineNo, line));
    }
    return true;
}

void FileManager::writeText(const std::string& file, const std::string& content) const {
    std::ofstream out(pathOf(file).c_str(), std::ios::trunc);
    if (!out) throw FileException("Cannot open '" + pathOf(file) + "' for writing");
    out << content;
    if (!out) throw FileException("Write failed for '" + pathOf(file) + "'");
}

// ------------------------------------------------------------------- saving
void FileManager::saveCustomers(const DynArray<Customer>& list) const {
    std::vector<std::string> lines;
    for (int i = 0; i < list.size(); ++i) lines.push_back(list[i].toFileLine());
    writeLines(CUSTOMERS_FILE, "Customers: id|name|phone|address|loyaltyPoints|ordersPlaced", lines);
}

void FileManager::saveAgents(const DynArray<DeliveryAgent*>& list) const {
    std::vector<std::string> lines;
    for (int i = 0; i < list.size(); ++i) lines.push_back(list[i]->toFileLine());   // polymorphic call
    writeLines(AGENTS_FILE, "Agents: vehicle(B/S/C)|id|name|phone|baseArea|available(1/0)|rating|deliveries", lines);
}

void FileManager::saveRoutes(const DynArray<Route>& list) const {
    std::vector<std::string> lines;
    for (int i = 0; i < list.size(); ++i) lines.push_back(list[i].toFileLine());
    writeLines(ROUTES_FILE, "Routes: id|source|destination|distanceKm|trafficFactor", lines);
}

void FileManager::saveOrders(const DynArray<Order>& list) const {
    std::vector<std::string> lines;
    for (int i = 0; i < list.size(); ++i) lines.push_back(list[i].toFileLine());
    writeLines(ORDERS_FILE,
               "Orders: id|customerId|agentId|routeId|items|foodAmount|deliveryFee|status|createdAt|etaMin|rating", lines);
}

// ------------------------------------------------------------------ loading
LoadResult FileManager::loadCustomers(DynArray<Customer>& list) const {
    LoadResult r;
    std::vector<std::pair<int, std::string> > lines;
    r.found = readLines(CUSTOMERS_FILE, lines);
    for (const auto& entry : lines) {
        try {
            Customer c = Customer::fromFileLine(entry.second);
            for (int i = 0; i < list.size(); ++i)
                if (list[i].getId() == c.getId()) throw ValidationException("duplicate ID " + Utils::toStr(c.getId()));
            list.add(c);
            ++r.loaded;
        } catch (const AppException& e) {
            ++r.skipped;
            r.warnings.push_back(std::string(CUSTOMERS_FILE) + " line " + Utils::toStr(entry.first) + ": " + e.what());
        }
    }
    return r;
}

LoadResult FileManager::loadAgents(DynArray<DeliveryAgent*>& list) const {
    LoadResult r;
    std::vector<std::pair<int, std::string> > lines;
    r.found = readLines(AGENTS_FILE, lines);
    for (const auto& entry : lines) {
        DeliveryAgent* a = nullptr;
        try {
            a = DeliveryAgent::fromFileLine(entry.second);
            for (int i = 0; i < list.size(); ++i)
                if (list[i]->getId() == a->getId()) throw ValidationException("duplicate ID " + Utils::toStr(a->getId()));
            list.add(a);
            a = nullptr;                              // ownership moved into the list
            ++r.loaded;
        } catch (const AppException& e) {
            delete a;                                 // never leak a rejected object
            ++r.skipped;
            r.warnings.push_back(std::string(AGENTS_FILE) + " line " + Utils::toStr(entry.first) + ": " + e.what());
        }
    }
    return r;
}

LoadResult FileManager::loadRoutes(DynArray<Route>& list) const {
    LoadResult r;
    std::vector<std::pair<int, std::string> > lines;
    r.found = readLines(ROUTES_FILE, lines);
    for (const auto& entry : lines) {
        try {
            Route rt = Route::fromFileLine(entry.second);
            for (int i = 0; i < list.size(); ++i)
                if (list[i].getId() == rt.getId()) throw ValidationException("duplicate ID " + Utils::toStr(rt.getId()));
            list.add(rt);
            ++r.loaded;
        } catch (const AppException& e) {
            ++r.skipped;
            r.warnings.push_back(std::string(ROUTES_FILE) + " line " + Utils::toStr(entry.first) + ": " + e.what());
        }
    }
    return r;
}

LoadResult FileManager::loadOrders(DynArray<Order>& list) const {
    LoadResult r;
    std::vector<std::pair<int, std::string> > lines;
    r.found = readLines(ORDERS_FILE, lines);
    for (const auto& entry : lines) {
        try {
            Order o = Order::fromFileLine(entry.second);
            for (int i = 0; i < list.size(); ++i)
                if (list[i].getId() == o.getId()) throw ValidationException("duplicate ID " + Utils::toStr(o.getId()));
            list.add(o);
            ++r.loaded;
        } catch (const AppException& e) {
            ++r.skipped;
            r.warnings.push_back(std::string(ORDERS_FILE) + " line " + Utils::toStr(entry.first) + ": " + e.what());
        }
    }
    return r;
}
