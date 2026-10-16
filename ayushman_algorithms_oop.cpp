#include "stocklink.h"
#include <cstdio>

static void show_products(Product **items, int count)
{
    std::printf("\n%-5s %-25s %-15s %-12s %6s %6s\n",
                "ID", "Product", "Category", "Price", "Stock", "Held");
    std::puts("--------------------------------------------------------------------------");
    for (int i = 0; i < count; ++i) {
        Product *p = items[i];
        std::printf("%-5d %-25s %-15s Rs. %-8.2f %6d %6d\n", p->id, p->name,
                    p->category, p->price, inv_available(p), p->reserved);
    }
    if (count == 0) std::puts("No products match this view.");
}

/* The algorithm implementations remain in C; this class selects and presents them. */
class CatalogService {
private:
    const Inventory *inventory_;
public:
    explicit CatalogService(const Inventory *inventory) : inventory_(inventory) {}

    void showSorted(int algorithm) const
    {
        Product *items[MAX_PRODUCTS];
        int count = algo_sorted_by_price(inventory_, items, MAX_PRODUCTS, algorithm);
        show_products(items, count);
        std::printf("\nSort implementation: %s\n",
                    algorithm == 1 ? "insertion sort, O(n^2)" : "merge sort, O(n log n)");
    }

    void showPriceRange(float low, float high) const
    {
        Product *items[MAX_PRODUCTS];
        int count = algo_price_range(inventory_, low, high, items, MAX_PRODUCTS);
        std::printf("\nProducts priced from Rs. %.2f to Rs. %.2f:\n", low, high);
        show_products(items, count);
        std::puts("The C module uses binary bounds on a price-sorted array.");
    }
};

class ReportView {
protected:
    const Inventory *inventory_;
public:
    explicit ReportView(const Inventory *inventory) : inventory_(inventory) {}
    virtual ~ReportView() {}
    virtual void render() const = 0;
};

class SummaryReport : public ReportView {
public:
    explicit SummaryReport(const Inventory *inventory) : ReportView(inventory) {}
    void render() const override
    {
        InventorySummary summary;
        algo_summary(inventory_, &summary);
        std::puts("\n=== Inventory Summary ===");
        std::printf("Distinct products : %d\n", summary.product_count);
        std::printf("Units in stock    : %d\n", summary.units_in_stock);
        std::printf("Units reserved    : %d\n", summary.units_reserved);
        std::printf("Low-stock products: %d (5 or fewer available)\n", summary.low_stock_count);
        std::printf("Stock value       : Rs. %.2f\n", summary.inventory_value);
    }
};

class LowStockReport : public ReportView {
public:
    explicit LowStockReport(const Inventory *inventory) : ReportView(inventory) {}
    void render() const override
    {
        Product *items[MAX_PRODUCTS];
        int count = algo_low_stock(inventory_, items, MAX_PRODUCTS);
        std::puts("\n=== Low Stock Report ===");
        show_products(items, count);
        std::puts("Restock candidates have five or fewer units available.");
    }
};

extern "C" void oop_catalog_sorted(const Inventory *inv, int algorithm)
{
    CatalogService catalog(inv);
    catalog.showSorted(algorithm == 1 ? 1 : 2);
}

extern "C" void oop_price_range(const Inventory *inv, float low, float high)
{
    CatalogService catalog(inv);
    catalog.showPriceRange(low, high);
}

extern "C" void oop_report_show(const Inventory *inv, int report_type)
{
    SummaryReport summary(inv);
    LowStockReport lowStock(inv);
    const ReportView *report = report_type == 1 ?
        static_cast<const ReportView *>(&lowStock) :
        static_cast<const ReportView *>(&summary);
    report->render();
}

/* Export uses C-calculated figures; C++ owns only the report's text presentation. */
class ReportExporter {
private:
    const Inventory *inventory_;
public:
    explicit ReportExporter(const Inventory *inventory) : inventory_(inventory) {}

    int writeText(const char *filename) const
    {
        FILE *file = std::fopen(filename, "w");
        InventorySummary summary;
        Product *items[MAX_PRODUCTS];
        int count, i;
        if (!file) {
            std::puts("Could not create the report file.");
            return 0;
        }
        algo_summary(inventory_, &summary);
        count = algo_low_stock(inventory_, items, MAX_PRODUCTS);
        std::fprintf(file, "StockLink Inventory Report\n==========================\n");
        std::fprintf(file, "Products: %d\nUnits in stock: %d\nUnits reserved: %d\n",
                     summary.product_count, summary.units_in_stock, summary.units_reserved);
        std::fprintf(file, "Low-stock products: %d\nInventory value: Rs. %.2f\n\n",
                     summary.low_stock_count, summary.inventory_value);
        std::fprintf(file, "LOW STOCK DETAILS\nID\tName\tAvailable\tPrice\n");
        for (i = 0; i < count; ++i)
            std::fprintf(file, "%d\t%s\t%d\t%.2f\n", items[i]->id, items[i]->name,
                         inv_available(items[i]), items[i]->price);
        if (std::fclose(file) != 0) {
            std::puts("The report could not be fully written.");
            return 0;
        }
        std::puts("Report exported to stocklink_report.txt.");
        return 1;
    }
};

extern "C" void oop_report_export(const Inventory *inv)
{
    ReportExporter exporter(inv);
    exporter.writeText("stocklink_report.txt");
}
