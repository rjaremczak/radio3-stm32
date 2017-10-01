//
// Created by Robert Jaremczak on 2016.12.11.
//

#include "usbd_error.h"

void usbd_resetableError(unsigned err __attribute__ ((unused))) {
    // TODO: implement proper error indication logic
    // for (int i = 0; i < 3; ++i) { board_indicateError(err); }
    NVIC_SystemReset();
}
