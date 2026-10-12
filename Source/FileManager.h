// FileManager.h - All disk I/O for the system (persistence layer)
// Format: one record per line, fields separated by '|', lines starting with '#' are comments.
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Customer.h"
#include "DeliveryAgent.h"
#include "DynArray.h"
#include "Order.h"
#include "Route.h"

#include <string>
#include <vector>

struct LoadResult {
    bool found;                              // false when the file does not exist yet
    int  loaded;
    int  skipped;
    std::vector<std::string> warnings;       // one message per rejected line
    LoadResult() : found(false), loaded(0), skipped(0) {}
};

class FileManager {
private:
    std::string dataDir;

    std::string pathOf(const std::string& file) const;
    void writeLines(const std::string& file, const std::string& header,
                    const std::vector<std::string>& lines) const;
    bool readLines(const std::string& file, std::vector<std::pair<int, std::string> >& out) const;

public:
    static const char* CUSTOMERS_FILE;
    static const char* AGENTS_FILE;
    static const char* ROUTES_FILE;
    static const char* ORDERS_FILE;
    static const char* REPORT_FILE;

    explicit FileManager(const std::string& dataDir);
    static std::string locateDataDir();            // finds ./Data, ../Data or creates ./Data
    const std::string& getDataDir() const { return dataDir; }

    void saveCustomers(const DynArray<Customer>& list) const;
    void saveAgents(const DynArray<DeliveryAgent*>& list) const;
    void saveRoutes(const DynArray<Route>& list) const;
    void saveOrders(const DynArray<Order>& list) const;

    LoadResult loadCustomers(DynArray<Customer>& list) const;
    LoadResult loadAgents(DynArray<DeliveryAgent*>& list) const;
    LoadResult loadRoutes(DynArray<Route>& list) const;
    LoadResult loadOrders(DynArray<Order>& list) const;

    void writeText(const std::string& file, const std::string& content) const;
};

#endif
