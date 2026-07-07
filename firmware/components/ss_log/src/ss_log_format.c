// SPDX-License-Identifier: Apache-2.0
// Copyright (c) 2025 SS-SP Project Contributors
//
// ss_log_format.c — pure formatter core + `%k` redaction (see ss_log_format.h).
//
// AUDIT NOTE: this translation unit is the single point at which sensitive
// arguments could ever be turned into output. It has no ESP-IDF dependency and
// only these headers, so the redaction path can be reasoned about in isolation.

#include "ss_log_format.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Length-modifier codes. hh/h are tracked for spec fidelity but read as `int`
// (they are promoted to int in a va_list, exactly like the default case).
enum {
    LEN_NONE = 0,
    LEN_hh,
    LEN_h,
    LEN_l,
    LEN_ll,
    LEN_z,
    LEN_t,
    LEN_j,
};

// Append a NUL-terminated string, tracking the would-be length in *total and
// never writing past out[out_size - 1] (the reserved NUL slot).
static void append_str(char* out, size_t out_size, size_t* total, const char* s)
{
    for (; *s != '\0'; ++s) {
        if (*total + 1U < out_size) { out[*total] = *s; }
        (*total)++;
    }
}

int ss_log_vformat(char* out, size_t out_size, const char* fmt, va_list args)
{
    size_t total = 0;
    const char* p = (fmt != NULL) ? fmt : "(null)";

    while (*p != '\0') {
        if (*p != '%') {
            if (total + 1U < out_size) { out[total] = *p; }
            total++;
            p++;
            continue;
        }

        // *p == '%'
        p++;

        if (*p == '%') { // literal percent
            if (total + 1U < out_size) { out[total] = '%'; }
            total++;
            p++;
            continue;
        }

        if (*p == 'k') { // redaction: consume one const void*, never deref it
            (void)va_arg(args, const void*);
            append_str(out, out_size, &total, "[REDACTED]");
            p++;
            continue;
        }

        // --- Generic conversion spec: reconstruct it into `spec`, then hand the
        //     single conversion to snprintf. ------------------------------------
        char spec[32];
        size_t si = 0;
        int overflow = 0;

// Append one char to `spec`, flagging overflow instead of writing out of bounds.
#define PUT(ch)                                                                                    \
    do {                                                                                           \
        if (si + 1U >= sizeof spec) {                                                              \
            overflow = 1;                                                                          \
        } else {                                                                                   \
            spec[si++] = (char)(ch);                                                               \
        }                                                                                          \
    } while (0)

        PUT('%');

        // flags
        while (*p == '-' || *p == '+' || *p == ' ' || *p == '0' || *p == '#') {
            PUT(*p);
            p++;
        }

        // width
        int has_width_star = 0;
        if (*p == '*') {
            has_width_star = 1;
            PUT('*');
            p++;
        } else {
            while (*p >= '0' && *p <= '9') {
                PUT(*p);
                p++;
            }
        }

        // precision
        int has_prec_star = 0;
        if (*p == '.') {
            PUT('.');
            p++;
            if (*p == '*') {
                has_prec_star = 1;
                PUT('*');
                p++;
            } else {
                while (*p >= '0' && *p <= '9') {
                    PUT(*p);
                    p++;
                }
            }
        }

        // length
        int len = LEN_NONE;
        if (*p == 'h') {
            PUT('h');
            p++;
            if (*p == 'h') {
                PUT('h');
                p++;
                len = LEN_hh;
            } else {
                len = LEN_h;
            }
        } else if (*p == 'l') {
            PUT('l');
            p++;
            if (*p == 'l') {
                PUT('l');
                p++;
                len = LEN_ll;
            } else {
                len = LEN_l;
            }
        } else if (*p == 'z') {
            PUT('z');
            p++;
            len = LEN_z;
        } else if (*p == 't') {
            PUT('t');
            p++;
            len = LEN_t;
        } else if (*p == 'j') {
            PUT('j');
            p++;
            len = LEN_j;
        }

        char conv = *p;
        if (conv == '\0') { // truncated spec at end of string
            append_str(out, out_size, &total, "[!fmt]");
            break;
        }
        PUT(conv);
        p++;

#undef PUT

        if (overflow) { // spec longer than we will format — fail closed
            append_str(out, out_size, &total, "[!fmt]");
            break;
        }
        spec[si] = '\0';

        // `*` width/precision each consume an int, in order, before the value.
        int width_val = 0;
        int prec_val = 0;
        if (has_width_star) { width_val = va_arg(args, int); }
        if (has_prec_star) { prec_val = va_arg(args, int); }

        size_t avail = (total < out_size) ? (out_size - total) : 0U;
        char* dst = (avail != 0U) ? (out + total) : NULL;
        int n = 0;

        // `spec` is a locally-reconstructed, whitelisted single-conversion format
        // string — never attacker-influenced. -Wformat-nonliteral is off in this
        // project's warning set, but suppress it explicitly so the intent (and the
        // audit) is unambiguous.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"

// Emit one conversion, forwarding any `*` width/precision ints in order. Only
// the taken ?: branch is evaluated, so va_arg for the value fires exactly once.
#define SNP(v)                                                                                     \
    n = has_width_star ? (has_prec_star ? snprintf(dst, avail, spec, width_val, prec_val, (v))     \
                                        : snprintf(dst, avail, spec, width_val, (v)))              \
                       : (has_prec_star ? snprintf(dst, avail, spec, prec_val, (v))                \
                                        : snprintf(dst, avail, spec, (v)))

        switch (conv) {
        case 'd':
        case 'i':
            switch (len) {
            case LEN_l:
                SNP(va_arg(args, long));
                break;
            case LEN_ll:
                SNP(va_arg(args, long long));
                break;
            case LEN_z:
                SNP(va_arg(args, size_t));
                break;
            case LEN_t:
                SNP(va_arg(args, ptrdiff_t));
                break;
            case LEN_j:
                SNP(va_arg(args, intmax_t));
                break;
            default: // none / hh / h — read as promoted int
                SNP(va_arg(args, int));
                break;
            }
            break;
        case 'u':
        case 'o':
        case 'x':
        case 'X':
            switch (len) {
            case LEN_l:
                SNP(va_arg(args, unsigned long));
                break;
            case LEN_ll:
                SNP(va_arg(args, unsigned long long));
                break;
            case LEN_z:
                SNP(va_arg(args, size_t));
                break;
            case LEN_t:
                SNP(va_arg(args, size_t));
                break;
            case LEN_j:
                SNP(va_arg(args, uintmax_t));
                break;
            default:
                SNP(va_arg(args, unsigned int));
                break;
            }
            break;
        case 'c':
            SNP(va_arg(args, int));
            break;
        case 's':
            SNP(va_arg(args, const char*));
            break;
        case 'p':
            SNP(va_arg(args, void*));
            break;
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            SNP(va_arg(args, double));
            break;
        case 'n': // write-back primitive: never honored in a log formatter
            append_str(out, out_size, &total, "[!fmt]");
            goto done;
        default: { // unknown conversion — fail closed, stop consuming args
            char cc[2] = {conv, '\0'};
            append_str(out, out_size, &total, "[!fmt:");
            append_str(out, out_size, &total, cc);
            append_str(out, out_size, &total, "]");
            goto done;
        }
        }

#undef SNP
#pragma GCC diagnostic pop

        if (n > 0) { total += (size_t)n; }
    }

done:
    if (out_size > 0U) { out[(total < out_size) ? total : (out_size - 1U)] = '\0'; }
    return (int)total;
}
