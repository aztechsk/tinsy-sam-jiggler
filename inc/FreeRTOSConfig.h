/*
 * FreeRTOSConfig.h
 *
 * Autors: Jan Rusnak.
 * (c) 2020 AZTech.
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "clocks.h"

#define IDLE_STACK_SIZE 120

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configUSE_IDLE_HOOK                     1
#define configUSE_MALLOC_FAILED_HOOK		0
#define configUSE_DAEMON_TASK_STARTUP_HOOK	0
#define configUSE_TICK_HOOK			0
#define configCPU_CLOCK_HZ                      F_MCK
#define configTICK_RATE_HZ                      500U
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                IDLE_STACK_SIZE
#define configMAX_TASK_NAME_LEN                 12
#define configUSE_TRACE_FACILITY		1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            0
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_ALTERNATIVE_API		0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_QUEUE_SETS                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
//#define configTOTAL_HEAP_SIZE
//#define configAPPLICATION_ALLOCATED_HEAP
#define configGENERATE_RUN_TIME_STATS		0
#define configUSE_CO_ROUTINES                   0
#define configUSE_TIMERS                        0
#define configASSERT(x) if((x) == 0) {taskDISABLE_INTERRUPTS(); for(;;);}
// SAM3 SAM4 Cortex-M __NVIC_PRIO_BITS = 4
// KERNEL_INTERRUPT 15 (low priority: 15) = 1111xxxx (0xF0)
#define configKERNEL_INTERRUPT_PRIORITY         0xF0
// API_CALL (high priority: 10) = 1010xxxx (0xA0)
#define configLIBRARY_MAX_API_CALL_INTERRUPT_PRIORITY 10
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0xA0
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#define INCLUDE_vTaskPrioritySet             1
#define INCLUDE_uxTaskPriorityGet            1
#define INCLUDE_vTaskDelete                  0
#define INCLUDE_vTaskSuspend                 1
#define INCLUDE_vTaskDelayUntil              1
#define INCLUDE_vTaskDelay                   1
#define INCLUDE_xTaskGetIdleTaskHandle       0
#define INCLUDE_xTaskAbortDelay              0
#define INCLUDE_xQueueGetMutexHolder         0
#define INCLUDE_xSemaphoreGetMutexHolder     0
#define INCLUDE_xTaskGetHandle               1
#define INCLUDE_uxTaskGetStackHighWaterMark  0
#define INCLUDE_uxTaskGetStackHighWaterMark2 0
#define INCLUDE_eTaskGetState                1
#define INCLUDE_xTaskResumeFromISR           1
#define INCLUDE_xTimerPendFunctionCall       0
#define INCLUDE_xTaskGetSchedulerState       1
#define INCLUDE_xTaskGetCurrentTaskHandle    1

#define TASK_PRIO_HIGH 4
#define TASK_PRIO_LOW 1

#define ms_to_os_ticks(xTimeInMs) ((TickType_t) ((((unsigned int) (xTimeInMs) * configTICK_RATE_HZ) + 999U) / 1000U))

#endif
