#ifndef _USBD_ERROR_H
#define _USBD_ERROR_H 1

#include <stm32f10x.h>

/* Obsługa błędów, których przyczyna może ustąpić po wyzerowaniu. */
void usbd_resetableError(unsigned err);

#endif
