/*
 * sysconf.h
 *
 * Autors: Jan Rusnak.
 * (c) 2024 AZTech.
 */

#ifndef SYSCONF_H
#define SYSCONF_H

#if defined(TINSY_SAM_BOARD)
////////////////////////////////////////////////////////////////////////////////
// PMC
#define PMC_UPDATE_SYS_CORE_CLK 1

////////////////////////////////////////////////////////////////////////////////
// WD
//#define WD_EXPIRE_WDV WD_EXP_600MS
//#define WD_EXPIRE_WDD WD_EXP_550MS
#define WD_EXPIRE_WDV WD_EXP_10S
#define WD_EXPIRE_WDD WD_EXP_10S
#define WD_IDLE_HALT 1

////////////////////////////////////////////////////////////////////////////////
// RSTC
#define RSTC_EXT_RESET_LENGTH 1

////////////////////////////////////////////////////////////////////////////////
// EEFC
#define EEFC_FLASH_CMD 0

////////////////////////////////////////////////////////////////////////////////
// UART
#define UART_RX_BYTE 1
#define UART_HDLC 0

////////////////////////////////////////////////////////////////////////////////
// USART
#define USART_RX_CHAR 1
#define USART_ADR_CHAR 0
#define USART_ADR_HDLC 0
#define USART_ADR_HDLC_MAX_ADR 0xFA
#define USART_ADR_HDLC_EXT_STATS 1
#define USART_ADR_HDLC_PERR_DUMP_SIZE 7
#define USART_HDLC 0
#define USART_YIT 0

////////////////////////////////////////////////////////////////////////////////
// SLEEP
#define SLEEP_FEAT 1
#define SLEEP_NOT_USE_WFE 1
#define SLEEP_FLASH_LP_MODE PMC_FLASH_LPM_IDLE
#define SLEEP_FIRST_ARY_SIZE 5
#define SLEEP_SECOND_ARY_SIZE 5
#define SLEEP_LAST_ARY_SIZE 5
#define SLEEP_LOG_STATE 1
#define SLEEP_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 30)

////////////////////////////////////////////////////////////////////////////////
// PIO
#define PIOA_INTR 1
#define PIOA_INTR_CLBK_ARRAY_SIZE 2
#define PIOB_INTR 1
#define PIOB_INTR_CLBK_ARRAY_SIZE 2

////////////////////////////////////////////////////////////////////////////////
// TMC
#define TMC_TC0 0
#define TMC_TC1 0
#define TMC_TC2 1
#define TMC_TC3 0
#define TMC_TC4 0
#define TMC_TC5 0

////////////////////////////////////////////////////////////////////////////////
// SPIM
#define SPIM 0

////////////////////////////////////////////////////////////////////////////////
// USB_JIG
#define USB_JIG_VENDORID  0x03EB
#define USB_JIG_PRODUCTID 0x201C
#define USB_JIG_KEYB_IFACE 1
#define USB_JIG_KEYB_IDLE_MS 500
#define USB_JIG_IN_M_ENDP_NUM 6
#define USB_JIG_IN_M_ENDP_MAX_PKT_SIZE 64
#define USB_JIG_IN_M_ENDP_POLLED_MS 0x0A
#define USB_JIG_IN_K_ENDP_NUM 7
#define USB_JIG_IN_K_ENDP_MAX_PKT_SIZE 64
#define USB_JIG_IN_K_ENDP_POLLED_MS 0x0A
#define UDP_EVNT_QUE_SIZE 20
#define UDP_LOG_INTR_EVENTS 0
#define UDP_LOG_STATE_EVENTS 0
#define UDP_LOG_ENDP_EVENTS 0
#define UDP_LOG_OUT_IRP_EVENTS 0
#define UDP_LOG_OUT_IRP_EVENTS_ALL 1
#define UDP_LOG_ERR_EVENTS 1
#define UDP_DBG_ISR_QUE 1
#define USB_LOG_CTL_REQ_EVENTS 0
#define USB_LOG_CTL_REQ_STP_EVENTS 0
#define USB_LOG_CTL_REQ_CMD_EVENTS 0
#define USB_LOG_EVENTS_QUEUE_SIZE 50
#define USB_LOG_EVENTS_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define USB_LOG_EVENTS_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 50)

////////////////////////////////////////////////////////////////////////////////
// ADC
#define ADC_SW_TRG_1CH   0
#define ADC_SW_TRG_1CH_N 0
#define ADC_SW_TRG_XCH   0

