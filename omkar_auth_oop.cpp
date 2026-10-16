#include "stocklink.h"
#include <cstdio>

/* C owns credential verification; these C++ classes define role behaviour. */
class AccountView {
public:
    virtual ~AccountView() {}
    virtual const char *title() const = 0;
    virtual const char *homeMessage() const = 0;
    virtual int canManage() const { return 0; }
    virtual int canSupply() const { return 0; }
};

class CustomerAccount : public AccountView {
public:
    const char *title() const override { return "Customer"; }
    const char *homeMessage() const override { return "Browse products and place orders."; }
};

class ManagerAccount : public AccountView {
public:
    const char *title() const override { return "Manager"; }
    const char *homeMessage() const override { return "Manage products, orders and reports."; }
    int canManage() const override { return 1; }
};

class SupplierAccount : public AccountView {
public:
    const char *title() const override { return "Supplier"; }
    const char *homeMessage() const override { return "Review low-stock products and replenish stock."; }
    int canSupply() const override { return 1; }
};

/* The service owns one role object of each type, avoiding repeated setup. */
class RoleService {
private:
    CustomerAccount customer_;
    ManagerAccount manager_;
    SupplierAccount supplier_;

    const AccountView *select(int role) const
    {
        if (role == ROLE_MANAGER) return &manager_;
        if (role == ROLE_SUPPLIER) return &supplier_;
        if (role == ROLE_CUSTOMER) return &customer_;
        return nullptr;
    }
public:
    const char *name(int role) const
    {
        const AccountView *view = select(role);
        return view ? view->title() : "Unknown";
    }
    int canManage(int role) const
    {
        const AccountView *view = select(role);
        return view ? view->canManage() : 0;
    }
    int canSupply(int role) const
    {
        const AccountView *view = select(role);
        return view ? view->canSupply() : 0;
    }
    void greet(int role) const
    {
        const AccountView *view = select(role);
        if (view) std::printf("Role: %s. %s\n", view->title(), view->homeMessage());
    }
};

extern "C" const char *oop_role_name(int role)
{
    static RoleService roles;
    return roles.name(role);
}

extern "C" int oop_role_can_manage(int role)
{
    static RoleService roles;
    return roles.canManage(role);
}

extern "C" int oop_role_can_supply(int role)
{
    static RoleService roles;
    return roles.canSupply(role);
}

extern "C" void oop_role_greet(int role)
{
    static RoleService roles;
    roles.greet(role);
}

/* This class coordinates account requests; C still validates and stores data. */
class AccountAdminService {
private:
    AuthStore *store_;
public:
    explicit AccountAdminService(AuthStore *store) : store_(store) {}

    int create(const char *username, const char *password, int role) const
    {
        if (auth_register(store_, username, password, static_cast<Role>(role))) {
            std::printf("Account '%s' created as %s.\n", username,
                        role == ROLE_SUPPLIER ? "Supplier" : "Customer");
            return 1;
        }
        std::puts("Account creation failed: use a unique 3+ character username and 6+ character password.");
        return 0;
    }

    int changePassword(const char *username, const char *oldPassword,
                       const char *newPassword) const
    {
        if (auth_change_password(store_, username, oldPassword, newPassword)) {
            std::puts("Password changed for this session.");
            return 1;
        }
        std::puts("Password change failed: verify the old password and use at least six new characters.");
        return 0;
    }
};

extern "C" int oop_auth_create_account(AuthStore *store, const char *username,
                                       const char *password, int role)
{
    return AccountAdminService(store).create(username, password, role);
}

extern "C" int oop_auth_change_password(AuthStore *store, const char *username,
                                        const char *old_password, const char *new_password)
{
    return AccountAdminService(store).changePassword(username, old_password, new_password);
}
