#include "stocklink.h"
#include <stdlib.h>
#include <string.h>

static unsigned int product_bucket(int id)
{
    unsigned int value = (unsigned int)(id < 0 ? -id : id);
    return value % HASH_BUCKETS;
}

void inv_init(Inventory *inv)
{
    if (!inv) return;
    memset(inv, 0, sizeof(*inv));
    inv->capacity = 8;
    inv->items = (Product **)calloc((size_t)inv->capacity, sizeof(Product *));
    if (!inv->items) inv->capacity = 0;
}

void inv_destroy(Inventory *inv)
{
    int i;
    if (!inv) return;
    for (i = 0; i < inv->count; ++i) free(inv->items[i]);
    free(inv->items);
    memset(inv, 0, sizeof(*inv));
}

int inv_available(const Product *product)
{
    if (!product) return 0;
    return product->stock - product->reserved;
}

Product *inv_find(const Inventory *inv, int id)
{
    Product *node;
    if (!inv) return NULL;
    node = inv->buckets[product_bucket(id)];
    while (node) {
        if (node->id == id) return node;
        node = node->hash_next;
    }
    return NULL;
}

static int reserve_array_slot(Inventory *inv)
{
    Product **grown;
    int next_capacity;
    if (inv->count < inv->capacity) return 1;
    next_capacity = inv->capacity == 0 ? 8 : inv->capacity * 2;
    if (next_capacity > MAX_PRODUCTS) next_capacity = MAX_PRODUCTS;
    if (next_capacity <= inv->capacity) return 0;
    grown = (Product **)realloc(inv->items, (size_t)next_capacity * sizeof(Product *));
    if (!grown) return 0;
    inv->items = grown;
    inv->capacity = next_capacity;
    return 1;
}

int inv_add(Inventory *inv, int id, const char *name, const char *category,
            float price, int stock)
{
    Product *product;
    unsigned int bucket;
    int i;
    if (!inv || id <= 0 || !name || name[0] == '\0' || price < 0 || stock < 0 ||
        inv->count >= MAX_PRODUCTS || inv_find(inv, id)) return 0;
    /* Names are unique too; a linear check keeps this reduced version easy to explain. */
    for (i = 0; i < inv->count; ++i)
        if (util_icmp(inv->items[i]->name, name) == 0) return 0;
    if (!reserve_array_slot(inv)) return 0;
    product = (Product *)calloc(1, sizeof(Product));
    if (!product) return 0;
    product->id = id;
    util_copy(product->name, name, NAME_LEN);
    util_copy(product->category, category, CATEGORY_LEN);
    product->price = price;
    product->stock = stock;
    bucket = product_bucket(id);
    product->hash_next = inv->buckets[bucket];
    inv->buckets[bucket] = product;
    inv->items[inv->count++] = product;
    return 1;
}

int inv_update(Inventory *inv, int id, float price, int stock)
{
    Product *product = inv_find(inv, id);
    if (!product || price < 0 || stock < product->reserved) return 0;
    product->price = price;
    product->stock = stock;
    return 1;
}

int inv_remove(Inventory *inv, int id)
{
    unsigned int bucket;
    Product *current, *previous = NULL;
    int i;
    if (!inv) return 0;
    bucket = product_bucket(id);
    current = inv->buckets[bucket];
    while (current && current->id != id) {
        previous = current;
        current = current->hash_next;
    }
    if (!current || current->reserved > 0) return 0;
    if (previous) previous->hash_next = current->hash_next;
    else inv->buckets[bucket] = current->hash_next;
    for (i = 0; i < inv->count; ++i) {
        if (inv->items[i] == current) {
            for (; i + 1 < inv->count; ++i) inv->items[i] = inv->items[i + 1];
            --inv->count;
            break;
        }
    }
    free(current);
    return 1;
}

int inv_restock(Inventory *inv, int id, int quantity)
{
    Product *product = inv_find(inv, id);
    if (!product || quantity <= 0 || product->stock > 100000 - quantity) return 0;
    product->stock += quantity;
    return 1;
}

int inv_collect(const Inventory *inv, Product **out, int capacity)
{
    int i, count;
    if (!inv || !out || capacity <= 0) return 0;
    count = inv->count < capacity ? inv->count : capacity;
    for (i = 0; i < count; ++i) out[i] = inv->items[i];
    return count;
}

int inv_search(const Inventory *inv, const char *query, Product **out, int capacity)
{
    int i, found = 0;
    if (!inv || !query || !out || capacity <= 0) return 0;
    for (i = 0; i < inv->count && found < capacity; ++i) {
        Product *p = inv->items[i];
        if (util_contains_ci(p->name, query) || util_contains_ci(p->category, query))
            out[found++] = p;
    }
    return found;
}

int inv_prefix(const Inventory *inv, const char *prefix, Product **out, int capacity)
{
    int i, found = 0;
    if (!inv || !prefix || !out || capacity <= 0) return 0;
    for (i = 0; i < inv->count && found < capacity; ++i)
        if (util_prefix_ci(inv->items[i]->name, prefix)) out[found++] = inv->items[i];
    return found;
}

void inv_seed_demo(Inventory *inv)
{
    inv_add(inv, 101, "Wireless Mouse", "Accessories", 599.0f, 35);
    inv_add(inv, 102, "Mechanical Keyboard", "Accessories", 2499.0f, 12);
    inv_add(inv, 103, "USB-C Cable", "Accessories", 199.0f, 50);
    inv_add(inv, 104, "Bluetooth Speaker", "Audio", 1799.0f, 8);
    inv_add(inv, 105, "Webcam", "Electronics", 3299.0f, 5);
    inv_add(inv, 106, "Laptop Stand", "Workspace", 899.0f, 16);
    inv_add(inv, 107, "Headphones", "Audio", 1499.0f, 18);
    inv_add(inv, 108, "Power Bank", "Electronics", 1299.0f, 4);
}

/* Save stable product data; active reservations are session-only because orders reset. */
int inv_save(const Inventory *inv, const char *filename)
{
    FILE *file;
    int i;
    if (!inv || !filename) return 0;
    file = fopen(filename, "w");
    if (!file) return 0;
    for (i = 0; i < inv->count; ++i) {
        const Product *p = inv->items[i];
        if (fprintf(file, "%d\t%s\t%s\t%.2f\t%d\n", p->id, p->name,
                    p->category, p->price, p->stock) < 0) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

int inv_load(Inventory *inv, const char *filename)
{
    FILE *file;
    char line[160];
    int loaded = 0;
    if (!inv || !filename || inv->count != 0) return 0;
    file = fopen(filename, "r");
    if (!file) return 0;
    while (fgets(line, sizeof(line), file)) {
        int id, stock;
        float price;
        char name[NAME_LEN], category[CATEGORY_LEN];
        if (sscanf(line, "%d\t%47[^\t]\t%31[^\t]\t%f\t%d",
                   &id, name, category, &price, &stock) == 5 &&
            inv_add(inv, id, name, category, price, stock)) ++loaded;
    }
    fclose(file);
    return loaded;
}
