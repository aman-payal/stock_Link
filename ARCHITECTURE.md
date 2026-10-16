# StockLink — file interaction and code-flow guide

## 1. Overall call flow

```text
Operating system
    |
    v
stocklink_oop.cpp                 C++ process entry; contains main() only
    |
    v
stocklink_c_main() in main.c      C menus, input prompts, login flow, app lifecycle
    |
    +---- auth_login() ------------> omkar_auth.c
    |       |
    |       +---- role title/permissions/greeting ---> omkar_auth_oop.cpp
    |                                                (virtual AccountView hierarchy)
    |
    +---- inventory menus --------> nikhil_inventory_oop.cpp
    |                                   |
    |                                   +--> nikhil_inventory.c
    |                                        hash table, product CRUD, search, save/load
    |
    +---- order menus ------------> aman_orders_oop.cpp
    |                                   |
    |                                   +--> aman_orders_suppliers.c
    |                                        FIFO queue, reservation, cancellation, undo stack
    |
    +---- relationship menus -----> rahul_relationships_oop.cpp
    |                                   |
    |                                   +--> rahul_relationships.c
    |                                        adjacency list, BFS, DFS
    |
    +---- catalogue/range/reports -> ayushman_algorithms_oop.cpp
                                        |
                                        +--> ayushman_algorithms.c
                                             sorts, binary bounds, summary calculations

Shared contract: stocklink.h
Safe input/string functions: utils.c
Persistent file: sl_inventory.dat (products only)
Generated report: stocklink_report.txt
```

`main.c` owns menu navigation and reads user input. Each member's C++ file owns that member's class definitions and offers small `extern "C"` bridge functions for calls from C. The bridge invokes the class. The class coordinates/presents the operation and calls the one C implementation for the data structure or algorithm. The C function is not repeated in C++.

## 2. Shared files

### `stocklink.h`
Defines the common record types (`Product`, `Inventory`, `Account`, `Order`, `OrderSystem`, `ProductGraph`) and declares all C functions and C++ bridge functions. It is a contract: the data layout and function signatures are declared here, but algorithms are implemented in the member `.c` files.

### `utils.c`
Provides bounded console input, integer/float validation, case-insensitive comparison, substring search, prefix matching and safe string copying. This keeps input helpers out of the five member modules.

### `main.c`
Initialises the application, seeds or loads products, prompts for login, dispatches to the proper role menu and saves products at shutdown. It does not define project classes. Menu actions call C functions when the operation is authentication/menu control, or call the member-specific C++ bridge when they should use a service class.

### `stocklink_oop.cpp`
Contains only `main()` and calls `stocklink_c_main()`. It does not contain member classes.

## 3. Member-by-member explanation

### Nikhil — Inventory

**C file: `nikhil_inventory.c`**

- Stores product nodes in a hash table with separate chaining by product ID.
- Keeps a growable array of pointers so the catalogue can be traversed without scanning all hash buckets.
- Owns add, update, remove, available-stock calculation, keyword search and prefix search.
- Prevents deleting products whose stock is reserved.
- Owns `inv_save` and `inv_load` for `sl_inventory.dat`.

**C++ file: `nikhil_inventory_oop.cpp`**

- Defines `InventoryService`, which encapsulates an `Inventory*` as private state.
- Provides product list, CRUD, keyword search and prefix-suggestion presentation.
- The service delegates all actual product changes/search scans and file persistence to C; it does not keep a second copy of product data.

**Demo:** manager lists/adds/updates/removes a product; customer searches for `audio` or prefix `wire`; add a duplicate ID/name; try removing a product with reserved stock. Restart the app to show catalogue persistence.

### Omkar — Authentication and roles

**C file: `omkar_auth.c`**

- Stores accounts in a bounded account table.
- Hashes passwords with a per-account salt and the educational FNV-1a digest.
- Checks credentials, tracks failed attempts, locks after three failures, registers customer/supplier accounts, changes passwords, unlocks accounts, and supports manager password reset.
- Uses one `account_index()` helper for username lookup, avoiding repeated search loops across login, unlock and password operations.
- Supplies account summaries and account-list records to the menu layer.

