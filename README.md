# QIDS C SDK (`libqids`)

[![Version](https://img.shields.io/badge/version-v1.3.3-blue.svg)](https://github.com/Raunakg2005/qids-c)
[![License](https://img.shields.io/badge/license-Proprietary-red.svg)](LICENSE)
[![Standards](https://img.shields.io/badge/standard-C99%20%7C%20C%2B%2B11-emerald.svg)](include/qids.h)

C99 primitive library for the **QIDS** quantum digital signature stack: constant-time comparison, 64-bit Toeplitz universal hashing, and a Wald SPRT detector.

It has no network code and does not sign documents. Signing and verification happen on a QIDS gateway, whose one-time universal-hash key is what makes a tag unforgeable. For gateway clients, see the Python, TypeScript, Go and Java SDKs.

---

## Features

- **Constant-time comparison**: byte equality, Hamming distance and threshold checks with no data-dependent branches (`qids_ct_eq`, `qids_ct_distance`, `qids_ct_threshold`).
- **64-bit Toeplitz universal hashing**: carry-less multiplication over GF(2^64) (`qids_toeplitz_hash_64`). This is a keyed hash, and it is only as secret as the polynomial and seed you pass it.
- **Wald SPRT detector**: a sequential probability ratio test for error-rate streams such as QBER (`qids_sprt_new`, `qids_sprt_update`).
- **No runtime dependencies**: standard C99, linked with `libm`.

---

## Installation & Integration

### 1. CMake FetchContent (Recommended)

Add to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    qids
    GIT_REPOSITORY https://github.com/Raunakg2005/qids-c.git
    GIT_TAG        v1.3.3
)
FetchContent_MakeAvailable(qids)

target_link_libraries(your_application PRIVATE qids::qids)
```

### 2. Manual Build (GNU Make)

```bash
git clone https://github.com/Raunakg2005/qids-c.git
cd qids-c
make
make test
sudo cp libqids.a /usr/local/lib/
sudo cp include/qids.h /usr/local/include/
```

---

## Quickstart

```c
#include <qids.h>
#include <stdio.h>

int main(void) {
    printf("QIDS C Library Version: %s\n", qids_version());

    // 1. Constant-time comparison
    uint8_t tag_a[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t tag_b[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    if (qids_ct_eq(tag_a, tag_b, 8)) {
        printf("Tags match under constant-time verification.\n");
    }

    // 2. Toeplitz universal hash (keyed: poly_lo and seed are the key)
    const char *payload = "TRANSACTION_PAYLOAD_DATA";
    uint64_t poly_lo = 0x1B; // x^64 + x^4 + x^3 + x + 1
    uint64_t seed = 0xCAFEBABE12345678ULL;
    uint64_t digest = qids_toeplitz_hash_64((const uint8_t*)payload, 24, poly_lo, seed);
    printf("Hash digest: 0x%016llX\n", (unsigned long long)digest);

    // 3. Wald SPRT detector
    QidsSprtDetector *det = qids_sprt_new(0.02, 0.25, 1e-4, 1e-4);
    QidsSprtState state = qids_sprt_update(det, false);
    if (state == QIDS_SPRT_CONTINUE) {
        printf("Channel state normal, sampling continuing.\n");
    }
    qids_sprt_free(det);

    return 0;
}
```

---

## License

Strictly **Proprietary and Confidential**. Copyright &copy; 2026 QIDS. All Rights Reserved.
Unauthorized copying, decompilation, or redistribution is strictly prohibited. See [LICENSE](LICENSE).
