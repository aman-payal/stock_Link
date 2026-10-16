#include "stocklink.h"
#include <stdlib.h>

static int before_price(const Product *a, const Product *b)
{
    if (a->price < b->price) return 1;
    if (a->price > b->price) return 0;
    return a->id < b->id;
}

static void insertion_sort(Product **items, int count)
{
    int i;
    for (i = 1; i < count; ++i) {
        Product *current = items[i];
        int j = i - 1;
        while (j >= 0 && before_price(current, items[j])) {
            items[j + 1] = items[j];
            --j;
        }
        items[j + 1] = current;
    }
}

static void merge(Product **items, Product **temp, int left, int middle, int right)
{
    int i = left, j = middle, k = left;
    while (i < middle && j < right) {
        if (before_price(items[i], items[j])) temp[k++] = items[i++];
        else temp[k++] = items[j++];
    }
    while (i < middle) temp[k++] = items[i++];
    while (j < right) temp[k++] = items[j++];
    for (i = left; i < right; ++i) items[i] = temp[i];
}

static void merge_sort_rec(Product **items, Product **temp, int left, int right)
{
    int middle;
    if (right - left < 2) return;
    middle = left + (right - left) / 2;
    merge_sort_rec(items, temp, left, middle);
    merge_sort_rec(items, temp, middle, right);
    merge(items, temp, left, middle, right);
}

int algo_sorted_by_price(const Inventory *inv, Product **out, int capacity,
                         int algorithm)
{
    Product **temp;
    int count;
    if (!inv || !out || capacity <= 0) return 0;
    count = inv_collect(inv, out, capacity);
    if (count < 2) return count;
    if (algorithm == 1) {
        insertion_sort(out, count);
        return count;
    }
    temp = (Product **)malloc((size_t)count * sizeof(Product *));
    if (!temp) {
        insertion_sort(out, count);
        return count;
    }
    merge_sort_rec(out, temp, 0, count);
    free(temp);
    return count;
}

int algo_price_range(const Inventory *inv, float low, float high,
                     Product **out, int capacity)
{
    int count, left, right, first, last, i;
    if (low > high) return 0;
    count = algo_sorted_by_price(inv, out, capacity, 2);
    left = 0;
    right = count;
    while (left < right) {
        int middle = left + (right - left) / 2;
        if (out[middle]->price < low) left = middle + 1;
        else right = middle;
    }
    first = left;
    left = 0;
    right = count;
    while (left < right) {
        int middle = left + (right - left) / 2;
        if (out[middle]->price <= high) left = middle + 1;
        else right = middle;
    }
    last = left;
    for (i = first; i < last; ++i) out[i - first] = out[i];
    return last - first;
}

void algo_summary(const Inventory *inv, InventorySummary *summary)
{
    Product *items[MAX_PRODUCTS];
    int i, count;
    if (!summary) return;
    summary->product_count = 0;
    summary->units_in_stock = 0;
    summary->units_reserved = 0;
    summary->low_stock_count = 0;
    summary->inventory_value = 0.0;
    if (!inv) return;
    count = inv_collect(inv, items, MAX_PRODUCTS);
    summary->product_count = count;
    for (i = 0; i < count; ++i) {
        Product *p = items[i];
        summary->units_in_stock += p->stock;
        summary->units_reserved += p->reserved;
        summary->inventory_value += (double)p->price * p->stock;
        if (inv_available(p) <= 5) ++summary->low_stock_count;
    }
}

int algo_low_stock(const Inventory *inv, Product **out, int capacity)
{
    Product *items[MAX_PRODUCTS];
    int i, count, found = 0;
    if (!inv || !out || capacity <= 0) return 0;
    count = inv_collect(inv, items, MAX_PRODUCTS);
    for (i = 0; i < count && found < capacity; ++i)
        if (inv_available(items[i]) <= 5) out[found++] = items[i];
    return found;
}
