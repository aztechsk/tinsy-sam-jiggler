/*
 * jiggler.c
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
#include "criterr.h"
#include "msgconf.h"
#include "hwerr.h"
#include "cmdln.h"
#include "ledui.h"
#include "udp.h"
#include "main.h"
#include "btn1.h"
#include "pmc.h"
#include "sleep.h"
#include "usb_ctl_req.h"
#include "usb_jiggler.h"
#include "tools.h"
#include "jiggler.h"
#include <stdlib.h>
#include <string.h>

#define UDP_IN_IRP_ERR_WAIT (500 / portTICK_PERIOD_MS)
#define BTN_PRESS_TIME (50 / portTICK_PERIOD_MS)
#define KEY_PRESS_TIME (50 / portTICK_PERIOD_MS)
#define JIG_DLY_TIME (10 / portTICK_PERIOD_MS)
#define CTL_STM_DFLT_SUSP_WAIT (10000 / portTICK_PERIOD_MS)
#define MV_POINTER_WAIT (10 / portTICK_PERIOD_MS)
#define JIG_NOSLEEP_TIME_CNT 1000
#define JIG_WHEEL_RND_MASK 0x1FF

enum axis {
	AXIS_X,
	AXIS_Y
};

enum m_event_type {
	POINTER,
	WHEEL,
	BUTTON
};

enum jig_type {
	JIG_WORK,
	JIG_NOSLEEP
};

struct pointer {
	enum m_event_type type;
	int8_t x;
	int8_t y;
};

struct wheel {
	enum m_event_type type;
	int8_t w;
};

struct button {
	enum m_event_type type;
	uint8_t bflags;
};

union m_event {
	enum m_event_type type;
	struct pointer pointer;
	struct wheel wheel;
	struct button button;
};

#if USB_JIG_KEYB_IFACE == 1
enum k_event_type {
	KPRES,
	KREL,
	KMOD
};

struct genkey {
	enum k_event_type type;
	uint8_t code;
};

struct modkey {
	enum k_event_type type;
	uint8_t bmp;
};

union k_event {
	enum k_event_type type;
	struct genkey genkey;
	struct modkey modkey;
};
#endif

static QueueHandle_t m_event_que, udp_que;
#if USB_JIG_KEYB_IFACE == 1
static QueueHandle_t k_event_que;
#endif
static TaskHandle_t ctl_hndl, m_inrep_hndl, jig_hndl;
#if USB_JIG_KEYB_IFACE == 1
static TaskHandle_t k_inrep_hndl;
#if LOG_KEYB_LEDS == 1
static TaskHandle_t k_led_hndl;
#endif
#endif
static uint8_t bflags;
static p_stf_t jig_stmf, ctl_stmf;
static volatile boolean_t jig_stop, jig_force_stop;
static volatile enum jig_type jig_type;

static struct btn1_dsc jigbtn = {
        .pin = JIGBTN_PIN,
	.cont = JIGBTN_CONT,
	.mode = BTN_REPORT_MODE,
        .active_lev = LOW,
	.pull_res = FALSE,
        .evnt_que_size = JIGBTN_EVNT_QUE_SIZE,
        .tsk_nm = "JIGBTN",
};

static struct {
	int m_in_irp_ok_cnt;
	int m_in_irp_enrdy_cnt;
	int m_in_irp_eintr_cnt;
#if USB_JIG_KEYB_IFACE == 1
	int k_in_irp_ok_cnt;
	int k_in_irp_enrdy_cnt;
	int k_in_irp_eintr_cnt;
#endif
	int jig_que_full_cnt;
} stats;

static void ctl_tsk(void *p);
static gfp_t ctl_stm_dflt(void);
static gfp_t ctl_stm_dflt_susp(void);
static gfp_t ctl_stm_adr(void);
static gfp_t ctl_stm_cnfg(void);
static void sleep_clbk(enum sleep_cmd cmd, ...);
static void m_inrep_tsk(void *p);
static void cmd_p(char ax, int mv);
static void cmd_w(int mv);
static void cmd_b(char b, int st);
static void cmd_be(char b);
static void jig_tsk(void *p);
static gfp_t jig_stm_off(void);
static gfp_t jig_stm_start(void);
static gfp_t jig_stm_work(void);
static gfp_t jig_stm_nosleep(void);
static gfp_t jig_stm_end(void);
static void mv_pointer_ax(enum axis ax, int mv);
static void mv_pointer_ud(int mv);
static void click_l(void);
#if USB_JIG_KEYB_IFACE == 1
static void k_inrep_tsk(void *p);
static boolean_t is_key_act(const struct keyb_report *kr, uint8_t key);
#if LOG_KEYB_LEDS == 1
static void k_led_tsk(void *p);
#endif
static void cmd_kp(int kc);
static void cmd_kr(int kc);
static void cmd_km(int bmp);
static void cmd_kk(int kc);
#endif

/**
 * init_jiggler
 */
