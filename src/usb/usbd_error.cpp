//
// Created by Robert Jaremczak on 2016.12.11.
//

#include "usbd_error.h"

/* Obsługa błędów, których przyczyna może ustąpić po wyzerowaniu. */
void usbd_resetableError(unsigned err) {
    for (int i = 0; i < 3; ++i) { board_indicateError(err); }
    NVIC_SystemReset();
}
