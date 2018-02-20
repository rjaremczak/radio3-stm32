#pragma once

#include <cstdint>
#include <ElectronicSignature.h>

namespace license {
    struct LicenseData {
        char product[24];
        char owner[24];
        uint32_t uniqueId0;
        uint32_t uniqueId1;
        uint32_t uniqueId2;
        bool verified() { return uniqueId0==core::uniqueId0() && uniqueId1==core::uniqueId1() && uniqueId2==core::uniqueId2(); }
    } __attribute__((packed));

    extern const LicenseData encryptedLicenseData;
}
