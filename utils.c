#include "stocklink.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int input_eof = 0;

void util_copy(char *dest, const char *source, int capacity)
{
    if (!dest || capacity <= 0) return;
    if (!source) source = "";
    strncpy(dest, source, (size_t)capacity - 1U);
    dest[capacity - 1] = '\0';
}

void util_read_line(const char *prompt, char *buffer, int capacity)
{
    int ch;
    if (!buffer || capacity < 2) return;
    printf("%s", prompt ? prompt : "");
    if (input_eof || !fgets(buffer, capacity, stdin)) {
        input_eof = 1;
        buffer[0] = '\0';
        return;
    }
    if (strchr(buffer, '\n')) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        while ((ch = getchar()) != '\n' && ch != EOF) { /* Drain long input. */ }
    }
}

int util_read_int(const char *prompt, int low, int high)
{
    char buffer[80];
    char *end;
    long value;
    for (;;) {
        if (input_eof) return low;
        util_read_line(prompt, buffer, (int)sizeof(buffer));
        if (input_eof) return low;
        value = strtol(buffer, &end, 10);
        while (isspace((unsigned char)*end)) ++end;
        if (buffer[0] != '\0' && *end == '\0' && value >= low && value <= high)
            return (int)value;
        printf("Enter a whole number from %d to %d.\n", low, high);
    }
}

float util_read_float(const char *prompt, float low, float high)
{
    char buffer[80];
    char *end;
    float value;
    for (;;) {
        if (input_eof) return low;
        util_read_line(prompt, buffer, (int)sizeof(buffer));
        if (input_eof) return low;
        value = strtof(buffer, &end);
        while (isspace((unsigned char)*end)) ++end;
        if (buffer[0] != '\0' && *end == '\0' && value >= low && value <= high)
            return value;
        printf("Enter a number from %.2f to %.2f.\n", low, high);
    }
}

int util_icmp(const char *a, const char *b)
{
    unsigned char ca, cb;
    if (!a) a = "";
    if (!b) b = "";
    do {
        ca = (unsigned char)tolower((unsigned char)*a++);
        cb = (unsigned char)tolower((unsigned char)*b++);
        if (ca != cb) return (int)ca - (int)cb;
    } while (ca && cb);
    return (int)ca - (int)cb;
}

int util_contains_ci(const char *text, const char *query)
{
    const char *start;
    if (!text || !query) return 0;
    if (*query == '\0') return 1;
    for (start = text; *start; ++start) {
        const unsigned char *left = (const unsigned char *)start;
        const unsigned char *right = (const unsigned char *)query;
        while (*left && *right && tolower(*left) == tolower(*right)) {
            ++left;
            ++right;
        }
        if (*right == '\0') return 1;
    }
    return 0;
}

int util_prefix_ci(const char *text, const char *prefix)
{
    if (!text || !prefix) return 0;
    while (*prefix) {
        if (tolower((unsigned char)*text) != tolower((unsigned char)*prefix)) return 0;
        ++text;
        ++prefix;
    }
    return 1;
}

void util_pause(void)
{
    char buffer[8];
    util_read_line("Press Enter to continue...", buffer, (int)sizeof(buffer));
}
