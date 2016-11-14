/*
 * Radio3 - general purpose tool for hams
 * Author: Robert Jaremczak, SQ6DGT
 *
 * Application's entry point
 *
 */

#include "radio3.h"
#include "diag/Trace.h"

void main(void)
{
	radio3_init();
	radio3_start();
}
