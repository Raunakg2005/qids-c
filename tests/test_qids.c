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

/* Known answers recorded from the v1.3.4 build, whose carry-less multiply
 * used unsigned __int128 on GCC and Clang. The portable 64-bit path that
 * replaced it must give the same hash for every input. */
static void test_toeplitz_known_answers(void) {
    static const char *msgs[] = {"", "a", "QIDS", "TRANSFER 1,000,000 USD TO ACCT-48910",
        "The quick brown fox jumps over the lazy dog, twice over: the quick brown fox."};
    static const uint64_t polys[] = {0x1BULL, 0xFFFFFFFFFFFFFFFFULL, 0x8000000000000001ULL};
    static const uint64_t seeds[] = {0ULL, 0xDEADBEEFCAFEBABEULL, 0xFFFFFFFFFFFFFFFFULL};
    static const struct { int msg, poly, seed; uint64_t hash; } kat[] = {
    {0, 0, 0, 0x0000000000000000ULL},
    {0, 0, 1, 0xDEADBEEFCAFEBABEULL},
    {0, 0, 2, 0xFFFFFFFFFFFFFFFFULL},
    {0, 1, 0, 0x0000000000000000ULL},
    {0, 1, 1, 0xDEADBEEFCAFEBABEULL},
    {0, 1, 2, 0xFFFFFFFFFFFFFFFFULL},
    {0, 2, 0, 0x0000000000000000ULL},
    {0, 2, 1, 0xDEADBEEFCAFEBABEULL},
    {0, 2, 2, 0xFFFFFFFFFFFFFFFFULL},
    {1, 0, 0, 0x0000000000000000ULL},
    {1, 0, 1, 0x3C8C1E4F3FE071F9ULL},
    {1, 0, 2, 0x0000000000000D34ULL},
    {1, 1, 0, 0x0000000000000000ULL},
    {1, 1, 1, 0xC373E1B0C01F8122ULL},
    {1, 1, 2, 0x00000000000000C3ULL},
    {1, 2, 0, 0x0000000000000000ULL},
    {1, 2, 1, 0xBC8C1E4F3FE07E59ULL},
    {1, 2, 2, 0x000000000000007EULL},
    {2, 0, 0, 0x0000000000000000ULL},
    {2, 0, 1, 0x9DD7B0C8435C9D58ULL},
    {2, 0, 2, 0xFFFFFFF0DD287755ULL},
    {2, 1, 0, 0x0000000000000000ULL},
    {2, 1, 1, 0x9DD7B0C598E97A1AULL},
    {2, 1, 2, 0x00000000E5114945ULL},
    {2, 2, 0, 0x0000000000000000ULL},
    {2, 2, 1, 0x9DD7B0C5266BBE44ULL},
    {2, 2, 2, 0xFFFFFFFF975F7A28ULL},
    {3, 0, 0, 0x0000000000000000ULL},
    {3, 0, 1, 0xB613B2101E4467BAULL},
    {3, 0, 2, 0x33227A8B0923766DULL},
    {3, 1, 0, 0x0000000000000000ULL},
    {3, 1, 1, 0x93C74D1E2C9B5CA9ULL},
    {3, 1, 2, 0xA1D7F4934B40C33AULL},
    {3, 2, 0, 0x0000000000000000ULL},
    {3, 2, 1, 0x32A1592C1D083909ULL},
    {3, 2, 2, 0x8479A25427BA825EULL},
    {4, 0, 0, 0x0000000000000000ULL},
    {4, 0, 1, 0x89B39D1B7B3C839EULL},
    {4, 0, 2, 0xD1C5139FE4CF1698ULL},
    {4, 1, 0, 0x0000000000000000ULL},
    {4, 1, 1, 0xF124D87383518571ULL},
    {4, 1, 2, 0x58F58CD04A5CD526ULL},
    {4, 2, 0, 0x0000000000000000ULL},
    {4, 2, 1, 0x918D5E543F3921BCULL},
    {4, 2, 2, 0x2731D2E31E5434E7ULL}
    };
    size_t i;

    printf("[*] Testing Toeplitz hash known answers...\n");
    for (i = 0; i < sizeof kat / sizeof kat[0]; i++) {
        const char *m = msgs[kat[i].msg];
        uint64_t h = qids_toeplitz_hash_64((const uint8_t *)m, strlen(m),
                                           polys[kat[i].poly], seeds[kat[i].seed]);
        if (h != kat[i].hash) {
            fprintf(stderr, "known answer %u: got %016llx, want %016llx\n", (unsigned)i,
                    (unsigned long long)h, (unsigned long long)kat[i].hash);
            exit(1);
        }
    }
    printf("    -> %u known answers PASSED.\n", (unsigned)i);
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
    test_toeplitz_known_answers();
    test_sprt_detector();

    printf("\n>>> ALL QIDS C SDK TESTS PASSED SUCCESSFULLY <<<\n");
    return 0;
}