void init_jiggler(void)
{
	jigbtn.qset = jig_ctl_qset;
	add_btn1_dev(&jigbtn);
	udp_que = get_udp_evnt_que();
	m_event_que = xQueueCreate(M_INREP_EVENT_QUE_SIZE, sizeof(union m_event));
	if (m_event_que == NULL) {
		crit_err_exit(MALLOC_ERROR);
	}
#if USB_JIG_KEYB_IFACE == 1
	k_event_que = xQueueCreate(K_INREP_EVENT_QUE_SIZE, sizeof(union k_event));
	if (k_event_que == NULL) {
		crit_err_exit(MALLOC_ERROR);
	}
#endif
	reg_sleep_clbk(sleep_clbk, SLEEP_PRIO_SUSP_FIRST);
        if (pdPASS != xTaskCreate(jig_tsk, "JIG", JIG_TASK_STACK_SIZE, NULL,
                                  JIG_TASK_PRIO, &jig_hndl)) {
                crit_err_exit(MALLOC_ERROR);
        }
	if (pdPASS != xTaskCreate(m_inrep_tsk, "MINREP", M_INREP_TASK_STACK_SIZE, NULL,
                                  M_INREP_TASK_PRIO, &m_inrep_hndl)) {
                crit_err_exit(MALLOC_ERROR);
        }
#if USB_JIG_KEYB_IFACE == 1
	if (pdPASS != xTaskCreate(k_inrep_tsk, "KINREP", K_INREP_TASK_STACK_SIZE, NULL,
                                  K_INREP_TASK_PRIO, &k_inrep_hndl)) {
                crit_err_exit(MALLOC_ERROR);
        }
#if LOG_KEYB_LEDS == 1
	if (pdPASS != xTaskCreate(k_led_tsk, "KLED", K_LED_TASK_STACK_SIZE, NULL,
                                  K_LED_TASK_PRIO, &k_led_hndl)) {
                crit_err_exit(MALLOC_ERROR);
        }
#endif
#endif
        if (pdPASS != xTaskCreate(ctl_tsk, "CTL", CTL_TASK_STACK_SIZE, NULL,
                                  CTL_TASK_PRIO, &ctl_hndl)) {
                crit_err_exit(MALLOC_ERROR);
        }
	add_command_char_int("p", cmd_p);
	add_command_int("w", cmd_w);
	add_command_char_int("b", cmd_b);
	add_command_char("be", cmd_be);
#if USB_JIG_KEYB_IFACE == 1
	add_command_int("kp", cmd_kp);
	add_command_int("kr", cmd_kr);
	add_command_int("km", cmd_km);
	add_command_int("kk", cmd_kk);
#endif
}

/**
 * ctl_tsk
 */
static void ctl_tsk(void *p)
{
	ctl_stmf = ctl_stm_dflt;
	while (TRUE) {
		ctl_stmf = (p_stf_t) (*ctl_stmf)();
	}
}

/**
 * ctl_stm_dflt
 */
static gfp_t ctl_stm_dflt(void)
{
	QueueSetMemberHandle_t qs;
	enum udp_state us;
	struct btn_evnt be;

	qs = xQueueSelectFromSet(jig_ctl_qset, portMAX_DELAY);
	if (qs == udp_que) {
		if (pdTRUE == xQueueReceive(udp_que, &us, 0)) {
			switch (us) {
			case UDP_STATE_DEFAULT :
				return ((gfp_t) ctl_stm_dflt);
			case UDP_STATE_ADDRESSED :
				return ((gfp_t) ctl_stm_adr);
			case UDP_STATE_SUSPENDED :
				return ((gfp_t) ctl_stm_dflt_susp);
			default :
				crit_err_exit(UNEXP_PROG_STATE);
				break;
			}
		} else {
			crit_err_exit(UNEXP_PROG_STATE);
		}
	} else if (qs == jigbtn.evnt_que) {
		if (pdTRUE != xQueueReceive(jigbtn.evnt_que, &be, 0)) {
			crit_err_exit(UNEXP_PROG_STATE);
		}
	} else {
		crit_err_exit(UNEXP_PROG_STATE);
	}
	return ((gfp_t) ctl_stm_dflt);
}

