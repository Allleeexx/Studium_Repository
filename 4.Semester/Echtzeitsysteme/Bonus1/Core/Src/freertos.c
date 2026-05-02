/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  uint32_t timeUs;
  uint32_t task;
  uint8_t event;
} TraceEntry;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CPU_LOAD_WINDOW_US   470000U
#define CPU_LOAD_STEP_US     100U
#define TRACE_BUFFER_SIZE    128U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
volatile uint32_t cpuLoadPermille = 0;
volatile uint32_t cpuIdlePermille = 0;
volatile uint32_t cpuLoadMeasureUs = 0;
volatile uint32_t cpuLoadIdleUs = 0;

volatile TraceEntry traceBuffer[TRACE_BUFFER_SIZE];
volatile uint32_t traceIndex = 0;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void busy_delay(uint32_t us);
static uint32_t TraceTimeUs(void);

/* USER CODE END FunctionPrototypes */

/* USER CODE BEGIN 1 */
static uint32_t TraceTimeUs(void)
{
  return DWT->CYCCNT / (SystemCoreClock / 1000000U);
}

void TraceTaskSwitchedIn(void *task)
{
  uint32_t i = traceIndex & (TRACE_BUFFER_SIZE - 1U);
  traceBuffer[i].timeUs = TraceTimeUs();
  traceBuffer[i].task = (uint32_t)(uintptr_t)task;
  traceBuffer[i].event = 1U;
  traceIndex++;
}

void TraceTaskSwitchedOut(void *task)
{
  uint32_t i = traceIndex & (TRACE_BUFFER_SIZE - 1U);
  traceBuffer[i].timeUs = TraceTimeUs();
  traceBuffer[i].task = (uint32_t)(uintptr_t)task;
  traceBuffer[i].event = 0U;
  traceIndex++;
}
/* USER CODE END 1 */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
  static uint32_t startCycles = 0;
  static uint32_t idleTimeUs = 0;
  const uint32_t cyclesPerUs = SystemCoreClock / 1000000U;

  if (startCycles == 0U)
  {
    startCycles = DWT->CYCCNT;
  }

  busy_delay(CPU_LOAD_STEP_US);
  idleTimeUs += CPU_LOAD_STEP_US;

  uint32_t measureTimeUs = (DWT->CYCCNT - startCycles) / cyclesPerUs;

  if (measureTimeUs >= CPU_LOAD_WINDOW_US)
  {
    cpuLoadMeasureUs = measureTimeUs;
    cpuLoadIdleUs = idleTimeUs;
    cpuIdlePermille = (idleTimeUs * 1000U) / measureTimeUs;

    if (cpuIdlePermille > 1000U)
    {
      cpuIdlePermille = 1000U;
    }

    cpuLoadPermille = 1000U - cpuIdlePermille;
    idleTimeUs = 0U;
    startCycles = DWT->CYCCNT;
  }
}
/* USER CODE END 2 */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

