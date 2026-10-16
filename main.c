#include "stocklink.h"
#include <stdio.h>
#include <string.h>

static void heading(const char *title)
{
    printf("\n============================================================\n");
    printf("  %s\n", title);
    printf("============================================================\n");
}

static void show_main_menu(void)
{
    heading("StockLink | Inventory and Order Management");
    puts("1. Login");
    puts("0. Exit");
}

static void customer_menu(StockLinkApp *app, const char *username)
{
    int choice;
    do {
        heading("Customer Dashboard");
        puts("1. Browse products (insertion or merge sort)");
        puts("2. Search products by name/category");
        puts("3. Product-name prefix suggestions");
        puts("4. Related-product recommendations");
        puts("5. Search products by price range");
        puts("6. Place an order");
        puts("7. View my pending orders");
        puts("8. Change my password");
        puts("9. Cancel one of my pending orders");
        puts("0. Logout");
        choice = util_read_int("Choice: ", 0, 9);
        switch (choice) {
        case 1: {
            int algorithm = util_read_int("Sort: 1) insertion  2) merge: ", 1, 2);
            oop_catalog_sorted(&app->inventory, algorithm);
            break;
        }
        case 2: {
            char query[NAME_LEN];
            util_read_line("Keyword: ", query, (int)sizeof(query));
            oop_inventory_search(&app->inventory, query);
            break;
        }
        case 3: {
            char prefix[NAME_LEN];
            util_read_line("Product name starts with: ", prefix, (int)sizeof(prefix));
            oop_inventory_prefix(&app->inventory, prefix);
            break;
        }
        case 4: {
            int id = util_read_int("Product ID: ", 1, 999999);
            oop_relationship_recommend(&app->relationships, &app->inventory, id);
            break;
        }
        case 5: {
            float low = util_read_float("Minimum price: ", 0.0f, 1000000.0f);
            float high = util_read_float("Maximum price: ", low, 1000000.0f);
            oop_price_range(&app->inventory, low, high);
            break;
        }
        case 6: {
            int id = util_read_int("Product ID: ", 1, 999999);
            int qty = util_read_int("Quantity: ", 1, 1000);
            oop_order_place(&app->orders, &app->inventory, username, id, qty);
            break;
        }
        case 7:
            oop_order_list(&app->orders, username);
            break;
        case 8: {
            char oldPassword[PASSWORD_LEN], newPassword[PASSWORD_LEN];
            util_read_line("Current password: ", oldPassword, (int)sizeof(oldPassword));
            util_read_line("New password (6+ characters): ", newPassword, (int)sizeof(newPassword));
            oop_auth_change_password(&app->users, username, oldPassword, newPassword);
            break;
        }
        case 9: {
            int orderId = util_read_int("Pending order ID to cancel: ", 1, 999999);
            oop_order_cancel(&app->orders, &app->inventory, orderId, username, 0);
            break;
        }
        default: break;
        }
        if (choice != 0) util_pause();
    } while (choice != 0);
}

static void manager_reports(StockLinkApp *app)
{
    int choice;
    do {
        heading("Manager | Reports and Search");
        puts("1. Inventory summary");
        puts("2. Low-stock report");
        puts("3. Price-range report");
        puts("4. Export summary and low-stock report to text file");
        puts("0. Back");
        choice = util_read_int("Choice: ", 0, 4);
        if (choice == 1) oop_report_show(&app->inventory, 0);
        else if (choice == 2) oop_report_show(&app->inventory, 1);
        else if (choice == 3) {
            float low = util_read_float("Minimum price: ", 0.0f, 1000000.0f);
            float high = util_read_float("Maximum price: ", low, 1000000.0f);
            oop_price_range(&app->inventory, low, high);
        } else if (choice == 4) {
            oop_report_export(&app->inventory);
        }
        if (choice != 0) util_pause();
    } while (choice != 0);
}

