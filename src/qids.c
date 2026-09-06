/**
 * @file qids.c
 * @brief Implementation of QIDS C99 Client Library.
 *
 * Copyright (c) 2026 QIDS. All Rights Reserved.
 * Strictly Proprietary and Confidential.
 */

#include "qids.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

const char* qids_version(void) {
    return QIDS_VERSION_STRING;
}

/* ========================================================================= */
/* 1. Constant-Time Primitives                                              */
/* ========================================================================= */

bool qids_ct_eq(const uint8_t *a, const uint8_t *b, size_t len) {
    if (!a || !b) {
        return false;
    }
    uint8_t diff = 0;
    for (size_t i = 0; i < len; ++i) {
        diff |= (uint8_t)(a[i] ^ b[i]);
    }
    return diff == 0;
}

static inline int popcount8(uint8_t v) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcount((unsigned int)v);
#elif defined(_MSC_VER)
    return __popcnt16((unsigned short)v);
#else
    v = v - ((v >> 1) & 0x55);
    v = (v & 0x33) + ((v >> 2) & 0x33);
    return ((v + (v >> 4)) & 0x0F);
#endif
}

int64_t qids_ct_distance(const uint8_t *a, const uint8_t *b, size_t len) {
    if (!a || !b) {
        return -1;
    }
    int64_t dist = 0;
    for (size_t i = 0; i < len; ++i) {
        dist += popcount8((uint8_t)(a[i] ^ b[i]));
    }
    return dist;
}

bool qids_ct_threshold(size_t observed_mismatches, size_t max_allowed) {
    return observed_mismatches <= max_allowed;
}

/* ========================================================================= */
/* 2. Toeplitz LFSR Universal Hashing over GF(2^64)                         */
/* ========================================================================= */

static const uint8_t REVERSE_BYTE_TABLE[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

static void clmul_64(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo) {
#if defined(__SIZEOF_INT128__)
    unsigned __int128 r = 0;
    unsigned __int128 a128 = a;
    while (b > 0) {
        if (b & 1) {
            r ^= a128;
        }
        a128 <<= 1;
        b >>= 1;
    }
    *hi = (uint64_t)(r >> 64);
    *lo = (uint64_t)r;
#else
    uint64_t r_hi = 0, r_lo = 0;
    for (int i = 0; i < 64; ++i) {
        if ((b >> i) & 1ULL) {
            r_lo ^= (a << i);
            if (i > 0) {
                r_hi ^= (a >> (64 - i));
            }
        }
    }
    *hi = r_hi;
    *lo = r_lo;
#endif
}

static inline int clz64(uint64_t v) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_clzll(v);
#elif defined(_MSC_VER)
    unsigned long index;
    if (_BitScanReverse64(&index, v)) {
        return 63 - (int)index;
    }
    return 64;
#else
    if (v == 0) return 64;
    int n = 0;
    if ((v & 0xFFFFFFFF00000000ULL) == 0) { n += 32; v <<= 32; }
    if ((v & 0xFFFF000000000000ULL) == 0) { n += 16; v <<= 16; }
    if ((v & 0xFF00000000000000ULL) == 0) { n += 8;  v <<= 8;  }
    if ((v & 0xF000000000000000ULL) == 0) { n += 4;  v <<= 4;  }
    if ((v & 0xC000000000000000ULL) == 0) { n += 2;  v <<= 2;  }
    if ((v & 0x8000000000000000ULL) == 0) { n += 1; }
    return n;
#endif
}

static uint64_t poly_mod_64(uint64_t v_hi, uint64_t v_lo, uint64_t poly_lo) {
    while (v_hi != 0) {
        int lz = clz64(v_hi);
        int deg = 127 - lz;
        int shift = deg - 64;

        if (shift >= 64) {
            int s = shift - 64;
            v_hi ^= (1ULL << s);
        } else if (shift == 0) {
            v_hi ^= 1ULL;
            v_lo ^= poly_lo;
        } else {
            v_hi ^= (1ULL << shift) | (poly_lo >> (64 - shift));
            v_lo ^= (poly_lo << shift);
        }
    }
    return v_lo;
}

static inline uint64_t gf_mul_64(uint64_t a, uint64_t b, uint64_t poly_lo) {
    uint64_t hi, lo;
    clmul_64(a, b, &hi, &lo);
    return poly_mod_64(hi, lo, poly_lo);
}

