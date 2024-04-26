/*
 * board.h
 *
 * Autors: Jan Rusnak.
 * (c) 2024 AZTech.
 */

#ifndef BOARD_H
#define BOARD_H

#if defined(TINSY_SAM_BOARD)
////////////////////////////////////////////////////////////////////////////////
// PIO
#define PIOA_CLOCK 1
#define PIOA_DEBOUNCE_FILTER_MS 1
#define PIOA_DEBOUNCE_FILTER_US 100
#define PIOB_CLOCK 0

////////////////////////////////////////////////////////////////////////////////
// LEDUI
#define LEDUI1_IO_PIN PIO_PB0
#define LEDUI1_IO_CONT PIOB
#define LEDUI2_IO_PIN PIO_PB1
#define LEDUI2_IO_CONT PIOB
#define LEDUI3_IO_PIN PIO_PB2
#define LEDUI3_IO_CONT PIOB
#define LEDUI4_IO_PIN PIO_PB3
#define LEDUI4_IO_CONT PIOB
#define LEDUI_ANODE_ON_IO_PIN 0

////////////////////////////////////////////////////////////////////////////////
// RGB
#define RGB_R_IO_PIN PIO_PA18
#define RGB_R_IO_CONT PIOA
#define RGB_G_IO_PIN PIO_PA19
#define RGB_G_IO_CONT PIOA
#define RGB_B_IO_PIN PIO_PA17
#define RGB_B_IO_CONT PIOA

////////////////////////////////////////////////////////////////////////////////
// PSU
#define PWR_ON_IO_PIN PIO_PA20
#define PWR_ON_IO_CONT PIOA

////////////////////////////////////////////////////////////////////////////////
// RSTC
#define RSTC_USER_RESET_ENABLED 1

////////////////////////////////////////////////////////////////////////////////
// CKGR
#define CKGR_PLL_LOCK_COUNT 10
#define CKGR_XTAL_STARTUP_TM 8
#define CKGR_PLLA_MUL 31
#define CKGR_PLLA_DIV 3

////////////////////////////////////////////////////////////////////////////////
// MATRIX
#define MATRIX_CCFG_SYSIO 0

////////////////////////////////////////////////////////////////////////////////
// USB_UDP
#define USB_UDP_PLL_UNIT PLL_UNIT_B
#define USB_UDP_PLL_MUL 7
#define USB_UDP_PLL_DIV 1
#define USB_UDP_PLL_DIV2 FALSE
#define USB_UDP_USBCLK_DIV 1

////////////////////////////////////////////////////////////////////////////////
// JIGBTN
#define JIGBTN_PIN PIO_PA8
#define JIGBTN_CONT PIOA

#else
 #error "Board type not defined"
#endif

#endif