static void manager_menu(StockLinkApp *app)
{
    int choice;
    do {
        heading("Manager Dashboard");
        puts("1. View inventory");
        puts("2. Add product");
        puts("3. Update product price and stock");
        puts("4. Remove product");
        puts("5. View pending orders");
        puts("6. Process next order (FIFO)");
        puts("7. Undo the last processed order");
        puts("8. Reports and price-range search");
        puts("9. Unlock an account");
        puts("10. Link two related products");
        puts("11. View catalog sorted by price");
        puts("12. Create customer or supplier account");
        puts("13. Cancel a pending order (manager override)");
        puts("14. View connected product bundle (DFS)");
        puts("15. List accounts and lock status");
        puts("16. Reset customer/supplier password and unlock");
        puts("17. Account summary");
        puts("0. Logout");
        choice = util_read_int("Choice: ", 0, 17);
        switch (choice) {
        case 1:
            oop_inventory_list(&app->inventory);
            break;
        case 2: {
            int id = util_read_int("New product ID: ", 1, 999999);
            char name[NAME_LEN], category[CATEGORY_LEN];
            float price;
            int stock;
            util_read_line("Name: ", name, (int)sizeof(name));
            util_read_line("Category: ", category, (int)sizeof(category));
            price = util_read_float("Price: ", 0.0f, 1000000.0f);
            stock = util_read_int("Opening stock: ", 0, 100000);
            oop_inventory_add(&app->inventory, id, name, category, price, stock);
            break;
        }
        case 3: {
            int id = util_read_int("Product ID: ", 1, 999999);
            float price = util_read_float("New price: ", 0.0f, 1000000.0f);
            int stock = util_read_int("New total stock (cannot be below reserved): ", 0, 100000);
            oop_inventory_update(&app->inventory, id, price, stock);
            break;
        }
        case 4: {
            int id = util_read_int("Product ID to remove: ", 1, 999999);
            oop_inventory_remove(&app->inventory, id);
            break;
        }
        case 5: {
            OrderSummary summary;
            orders_get_summary(&app->orders, &summary);
            printf("Pending orders: %d | reserved units: %d | undoable processed orders: %d\n",
                   summary.pending_orders, summary.pending_units, summary.undoable_orders);
            oop_order_list(&app->orders, NULL);
            break;
        }
        case 6:
            oop_order_process(&app->orders, &app->inventory);
            break;
        case 7:
            oop_order_undo(&app->orders, &app->inventory);
            break;
        case 8:
            manager_reports(app);
            break;
        case 9: {
            char username[USER_LEN];
            util_read_line("Username to unlock: ", username, (int)sizeof(username));
            if (auth_unlock(&app->users, username)) printf("Account '%s' unlocked.\n", username);
            else puts("Username not found.");
            break;
        }
        case 10: {
            int first = util_read_int("First product ID: ", 1, 999999);
            int second = util_read_int("Second product ID: ", 1, 999999);
            int strength = util_read_int("Relationship strength (1-10): ", 1, 10);
            oop_relationship_link(&app->relationships, &app->inventory,
                                  first, second, strength);
            break;
        }
        case 11: {
            int algorithm = util_read_int("Sort: 1) insertion  2) merge: ", 1, 2);
            oop_catalog_sorted(&app->inventory, algorithm);
            break;
        }
        case 12: {
            char username[USER_LEN], password[PASSWORD_LEN];
            int roleChoice;
            util_read_line("New username (letters, digits, underscore): ", username,
                           (int)sizeof(username));
            util_read_line("Temporary password (6+ characters): ", password,
                           (int)sizeof(password));
            roleChoice = util_read_int("Role: 1) customer  3) supplier: ", 1, 3);
            if (roleChoice != ROLE_CUSTOMER && roleChoice != ROLE_SUPPLIER)
                puts("Only customer and supplier accounts may be created here.");
            else
                oop_auth_create_account(&app->users, username, password, roleChoice);
            break;
        }
        case 13: {
            int orderId = util_read_int("Pending order ID to cancel: ", 1, 999999);
            oop_order_cancel(&app->orders, &app->inventory, orderId, NULL, 1);
            break;
        }
        case 14: {
            int id = util_read_int("Product ID: ", 1, 999999);
            oop_relationship_bundle(&app->relationships, &app->inventory, id);
            break;
        }
        case 15: {
            AccountInfo accounts[MAX_USERS];
            int count = auth_collect_accounts(&app->users, accounts, MAX_USERS);
            puts("\nUsername                       Role       Locked   Failed attempts");
            for (int i = 0; i < count; ++i)
                printf("%-30s %-10s %-8s %d\n", accounts[i].username,
                       oop_role_name(accounts[i].role), accounts[i].locked ? "YES" : "NO",
                       accounts[i].failed_attempts);
            break;
        }
        case 16: {
            char username[USER_LEN], password[PASSWORD_LEN];
            util_read_line("Customer/supplier username: ", username, (int)sizeof(username));
            util_read_line("New temporary password (letters and digits, 6+ chars): ",
                           password, (int)sizeof(password));
            if (auth_reset_password(&app->users, username, password))
                puts("Password reset and lockout cleared.");
            else
                puts("Reset failed: account must be a customer/supplier and password must contain letters and digits.");
            break;
        }
        case 17: {
            AuthSummary summary;
            auth_get_summary(&app->users, &summary);
            printf("Accounts: %d total | %d customers | %d managers | %d suppliers | %d locked\n",
                   summary.total, summary.customers, summary.managers, summary.suppliers,
                   summary.locked);
            break;
        }
        default: break;
        }
        if (choice != 0) util_pause();
    } while (choice != 0);
}

