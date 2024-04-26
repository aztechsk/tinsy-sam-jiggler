/*
 * tm.c
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
#include "cmdln.h"
#include "ledui.h"
#include "wd.h"
#include "udp.h"
#include "jiggler.h"
#include "sleep.h"
#include "main.h"
#include "tm.h"

static TaskHandle_t tsk_hndl;
static const char *tsk_nm = "TM";
static int uptm;
static void (*clbk_arr[TIME_BASE_CLBK_ARRAY_SIZE])(unsigned int);
static volatile boolean_t sleep_req;
static boolean_t diswd;

static void tm_tsk(void *p);
static void sleep_clbk(enum sleep_cmd cmd, ...);
static void cmd_diswd(void);

/**
 * init_tm
 */
void init_tm(void)
{
        if (pdPASS != xTaskCreate(tm_tsk, tsk_nm, TM_TASK_STACK_SIZE, NULL,
                                  TM_TASK_PRIO, &tsk_hndl)) {
                crit_err_exit(MALLOC_ERROR);
        }
	add_command_noargs("diswd", cmd_diswd);
	reg_sleep_clbk(sleep_clbk, SLEEP_PRIO_SUSP_FIRST);
}

/**
 * tm_tsk
 */
static void tm_tsk(void *p)
{
	static TickType_t xLastWakeTime;
	static int cnt = 1000 / TIME_BASE_MS;
	static unsigned int tmbs;

	vTaskDelay(20 / portTICK_PERIOD_MS);
	set_ledui_all_leds_state(LEDUI_LED_ON);
	vTaskDelay(150 / portTICK_PERIOD_MS);
	set_ledui_all_leds_state(LEDUI_LED_OFF);
	udp_pullup_on();
        init_jiggler();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        init_wd();
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		vTaskDelayUntil(&xLastWakeTime, TIME_BASE_MS / portTICK_PERIOD_MS);
		if (sleep_req) {
			sleep_req = FALSE;
#if SLEEP_LOG_STATE == 1
                        msg(INF, "tm.c: %s suspended\n", tsk_nm);
#endif
			vTaskSuspend(NULL);
#if SLEEP_LOG_STATE == 1
			msg(INF, "tm.c: %s resumed\n", tsk_nm);
#endif
                        cnt = 1000 / TIME_BASE_MS;
                        xLastWakeTime = xTaskGetTickCount();
			continue;
		}
		if (!diswd) {
			wd_rst();
		}
		if (!--cnt) {
			cnt = 1000 / TIME_BASE_MS;
			uptm++;
		}
                tmbs++;
		for (int i = 0; i < TIME_BASE_CLBK_ARRAY_SIZE; i++) {
			if (!clbk_arr[i]) {
				break;
			} else {
				(*clbk_arr[i])(tmbs);
			}
		}
	}
}

/**
 * get_uptm
 */
int get_uptm(void)
{
	return (uptm);
}

/**
 * add_tm_clbk
 */
boolean_t add_tm_clbk(void (*clbk)(unsigned int))
{
	taskENTER_CRITICAL();
	for (int i = 0; i < TIME_BASE_CLBK_ARRAY_SIZE; i++) {
		if (!clbk_arr[i]) {
			clbk_arr[i] = clbk;
                        taskEXIT_CRITICAL();
			return (TRUE);
		}
	}
        taskEXIT_CRITICAL();
        return (FALSE);
}

/**
 * sleep_clbk
 */
static void sleep_clbk(enum sleep_cmd cmd, ...)
{
	if (cmd == SLEEP_CMD_SUSP) {
		sleep_req = TRUE;
		while (eSuspended != eTaskGetState(tsk_hndl)) {
			taskYIELD();
		}
	} else {
		vTaskResume(tsk_hndl);
	}
}

/**
 * cmd_diswd
 */
static void cmd_diswd(void)
{
	msg(INF, cmd_accp);
	diswd = TRUE;
}