/**
 * ctl_stm_dflt_susp
 */
static gfp_t ctl_stm_dflt_susp(void)
{
	QueueSetMemberHandle_t qs;
	enum udp_state us;
	struct btn_evnt be;

	qs = xQueueSelectFromSet(jig_ctl_qset, CTL_STM_DFLT_SUSP_WAIT);
	if (qs == udp_que) {
		if (pdTRUE == xQueueReceive(udp_que, &us, 0)) {
			if (us == UDP_STATE_DEFAULT) {
				return ((gfp_t) ctl_stm_dflt);
			} else {
				crit_err_exit(UNEXP_PROG_STATE);
			}
		} else {
			crit_err_exit(UNEXP_PROG_STATE);
		}
	} else if (qs == jigbtn.evnt_que) {
		if (pdTRUE != xQueueReceive(jigbtn.evnt_que, &be, 0)) {
			crit_err_exit(UNEXP_PROG_STATE);
		}
		return ((gfp_t) ctl_stm_dflt_susp);
	} else {
#if SLEEP_LOG_STATE == 1
		msg(INF, "jiggler.c: CTL suspended\n");
#endif
		enable_pmc_frst(PMC_FRST_USB, 0);
		start_sleep(SLEEP_MODE_WAIT);
		vTaskSuspend(NULL);
#if SLEEP_LOG_STATE == 1
		msg(INF, "jiggler.c: CTL resumed\n");
#endif
		return ((gfp_t) ctl_stm_dflt);
	}
	return ((gfp_t) ctl_stm_dflt_susp);
}

/**
 * ctl_stm_adr
 */
static gfp_t ctl_stm_adr(void)
{
	QueueSetMemberHandle_t qs;
	enum udp_state us;
	struct btn_evnt be;

	qs = xQueueSelectFromSet(jig_ctl_qset, portMAX_DELAY);
	if (qs == udp_que) {
		if (pdTRUE == xQueueReceive(udp_que, &us, 0)) {
			switch (us) {
			case UDP_STATE_DEFAULT :
				return ((gfp_t) ctl_stm_dflt);
			case UDP_STATE_ADDRESSED :
				return ((gfp_t) ctl_stm_adr);
			case UDP_STATE_CONFIGURED :
				set_ledui_led_state(LEDUI4, LEDUI_LED_ON);
				vTaskResume(m_inrep_hndl);
#if USB_JIG_KEYB_IFACE == 1
				vTaskResume(k_inrep_hndl);
#endif
				return ((gfp_t) ctl_stm_cnfg);
			case UDP_STATE_SUSPENDED :
#if SLEEP_LOG_STATE == 1
				msg(INF, "jiggler.c: CTL suspended\n");
#endif
				enable_pmc_frst(PMC_FRST_USB, 0);
				start_sleep(SLEEP_MODE_WAIT);
				vTaskSuspend(NULL);
#if SLEEP_LOG_STATE == 1
				msg(INF, "jiggler.c: CTL resumed\n");
#endif
				return ((gfp_t) ctl_stm_adr);
			default :
				crit_err_exit(UNEXP_PROG_STATE);
				break;
			}
		} else {
			crit_err_exit(UNEXP_PROG_STATE);
		}
	} else if (qs == jigbtn.evnt_que) {
		if (pdTRUE != xQueueReceive(jigbtn.evnt_que, &be, 0)) {
			crit_err_exit(UNEXP_PROG_STATE);
		}
	} else {
		crit_err_exit(UNEXP_PROG_STATE);
	}
	return ((gfp_t) ctl_stm_adr);
}

/**
 * ctl_stm_cnfg
 */
