// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_log_format.c — host harness for the ss_log redaction core.
//
// No Unity dependency: a tiny CHECK macro exits nonzero on first failure.
// Built with -fsanitize=address,undefined so a stray dereference of the %k
// pointer, or any buffer overrun, aborts the run.
//
// The pure core (ss_log_format.c) has no levels or colors; those live in the
// IDF sink (ss_log.c) and are covered by the host-tests.yml grep step instead
// of being compiled here (see test 8).

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ss_log_format.h"

static int g_checks = 0;
static int g_fails = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        g_checks++;                                                                                \
        if (!(cond)) {                                                                             \
            g_fails++;                                                                             \
            printf("  FAIL: %s (line %d)\n", #cond, __LINE__);                                     \
        }                                                                                          \
    } while (0)

// Thin varargs wrapper so tests can call the va_list-based core directly.
static int fmt(char* out, size_t out_size, const char* f, ...)
{
    va_list ap;
    va_start(ap, f);
    int n = ss_log_vformat(out, out_size, f, ap);
    va_end(ap);
    return n;
}

// Differential oracle: ss_log_vformat must byte-for-byte match vsnprintf for any
// format WITHOUT %k, including the return value.
static void diff_check(const char* f, ...)
{
    char a[256];
    char b[256];
    va_list ap;

    va_start(ap, f);
    int na = ss_log_vformat(a, sizeof a, f, ap);
    va_end(ap);

    va_start(ap, f);
    int nb = vsnprintf(b, sizeof b, f, ap);
    va_end(ap);

    g_checks++;
    if (strcmp(a, b) != 0 || na != nb) {
        g_fails++;
        printf("  FAIL: diff fmt=\"%s\": got=\"%s\"(%d) want=\"%s\"(%d)\n", f, a, na, b, nb);
    }
}

int main(void)
{
    char out[256];

    // 1. %k emits exactly "[REDACTED]" and the secret bytes appear NOWHERE.
    {
        const char* secret = "S3CR3TKEY";
        int n = fmt(out, sizeof out, "%k", (const void*)secret);
        CHECK(strcmp(out, "[REDACTED]") == 0);
        CHECK(n == (int)strlen("[REDACTED]"));
        CHECK(strstr(out, "S3CR3TKEY") == NULL);
        printf("case 1  %%k redacts, secret absent: %s\n", out);
    }

    // 2. Pointer is never dereferenced: an invalid pointer must not crash.
    {
        int n = fmt(out, sizeof out, "key=%k", (const void*)0x1);
        CHECK(strcmp(out, "key=[REDACTED]") == 0);
        CHECK(n == (int)strlen("key=[REDACTED]"));
        printf("case 2  %%k never derefs (0x1 safe): %s\n", out);
    }

    // 3. Args AFTER %k still format correctly.
    {
        const char* key = "TOPSECRET";
        int n = fmt(out, sizeof out, "user=%s key=%k n=%d", "bob", (const void*)key, 42);
        CHECK(strcmp(out, "user=bob key=[REDACTED] n=42") == 0);
        CHECK(n == (int)strlen("user=bob key=[REDACTED] n=42"));
        CHECK(strstr(out, "TOPSECRET") == NULL);
        printf("case 3  args after %%k align: %s\n", out);
    }

    // 4. Multiple %k in one format string.
    {
        int n = fmt(out, sizeof out, "a=%k b=%k c=%d", (const void*)0x1, (const void*)0x2, 7);
        CHECK(strcmp(out, "a=[REDACTED] b=[REDACTED] c=7") == 0);
        CHECK(n == (int)strlen("a=[REDACTED] b=[REDACTED] c=7"));
        printf("case 4  multiple %%k: %s\n", out);
    }

    // 5. Differential matrix vs vsnprintf (standard conversions, no %k).
    {
        diff_check("literal only, no conv");
        diff_check("%%");
        diff_check("pct=%% val=%d", 5);
        diff_check("d=%d i=%i u=%u", -3, -4, 5u);
        diff_check("o=%o x=%x X=%X", 8u, 255u, 255u);
        diff_check("hex=%08x", 0xABCDu);
        diff_check("s=%.3s", "abcdef");
        diff_check("star=%*d", 6, 42);
        diff_check("starprec=%.*f", 2, 3.14159);
        diff_check("both=%*.*f", 10, 3, 2.71828);
        diff_check("c=%c", 'Z');
        diff_check("p=%p", (void*)0xDEAD);
        diff_check("u=%u llu=%llu zu=%zu", 1u, 2ull, (size_t)3);
        diff_check("ld=%ld lld=%lld", 100000L, 100000000000LL);
        diff_check("f=%f e=%e g=%g", 1.5, 1.5, 1.5);
        diff_check("F=%F E=%E G=%G", 2.5, 2.5, 2.5);
        diff_check("flags=[%+d][% d][%-5d|][%#x]", 3, 3, 3, 255u);
        diff_check("j=%jd t=%td", (intmax_t)-9, (ptrdiff_t)9);
        diff_check("hh=%hhd h=%hd", 200, 30000);
        printf("case 5  differential vs vsnprintf: %d checks\n", g_checks);
    }

    // 6. Truncation safety: tiny buffer, no overflow, NUL-terminated, snprintf
    //    return semantics preserved.
    {
        char tiny[5];
        char full[64];
        int n = fmt(tiny, sizeof tiny, "abcdefgh %d", 123);
        // snprintf's return is the full would-be length regardless of buffer;
        // derive both the oracle length and the expected truncation from a big
        // buffer to avoid tripping -Wformat-truncation on a deliberately tiny one.
        int rn = snprintf(full, sizeof full, "abcdefgh %d", 123);
        char expect[5];
        memcpy(expect, full, sizeof expect - 1);
        expect[sizeof expect - 1] = '\0';
        CHECK(n == rn);
        CHECK(strcmp(tiny, expect) == 0);
        CHECK(strlen(tiny) < sizeof tiny);
        // out_size == 0 must write nothing yet still report the full length.
        int n0 = fmt(NULL, 0, "hello %d", 9);
        CHECK(n0 == (int)strlen("hello 9"));
        printf("case 6  truncation-safe: tiny=\"%s\" n=%d, size0 n=%d\n", tiny, n, n0);
    }

    // 7. Unknown conversion is fail-closed: stop, do NOT print the trailing %d.
    {
        int n = fmt(out, sizeof out, "a %q b %d", 42);
        CHECK(strcmp(out, "a [!fmt:q]") == 0);
        CHECK(n == (int)strlen("a [!fmt:q]"));
        CHECK(strstr(out, "b") == NULL);
        CHECK(strstr(out, "42") == NULL);
        printf("case 7  unknown conv fail-closed: %s\n", out);

        // %n is likewise rejected outright.
        int m = fmt(out, sizeof out, "x %n y");
        CHECK(strcmp(out, "x [!fmt]") == 0);
        CHECK(m == (int)strlen("x [!fmt]"));
        printf("case 7  %%n rejected: %s\n", out);
    }

    printf("\n%d checks, %d failures\n", g_checks, g_fails);
    return (g_fails == 0) ? 0 : 1;
}