uint64_t qids_toeplitz_hash_64(const uint8_t *data, size_t len, uint64_t poly_lo, uint64_t seed) {
    // 1. Bit reverse data + injectivity terminator 0x01 + pad to multiple of 8
    size_t padded_len = ((len + 1 + 7) / 8) * 8;
    uint8_t *buf = (uint8_t*)calloc(padded_len, sizeof(uint8_t));
    if (!buf) {
        return 0;
    }

    for (size_t i = 0; i < len; ++i) {
        buf[i] = REVERSE_BYTE_TABLE[data[i]];
    }
    buf[len] = 0x01; // injectivity terminator bit

    size_t num_chunks = padded_len / 8;
    uint64_t *chunks = (uint64_t*)malloc(num_chunks * sizeof(uint64_t));
    if (!chunks) {
        free(buf);
        return 0;
    }

    for (size_t i = 0; i < num_chunks; ++i) {
        uint64_t val = 0;
        for (int b = 0; b < 8; ++b) {
            val |= ((uint64_t)buf[i * 8 + b]) << (b * 8);
        }
        chunks[i] = val;
    }
    free(buf);

    // 2. Horner evaluation from highest degree down to 0
    uint64_t x_n = poly_lo;
    uint64_t acc = 0;
    for (size_t i = num_chunks; i > 0; --i) {
        acc = gf_mul_64(acc, x_n, poly_lo) ^ chunks[i - 1];
    }
    free(chunks);

    return gf_mul_64(acc, seed, poly_lo);
}

/* ========================================================================= */
/* 3. Wald SPRT Physical-Layer Threat Detector                               */
/* ========================================================================= */

struct QidsSprtDetector {
    double p0;
    double p1;
    double alpha;
    double beta;
    double llr;
    size_t n_samples;
    size_t n_errors;
    QidsSprtState state;
    double log_err_ratio;
    double log_ok_ratio;
    double upper_bound;
    double lower_bound;
};

QidsSprtDetector* qids_sprt_new(double p0, double p1, double alpha, double beta) {
    if (!(0.0 <= p0 && p0 < p1 && p1 <= 1.0)) {
        return NULL;
    }
    if (!(0.0 < alpha && alpha < 1.0 && 0.0 < beta && beta < 1.0)) {
        return NULL;
    }

    QidsSprtDetector *det = (QidsSprtDetector*)malloc(sizeof(QidsSprtDetector));
    if (!det) {
        return NULL;
    }

    double q0 = (p0 < 1e-6) ? 1e-6 : p0;
    double q1 = (p1 > 1.0 - 1e-6) ? (1.0 - 1e-6) : p1;

    det->p0 = p0;
    det->p1 = p1;
    det->alpha = alpha;
    det->beta = beta;
    det->llr = 0.0;
    det->n_samples = 0;
    det->n_errors = 0;
    det->state = QIDS_SPRT_CONTINUE;

    det->log_err_ratio = log(q1 / q0);
    det->log_ok_ratio = log((1.0 - q1) / (1.0 - q0));
    det->upper_bound = log((1.0 - beta) / alpha);
    det->lower_bound = log(beta / (1.0 - alpha));

    return det;
}

QidsSprtState qids_sprt_update(QidsSprtDetector *detector, bool is_error) {
    if (!detector || detector->state != QIDS_SPRT_CONTINUE) {
        return detector ? detector->state : QIDS_SPRT_CONTINUE;
    }

    detector->n_samples++;
    if (is_error) {
        detector->n_errors++;
        detector->llr += detector->log_err_ratio;
    } else {
        detector->llr += detector->log_ok_ratio;
    }

    if (detector->llr >= detector->upper_bound) {
        detector->state = QIDS_SPRT_ACCEPT_H1;
    } else if (detector->llr <= detector->lower_bound) {
        detector->state = QIDS_SPRT_ACCEPT_H0;
    }

    return detector->state;
}

double qids_sprt_get_llr(const QidsSprtDetector *detector) {
    return detector ? detector->llr : 0.0;
}

void qids_sprt_reset(QidsSprtDetector *detector) {
    if (detector) {
        detector->llr = 0.0;
        detector->n_samples = 0;
        detector->n_errors = 0;
        detector->state = QIDS_SPRT_CONTINUE;
    }
}

void qids_sprt_free(QidsSprtDetector *detector) {
    if (detector) {
        free(detector);
    }
}