static gfp_t ctl_stm_cnfg(void)
{
	QueueSetMemberHandle_t qs;
	enum udp_state us;
	struct btn_evnt btn_evnt;

	qs = xQueueSelectFromSet(jig_ctl_qset, portMAX_DELAY);
	if (qs == udp_que) {
		if (pdTRUE == xQueueReceive(udp_que, &us, 0)) {
			if (us == UDP_STATE_DEFAULT || us == UDP_STATE_ADDRESSED) {
				set_ledui_led_state(LEDUI4, LEDUI_LED_OFF);
				if (eSuspended != eTaskGetState(jig_hndl)) {
					taskENTER_CRITICAL();
					jig_force_stop = TRUE;
					jig_stop = TRUE;
					taskEXIT_CRITICAL();
					while (eSuspended != eTaskGetState(jig_hndl));
				}
				if (us == UDP_STATE_DEFAULT) {
					return ((gfp_t) ctl_stm_dflt);
				} else {
					return ((gfp_t) ctl_stm_adr);
				}
			} else if (us == UDP_STATE_CONFIGURED) {
				return ((gfp_t) ctl_stm_cnfg);
			} else if (us == UDP_STATE_SUSPENDED) {
				boolean_t wake_jig = FALSE;
				if (eSuspended != eTaskGetState(jig_hndl)) {
					wake_jig = TRUE;
					vTaskSuspend(jig_hndl);
#if SLEEP_LOG_STATE == 1
					msg(INF, "jiggler.c: JIG suspended\n");
#endif
				}
#if SLEEP_LOG_STATE == 1
				msg(INF, "jiggler.c: CTL suspended\n");
#endif
				enable_pmc_frst(PMC_FRST_USB, 0);
				start_sleep(SLEEP_MODE_WAIT);
				vTaskSuspend(NULL);
#if SLEEP_LOG_STATE == 1
				msg(INF, "jiggler.c: CTL resumed\n");
#endif
				if (wake_jig) {
					vTaskResume(jig_hndl);
#if SLEEP_LOG_STATE == 1
					msg(INF, "jiggler.c: JIG resumed\n");
#endif
				}
				return ((gfp_t) ctl_stm_cnfg);
			} else {
				crit_err_exit(UNEXP_PROG_STATE);
			}
		} else {
			crit_err_exit(UNEXP_PROG_STATE);
		}
	} else if (qs == jigbtn.evnt_que) {
		if (pdTRUE == xQueueReceive(jigbtn.evnt_que, &btn_evnt, 0)) {
			if (btn_evnt.type == BTN_PRESSED_DOWN) {
				if (btn_evnt.time > JIG_BTN_MOD_SEL_TM) {
					jig_type = JIG_WORK;
				} else {
					jig_type = JIG_NOSLEEP;
				}
				if (eSuspended != eTaskGetState(jig_hndl)) {
					jig_stop = TRUE;
				} else {
					vTaskResume(jig_hndl);
				}
			} else {
				crit_err_exit(UNEXP_PROG_STATE);
			}
		} else {
			crit_err_exit(UNEXP_PROG_STATE);
		}
	} else {
		crit_err_exit(UNEXP_PROG_STATE);
	}
	return ((gfp_t) ctl_stm_cnfg);
}

/**
 * sleep_clbk
 */
static void sleep_clbk(enum sleep_cmd cmd, ...)
{
	if (cmd == SLEEP_CMD_WAKE) {
		vTaskResume(ctl_hndl);
	}
}

/**
 * m_inrep_tsk
 */
static void m_inrep_tsk(void *p)
{
	static int ret;
	static union m_event event;
	static boolean_t pointer, wheel, button;

	vTaskSuspend(NULL);
	msg(INF, "jiggler.c: mouse reporting started\n");
	for (;;) {
		pointer = wheel = button = FALSE;
		mouse_report.x = 0;
		mouse_report.y = 0;
		mouse_report.w = 0;
		for (;;) {
			if (pdFALSE == xQueuePeek(m_event_que, &event, 0)) {
				break;
			}
			if (event.type == POINTER) {
				if (pointer) {
					break;
				}
				mouse_report.x = event.pointer.x;
				mouse_report.y = event.pointer.y;
				pointer = TRUE;
			} else if (event.type == WHEEL) {
				if (wheel) {
					break;
				}
				mouse_report.w = event.wheel.w;
				wheel = TRUE;
			} else if (event.type == BUTTON) {
				if (button) {
					break;
				}
				mouse_report.bm = event.button.bflags;
				button = TRUE;
			} else {
				crit_err_exit(UNEXP_PROG_STATE);
			}
			xQueueReceive(m_event_que, &event, 0);
		}
		while (TRUE) {
			if (0 != (ret = udp_in_irp(USB_JIG_IN_M_ENDP_NUM, &mouse_report,
			                           sizeof(struct mouse_report), TRUE))) {
				if (ret == -ENRDY) {
					stats.m_in_irp_enrdy_cnt++;
				} else if (ret == -EINTR) {
					stats.m_in_irp_eintr_cnt++;
				} else {
					crit_err_exit(UNEXP_PROG_STATE);
				}
				vTaskDelay(UDP_IN_IRP_ERR_WAIT);
				continue;
			} else {
				stats.m_in_irp_ok_cnt++;
				break;
			}
		}
		xQueuePeek(m_event_que, &event, portMAX_DELAY);
	}
}

