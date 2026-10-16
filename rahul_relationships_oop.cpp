#include "stocklink.h"
#include <cstdio>

/* This class coordinates graph operations and translates IDs into product names. */
class ProductRelationshipService {
private:
    ProductGraph *graph_;
    const Inventory *inventory_;

    void printProduct(int id) const
    {
        Product *product = inv_find(inventory_, id);
        if (product)
            std::printf("  %d - %s (Rs. %.2f)\n", product->id, product->name, product->price);
        else
            std::printf("  %d - product no longer in inventory\n", id);
    }
public:
    ProductRelationshipService(ProductGraph *graph, const Inventory *inventory)
        : graph_(graph), inventory_(inventory) {}

    void link(int first, int second, int strength) const
    {
        if (first == second || !inv_find(inventory_, first) || !inv_find(inventory_, second)) {
            std::puts("Both product IDs must exist and must be different.");
            return;
        }
        if (rel_add_link(graph_, first, second, strength))
            std::puts("Relationship saved. Strength ranges from 1 to 10.");
        else
            std::puts("Relationship not saved; one product may have reached its link limit.");
    }

    void recommend(int productId) const
    {
        int ids[MAX_VERTICES];
        int count;
        Product *product = inv_find(inventory_, productId);
        if (!product) {
            std::puts("Product ID not found.");
            return;
        }
        count = rel_recommend(graph_, productId, 2, ids, MAX_VERTICES);
        std::printf("\nRelated products for %s:\n", product->name);
        for (int i = 0; i < count; ++i) printProduct(ids[i]);
        if (count == 0) std::puts("No related products yet.");
    }

    void bundle(int productId) const
    {
        int ids[MAX_VERTICES];
        int count;
        Product *product = inv_find(inventory_, productId);
        if (!product) {
            std::puts("Product ID not found.");
            return;
        }
        count = rel_bundle(graph_, productId, ids, MAX_VERTICES);
        std::printf("\nConnected product bundle for %s (DFS):\n", product->name);
        printProduct(productId);
        for (int i = 0; i < count; ++i) printProduct(ids[i]);
        if (count == 0) std::puts("No other products in this group yet.");
    }
};

extern "C" void oop_relationship_link(ProductGraph *graph, const Inventory *inv,
                                      int product_a, int product_b, int strength)
{
    ProductRelationshipService service(graph, inv);
    service.link(product_a, product_b, strength);
}

extern "C" void oop_relationship_recommend(const ProductGraph *graph, const Inventory *inv,
                                           int product_id)
{
    ProductRelationshipService service(const_cast<ProductGraph *>(graph), inv);
    service.recommend(product_id);
}

extern "C" void oop_relationship_bundle(const ProductGraph *graph, const Inventory *inv,
                                       int product_id)
{
    ProductRelationshipService service(const_cast<ProductGraph *>(graph), inv);
    service.bundle(product_id);
}
