#include "stocklink.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static int password_policy_ok(const char *password);

/* Find one account in a single place instead of repeating this loop everywhere. */
static int account_index(const AuthStore *store, const char *username)
{
    int i;
    if (!store || !username) return -1;
    for (i = 0; i < store->count; ++i)
        if (util_icmp(store->accounts[i].username, username) == 0) return i;
    return -1;
}

/* Educational salted FNV-1a digest; suitable for a class project, not production auth. */
static unsigned long long password_digest(const char *password, const char *salt)
{
    unsigned long long hash = 1469598103934665603ULL;
    const unsigned char *p;
    for (p = (const unsigned char *)salt; *p; ++p) {
        hash ^= *p;
        hash *= 1099511628211ULL;
    }
    for (p = (const unsigned char *)password; *p; ++p) {
        hash ^= *p;
        hash *= 1099511628211ULL;
    }
    return hash;
}

static int add_account(AuthStore *store, const char *name, const char *password,
                       const char *salt, Role role)
{
    Account *account;
    if (!store || store->count >= MAX_USERS) return 0;
    account = &store->accounts[store->count++];
    memset(account, 0, sizeof(*account));
    util_copy(account->username, name, USER_LEN);
    util_copy(account->salt, salt, (int)sizeof(account->salt));
    account->password_hash = password_digest(password, salt);
    account->role = role;
    return 1;
}

void auth_init(AuthStore *store)
{
    if (!store) return;
    memset(store, 0, sizeof(*store));
    add_account(store, "admin", "admin123", "sl-mgr-1", ROLE_MANAGER);
    add_account(store, "demo", "demo123", "sl-cus-1", ROLE_CUSTOMER);
    add_account(store, "freshmart", "fresh123", "sl-sup-1", ROLE_SUPPLIER);
}

AuthStatus auth_login(AuthStore *store, const char *username,
                      const char *password, Role *role_out)
{
    int index = account_index(store, username);
    Account *account;
    if (role_out) *role_out = ROLE_NONE;
    if (!store || !username || !password) return AUTH_NOT_FOUND;
    if (index < 0) return AUTH_NOT_FOUND;
    account = &store->accounts[index];
    if (account->locked) return AUTH_LOCKED;
    if (password_digest(password, account->salt) == account->password_hash) {
        account->failed_attempts = 0;
        if (role_out) *role_out = account->role;
        return AUTH_OK;
    }
    if (++account->failed_attempts >= 3) {
        account->locked = 1;
        return AUTH_LOCKED;
    }
    return AUTH_BAD_PASSWORD;
}

int auth_unlock(AuthStore *store, const char *username)
{
    int index = account_index(store, username);
    Account *account;
    if (index < 0) return 0;
    account = &store->accounts[index];
    account->locked = account->failed_attempts = 0;
    return 1;
}

int auth_locked(const AuthStore *store, const char *username)
{
    int index = account_index(store, username);
    return index < 0 ? 0 : store->accounts[index].locked;
}

static int valid_new_username(const char *username)
{
    size_t length;
    const unsigned char *p;
    if (!username) return 0;
    length = strlen(username);
    if (length < 3 || length >= USER_LEN) return 0;
    for (p = (const unsigned char *)username; *p; ++p)
        if (!isalnum(*p) && *p != '_') return 0;
    return 1;
}

int auth_register(AuthStore *store, const char *username, const char *password, Role role)
{
    Account *account;
    unsigned long salt_value = 5381UL;
    const unsigned char *p;
    if (!store || !valid_new_username(username) || !password || strlen(password) < 6 ||
        (role != ROLE_CUSTOMER && role != ROLE_SUPPLIER) || store->count >= MAX_USERS ||
        !password_policy_ok(password) || account_index(store, username) >= 0)
        return 0;

    account = &store->accounts[store->count];
    memset(account, 0, sizeof(*account));
    util_copy(account->username, username, USER_LEN);
    /* Deterministic demo salt; production software should use a cryptographic RNG. */
    for (p = (const unsigned char *)username; *p; ++p)
        salt_value = ((salt_value << 5) + salt_value) ^ *p;
    (void)snprintf(account->salt, sizeof(account->salt), "%08lx%02x",
                   salt_value & 0xffffffffUL, (unsigned int)store->count);
    account->password_hash = password_digest(password, account->salt);
    account->role = role;
    ++store->count;
    return 1;
}

int auth_change_password(AuthStore *store, const char *username,
                         const char *old_password, const char *new_password)
{
    int index = account_index(store, username);
    Account *account;
    if (index < 0 || !old_password || !password_policy_ok(new_password)) return 0;
    account = &store->accounts[index];
    if (account->locked || password_digest(old_password, account->salt) != account->password_hash)
        return 0;
    account->password_hash = password_digest(new_password, account->salt);
    account->failed_attempts = 0;
    return 1;
}

static int password_policy_ok(const char *password)
{
    int has_letter = 0, has_digit = 0;
    const unsigned char *p;
    if (!password || strlen(password) < 6) return 0;
    for (p = (const unsigned char *)password; *p; ++p) {
        if (isalpha(*p)) has_letter = 1;
        if (isdigit(*p)) has_digit = 1;
    }
    return has_letter && has_digit;
}

int auth_reset_password(AuthStore *store, const char *username, const char *new_password)
{
    int index = account_index(store, username);
    Account *account;
    unsigned long long new_salt;
    if (index < 0 || !password_policy_ok(new_password)) return 0;
    account = &store->accounts[index];
    if (account->role == ROLE_MANAGER) return 0;
    new_salt = password_digest(new_password, account->salt) ^ account->password_hash;
    (void)snprintf(account->salt, sizeof(account->salt), "R%012llx",
                   new_salt & 0xffffffffffffULL);
    account->password_hash = password_digest(new_password, account->salt);
    account->failed_attempts = account->locked = 0;
    return 1;
}

int auth_collect_accounts(const AuthStore *store, AccountInfo *out, int capacity)
{
    int i, count;
    if (!store || !out || capacity <= 0) return 0;
    count = store->count < capacity ? store->count : capacity;
    for (i = 0; i < count; ++i) {
        util_copy(out[i].username, store->accounts[i].username, USER_LEN);
        out[i].role = store->accounts[i].role;
        out[i].locked = store->accounts[i].locked;
        out[i].failed_attempts = store->accounts[i].failed_attempts;
    }
    return count;
}

void auth_get_summary(const AuthStore *store, AuthSummary *summary)
{
    int i;
    if (!summary) return;
    memset(summary, 0, sizeof(*summary));
    if (!store) return;
    summary->total = store->count;
    for (i = 0; i < store->count; ++i) {
        const Account *account = &store->accounts[i];
        if (account->role == ROLE_CUSTOMER) ++summary->customers;
        else if (account->role == ROLE_MANAGER) ++summary->managers;
        else if (account->role == ROLE_SUPPLIER) ++summary->suppliers;
        if (account->locked) ++summary->locked;
    }
}