/**
 * cmd_p
 */
static void cmd_p(char ax, int mv)
{
	union m_event event;

	event.type = POINTER;
	event.pointer.x = 0;
	event.pointer.y = 0;
	if (mv < -127 || mv > 127) {
		msg(INF, "bad param\n");
		return;
	}
	if (ax == 'x') {
		event.pointer.x = mv;
	} else if (ax == 'y') {
		event.pointer.y = mv;
	} else {
		msg(INF, "bad param\n");
	}
	if (pdTRUE == xQueueSend(m_event_que, &event, 0)) {
		msg(INF, "sent\n");
	} else {
		msg(INF, "full\n");
	}
}

/**
 * cmd_w
 */
static void cmd_w(int mv)
{
	union m_event event;

	event.type = WHEEL;
	if (mv < -127 || mv > 127) {
		msg(INF, "bad param\n");
		return;
	}
	event.wheel.w = mv;
	if (pdTRUE == xQueueSend(m_event_que, &event, 0)) {
		msg(INF, "sent\n");
	} else {
		msg(INF, "full\n");
	}
}

/**
 * cmd_b
 */
static void cmd_b(char b, int st)
{
	union m_event event;

	if (st != 0 && st != 1) {
		msg(INF, "bad param\n");
		return;
	}
	event.type = BUTTON;
	if (b == 'l') {
		if (st) {
			bflags |= 0x01;
		} else {
			bflags &= ~0x01;
		}
	} else if (b == 'r') {
		if (st) {
			bflags |= 0x02;
		} else {
			bflags &= ~0x02;
		}
	} else if (b == 'm') {
		if (st) {
			bflags |= 0x04;
		} else {
			bflags &= ~0x04;
		}
	} else {
		msg(INF, "bad param\n");
		return;
	}
	event.button.bflags = bflags;
	if (pdTRUE == xQueueSend(m_event_que, &event, 0)) {
		msg(INF, "sent\n");
	} else {
		msg(INF, "full\n");
	}
}

/**
 * cmd_be
 */
static void cmd_be(char b)
{
	union m_event event;

	event.type = BUTTON;
	if (b == 'l') {
		bflags |= 0x01;
	} else if (b == 'r') {
		bflags |= 0x02;
	} else if (b == 'm') {
		bflags |= 0x04;
	} else {
		msg(INF, "bad param\n");
		return;
	}
	event.button.bflags = bflags;
	if (pdFALSE == xQueueSend(m_event_que, &event, 0)) {
		msg(INF, "full\n");
		return;
	}
	if (b == 'l') {
		bflags &= ~0x01;
	} else if (b == 'r') {
		bflags &= ~0x02;
	} else if (b == 'm') {
		bflags &= ~0x04;
	}
	event.button.bflags = bflags;
	vTaskDelay(BTN_PRESS_TIME);
	if (pdTRUE == xQueueSend(m_event_que, &event, 0)) {
		msg(INF, "sent\n");
	} else {
		msg(INF, "full\n");
	}
}

/**
 * jig_tsk
 */
static void jig_tsk(void *p)
{
	jig_stmf = jig_stm_off;
	while (TRUE) {
		jig_stmf = (p_stf_t) (*jig_stmf)();
	}
}

/**
 * jig_stm_off
 */
static gfp_t jig_stm_off(void)
{
	static boolean_t tgl;

	if (tgl) {
		msg(INF, "jiggler.c: autojig stopped\n");
		set_ledui_led_state(LEDUI1, LEDUI_LED_OFF);
	} else {
		tgl = TRUE;
	}
	vTaskSuspend(NULL);
	jig_stop = FALSE;
	jig_force_stop = FALSE;
	return ((gfp_t) jig_stm_start);
}

