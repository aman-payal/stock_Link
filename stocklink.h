#ifndef STOCKLINK_H
#define STOCKLINK_H

/* Shared C/C++ contract for the reduced, explainable StockLink project. */
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PRODUCTS 100
#define HASH_BUCKETS 101
#define MAX_USERS 10
#define MAX_ORDERS 100
#define MAX_UNDO 50
#define MAX_VERTICES 100
#define MAX_DEGREE 12
#define NAME_LEN 48
#define CATEGORY_LEN 32
#define USER_LEN 32
#define PASSWORD_LEN 64

typedef enum {
    ROLE_NONE = 0,
    ROLE_CUSTOMER = 1,
    ROLE_MANAGER = 2,
    ROLE_SUPPLIER = 3
} Role;

typedef struct Product {
    int id;
    char name[NAME_LEN];
    char category[CATEGORY_LEN];
    float price;
    int stock;
    int reserved;
    struct Product *hash_next;
} Product;

typedef struct {
    Product **items;        /* Growable array used for traversal and display. */
    int count;
    int capacity;
    Product *buckets[HASH_BUCKETS]; /* Separate-chaining hash table by ID. */
} Inventory;

typedef struct {
    char username[USER_LEN];
    char salt[16];
    unsigned long long password_hash;
    Role role;
    int failed_attempts;
    int locked;
} Account;

typedef struct {
    Account accounts[MAX_USERS];
    int count;
} AuthStore;

typedef struct {
    char username[USER_LEN];
    Role role;
    int locked;
    int failed_attempts;
} AccountInfo;

typedef struct {
    int total;
    int customers;
    int managers;
    int suppliers;
    int locked;
} AuthSummary;

typedef enum {
    AUTH_OK = 0,
    AUTH_BAD_PASSWORD,
    AUTH_LOCKED,
    AUTH_NOT_FOUND
} AuthStatus;

typedef struct {
    int id;
    char owner[USER_LEN];
    int product_id;
    int quantity;
} Order;

typedef struct {
    Order queue[MAX_ORDERS];
    int front;
    int count;
    int next_id;
    Order undo_stack[MAX_UNDO];
    int undo_top;
} OrderSystem;

typedef struct {
    int pending_orders;
    int pending_units;
    int undoable_orders;
} OrderSummary;

typedef struct {
    int product_id;
    int strength;
} GraphEdge;

typedef struct {
    int product_ids[MAX_VERTICES];
    int degree[MAX_VERTICES];
    GraphEdge edges[MAX_VERTICES][MAX_DEGREE];
    int vertex_count;
} ProductGraph;

typedef struct {
    int product_count;
    int units_in_stock;
    int units_reserved;
    int low_stock_count;
    double inventory_value;
} InventorySummary;

typedef struct {
    Inventory inventory;
    AuthStore users;
    OrderSystem orders;
    ProductGraph relationships;
} StockLinkApp;

/* Utilities: small, safe console-input and string helpers in utils.c. */
void util_copy(char *dest, const char *source, int capacity);
void util_read_line(const char *prompt, char *buffer, int capacity);
int util_read_int(const char *prompt, int low, int high);
float util_read_float(const char *prompt, float low, float high);
int util_icmp(const char *a, const char *b);
int util_contains_ci(const char *text, const char *query);
int util_prefix_ci(const char *text, const char *prefix);
void util_pause(void);

/* Nikhil: C inventory store and its data operations. */
void inv_init(Inventory *inv);
void inv_destroy(Inventory *inv);
void inv_seed_demo(Inventory *inv);
Product *inv_find(const Inventory *inv, int id);
int inv_add(Inventory *inv, int id, const char *name, const char *category,
            float price, int stock);
int inv_update(Inventory *inv, int id, float price, int stock);
int inv_remove(Inventory *inv, int id);
int inv_restock(Inventory *inv, int id, int quantity);
int inv_available(const Product *product);
int inv_collect(const Inventory *inv, Product **out, int capacity);
int inv_search(const Inventory *inv, const char *query, Product **out, int capacity);
int inv_prefix(const Inventory *inv, const char *prefix, Product **out, int capacity);
int inv_save(const Inventory *inv, const char *filename);
int inv_load(Inventory *inv, const char *filename);

/* Omkar: authentication is implemented only in C. */
void auth_init(AuthStore *store);
AuthStatus auth_login(AuthStore *store, const char *username,
                      const char *password, Role *role_out);
