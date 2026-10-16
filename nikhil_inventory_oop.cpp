#include "stocklink.h"
#include <cstdio>

/* OOP responsibility: provide a controlled service interface around C inventory data. */
class InventoryService {
private:
    Inventory *inventory_;

    void printProduct(const Product *p) const
    {
        if (!p) return;
        std::printf("%-5d %-25s %-15s Rs. %-8.2f %5d %5d\n",
                    p->id, p->name, p->category, p->price,
                    inv_available(p), p->reserved);
    }

    void printHeader() const
    {
        std::printf("\n%-5s %-25s %-15s %-12s %5s %5s\n",
                    "ID", "Product", "Category", "Price", "Avail.", "Held");
        std::printf("--------------------------------------------------------------------------\n");
    }

public:
    explicit InventoryService(Inventory *inventory) : inventory_(inventory) {}

    void list() const
    {
        Product *items[MAX_PRODUCTS];
        int count = inv_collect(inventory_, items, MAX_PRODUCTS);
        printHeader();
        for (int i = 0; i < count; ++i) printProduct(items[i]);
        if (count == 0) std::puts("No products are currently available.");
    }

    int load(const char *filename) const
    {
        int count = inv_load(inventory_, filename);
        if (count > 0) std::printf("Loaded %d saved product(s).\n", count);
        return count;
    }

    int save(const char *filename) const
    {
        if (inv_save(inventory_, filename)) {
            std::puts("Product inventory saved.");
            return 1;
        }
        std::puts("Warning: product inventory could not be saved.");
        return 0;
    }

    void add(int id, const char *name, const char *category, float price, int stock) const
    {
        if (inv_add(inventory_, id, name, category, price, stock))
            std::puts("Product added successfully.");
        else
            std::puts("Add failed: check the ID, name, price, stock and capacity.");
    }

    void update(int id, float price, int stock) const
    {
        if (inv_update(inventory_, id, price, stock))
            std::puts("Product updated successfully.");
        else
            std::puts("Update failed: product missing or stock is below reserved quantity.");
    }

    void remove(int id) const
    {
        if (inv_remove(inventory_, id)) std::puts("Product removed.");
        else std::puts("Remove failed: product missing or currently has reserved stock.");
    }

    void search(const char *query) const
    {
        Product *matches[MAX_PRODUCTS];
        int count = inv_search(inventory_, query, matches, MAX_PRODUCTS);
        printHeader();
        for (int i = 0; i < count; ++i) printProduct(matches[i]);
        if (count == 0) std::puts("No matching products found.");
    }

    void prefix(const char *text) const
    {
        Product *matches[MAX_PRODUCTS];
        int count = inv_prefix(inventory_, text, matches, MAX_PRODUCTS);
        std::printf("\nProduct-name suggestions for '%s':\n", text);
        for (int i = 0; i < count; ++i)
            std::printf("  %d - %s (Rs. %.2f)\n", matches[i]->id,
                        matches[i]->name, matches[i]->price);
        if (count == 0) std::puts("No suggestions found.");
    }
};

extern "C" void oop_inventory_list(const Inventory *inv)
{
    InventoryService service(const_cast<Inventory *>(inv));
    service.list();
}

extern "C" int oop_inventory_load(Inventory *inv, const char *filename)
{
    InventoryService service(inv);
    return service.load(filename);
}

extern "C" int oop_inventory_save(const Inventory *inv, const char *filename)
{
    InventoryService service(const_cast<Inventory *>(inv));
    return service.save(filename);
}

extern "C" void oop_inventory_add(Inventory *inv, int id, const char *name,
                                  const char *category, float price, int stock)
{
    InventoryService service(inv);
    service.add(id, name, category, price, stock);
}

extern "C" void oop_inventory_update(Inventory *inv, int id, float price, int stock)
{
    InventoryService service(inv);
    service.update(id, price, stock);
}

extern "C" void oop_inventory_remove(Inventory *inv, int id)
{
    InventoryService service(inv);
    service.remove(id);
}

extern "C" void oop_inventory_search(const Inventory *inv, const char *query)
{
    InventoryService service(const_cast<Inventory *>(inv));
    service.search(query);
}

extern "C" void oop_inventory_prefix(const Inventory *inv, const char *prefix)
{
    InventoryService service(const_cast<Inventory *>(inv));
    service.prefix(prefix);
}