/**
 * jig_stm_start
 */
static gfp_t jig_stm_start(void)
{
	if (jig_type == JIG_WORK) {
		set_ledui_led_state(LEDUI1, LEDUI_LED_BLINK_FAST_STDF, LEDUI_BLINK_START_ON);
		msg(INF, "jiggler.c: autojig started (JIG_WORK)\n");
	} else {
		set_ledui_led_state(LEDUI1, LEDUI_LED_BLINK_SLOW_STDF, LEDUI_BLINK_START_ON);
		msg(INF, "jiggler.c: autojig started (JIG_NOSLEEP)\n");
	}
	mv_pointer_ud(JIG_MV_POI_SE);
	if (jig_stop) {
		return ((gfp_t) jig_stm_off);
	}
	if (jig_type == JIG_WORK) {
		click_l();
		return ((gfp_t) jig_stm_work);
	} else {
		return ((gfp_t) jig_stm_nosleep);
	}
}

/**
 * jig_stm_work
 */
static gfp_t jig_stm_work(void)
{
	union m_event event;
	int r;

	event.type = WHEEL;
	event.wheel.w = 1;
	for (;;) {
		if (event.wheel.w == 1) {
			event.wheel.w = -1;
		} else {
			event.wheel.w = 1;
		}
		for (int i = 0; i < JIG_WHEEL_ACT_CNT; i++) {
			r = rand() & JIG_WHEEL_RND_MASK;
			for (int j = 0; j < r + JIG_MIN_WHEEL_TIME_CNT; j++) {
				if (jig_stop) {
					return ((gfp_t) jig_stm_end);
				}
				vTaskDelay(JIG_DLY_TIME);
			}
			if (pdTRUE != xQueueSend(m_event_que, &event, 0)) {
				stats.jig_que_full_cnt++;
			}
		}
		r = rand() & JIG_WHEEL_RND_MASK;
		for (int j = 0; j < r + JIG_MIN_WHEEL_TIME_CNT; j++) {
			if (jig_stop) {
				return ((gfp_t) jig_stm_end);
			}
			vTaskDelay(JIG_DLY_TIME);
		}
		mv_pointer_ax(AXIS_X, JIG_MV_POI_W);
		if (jig_stop) {
			return ((gfp_t) jig_stm_end);
		}
		click_l();
	}
}

/**
 * jig_stm_nosleep
 */
static gfp_t jig_stm_nosleep(void)
{
	union m_event event;

	event.type = POINTER;
	event.pointer.x = 1;
	event.pointer.y = 0;
	for (;;) {
		if (event.pointer.x == 1) {
			event.pointer.x = -1;
		} else {
			event.pointer.x = 1;
		}
		for (int i = 0; i < JIG_NOSLEEP_TIME_CNT; i++) {
			if (jig_stop) {
				return ((gfp_t) jig_stm_end);
			}
			vTaskDelay(JIG_DLY_TIME);
		}
		if (pdTRUE != xQueueSend(m_event_que, &event, 0)) {
			stats.jig_que_full_cnt++;
		}
	}
}

/**
 * jig_stm_end
 */
static gfp_t jig_stm_end(void)
{
	mv_pointer_ax(AXIS_Y, JIG_MV_POI_SE);
	mv_pointer_ax(AXIS_X, JIG_MV_POI_SE);
	return ((gfp_t) jig_stm_off);
}

/**
 * mv_pointer_ax
 */
static void mv_pointer_ax(enum axis ax, int mv)
{
	union m_event event;
	int x;

	event.type = POINTER;
	event.pointer.x = 0;
	event.pointer.y = 0;
	for (int j = 0; j < 4; j++) {
		if (j == 0 || j == 3) {
			x = 1;
		} else {
			x = -1;
		}
		for (int i = 0; i < mv; i++) {
			if (jig_force_stop) {
				return;
			}
			vTaskDelay(MV_POINTER_WAIT);
			if (x > 0) {
				if (ax == AXIS_X) {
					event.pointer.x = x++;
				} else {
					event.pointer.y = x++;
				}
			} else {
				if (ax == AXIS_X) {
					event.pointer.x = x--;
				} else {
					event.pointer.y = x--;
				}
			}
			if (pdTRUE != xQueueSend(m_event_que, &event, 0)) {
				stats.jig_que_full_cnt++;
			}
		}
	}
}