static void supplier_menu(StockLinkApp *app)
{
    int choice;
    do {
        heading("Supplier Dashboard");
        puts("1. View low-stock products");
        puts("2. View current inventory");
        puts("3. Replenish a product");
        puts("0. Logout");
        choice = util_read_int("Choice: ", 0, 3);
        if (choice == 1) oop_report_show(&app->inventory, 1);
        else if (choice == 2) oop_inventory_list(&app->inventory);
        else if (choice == 3) {
            int id = util_read_int("Product ID: ", 1, 999999);
            int qty = util_read_int("Quantity to deliver: ", 1, 100000);
            oop_supplier_restock(&app->inventory, id, qty);
        }
        if (choice != 0) util_pause();
    } while (choice != 0);
}

static void handle_login(StockLinkApp *app)
{
    char username[USER_LEN], password[PASSWORD_LEN];
    Role role = ROLE_NONE;
    AuthStatus result;
    util_read_line("Username: ", username, (int)sizeof(username));
    util_read_line("Password: ", password, (int)sizeof(password));
    result = auth_login(&app->users, username, password, &role);
    if (result == AUTH_LOCKED) {
        puts("Account locked after three failed attempts. Ask the manager to unlock it.");
        return;
    }
    if (result == AUTH_BAD_PASSWORD) {
        puts("Incorrect password. Two more failed attempts will lock the account.");
        return;
    }
    if (result != AUTH_OK) {
        puts("Unknown username.");
        return;
    }
    printf("\nLogin successful. Welcome, %s (%s).\n", username, oop_role_name(role));
    oop_role_greet(role);
    if (oop_role_can_manage(role)) manager_menu(app);
    else if (oop_role_can_supply(role)) supplier_menu(app);
    else customer_menu(app, username);
}

int stocklink_c_main(void)
{
    StockLinkApp app;
    int choice;
    memset(&app, 0, sizeof(app));
    inv_init(&app.inventory);
    if (!oop_inventory_load(&app.inventory, "sl_inventory.dat")) {
        inv_destroy(&app.inventory);
        inv_init(&app.inventory);
        inv_seed_demo(&app.inventory);
    }
    auth_init(&app.users);
    orders_init(&app.orders);
    rel_init(&app.relationships);
    rel_seed_demo(&app.relationships);
    if (app.inventory.count == 0) {
        fputs("Unable to initialise the inventory.\n", stderr);
        inv_destroy(&app.inventory);
        return 1;
    }
    do {
        show_main_menu();
        choice = util_read_int("Choice: ", 0, 1);
        if (choice == 1) handle_login(&app);
    } while (choice != 0);
    oop_inventory_save(&app.inventory, "sl_inventory.dat");
    inv_destroy(&app.inventory);
    puts("StockLink closed. Product data is saved; demo accounts and orders reset each run.");
    return 0;
}
