/**
 * @file test_qids.c
 * @brief Comprehensive Verification Test Suite for QIDS C SDK.
 *
 * Copyright (c) 2026 QIDS. All Rights Reserved.
 * Strictly Proprietary and Confidential.
 */

#include "qids.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_version(void) {
    printf("[*] Testing version macros agree...\n");
    /* v1.3.3 shipped with QIDS_VERSION_PATCH 2 beside QIDS_VERSION_STRING "1.3.3". */
    char built[32];
    snprintf(built, sizeof built, "%d.%d.%d",
             QIDS_VERSION_MAJOR, QIDS_VERSION_MINOR, QIDS_VERSION_PATCH);
    if (strcmp(built, QIDS_VERSION_STRING) != 0 || strcmp(qids_version(), QIDS_VERSION_STRING) != 0) {
        fprintf(stderr, "version mismatch: macros say %s, string says %s, qids_version() says %s\n",
                built, QIDS_VERSION_STRING, qids_version());
        exit(1);
    }
    printf("    [+] %s\n", built);
}

static void test_constant_time(void) {
    printf("[*] Testing C Constant-Time Primitives...\n");

    uint8_t a[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
    uint8_t b[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
    uint8_t c[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x05}; // 1 bit mismatch

    // Equality check
    assert(qids_ct_eq(a, b, sizeof(a)) == true);
    assert(qids_ct_eq(a, c, sizeof(a)) == false);

    // Hamming distance check
    assert(qids_ct_distance(a, b, sizeof(a)) == 0);
    assert(qids_ct_distance(a, c, sizeof(a)) == 1);

    // Threshold check
    assert(qids_ct_threshold(4, 5) == true);
    assert(qids_ct_threshold(5, 5) == true);
    assert(qids_ct_threshold(6, 5) == false);

    printf("    -> Constant-time verification PASSED.\n");
}

static void test_toeplitz_hash(void) {
    printf("[*] Testing 64-bit Toeplitz LFSR Universal Hashing...\n");

    const char *msg1 = "FINANCIAL_TRANSACTION_ORDER_2026";
    const char *msg2 = "FINANCIAL_TRANSACTION_ORDER_2027";
    uint64_t poly_lo = 0x1B; // x^64 + x^4 + x^3 + x + 1
    uint64_t seed = 0xCAFEBABE12345678ULL;

    uint64_t h1 = qids_toeplitz_hash_64((const uint8_t*)msg1, strlen(msg1), poly_lo, seed);
    uint64_t h2 = qids_toeplitz_hash_64((const uint8_t*)msg1, strlen(msg1), poly_lo, seed);
    uint64_t h_alt = qids_toeplitz_hash_64((const uint8_t*)msg2, strlen(msg2), poly_lo, seed);

    // Determinism
    assert(h1 == h2);
    assert(h1 != 0);

    // Avalanche effect / uniqueness
    assert(h1 != h_alt);

    printf("    -> Toeplitz LFSR Hash PASSED (Digest: 0x%016llX).\n", (unsigned long long)h1);
}

static void test_sprt_detector(void) {
    printf("[*] Testing Wald SPRT Physical-Layer Threat Detector...\n");

    // p0 = 0.02 (honest), p1 = 0.25 (intercept-resend), alpha = 1e-4, beta = 1e-4
    QidsSprtDetector *det = qids_sprt_new(0.02, 0.25, 1e-4, 1e-4);
    assert(det != NULL);

    // Attack sequence: 1 error every 4 measurements
    bool attack_stream[] = {
        true, false, false, false,
        true, false, false, false,
        true, false, false, false,
        true, false, false, false,
        true, false, false, false,
        true, false, false, false
    };
    size_t count = sizeof(attack_stream) / sizeof(attack_stream[0]);

    QidsSprtState state = QIDS_SPRT_CONTINUE;
    for (size_t i = 0; i < count; i++) {
        state = qids_sprt_update(det, attack_stream[i]);
        if (state != QIDS_SPRT_CONTINUE) {
            break;
        }
    }

    assert(state == QIDS_SPRT_ACCEPT_H1);
    assert(qids_sprt_get_llr(det) > 0.0);

    // Reset and honest sequence: 0 errors
    qids_sprt_reset(det);
    assert(qids_sprt_get_llr(det) == 0.0);

    for (int i = 0; i < 40; i++) {
        state = qids_sprt_update(det, false);
        if (state != QIDS_SPRT_CONTINUE) {
            break;
        }
    }
    assert(state == QIDS_SPRT_ACCEPT_H0);

    qids_sprt_free(det);
    printf("    -> Wald SPRT Detector Lifecycle PASSED.\n");
}

int main(void) {
    printf("=====================================================\n");
    printf("  QIDS C SDK v%s Verification Test Suite          \n", qids_version());
    printf("=====================================================\n");

    test_version();
    test_constant_time();
    test_toeplitz_hash();
    test_sprt_detector();

    printf("\n>>> ALL QIDS C SDK TESTS PASSED SUCCESSFULLY <<<\n");
    return 0;
}