/**
 * mv_pointer_ud
 */
static void mv_pointer_ud(int mv)
{
	union m_event event;
	int x, y;

	event.type = POINTER;
	for (int j = 0; j < 4; j++) {
		if (j == 0) {
			x = 1;
			y = -1;
		} else if (j == 1) {
			x = -1;
			y = 1;
		} else if (j == 2) {
			x = -1;
			y = -1;
		} else {
			x = 1;
			y = 1;
		}
		for (int i = 0; i < mv; i++) {
			if (jig_force_stop) {
				return;
			}
			vTaskDelay(MV_POINTER_WAIT);
			if (x > 0) {
				event.pointer.x = x++;
			} else {
				event.pointer.x = x--;
			}
			if (y > 0) {
				event.pointer.y = y++;
			} else {
				event.pointer.y = y--;
			}
			if (pdTRUE != xQueueSend(m_event_que, &event, 0)) {
				stats.jig_que_full_cnt++;
			}
		}
	}
}

/**
 * click_l
 */
static void click_l(void)
{
	union m_event event;

	if (jig_force_stop) {
		return;
	}
	event.type = BUTTON;
	bflags |= 0x01;
	event.button.bflags = bflags;
	if (pdTRUE != xQueueSend(m_event_que, &event, 0)) {
		stats.jig_que_full_cnt++;
	}
	vTaskDelay(BTN_PRESS_TIME);
	bflags &= ~0x01;
	event.button.bflags = bflags;
	if (pdTRUE != xQueueSend(m_event_que, &event, 0)) {
		stats.jig_que_full_cnt++;
	}
}

#if USB_JIG_KEYB_IFACE == 1
/**
 * k_inrep_tsk
 */
static void k_inrep_tsk(void *p)
{
	static int ret;
	static union k_event event;
	static struct keyb_report kr;
	static boolean_t fst = TRUE;

	vTaskSuspend(NULL);
	msg(INF, "jiggler.c: keyboard reporting started\n");
	for (;;) {
		for (;;) {
			if (fst) {
				fst = FALSE;
				break;
			}
			if (event.type == KPRES) {
				if (!is_key_act(&keyb_report, event.genkey.code)) {
					int i;
					for (i = 0; i < KEYB_REPORT_KEY_ARY_SIZE; i++) {
						if (keyb_report.keys[i] == 0) {
							keyb_report.keys[i] = event.genkey.code;
							break;
						}
					}
					if (i != KEYB_REPORT_KEY_ARY_SIZE) {
						break;
					} else {
						msg(INF, "jiggler.c: keyb_report array full!\n");
					}
				}
			} else if (event.type == KREL) {
				if (is_key_act(&keyb_report, event.genkey.code)) {
					kr.mod = keyb_report.mod;
					kr.res = keyb_report.res;
					memset(&kr.keys, 0, KEYB_REPORT_KEY_ARY_SIZE);
					int j = 0;
					for (int i = 0; i < KEYB_REPORT_KEY_ARY_SIZE; i++) {
						if (keyb_report.keys[i] == 0) {
							break;
						}
						if (keyb_report.keys[i] != event.genkey.code) {
							kr.keys[j++] = keyb_report.keys[i];
						}
					}
					taskENTER_CRITICAL();
					keyb_report = kr;
					taskEXIT_CRITICAL();
					break;
				}
			} else if (event.type == KMOD) {
				if (event.modkey.bmp != keyb_report.mod) {
					keyb_report.mod = event.modkey.bmp;
					break;
				}
			} else {
				crit_err_exit(UNEXP_PROG_STATE);
			}
			xQueueReceive(k_event_que, &event, portMAX_DELAY);
		}
		while (TRUE) {
			if (0 != (ret = udp_in_irp(USB_JIG_IN_K_ENDP_NUM, &keyb_report,
			                           sizeof(struct keyb_report), TRUE))) {
				if (ret == -ENRDY) {
					stats.k_in_irp_enrdy_cnt++;
				} else if (ret == -EINTR) {
					stats.k_in_irp_eintr_cnt++;
				} else {
					crit_err_exit(UNEXP_PROG_STATE);
				}
				vTaskDelay(UDP_IN_IRP_ERR_WAIT);
				continue;
			} else {
				stats.k_in_irp_ok_cnt++;
				break;
			}
		}
		xQueueReceive(k_event_que, &event, portMAX_DELAY);
	}
}

