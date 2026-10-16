# StockLink Final Cross-Check Report

## 1. Project-size check

Count basis: physical lines in `.c`, `.cpp`, `.h`, and `.hpp` files, including blank lines and comments. This matches the file-type basis used for the original `StockLink_OOP_DS.zip` comparison.

| Measure | Lines |
|---|---:|
| Original ZIP source/header files | 3,689 |
| This submission source/header files | 2,176 |
| Retained line-count share | **59.0%** |
| Makefile (excluded from the comparable share) | 24 |

This line ratio is a measurable size proxy, not a claim that every source line represents an equal amount of functionality. The feature scope is intentionally a core subset of the original design. Omkar’s module was also streamlined by moving repeated username-search loops into one helper and removing redundant role-object setup; no authentication feature was intentionally removed.

## 2. Member-specific file pairs

Physical lines include comments and blank lines. The C/C++ balance differs by responsibility, rather than forcing every member to an exact ratio.

| Member | C file lines | C++ file lines | C share | C++ share |
|---|---:|---:|---:|---:|
| Nikhil | 210 | 143 | 59.5% | 40.5% |
| Omkar | 201 | 135 | 59.8% | 40.2% |
| Aman | 175 | 120 | 59.3% | 40.7% |
| Rahul | 130 | 85 | 60.5% | 39.5% |
| Ayushman | 126 | 144 | 46.7% | 53.3% |
| **All five pairs** | **842** | **627** | **57.3%** | **42.7%** |

Ayushman's C++ file is intentionally more class-heavy because it contains catalogue presentation, the polymorphic report interface and report export. The sort algorithms, binary range bounds and report calculations remain in his C file. The requested 60/40 division is approximate, not a reason to move presentation code into C or duplicate algorithms.

## 3. Duplicate-implementation audit

- `stocklink_oop.cpp` contains only the process `main()` entry point. It contains no project class definitions.
- Each member's project classes are defined in that member's own `.cpp` file.
- Hash-table operations, product CRUD, and catalogue persistence are implemented in `nikhil_inventory.c`. `InventoryService` calls those C functions and presents the results.
- Password hashing, credential matching, failed-attempt tracking, account lockout and account storage are implemented in `omkar_auth.c`. One `account_index()` helper handles username lookup to avoid repeating the same loop in several functions. C++ uses role classes for permissions and virtual role descriptions/greetings; it does not hash passwords.
- Queue operations, reservations, cancellation, processing, undo and reservation-invariant checks are implemented in `aman_orders_suppliers.c`. C++ coordinates the order workflow and outputs messages.
- Graph edges, BFS and DFS are implemented in `rahul_relationships.c`. C++ maps result IDs to product names and formats the output.
- Insertion sort, merge sort, binary price bounds, stock/value totals and low-stock identification are implemented in `ayushman_algorithms.c`. C++ selects the sort, formats reports and exports results; it does not implement a second sort or search algorithm.
- The code has no STL containers. C++ uses normal classes and virtual functions; the program is console-only.

## 4. Build and runtime checks

- Clean GCC/G++ build in C11/C++17 mode after the Omkar refactor: **passed with no compiler warnings**.
- Authentication unit checks after the refactor (valid login, three-failure lockout, unlock, registration, duplicate rejection, password change/reset and manager-reset protection): **passed**.
- Interactive application startup/exit and polymorphic role greeting: **passed**.
- Startup, exit and first-run demo seeding: passed.
- Product add, duplicate-name rejection, update/list flow and catalogue persistence across restart: passed.
- Customer order placement, reservation, manager processing, undo and queue/reservation invariant checks: passed.
- Customer cancellation and reserved-stock release: passed.
- BFS recommendations and DFS connected bundle: passed.
- Three failed logins, lockout, manager unlock and successful login: passed.
- Customer/supplier account creation, account list, password reset/unlock and login: passed.
- Password change and subsequent login: passed.
- Inventory summary, low-stock report and text report export: passed.

## 5. Intentional scope reductions

Not present in this edition: Trie autocomplete, AVL price tree, quick sort, heap sort, benchmark, min-heap stock prioritisation, Dijkstra supplier routing, supplier quote/dispatch workflow, order-learned graph updates and persistence of users/orders/relationships. Prefix suggestions use a linear scan, and product data is the only persistent state.

## 6. Security limitation

The salted FNV-1a digest exists to demonstrate the concept of hashing in a student project. It is not a production password-storage scheme.
