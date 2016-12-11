#ifndef _USBD_ERROR_H
#define _USBD_ERROR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f10x.h>
#include <board.h>

/* Obsługa błędów, których przyczyna może ustąpić po wyzerowaniu. */
void usbd_resetableError(unsigned err);

#ifdef __cplusplus
}
#endif

#endif
