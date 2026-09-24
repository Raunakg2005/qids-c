/**
 * @file qids.h
 * @brief C99/C++ Client API for Quantum Intrusion Detection System (QIDS).
 *
 * Provides carrier-grade constant-time cryptographic comparisons,
 * 64-bit Galois field Toeplitz LFSR universal hashing, and real-time
 * Wald Sequential Probability Ratio Test (SPRT) physical-layer threat detection.
 *
 * Copyright (c) 2026 QIDS. All Rights Reserved.
 * Strictly Proprietary and Confidential.
 */

#ifndef QIDS_H
#define QIDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QIDS_VERSION_MAJOR 1
#define QIDS_VERSION_MINOR 3
#define QIDS_VERSION_PATCH 2
#define QIDS_VERSION_STRING "1.3.3"

/**
 * @brief Constant-time byte equality check.
 *
 * Execution time is strictly data-independent to eliminate microarchitectural
 * cache-timing side-channels.
 *
 * @param a Pointer to first byte buffer.
 * @param b Pointer to second byte buffer.
 * @param len Length of buffers in bytes.
 * @return true if buffers are identical, false otherwise.
 */
bool qids_ct_eq(const uint8_t *a, const uint8_t *b, size_t len);

/**
 * @brief Constant-time Hamming distance between two byte buffers.
 *
 * @param a Pointer to first byte buffer.
 * @param b Pointer to second byte buffer.
 * @param len Length of buffers in bytes.
 * @return Total number of bit mismatches (Hamming distance), or -1 if invalid.
 */
int64_t qids_ct_distance(const uint8_t *a, const uint8_t *b, size_t len);

/**
 * @brief Constant-time threshold verification.
 *
 * @param observed_mismatches Number of observed bit mismatches.
 * @param max_allowed Maximum permitted threshold (e.g. s_a or s_v).
 * @return true if observed <= max_allowed, false otherwise.
 */
bool qids_ct_threshold(size_t observed_mismatches, size_t max_allowed);

/**
 * @brief 64-bit Toeplitz LFSR universal hash over GF(2^64).
 *
 * Evaluates Krawczyk's almost-universal hash family via Horner's rule
 * with injectivity termination bit 0x01.
 *
 * @param data Pointer to input message bytes.
 * @param len Length of message bytes.
 * @param poly_lo Lower 64 bits of irreducible polynomial (x^64 + poly_lo).
 * @param seed Random 64-bit universal hash key/seed.
 * @return 64-bit universal hash digest.
 */
uint64_t qids_toeplitz_hash_64(const uint8_t *data, size_t len, uint64_t poly_lo, uint64_t seed);

/**
 * @brief SPRT decision states.
 */
typedef enum {
    QIDS_SPRT_CONTINUE = 0,   /**< Insufficient evidence; continue sampling */
    QIDS_SPRT_ACCEPT_H0 = 1,  /**< Honest link accepted (normal physical noise) */
    QIDS_SPRT_ACCEPT_H1 = 2   /**< Threat detected: early alarm triggered */
} QidsSprtState;

/** Opaque handle to a Wald SPRT detector instance. */
typedef struct QidsSprtDetector QidsSprtDetector;

/**
 * @brief Allocate a new Wald SPRT detector.
 *
 * @param p0 Expected honest error rate (e.g., 0.02 for standard optical link).
 * @param p1 Attack threshold error rate (e.g., 0.1111 for QBER threshold).
 * @param alpha Allowed Type I false-alarm probability (e.g., 1e-4).
 * @param beta Allowed Type II missed-detection probability (e.g., 1e-4).
 * @return Pointer to detector instance, or NULL on invalid parameters.
 */
QidsSprtDetector* qids_sprt_new(double p0, double p1, double alpha, double beta);

/**
 * @brief Feed a single measurement outcome into the detector.
 *
 * @param detector Pointer to detector instance.
 * @param is_error true if error/mismatch observed, false if match.
 * @return Updated SPRT decision state.
 */
QidsSprtState qids_sprt_update(QidsSprtDetector *detector, bool is_error);

/**
 * @brief Query current cumulative log-likelihood ratio (LLR).
 */
double qids_sprt_get_llr(const QidsSprtDetector *detector);

/**
 * @brief Reset detector for a new session.
 */
void qids_sprt_reset(QidsSprtDetector *detector);

/**
 * @brief Free allocated detector resources.
 */
void qids_sprt_free(QidsSprtDetector *detector);

/**
 * @brief Returns the version string of the QIDS C library.
 */
const char* qids_version(void);

#ifdef __cplusplus
}
#endif

#endif /* QIDS_H */
