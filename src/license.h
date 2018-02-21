#pragma once

#include <cstdint>
#include <ElectronicSignature.h>

struct LicenseData {
    uint32_t uniqueId0;
    char product[24];
    uint32_t uniqueId1;
    char owner[24];
    uint32_t uniqueId2;
} __attribute__((packed));

static const uint32_t licenseEncryptionKey[] = {0xc1ffafc7, 0x95af7ad4, 0x9ac24b60, 0x5a245117,
                                                0x5ec07746, 0x04a55c10, 0x8de18894, 0x99c6bad9,
                                                0x01a347fe, 0xf87a01cd, 0xec0167a2, 0x3f29a42c,
                                                0x0321fe98, 0xfe69f6be, 0x6212c122, 0x90b974a1};

static void decryptLicenseData(const uint8_t *inputData, uint8_t *outputData) {
    const auto *key = reinterpret_cast<const uint8_t *>(licenseEncryptionKey);
    int size = sizeof(LicenseData);
    for (int i = 0; i < size; i++) outputData[i] = inputData[i] ^ key[i] ^ static_cast<uint8_t>(0xa3 - i);
}
