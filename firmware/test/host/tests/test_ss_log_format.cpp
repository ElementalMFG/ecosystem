// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// test_ss_log_format.cpp — gtest port of the ss_log redaction host harness.
//
// Faithful port of firmware/components/ss_log/test/host/test_ss_log_format.c
// (which stays as the quick make/ASan/UBSan dev loop). Every assertion is
// preserved: the %k security cases — including the never-dereference
// (const void*)0x1 case and "secret bytes appear nowhere" — plus the full
// differential-vs-vsnprintf conversion matrix, truncation safety, and the
// fail-closed handling of unknown conversions and %n.

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include "gtest/gtest.h"

extern "C" {
#include "ss_log_format.h"
}

namespace
{

// Thin varargs wrapper so tests can call the va_list-based core directly.
int fmt(char* out, size_t out_size, const char* f, ...)
{
    va_list ap;
    va_start(ap, f);
    int n = ss_log_vformat(out, out_size, f, ap);
    va_end(ap);
    return n;
}

// Differential oracle: ss_log_vformat must byte-for-byte match vsnprintf for any
// format WITHOUT %k, including the return value.
void diff_check(const char* f, ...)
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

    EXPECT_STREQ(a, b) << "fmt=\"" << f << "\"";
    EXPECT_EQ(na, nb) << "fmt=\"" << f << "\"";
}

} // namespace

// 1. %k emits exactly "[REDACTED]" and the secret bytes appear NOWHERE.
TEST(SsLogFormat, KRedactsSecretAbsent)
{
    char out[256];
    const char* secret = "S3CR3TKEY";
    int n = fmt(out, sizeof out, "%k", (const void*)secret);
    EXPECT_STREQ(out, "[REDACTED]");
    EXPECT_EQ(n, (int)strlen("[REDACTED]"));
    EXPECT_EQ(strstr(out, "S3CR3TKEY"), nullptr);
}

// 2. Pointer is never dereferenced: an invalid pointer must not crash.
TEST(SsLogFormat, KNeverDereferences)
{
    char out[256];
    int n = fmt(out, sizeof out, "key=%k", (const void*)0x1);
    EXPECT_STREQ(out, "key=[REDACTED]");
    EXPECT_EQ(n, (int)strlen("key=[REDACTED]"));
}

// 3. Args AFTER %k still format correctly.
TEST(SsLogFormat, ArgsAfterKAlign)
{
    char out[256];
    const char* key = "TOPSECRET";
    int n = fmt(out, sizeof out, "user=%s key=%k n=%d", "bob", (const void*)key, 42);
    EXPECT_STREQ(out, "user=bob key=[REDACTED] n=42");
    EXPECT_EQ(n, (int)strlen("user=bob key=[REDACTED] n=42"));
    EXPECT_EQ(strstr(out, "TOPSECRET"), nullptr);
}

// 4. Multiple %k in one format string.
TEST(SsLogFormat, MultipleK)
{
    char out[256];
    int n = fmt(out, sizeof out, "a=%k b=%k c=%d", (const void*)0x1, (const void*)0x2, 7);
    EXPECT_STREQ(out, "a=[REDACTED] b=[REDACTED] c=7");
    EXPECT_EQ(n, (int)strlen("a=[REDACTED] b=[REDACTED] c=7"));
}

// 5. Differential matrix vs vsnprintf (standard conversions, no %k).
TEST(SsLogFormat, DifferentialVsVsnprintf)
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
}

// 6. Truncation safety: tiny buffer, no overflow, NUL-terminated, snprintf
//    return semantics preserved.
TEST(SsLogFormat, TruncationSafe)
{
    char tiny[5];
    char full[64];
    int n = fmt(tiny, sizeof tiny, "abcdefgh %d", 123);
    // snprintf's return is the full would-be length regardless of buffer; derive
    // both the oracle length and the expected truncation from a big buffer to
    // avoid tripping -Wformat-truncation on a deliberately tiny one.
    int rn = snprintf(full, sizeof full, "abcdefgh %d", 123);
    char expect[5];
    memcpy(expect, full, sizeof expect - 1);
    expect[sizeof expect - 1] = '\0';
    EXPECT_EQ(n, rn);
    EXPECT_STREQ(tiny, expect);
    EXPECT_LT(strlen(tiny), sizeof tiny);
    // out_size == 0 must write nothing yet still report the full length.
    int n0 = fmt(nullptr, 0, "hello %d", 9);
    EXPECT_EQ(n0, (int)strlen("hello 9"));
}

// 7. Unknown conversion is fail-closed: stop, do NOT print the trailing %d.
TEST(SsLogFormat, UnknownConvFailClosed)
{
    char out[256];
    int n = fmt(out, sizeof out, "a %q b %d", 42);
    EXPECT_STREQ(out, "a [!fmt:q]");
    EXPECT_EQ(n, (int)strlen("a [!fmt:q]"));
    EXPECT_EQ(strstr(out, "b"), nullptr);
    EXPECT_EQ(strstr(out, "42"), nullptr);
}

// 7b. %n is likewise rejected outright (no write-back primitive in a formatter).
TEST(SsLogFormat, PercentNRejected)
{
    char out[256];
    int m = fmt(out, sizeof out, "x %n y");
    EXPECT_STREQ(out, "x [!fmt]");
    EXPECT_EQ(m, (int)strlen("x [!fmt]"));
}