/**
 * is_key_act
 */
static boolean_t is_key_act(const struct keyb_report *kr, uint8_t key)
{
	for (int i = 0; i < KEYB_REPORT_KEY_ARY_SIZE; i++) {
		if (kr->keys[i] == 0) {
			break;
		}
		if (kr->keys[i] == key) {
			return (TRUE);
		}
	}
	return (FALSE);
}

#if LOG_KEYB_LEDS == 1
/**
 * k_led_tsk
 */
static void k_led_tsk(void *p)
{
	static struct keyb_led_report rep;
	static char s[10];

	for (;;) {
		xQueueReceive(keyb_led_rep_que, &rep, portMAX_DELAY);
		prn_bv_str(s, rep.leds, 8);
		msg(INF, "keyb_leds: %s\n", s);
	}
}
#endif

/**
 * cmd_kp
 */
static void cmd_kp(int kc)
{
	union k_event event;

	if (kc < 1 || kc > 101) {
		msg(INF, "key code error\n");
	}
	event.type = KPRES;
	event.genkey.code = kc;
	if (pdTRUE == xQueueSend(k_event_que, &event, 0)) {
		msg(INF, "sent\n");
	} else {
		msg(INF, "full\n");
	}
}

/**
 * cmd_kr
 */
static void cmd_kr(int kc)
{
	union k_event event;

	if (kc < 1 || kc > 101) {
		msg(INF, "key code error\n");
	}
	event.type = KREL;
	event.genkey.code = kc;
	if (pdTRUE == xQueueSend(k_event_que, &event, 0)) {
		msg(INF, "sent\n");
	} else {
		msg(INF, "full\n");
	}
}

/**
 * cmd_km
 */
static void cmd_km(int bmp)
{
	union k_event event;

	event.type = KMOD;
	event.modkey.bmp = bmp;
	if (pdTRUE == xQueueSend(k_event_que, &event, 0)) {
		msg(INF, "sent\n");
	} else {
		msg(INF, "full\n");
	}
}

/**
 * cmd_kk
 */
static void cmd_kk(int kc)
{
	union k_event event;

	if (kc < 1 || kc > 101) {
		msg(INF, "key code error\n");
	}
	event.type = KPRES;
	event.genkey.code = kc;
	if (pdTRUE == xQueueSend(k_event_que, &event, 0)) {
		vTaskDelay(KEY_PRESS_TIME);
		event.type = KREL;
		if (pdTRUE == xQueueSend(k_event_que, &event, 0)) {
			msg(INF, "sent\n");
			return;
		}
	}
	msg(INF, "full\n");
}
#endif

/**
 * log_jiggler_stats
 */
void log_jiggler_stats(void)
{
	if (stats.m_in_irp_ok_cnt) {
		msg(INF, "jiggler.c: m_in_irp_ok=%d\n", stats.m_in_irp_ok_cnt);
	}
	if (stats.m_in_irp_enrdy_cnt) {
		msg(INF, "jiggler.c: m_in_irp_enrdy=%d\n", stats.m_in_irp_enrdy_cnt);
	}
	if (stats.m_in_irp_eintr_cnt) {
		msg(INF, "jiggler.c: m_in_irp_eintr=%d\n", stats.m_in_irp_eintr_cnt);
	}
#if USB_JIG_KEYB_IFACE == 1
	if (stats.k_in_irp_ok_cnt) {
		msg(INF, "jiggler.c: k_in_irp_ok=%d\n", stats.k_in_irp_ok_cnt);
	}
	if (stats.k_in_irp_enrdy_cnt) {
		msg(INF, "jiggler.c: k_in_irp_enrdy=%d\n", stats.k_in_irp_enrdy_cnt);
	}
	if (stats.k_in_irp_eintr_cnt) {
		msg(INF, "jiggler.c: k_in_irp_eintr=%d\n", stats.k_in_irp_eintr_cnt);
	}
#endif
	if (stats.jig_que_full_cnt) {
		msg(INF, "jiggler.c: jig_que_full=%d\n", stats.jig_que_full_cnt);
	}
}
