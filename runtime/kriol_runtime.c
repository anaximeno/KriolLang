/*
 * KriolLang Runtime Library.
 *
 * Provides small helper functions for the KriolLang runtime, such as printing values.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <gc.h>

void __kriol_gc_init(void) {
    GC_INIT();
}

void __kriol_print_nter(int64_t v) {
    printf("%lld", (long long)v);
}

void __kriol_print_num(double v) {
    printf("%g", v);
}

const char* __kriol_bool_to_str(int v) {
    return v ? "sin" : "nau";
}

void __kriol_print_bool(int v) {
    fputs(__kriol_bool_to_str(v), stdout);
}

void __kriol_print_textu(const char* s) {
    if (s) fputs(s, stdout);
}

void __kriol_println_nter(int64_t v) {
    __kriol_print_nter(v);
    putchar('\n');
}

void __kriol_println_num(double v) {
    __kriol_print_num(v);
    putchar('\n');
}

void __kriol_println_bool(int v) {
    __kriol_print_bool(v);
    putchar('\n');
}

void __kriol_println_textu(const char* s) {
    __kriol_print_textu(s);
    putchar('\n');
}

/* Build a formatted string (f-string support).
 *
 * Accepts a printf-style format string and variadic args.
 * Returns a GC-managed char*, memory is reclaimed automatically by the
 * Boehm GC.
 *
 * On allocation failure returns NULL.
 */
char* __kriol_format(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    // vsnprintf returns the number of characters that would have been written,
    // excluding null terminator. If needed is negative, an encoding error occurred.
    if (needed < 0) return NULL;

    char* buf = (char*)GC_malloc((size_t)needed + 1);

    if (!buf) return NULL;

    va_start(args, fmt);
    vsnprintf(buf, (size_t)needed + 1, fmt, args);
    va_end(args);

    return buf;
}

void __kriol_konfirma(int cond, int line) {
    if (!cond) {
        fprintf(stderr, "kriol: konfirma (assertion):  failed at the line %d\n", line);
        exit(1);
    }
}

void __kriol_check_bounds(int64_t index, int64_t size, int line) {
    if (index < 0 || index >= size) {
        fprintf(stderr, "kriol: array index out of bounds at line %d: %lld not in [0..%lld]\n",
                line, (long long)index, (long long)size - 1);
        exit(1);
    }
}
