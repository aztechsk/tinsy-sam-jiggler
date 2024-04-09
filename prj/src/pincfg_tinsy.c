/*
 * pincfg_tinsy.c
 *
 * Autors: Jan Rusnak.
 * (c) 2024 AZTech.
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <gentyp.h>
#include "sysconf.h"
#include "board.h"
#include <mmio.h>
#include "msgconf.h"
#include "criterr.h"
#include "pincfg.h"

/**
 * pincfg
 */
void pincfg(enum pincfg cfg)
{
	switch (cfg) {
	case PINCFG_INIT :
		// P01 - ADVREF
		// P02 - GND
		// P03 - PB0/AD4
		// P04 - PB1/AD5
		// P05 - PB2/AD6
		// P06 - PB3/AD7
		// P07 - VDDIN
		// P08 - VDDOUT
		// P09 - PA17/PGMD5/AD0
		// P10 - PA18/PGMD6/AD1
		// P11 - PA19/PGMD7/AD2
		// P12 - PA20/AD3
		// P13 - VDDIO
		// P14 - PA16/PGMD4
		// P15 - PA15/PGMD3
		// P16 - PA14/PGMD2
		// P17 - PA13/PGMD1
		// P18 - VDDCORE
		// P19 - PA12/PGMD0
		// P20 - PA11/PGMM3
		// P21 - PA10/PGMM2
		// P22 - PA9/PGMM1
		// P23 - PA8/XOUT32/PGMM0
		// P24 - PA7/XIN32/PGMNVALID
		// P25 - TDI/PB4
		// P26 - PA6/PGMNOE
		// P27 - PA5/PGMRDY
		// P28 - PA4/PGMNCMD
		// P29 - NRST
		// P30 - TST
		// P31 - PA3
		// P32 - PA2/PGMEN2
		// P33 - VDDIO
		// P34 - GND
		// P35 - PA1/PGMEN1
		// P36 - PA0/PGMEN0
		// P37 - TDO/TRACESWO/PB5
		// P38 - JTAGSEL
		// P39 - TMS/SWDIO/PB6
		// P40 - TCK/SWCLK/PB7
		// P41 - VDDCORE
		// P42 - ERASE/PB12
		// P43 - DDM/PB10
		// P44 - DDP/PB11
		// P45 - XOUT/PB8
		// P46 - XIN/PB9/PGMCK
		// P47 - VDDIO
		// P48 - VDDPLL
		break;
	case PINCFG_SLEEP :
		// P01 - ADVREF
		// P02 - GND
		// P03 - PB0/AD4
		// P04 - PB1/AD5
		// P05 - PB2/AD6
		// P06 - PB3/AD7
		// P07 - VDDIN
		// P08 - VDDOUT
		// P09 - PA17/PGMD5/AD0
		// P10 - PA18/PGMD6/AD1
		// P11 - PA19/PGMD7/AD2
		// P12 - PA20/AD3
		// P13 - VDDIO
		// P14 - PA16/PGMD4
		// P15 - PA15/PGMD3
		// P16 - PA14/PGMD2
		// P17 - PA13/PGMD1
		// P18 - VDDCORE
		// P19 - PA12/PGMD0
		// P20 - PA11/PGMM3
		// P21 - PA10/PGMM2
		// P22 - PA9/PGMM1
		// P23 - PA8/XOUT32/PGMM0
		// P24 - PA7/XIN32/PGMNVALID
		// P25 - TDI/PB4
		// P26 - PA6/PGMNOE
		// P27 - PA5/PGMRDY
		// P28 - PA4/PGMNCMD
		// P29 - NRST
		// P30 - TST
		// P31 - PA3
		// P32 - PA2/PGMEN2
		// P33 - VDDIO
		// P34 - GND
		// P35 - PA1/PGMEN1
		// P36 - PA0/PGMEN0
		// P37 - TDO/TRACESWO/PB5
		// P38 - JTAGSEL
		// P39 - TMS/SWDIO/PB6
		// P40 - TCK/SWCLK/PB7
		// P41 - VDDCORE
		// P42 - ERASE/PB12
		// P43 - DDM/PB10
		// P44 - DDP/PB11
		// P45 - XOUT/PB8
		// P46 - XIN/PB9/PGMCK
		// P47 - VDDIO
		// P48 - VDDPLL
		break;
	case PINCFG_WAKE :
		// P01 - ADVREF
		// P02 - GND
		// P03 - PB0/AD4
		// P04 - PB1/AD5
		// P05 - PB2/AD6
		// P06 - PB3/AD7
		// P07 - VDDIN
		// P08 - VDDOUT
		// P09 - PA17/PGMD5/AD0
		// P10 - PA18/PGMD6/AD1
		// P11 - PA19/PGMD7/AD2
		// P12 - PA20/AD3
		// P13 - VDDIO
		// P14 - PA16/PGMD4
		// P15 - PA15/PGMD3
		// P16 - PA14/PGMD2
		// P17 - PA13/PGMD1
		// P18 - VDDCORE
		// P19 - PA12/PGMD0
		// P20 - PA11/PGMM3
		// P21 - PA10/PGMM2
		// P22 - PA9/PGMM1
		// P23 - PA8/XOUT32/PGMM0
		// P24 - PA7/XIN32/PGMNVALID
		// P25 - TDI/PB4
		// P26 - PA6/PGMNOE
		// P27 - PA5/PGMRDY
		// P28 - PA4/PGMNCMD
		// P29 - NRST
		// P30 - TST
		// P31 - PA3
		// P32 - PA2/PGMEN2
		// P33 - VDDIO
		// P34 - GND
		// P35 - PA1/PGMEN1
		// P36 - PA0/PGMEN0
		// P37 - TDO/TRACESWO/PB5
		// P38 - JTAGSEL
		// P39 - TMS/SWDIO/PB6
		// P40 - TCK/SWCLK/PB7
		// P41 - VDDCORE
		// P42 - ERASE/PB12
		// P43 - DDM/PB10
		// P44 - DDP/PB11
		// P45 - XOUT/PB8
		// P46 - XIN/PB9/PGMCK
		// P47 - VDDIO
		// P48 - VDDPLL
		break;
	}
}