////////////////////////////////////////////////////////////////////////////////
// DACC
#define DACC_FREE_RUN 0

////////////////////////////////////////////////////////////////////////////////
// CHIP_ID
#define CHIP_ID 1

////////////////////////////////////////////////////////////////////////////////
// LEDUI
#define LEDUI 1
#define LEDUI_SLEEP 1
#define LEDUI_BASE_SWITCH_FREQ 100
#define LEDUI_BLINK_FAST_SWITCH 1
#define LEDUI_BLINK_NORMAL_SWITCH 5
#define LEDUI_BLINK_SLOW_SWITCH 15
#define LEDUI_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define LEDUI_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)

////////////////////////////////////////////////////////////////////////////////
// LED
#define LED 0
#define LED_BASE_FREQ 50
#define LED_ON_TIME 30
#define LED_TDV TC0
#define LED_TID ID_TC0
#define LED_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define LED_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)

////////////////////////////////////////////////////////////////////////////////
// BTN
#define BTN 0
#define BTN_SLEEP 1
#define BTN_INTR_QUE_SIZE 2
#define BTN_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define BTN_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)

////////////////////////////////////////////////////////////////////////////////
// BTN1
#define BTN1 1
#define BTN1_SLEEP 1
#define BTN1_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define BTN1_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define BTN1_CHECK_DELAY 5
#define BTN1_CHECK_DELAY_CNT 60

////////////////////////////////////////////////////////////////////////////////
// CRITERR
#define CRITERR 1
#define CRITERR_TDV TC0
#define CRITERR_TID ID_TC2
#define CRITERR_WD_RST 2

////////////////////////////////////////////////////////////////////////////////
// MEMNFO
#define V_TASK_LIST_BUFFER_SIZE 350

////////////////////////////////////////////////////////////////////////////////
// TERMOUT
#define TERMOUT 1
#define TERMOUT_SLEEP 1
#define TERMOUT_MAX_ROW_LENGTH 81
#define TERMOUT_BUFFER_SIZE 3072
#define TERMOUT_MAX_ROWS_IN_QUEUE 100
#define TERMOUT_SEND_CLS_ON_START 1
#define TERMOUT_TASK_PRIO (tskIDLE_PRIORITY + 2)
#define TERMOUT_STACK_SIZE (configMINIMAL_STACK_SIZE + 30)

////////////////////////////////////////////////////////////////////////////////
// TERMIN
#define TERMIN 1
#define TERMIN_SLEEP 1
#define TERMIN_MAX_ROW_LENGTH 64
#define TERMIN_START_ECHO_ON 1
#define TERMIN_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define TERMIN_STACK_SIZE (configMINIMAL_STACK_SIZE + 60)

////////////////////////////////////////////////////////////////////////////////
// CMDLN
#define CMDLN_PARSER 1
#define CMDLN_STRING_DELIMITER '\''

////////////////////////////////////////////////////////////////////////////////
// TM
#define TM_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define TM_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define TIME_BASE_MS 250
#define TIME_BASE_CLBK_ARRAY_SIZE 2

////////////////////////////////////////////////////////////////////////////////
// CRC
#define CRC_16_FUNC 1
#define CRC_CCIT_FUNC 0

////////////////////////////////////////////////////////////////////////////////
// JIGGLER
#define CTL_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define CTL_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define M_INREP_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define M_INREP_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define K_INREP_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define K_INREP_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define K_LED_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define K_LED_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define JIG_TASK_PRIO (tskIDLE_PRIORITY + 3)
#define JIG_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define M_INREP_EVENT_QUE_SIZE 2
#define K_INREP_EVENT_QUE_SIZE 5
#define LOG_KEYB_LEDS 0
#define JIG_MV_POI_SE 14
#define JIG_MV_POI_W 10
#define JIG_MIN_WHEEL_TIME_CNT 50
#define JIG_BTN_MOD_SEL_TM 500
#define JIG_WHEEL_ACT_CNT 15

////////////////////////////////////////////////////////////////////////////////
// JIGBTN
#define JIGBTN_EVNT_QUE_SIZE 2

////////////////////////////////////////////////////////////////////////////////
// SHIFT165
#define SHIFT165 0

////////////////////////////////////////////////////////////////////////////////
// SHIFT164
#define SHIFT164 0

#else
 #error "Board type not defined"
#endif

#endif
