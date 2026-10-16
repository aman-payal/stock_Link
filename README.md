# StockLink — Optimal 60% Submission

StockLink is a terminal-only inventory and order-management application built with C and C++17. This edition intentionally retains the most demonstrable core features rather than carrying every advanced feature from the full design.

## Build and run

Requirements: GCC and G++ with C11/C++17 support.

```sh
make clean
make
./stocklink
```

On Windows with MinGW, use `mingw32-make` if installed, then run `stocklink.exe`.

The project uses **no STL containers**. Arrays, the inventory hash table, order queue, undo stack, adjacency lists, and sorting/search routines are implemented manually. C++ uses ordinary classes, inheritance and virtual functions; it does not use `std::vector`, `std::map`, `std::unordered_map`, `std::list`, `std::queue`, `std::stack`, or STL trees.

## Demo accounts

| Role | Username | Password |
|---|---|---|
| Manager | `admin` | `admin123` |
| Customer | `demo` | `demo123` |
| Supplier | `freshmart` | `fresh123` |

New customer/supplier accounts and lock status exist for the current run only. The product catalogue saves to `sl_inventory.dat`; orders, account records and graph links reset on restart. `make clean` removes the saved catalogue and generated report.

## What's included

- **Inventory:** chained hash table by product ID, growable pointer array, unique product IDs/names, product CRUD, keyword search, prefix suggestions and product-catalogue save/load.
- **Authentication:** salted educational password digest, three-failure lockout, role-specific access, manager unlock/reset, customer/supplier account creation and password change.
- **Orders:** FIFO pending-order queue, reservation on placement, stock deduction on processing, cancellation of a pending order and LIFO undo of the last processed order. C checks the stock-reservation invariant after state changes.
- **Product relationships:** adjacency-list graph, BFS recommendations up to two links away, manager-created links and DFS connected-product bundles.
- **Algorithms/reports:** hand-written insertion sort and merge sort, binary bounds for price-range search, inventory/low-stock reports and text-file export.
- **OOP layer:** private service state, role inheritance, runtime polymorphism, composition and a virtual report interface. C++ service classes call the corresponding C API rather than reimplementing its algorithm.

## Deliberately excluded from this 60% edition

The full design's Trie-based autocomplete, AVL price tree, quick sort, heap sort, sorting benchmark, low-stock min-heap prioritisation, Dijkstra supplier routing, supplier quote/dispatch pipeline, order-learning graph updates, and persistence for orders/users/relationships are omitted. The retained prefix suggestion is a simple linear scan; it is **not** a Trie implementation.

These exclusions keep the project explainable and runnable while preserving all five members' responsibilities and a meaningful C/C++ split.

## Team files

| Member | C file — algorithm/data ownership | C++ file — OOP ownership |
|---|---|---|
| Nikhil | `nikhil_inventory.c` | `nikhil_inventory_oop.cpp` |
| Omkar | `omkar_auth.c` | `omkar_auth_oop.cpp` |
| Aman | `aman_orders_suppliers.c` | `aman_orders_oop.cpp` |
| Rahul | `rahul_relationships.c` | `rahul_relationships_oop.cpp` |
| Ayushman | `ayushman_algorithms.c` | `ayushman_algorithms_oop.cpp` |

Shared files: `stocklink.h` (data contracts and C/C++ bridge declarations), `utils.c` (safe input and string helpers), `main.c` (console menus and application flow), `stocklink_oop.cpp` (only the program entry point), and `Makefile` (build rules).

## Language percentage and project-size method

The original `StockLink_OOP_DS.zip` contains 3,689 physical lines across C/C++ source and header files. This edition contains 2,176 such lines, or approximately **59.0%** of that baseline. The count includes blank lines and comments, so treat it as a reproducible size estimate, not an exact percentage of business functionality.

Across the five member-specific file pairs, the physical-line split is approximately **57.3% C / 42.7% C++**. The C++ proportion differs by module; Ayushman's report presentation and polymorphic report classes are intentionally more class-heavy. The module-specific counts are recorded in `CROSSCHECK_REPORT.md`.

## Accuracy/security note

The password digest is a simple educational salted FNV-1a demonstration. It is not production-grade password storage; real systems should use a dedicated password-hashing algorithm with unique cryptographically random salts and other account protections.
