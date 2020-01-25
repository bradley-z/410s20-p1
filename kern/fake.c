/** @file fake.c
 *
 *  @brief Skeleton implementation of device-driver library.
 *  This file exists so that the "null project" builds and
 *  links without error.
 *
 *  @author Harry Q. Bovik (hbovik) <-- change this
 *  @bug This file should shrink repeatedly AND THEN BE DELETED.
 */

#include <p1kern.h>
#include <stdio.h>

int handler_install(void (*tickback)(unsigned int))
{
  return -1;
}

int
readchar(void)
{
  return -1;
}
