/*
 * appver_tinsy.h
 *
 * Autors: Jan Rusnak.
 * (c) 2024 AZTech.
 */

#ifndef APP_VER_H
#define APP_VER_H

#define VERSION_NUMBER_STRING "01.00"

#if defined(MAKE_VERSION_CODE)
const char *const app_ver = "tinsy-sam-jiggler v"
                            VERSION_NUMBER_STRING
                            "\n";
#else
extern const char *const app_ver;
#endif

#endif
