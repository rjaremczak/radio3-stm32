#include <license.h>
#include <crypt.h>

namespace license {
    const LicenseData encryptedLicenseData = crypt(
            LicenseData{"radio3 by SQ6DGT", "Robert Jaremczak SQ6DGT", 0x1234abcd, 0x2345bcde, 0x3456cdef});
}