**C++ file: `omkar_auth_oop.cpp`**

- Defines abstract `AccountView` and concrete `CustomerAccount`, `ManagerAccount`, and `SupplierAccount` classes.
- Uses virtual functions for role title, permissions and greeting behaviour. `RoleService` selects a derived instance through an `AccountView` pointer, demonstrating runtime polymorphism. The main flow calls the greeting after successful login.
- `AccountAdminService` wraps account-creation and password-change calls; credential verification and hashing stay in C.
- The role objects are owned by `RoleService` rather than re-created inside each role-query method.

**Demo:** log in with all three demo roles; enter a wrong password three times; unlock/reset a customer or supplier as manager; create a customer account; change a user's password. Account records reset when the program restarts.

### Aman — Orders and supplier restock

**C file: `aman_orders_suppliers.c`**

- Stores pending orders in a circular FIFO queue.
- On placement, checks available stock and reserves the requested quantity.
- On processing, removes the oldest order, reduces stock and releases the reservation.
- Keeps processed orders on a LIFO undo stack. Undo restores stock/reservation and pushes the order back to the queue front.
- Supports owner-checked pending-order cancellation, a compact order summary, and `orders_validate`, which checks that reserved quantities match pending orders.

**C++ file: `aman_orders_oop.cpp`**

- `OrderDesk` coordinates order actions and prints human-readable status messages.
- `SupplierDesk` coordinates inventory replenishment by calling `inv_restock` in C.
- It does not implement another queue or stack.

**Demo:** customer places an order; manager views pending totals and processes it; undo it; place and cancel a second pending order; supplier replenishes a low-stock product.

### Rahul — Product relationships

**C file: `rahul_relationships.c`**

- Stores product links as an adjacency list with strength values from 1 to 10.
- BFS recommends products up to two edges from the selected product.
- DFS finds a connected product bundle.
- Graph-edge capacity is checked in both directions before adding an undirected link.

**C++ file: `rahul_relationships_oop.cpp`**

- `ProductRelationshipService` coordinates link creation and recommendation/bundle display.
- It translates product IDs to names using the C inventory API; BFS and DFS remain implemented once in C.

**Demo:** view recommendations for product `101`; use manager option 10 to link two existing products; use manager option 14 to show the connected bundle.

### Ayushman — Algorithms and reports

**C file: `ayushman_algorithms.c`**

- Implements insertion sort and merge sort over product-pointer arrays, ordered by price and then ID.
- Implements binary lower/upper bounds to retrieve a price interval from the sorted array.
- Calculates inventory summary values and identifies low-stock products.

**C++ file: `ayushman_algorithms_oop.cpp`**

- `CatalogService` chooses a C sort and displays the sorted products or price-range result.
- `ReportView` is an abstract base class. `SummaryReport` and `LowStockReport` override `render()`, demonstrating polymorphism while taking their calculated data from C.
- `ReportExporter` formats C-calculated summary/low-stock data into `stocklink_report.txt`.

**Demo:** compare insertion vs merge sort; query a price range; run the summary and low-stock reports; export the report.

## 4. Important boundaries and intentional omissions

- Prefix suggestions use a simple C linear scan; this edition has no Trie.
- Price ranges use binary lower/upper bounds on the sorted catalogue; this edition has no AVL tree.
- Only insertion sort and merge sort are included; quick sort, heap sort and benchmark are omitted.
- Replenishment directly increases inventory; the extended design's min-heap priority, supplier quotation routing and Dijkstra supplier path are omitted.
- Product data persists; orders, user accounts and product relationships are in-memory for this edition.
- C++ classes do not duplicate hash-table, queue, stack, graph traversal, sort, or binary-search implementations.

## 5. Build and debug flow

```sh
make clean
make
./stocklink
```

`make clean` removes generated objects, executable, `sl_inventory.dat`, and `stocklink_report.txt`. The app starts with seeded product/demo-account data when no saved inventory file exists.
