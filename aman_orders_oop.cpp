#include "stocklink.h"
#include <cstdio>

/* Composition: order and supplier services use the C inventory/order state. */
class OrderDesk {
private:
    OrderSystem *orders_;
    Inventory *inventory_;
public:
    OrderDesk(OrderSystem *orders, Inventory *inventory)
        : orders_(orders), inventory_(inventory) {}

    void place(const char *owner, int productId, int quantity) const
    {
        int newId = 0;
        if (orders_place(orders_, inventory_, owner, productId, quantity, &newId))
            std::printf("Order #%d placed. Stock is reserved until processing.\n", newId);
        else
            std::puts("Order failed: check product ID, quantity and available stock.");
    }

    void list(const char *ownerFilter) const
    {
        Order matches[MAX_ORDERS];
        int count = orders_pending(orders_, matches, MAX_ORDERS);
        int shown = 0;
        std::puts("\nPending orders (FIFO order):");
        std::puts("Order   Owner                 Product ID   Quantity");
        std::puts("--------------------------------------------------");
        for (int i = 0; i < count; ++i) {
            if (ownerFilter && util_icmp(ownerFilter, matches[i].owner) != 0) continue;
            std::printf("%-7d %-21s %-12d %d\n", matches[i].id, matches[i].owner,
                        matches[i].product_id, matches[i].quantity);
            ++shown;
        }
        if (shown == 0) std::puts("No matching pending orders.");
    }

    void process() const
    {
        Order completed;
        if (orders_process_next(orders_, inventory_, &completed))
            std::printf("Processed order #%d. It can be undone once.\n", completed.id);
        else
            std::puts("No pending order, or processing is currently unavailable.");
    }

    void undo() const
    {
        Order restored;
        if (orders_undo_last(orders_, inventory_, &restored))
            std::printf("Undo complete: order #%d is pending again and stock was restored.\n",
                        restored.id);
        else
            std::puts("Nothing can be undone right now.");
    }

    void cancel(int orderId, const char *owner, int allowAny) const
    {
        Order cancelled;
        if (orders_cancel(orders_, inventory_, orderId, owner, allowAny, &cancelled))
            std::printf("Cancelled order #%d; %d reserved unit(s) released.\n",
                        cancelled.id, cancelled.quantity);
        else
            std::puts("Cancellation failed: order missing or not owned by this customer.");
    }
};

class SupplierDesk {
private:
    Inventory *inventory_;
public:
    explicit SupplierDesk(Inventory *inventory) : inventory_(inventory) {}

    void restock(int productId, int quantity) const
    {
        if (inv_restock(inventory_, productId, quantity))
            std::puts("Stock replenished successfully.");
        else
            std::puts("Restock failed: check the product ID and positive quantity.");
    }
};

extern "C" void oop_order_place(OrderSystem *orders, Inventory *inv, const char *owner,
                                int product_id, int quantity)
{
    OrderDesk desk(orders, inv);
    desk.place(owner, product_id, quantity);
}

extern "C" void oop_order_list(const OrderSystem *orders, const char *owner_filter)
{
    OrderDesk desk(const_cast<OrderSystem *>(orders), nullptr);
    desk.list(owner_filter);
}

extern "C" void oop_order_process(OrderSystem *orders, Inventory *inv)
{
    OrderDesk desk(orders, inv);
    desk.process();
}

extern "C" void oop_order_undo(OrderSystem *orders, Inventory *inv)
{
    OrderDesk desk(orders, inv);
    desk.undo();
}

extern "C" void oop_supplier_restock(Inventory *inv, int product_id, int quantity)
{
    SupplierDesk desk(inv);
    desk.restock(product_id, quantity);
}

extern "C" void oop_order_cancel(OrderSystem *orders, Inventory *inv, int order_id,
                                 const char *owner, int allow_any)
{
    OrderDesk desk(orders, inv);
    desk.cancel(order_id, owner, allow_any);
}