int auth_unlock(AuthStore *store, const char *username);
int auth_register(AuthStore *store, const char *username, const char *password, Role role);
int auth_change_password(AuthStore *store, const char *username,
                         const char *old_password, const char *new_password);
int auth_reset_password(AuthStore *store, const char *username, const char *new_password);
int auth_collect_accounts(const AuthStore *store, AccountInfo *out, int capacity);
void auth_get_summary(const AuthStore *store, AuthSummary *summary);
int auth_locked(const AuthStore *store, const char *username);

/* Aman: FIFO order queue and LIFO undo history. */
void orders_init(OrderSystem *orders);
int orders_place(OrderSystem *orders, Inventory *inv, const char *owner,
                 int product_id, int quantity, int *order_id_out);
int orders_pending(const OrderSystem *orders, Order *out, int capacity);
int orders_process_next(OrderSystem *orders, Inventory *inv, Order *out);
int orders_undo_last(OrderSystem *orders, Inventory *inv, Order *out);
int orders_cancel(OrderSystem *orders, Inventory *inv, int order_id,
                  const char *owner, int allow_any, Order *out);
void orders_get_summary(const OrderSystem *orders, OrderSummary *summary);
int orders_validate(const OrderSystem *orders, const Inventory *inv);

/* Rahul: adjacency-list product relationship graph and BFS recommendations. */
void rel_init(ProductGraph *graph);
int rel_add_link(ProductGraph *graph, int product_a, int product_b, int strength);
int rel_recommend(const ProductGraph *graph, int start_product_id,
                  int max_depth, int out_ids[], int capacity);
int rel_bundle(const ProductGraph *graph, int start_product_id,
               int out_ids[], int capacity);
void rel_seed_demo(ProductGraph *graph);

/* Ayushman: hand-written sorting, binary price-range search and summaries. */
int algo_sorted_by_price(const Inventory *inv, Product **out, int capacity,
                         int algorithm);
int algo_price_range(const Inventory *inv, float low, float high,
                     Product **out, int capacity);
void algo_summary(const Inventory *inv, InventorySummary *summary);
int algo_low_stock(const Inventory *inv, Product **out, int capacity);

/* C++ bridges. These functions are implemented in the member-specific .cpp files. */
void oop_inventory_list(const Inventory *inv);
int oop_inventory_load(Inventory *inv, const char *filename);
int oop_inventory_save(const Inventory *inv, const char *filename);
void oop_inventory_add(Inventory *inv, int id, const char *name,
                       const char *category, float price, int stock);
void oop_inventory_update(Inventory *inv, int id, float price, int stock);
void oop_inventory_remove(Inventory *inv, int id);
void oop_inventory_search(const Inventory *inv, const char *query);
void oop_inventory_prefix(const Inventory *inv, const char *prefix);

const char *oop_role_name(int role);
int oop_auth_create_account(AuthStore *store, const char *username,
                            const char *password, int role);
int oop_auth_change_password(AuthStore *store, const char *username,
                             const char *old_password, const char *new_password);
int oop_role_can_manage(int role);
int oop_role_can_supply(int role);
void oop_role_greet(int role);

void oop_order_place(OrderSystem *orders, Inventory *inv, const char *owner,
                     int product_id, int quantity);
void oop_order_list(const OrderSystem *orders, const char *owner_filter);
void oop_order_process(OrderSystem *orders, Inventory *inv);
void oop_order_undo(OrderSystem *orders, Inventory *inv);
void oop_order_cancel(OrderSystem *orders, Inventory *inv, int order_id,
                      const char *owner, int allow_any);
void oop_supplier_restock(Inventory *inv, int product_id, int quantity);

void oop_relationship_link(ProductGraph *graph, const Inventory *inv,
                           int product_a, int product_b, int strength);
void oop_relationship_recommend(const ProductGraph *graph, const Inventory *inv,
                                int product_id);
void oop_relationship_bundle(const ProductGraph *graph, const Inventory *inv,
                             int product_id);

void oop_catalog_sorted(const Inventory *inv, int algorithm);
void oop_price_range(const Inventory *inv, float low, float high);
void oop_report_show(const Inventory *inv, int report_type);
void oop_report_export(const Inventory *inv);

int stocklink_c_main(void);

#ifdef __cplusplus
}
#endif
#endif
