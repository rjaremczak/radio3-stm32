#pragma once

#include <license.h>

namespace license {
    static const uint32_t licenseEncryptionKey[] = {0xc1ffafc7, 0x95af7ad4, 0x9ac24b60, 0x5a245117,
                                                    0x5ec07746, 0x04a55c10, 0x8de18894, 0x99c6bad9,
                                                    0x01a347fe, 0xf87a01cd, 0xec0167a2, 0x3f29a42c,
                                                    0x0321fe98, 0xfe69f6be, 0x6212c122, 0x90b974a1};

    static constexpr LicenseData crypt(LicenseData inputData) {
        const auto *key = reinterpret_cast<const uint8_t *>(licenseEncryptionKey);
        auto *source = reinterpret_cast<uint8_t *>(&inputData);
        int size = sizeof(inputData);
        for (int i = 0; i < size; i++) source[i] = source[i] ^ key[i] ^ static_cast<uint8_t>(0xa3 - i);
        return inputData;
    }
}

