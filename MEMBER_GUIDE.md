# StockLink — quick guide for the five presenters

Each member should be able to explain the C algorithm first, the C++ class that calls it second, and one complete menu-to-function flow third.

## Nikhil
- **C:** chained hash table, growable pointer array, product CRUD, keyword/prefix scan, persistence.
- **C++:** `InventoryService`, private inventory pointer, service methods and display.
- **Likely questions:** What is a collision? Why chaining? Why use a pointer array as well as buckets? Why block deletion while stock is reserved?
- **Demo:** add/update/search a product, then restart to prove catalogue persistence.

## Omkar
- **C:** salted password digest, credential checking, three-strike lockout, account table and account management. A shared `account_index()` helper avoids repeating username-search loops.
- **C++:** abstract `AccountView`, three derived role types, virtual permissions/title/greeting methods, `RoleService`, `AccountAdminService`. Successful login calls the role greeting through a C++ bridge.
- **Likely questions:** What does salt do? What is a virtual function? Where is runtime dispatch? Why does a C++ role class not verify passwords?
- **Demo:** all role dashboards, failed-login lockout, manager unlock/reset, account creation and password change.

## Aman
- **C:** circular FIFO order queue, reservations, process/undo rules, cancellation, LIFO undo stack, invariant checker.
- **C++:** `OrderDesk` and `SupplierDesk`.
- **Likely questions:** Why FIFO? Why LIFO for undo? At what point is stock actually deducted? What invariant must always hold?
- **Demo:** place → process → undo; then place → cancel; show low-stock list and restock a product.

## Rahul
- **C:** adjacency-list graph, undirected weighted links, BFS recommendations, DFS connected bundle.
- **C++:** `ProductRelationshipService` and product-name presentation.
- **Likely questions:** BFS vs DFS? What does edge strength mean? Why is the graph stored separately from product records?
- **Demo:** recommendations, add a relation, inspect the connected bundle.

## Ayushman
- **C:** insertion sort, merge sort, binary price bounds, inventory summary and low-stock calculation.
- **C++:** `CatalogService`, abstract `ReportView`, derived reports and report exporter.
- **Likely questions:** Insertion sort O(n²) vs merge sort O(n log n); why sort before binary bounds? How does virtual `render()` work?
- **Demo:** sort the catalogue two ways, search a price range, show reports and export a text file.

## One architecture question everybody should know

A console action starts in `main.c`. The relevant C++ bridge constructs/calls that member's class. The class coordinates or formats the operation and calls the C function for the algorithm/data operation. Shared structs and declarations are in `stocklink.h`. `stocklink_oop.cpp` contains only `main()`; member classes are in their respective member `.cpp` files.
