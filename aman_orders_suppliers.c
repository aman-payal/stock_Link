#include "stocklink.h"
#include <string.h>
#include <assert.h>

void orders_init(OrderSystem *orders)
{
    if (!orders) return;
    memset(orders, 0, sizeof(*orders));
    orders->next_id = 1;
}

static int queue_push_back(OrderSystem *orders, const Order *order)
{
    int position;
    if (orders->count >= MAX_ORDERS) return 0;
    position = (orders->front + orders->count) % MAX_ORDERS;
    orders->queue[position] = *order;
    ++orders->count;
    return 1;
}

static int queue_push_front(OrderSystem *orders, const Order *order)
{
    if (orders->count >= MAX_ORDERS) return 0;
    orders->front = (orders->front - 1 + MAX_ORDERS) % MAX_ORDERS;
    orders->queue[orders->front] = *order;
    ++orders->count;
    return 1;
}

static int queue_pop_front(OrderSystem *orders, Order *out)
{
    if (orders->count <= 0) return 0;
    if (out) *out = orders->queue[orders->front];
    orders->front = (orders->front + 1) % MAX_ORDERS;
    --orders->count;
    return 1;
}

int orders_place(OrderSystem *orders, Inventory *inv, const char *owner,
                 int product_id, int quantity, int *order_id_out)
{
    Product *product;
    Order order;
    if (!orders || !inv || !owner || quantity <= 0 || orders->count >= MAX_ORDERS)
        return 0;
    product = inv_find(inv, product_id);
    if (!product || inv_available(product) < quantity) return 0;
    memset(&order, 0, sizeof(order));
    order.id = orders->next_id++;
    util_copy(order.owner, owner, USER_LEN);
    order.product_id = product_id;
    order.quantity = quantity;
    if (!queue_push_back(orders, &order)) return 0;
    product->reserved += quantity;
    if (order_id_out) *order_id_out = order.id;
    assert(orders_validate(orders, inv));
    return 1;
}

int orders_pending(const OrderSystem *orders, Order *out, int capacity)
{
    int i, count = 0;
    if (!orders || !out || capacity <= 0) return 0;
    for (i = 0; i < orders->count && count < capacity; ++i)
        out[count++] = orders->queue[(orders->front + i) % MAX_ORDERS];
    return count;
}

int orders_process_next(OrderSystem *orders, Inventory *inv, Order *out)
{
    Order completed;
    Product *product;
    if (!orders || !inv || orders->count <= 0 || orders->undo_top >= MAX_UNDO) return 0;
    completed = orders->queue[orders->front];
    product = inv_find(inv, completed.product_id);
    if (!product || product->stock < completed.quantity ||
        product->reserved < completed.quantity) return 0;
    queue_pop_front(orders, &completed);
    product->reserved -= completed.quantity;
    product->stock -= completed.quantity;
    orders->undo_stack[orders->undo_top++] = completed;
    if (out) *out = completed;
    assert(orders_validate(orders, inv));
    return 1;
}

int orders_undo_last(OrderSystem *orders, Inventory *inv, Order *out)
{
    Order restored;
    Product *product;
    if (!orders || !inv || orders->undo_top <= 0 || orders->count >= MAX_ORDERS) return 0;
    restored = orders->undo_stack[orders->undo_top - 1];
    product = inv_find(inv, restored.product_id);
    if (!product) return 0;
    if (!queue_push_front(orders, &restored)) return 0;
    --orders->undo_top;
    product->stock += restored.quantity;
    product->reserved += restored.quantity;
    if (out) *out = restored;
    assert(orders_validate(orders, inv));
    return 1;
}

/* Cancellation removes one pending order and releases its reserved quantity. */
int orders_cancel(OrderSystem *orders, Inventory *inv, int order_id,
                  const char *owner, int allow_any, Order *out)
{
    Order retained[MAX_ORDERS], cancelled;
    Product *product;
    int i, kept = 0, found = 0;
    if (!orders || !inv || order_id <= 0) return 0;
    for (i = 0; i < orders->count; ++i) {
        Order candidate = orders->queue[(orders->front + i) % MAX_ORDERS];
        int owner_matches = allow_any || (owner && util_icmp(owner, candidate.owner) == 0);
        if (!found && candidate.id == order_id && owner_matches) {
            cancelled = candidate;
            found = 1;
        } else {
            retained[kept++] = candidate;
        }
    }
    if (!found) return 0;
    product = inv_find(inv, cancelled.product_id);
    if (!product || product->reserved < cancelled.quantity) return 0;
    product->reserved -= cancelled.quantity;
    orders->front = 0;
    orders->count = kept;
    for (i = 0; i < kept; ++i) orders->queue[i] = retained[i];
    if (out) *out = cancelled;
    assert(orders_validate(orders, inv));
    return 1;
}


void orders_get_summary(const OrderSystem *orders, OrderSummary *summary)
{
    int i;
    if (!summary) return;
    summary->pending_orders = 0;
    summary->pending_units = 0;
    summary->undoable_orders = 0;
    if (!orders) return;
    summary->pending_orders = orders->count;
    summary->undoable_orders = orders->undo_top;
    for (i = 0; i < orders->count; ++i)
        summary->pending_units += orders->queue[(orders->front + i) % MAX_ORDERS].quantity;
}

/* Central invariant: every reserved unit belongs to exactly one pending order. */
int orders_validate(const OrderSystem *orders, const Inventory *inv)
{
    int i, j;
    if (!orders || !inv || orders->count < 0 || orders->count > MAX_ORDERS ||
        orders->undo_top < 0 || orders->undo_top > MAX_UNDO ||
        orders->front < 0 || orders->front >= MAX_ORDERS) return 0;
    for (i = 0; i < orders->count; ++i) {
        const Order *order = &orders->queue[(orders->front + i) % MAX_ORDERS];
        if (order->id <= 0 || order->quantity <= 0 || !inv_find(inv, order->product_id))
            return 0;
        for (j = i + 1; j < orders->count; ++j)
            if (order->id == orders->queue[(orders->front + j) % MAX_ORDERS].id) return 0;
    }
    for (i = 0; i < inv->count; ++i) {
        const Product *product = inv->items[i];
        int reserved_expected = 0;
        for (j = 0; j < orders->count; ++j) {
            const Order *order = &orders->queue[(orders->front + j) % MAX_ORDERS];
            if (order->product_id == product->id) reserved_expected += order->quantity;
        }
        if (reserved_expected != product->reserved || product->reserved < 0 ||
            product->reserved > product->stock) return 0;
    }
    return 1;
}
