/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  osThreadId_t handle;
  osSemaphoreId_t sem;
  uint32_t periodMs;
  uint32_t relativeDeadlineUs;
  uint32_t execTimeUs;
  uint16_t ledPin;

  volatile uint32_t releaseQueue[4];
  volatile uint32_t deadlineQueue[4];
  volatile uint8_t head;
  volatile uint8_t tail;
  volatile uint8_t jobs;
  volatile uint8_t active;

  volatile uint32_t activeReleaseCycle;
  volatile uint32_t activeDeadlineUs;
  volatile uint32_t nextReleaseMs;
  volatile uint32_t queueOverflows;

  volatile uint32_t *deadlineMiss;
  volatile uint32_t *responseUs;
  volatile uint32_t *responseMaxUs;
} EdfTask;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TaskA_PERIOD_MS      3U
#define TaskA_DEADLINE_MS    3U
#define TaskA_EXEC_TIME_US   1000U

#define TaskB_PERIOD_MS      5U
#define TaskB_DEADLINE_MS    4U
#define TaskB_EXEC_TIME_US   2000U

#define TaskC_PERIOD_MS      8U
#define TaskC_DEADLINE_MS    8U
#define TaskC_EXEC_TIME_US   1000U

#define TaskD_PERIOD_MS      14U
#define TaskD_DEADLINE_MS    14U
#define TaskD_EXEC_TIME_US   1000U

#define CPU_LOAD_WINDOW_US   470000U
#define CPU_LOAD_STEP_US     100U

#define EDF_TASK_COUNT       4U
#define EDF_QUEUE_LEN        4U
#define EDF_TASK_A           0U
#define EDF_TASK_B           1U
#define EDF_TASK_C           2U
#define EDF_TASK_D           3U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim7;

/* Definitions for TaskA */
osThreadId_t TaskAHandle;
const osThreadAttr_t TaskA_attributes = {
  .name = "TaskA",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for TaskB */
osThreadId_t TaskBHandle;
const osThreadAttr_t TaskB_attributes = {
  .name = "TaskB",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskC */
osThreadId_t TaskCHandle;
const osThreadAttr_t TaskC_attributes = {
  .name = "TaskC",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for TaskD */
osThreadId_t TaskDHandle;
const osThreadAttr_t TaskD_attributes = {
  .name = "TaskD",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SchedTask */
osThreadId_t SchedTaskHandle;
const osThreadAttr_t SchedTask_attributes = {
  .name = "SchedTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for startDefaultTas */
osThreadId_t startDefaultTasHandle;
const osThreadAttr_t startDefaultTas_attributes = {
  .name = "startDefaultTas",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for semTaskA */
osSemaphoreId_t semTaskAHandle;
const osSemaphoreAttr_t semTaskA_attributes = {
  .name = "semTaskA"
};
/* Definitions for semTaskB */
osSemaphoreId_t semTaskBHandle;
const osSemaphoreAttr_t semTaskB_attributes = {
  .name = "semTaskB"
};
/* Definitions for semTaskC */
osSemaphoreId_t semTaskCHandle;
const osSemaphoreAttr_t semTaskC_attributes = {
  .name = "semTaskC"
};
/* Definitions for semTaskD */
osSemaphoreId_t semTaskDHandle;
const osSemaphoreAttr_t semTaskD_attributes = {
  .name = "semTaskD"
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM7_Init(void);
void StartTaskA(void *argument);
void StartTaskB(void *argument);
void StartTaskC(void *argument);
void StartTaskD(void *argument);
void StartSchedTask(void *argument);
void startDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
static void DWT_Init(void);
void busy_delay(uint32_t us);
static uint32_t CyclesToUs(uint32_t cycles);
static uint8_t DeadlineMissed(uint32_t releaseCycle, uint32_t deadlineUs, volatile uint32_t *lastUs, volatile uint32_t *maxUs);
static void EdfInit(void);
static void EdfSetTask(uint32_t id, osThreadId_t handle, osSemaphoreId_t sem, uint32_t periodMs, uint32_t deadlineMs, uint32_t execTimeUs, uint16_t ledPin, volatile uint32_t *deadlineMiss, volatile uint32_t *responseUs, volatile uint32_t *responseMaxUs);
static void EdfActivate(uint32_t id, uint32_t nowMs);
static uint8_t EdfPopJob(uint32_t id);
static uint8_t EdfTaskReady(uint32_t id);
static uint32_t EdfTaskDeadline(uint32_t id);
static void EdfUpdatePriorities(void);
static void EdfWaitForJob(uint32_t id);
static void EdfFinishJob(uint32_t id);
static void EdfRunJob(uint32_t id);
static void ActivateTaskA(void);
static void ActivateTaskB(void);
static void ActivateTaskC(void);
static void ActivateTaskD(void);
static void CheckAndActivateTasks_1MS(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint32_t deadlineMissA = 0;
volatile uint32_t deadlineMissB = 0;
volatile uint32_t deadlineMissC = 0;
volatile uint32_t deadlineMissD = 0;

volatile uint32_t responseTaskAUs = 0;
volatile uint32_t responseTaskBUs = 0;
volatile uint32_t responseTaskCUs = 0;
volatile uint32_t responseTaskDUs = 0;
volatile uint32_t responseTaskAMaxUs = 0;
volatile uint32_t responseTaskBMaxUs = 0;
volatile uint32_t responseTaskCMaxUs = 0;
volatile uint32_t responseTaskDMaxUs = 0;

volatile uint32_t cpuLoadPermille = 0;
volatile uint32_t cpuIdlePermille = 0;
volatile uint32_t cpuLoadMeasureUs = 0;
volatile uint32_t cpuLoadIdleUs = 0;

static volatile uint32_t releaseTaskA = 0;
static volatile uint32_t releaseTaskB = 0;
static volatile uint32_t releaseTaskC = 0;
static volatile uint32_t releaseTaskD = 0;

static EdfTask edfTasks[EDF_TASK_COUNT];
volatile uint32_t edfReadyMask = 0;
volatile uint32_t edfCurrentOrder[EDF_TASK_COUNT] = {0};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  DWT_Init();

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of semTaskA */
  semTaskAHandle = osSemaphoreNew(1, 0, &semTaskA_attributes);

  /* creation of semTaskB */
  semTaskBHandle = osSemaphoreNew(1, 0, &semTaskB_attributes);

  /* creation of semTaskC */
  semTaskCHandle = osSemaphoreNew(1, 0, &semTaskC_attributes);

  /* creation of semTaskD */
  semTaskDHandle = osSemaphoreNew(1, 0, &semTaskD_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskA */
  TaskAHandle = osThreadNew(StartTaskA, NULL, &TaskA_attributes);

  /* creation of TaskB */
  TaskBHandle = osThreadNew(StartTaskB, NULL, &TaskB_attributes);

  /* creation of TaskC */
  TaskCHandle = osThreadNew(StartTaskC, NULL, &TaskC_attributes);

  /* creation of TaskD */
  TaskDHandle = osThreadNew(StartTaskD, NULL, &TaskD_attributes);

  /* creation of SchedTask */
  SchedTaskHandle = osThreadNew(StartSchedTask, NULL, &SchedTask_attributes);

  /* creation of startDefaultTas */
  startDefaultTasHandle = osThreadNew(startDefaultTask, NULL, &startDefaultTas_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  EdfInit();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 124;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 99;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2S3_WS_Pin */
  GPIO_InitStruct.Pin = I2S3_WS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(I2S3_WS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : I2S3_MCK_Pin I2S3_SCK_Pin I2S3_SD_Pin */
  GPIO_InitStruct.Pin = I2S3_MCK_Pin|I2S3_SCK_Pin|I2S3_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_FS_Pin */
  GPIO_InitStruct.Pin = VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_ID_Pin OTG_FS_DM_Pin OTG_FS_DP_Pin */
  GPIO_InitStruct.Pin = OTG_FS_ID_Pin|OTG_FS_DM_Pin|OTG_FS_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Audio_SCL_Pin Audio_SDA_Pin */
  GPIO_InitStruct.Pin = Audio_SCL_Pin|Audio_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void busy_delay(uint32_t us)
{
  const uint32_t cyclesPerUs = SystemCoreClock / 1000000U;
  const uint32_t delayCycles = us * cyclesPerUs;
  const uint32_t startCycles = DWT->CYCCNT;

  while ((uint32_t)(DWT->CYCCNT - startCycles) < delayCycles)
  {
    __NOP();
  }
}

static uint32_t CyclesToUs(uint32_t cycles)
{
  return cycles / (SystemCoreClock / 1000000U);
}

static uint8_t DeadlineMissed(uint32_t releaseCycle, uint32_t deadlineUs, volatile uint32_t *lastUs, volatile uint32_t *maxUs)
{
  uint32_t responseUs = CyclesToUs(DWT->CYCCNT - releaseCycle);
  *lastUs = responseUs;

  if (responseUs > *maxUs)
  {
    *maxUs = responseUs;
  }

  return (responseUs > deadlineUs) ? 1U : 0U;
}

static void EdfInit(void)
{
  EdfSetTask(EDF_TASK_A, TaskAHandle, semTaskAHandle, TaskA_PERIOD_MS, TaskA_DEADLINE_MS, TaskA_EXEC_TIME_US, GPIO_PIN_12, &deadlineMissA, &responseTaskAUs, &responseTaskAMaxUs);
  EdfSetTask(EDF_TASK_B, TaskBHandle, semTaskBHandle, TaskB_PERIOD_MS, TaskB_DEADLINE_MS, TaskB_EXEC_TIME_US, GPIO_PIN_13, &deadlineMissB, &responseTaskBUs, &responseTaskBMaxUs);
  EdfSetTask(EDF_TASK_C, TaskCHandle, semTaskCHandle, TaskC_PERIOD_MS, TaskC_DEADLINE_MS, TaskC_EXEC_TIME_US, GPIO_PIN_14, &deadlineMissC, &responseTaskCUs, &responseTaskCMaxUs);
  EdfSetTask(EDF_TASK_D, TaskDHandle, semTaskDHandle, TaskD_PERIOD_MS, TaskD_DEADLINE_MS, TaskD_EXEC_TIME_US, GPIO_PIN_15, &deadlineMissD, &responseTaskDUs, &responseTaskDMaxUs);
}

static void EdfSetTask(uint32_t id, osThreadId_t handle, osSemaphoreId_t sem, uint32_t periodMs, uint32_t deadlineMs, uint32_t execTimeUs, uint16_t ledPin, volatile uint32_t *deadlineMiss, volatile uint32_t *responseUs, volatile uint32_t *responseMaxUs)
{
  edfTasks[id].handle = handle;
  edfTasks[id].sem = sem;
  edfTasks[id].periodMs = periodMs;
  edfTasks[id].relativeDeadlineUs = deadlineMs * 1000U;
  edfTasks[id].execTimeUs = execTimeUs;
  edfTasks[id].ledPin = ledPin;
  edfTasks[id].head = 0U;
  edfTasks[id].tail = 0U;
  edfTasks[id].jobs = 0U;
  edfTasks[id].active = 0U;
  edfTasks[id].activeReleaseCycle = 0U;
  edfTasks[id].activeDeadlineUs = 0U;
  edfTasks[id].nextReleaseMs = periodMs;
  edfTasks[id].queueOverflows = 0U;
  edfTasks[id].deadlineMiss = deadlineMiss;
  edfTasks[id].responseUs = responseUs;
  edfTasks[id].responseMaxUs = responseMaxUs;
}

static void EdfActivate(uint32_t id, uint32_t nowMs)
{
  EdfTask *task = &edfTasks[id];
  uint32_t primask = __get_PRIMASK();
  uint32_t releaseCycle = DWT->CYCCNT;
  uint32_t absoluteDeadlineUs = CyclesToUs(releaseCycle) + task->relativeDeadlineUs;

  __disable_irq();
  task->nextReleaseMs = nowMs + task->periodMs;

  if (task->jobs < EDF_QUEUE_LEN)
  {
    task->releaseQueue[task->tail] = releaseCycle;
    task->deadlineQueue[task->tail] = absoluteDeadlineUs;
    task->tail = (task->tail + 1U) % EDF_QUEUE_LEN;
    task->jobs++;
  }
  else
  {
    task->queueOverflows++;
    (*task->deadlineMiss)++;
  }

  if (primask == 0U)
  {
    __enable_irq();
  }

  (void)osSemaphoreRelease(task->sem);
  EdfUpdatePriorities();
}

static uint8_t EdfPopJob(uint32_t id)
{
  EdfTask *task = &edfTasks[id];
  uint32_t primask = __get_PRIMASK();
  uint8_t jobFound = 0U;

  __disable_irq();
  if (task->jobs > 0U)
  {
    task->activeReleaseCycle = task->releaseQueue[task->head];
    task->activeDeadlineUs = task->deadlineQueue[task->head];
    task->head = (task->head + 1U) % EDF_QUEUE_LEN;
    task->jobs--;
    task->active = 1U;
    jobFound = 1U;
  }

  if (primask == 0U)
  {
    __enable_irq();
  }

  return jobFound;
}

static uint8_t EdfTaskReady(uint32_t id)
{
  return ((edfTasks[id].active != 0U) || (edfTasks[id].jobs > 0U)) ? 1U : 0U;
}

static uint32_t EdfTaskDeadline(uint32_t id)
{
  EdfTask *task = &edfTasks[id];

  if (task->active != 0U)
  {
    return task->activeDeadlineUs;
  }

  if (task->jobs > 0U)
  {
    return task->deadlineQueue[task->head];
  }

  return 0xFFFFFFFFU;
}

static void EdfUpdatePriorities(void)
{
  const osPriority_t prio[EDF_TASK_COUNT] = { osPriorityAboveNormal, osPriorityNormal, osPriorityBelowNormal, osPriorityLow };
  uint8_t used[EDF_TASK_COUNT] = {0};
  uint32_t rank;
  uint32_t i;

  edfReadyMask = 0U;
  for (i = 0U; i < EDF_TASK_COUNT; i++)
  {
    edfCurrentOrder[i] = 0xFFFFFFFFU;
    if (EdfTaskReady(i))
    {
      edfReadyMask |= (1U << i);
    }
    else
    {
      (void)osThreadSetPriority(edfTasks[i].handle, osPriorityLow);
    }
  }

  for (rank = 0U; rank < EDF_TASK_COUNT; rank++)
  {
    uint32_t bestTask = 0xFFFFFFFFU;
    uint32_t bestDeadline = 0xFFFFFFFFU;

    for (i = 0U; i < EDF_TASK_COUNT; i++)
    {
      uint32_t deadline = EdfTaskDeadline(i);

      if ((used[i] == 0U) && EdfTaskReady(i) && ((bestTask == 0xFFFFFFFFU) || ((int32_t)(deadline - bestDeadline) < 0)))
      {
        bestTask = i;
        bestDeadline = deadline;
      }
    }

    if (bestTask != 0xFFFFFFFFU)
    {
      used[bestTask] = 1U;
      edfCurrentOrder[rank] = bestTask;
      (void)osThreadSetPriority(edfTasks[bestTask].handle, prio[rank]);
    }
  }
}

static void EdfWaitForJob(uint32_t id)
{
  while (EdfPopJob(id) == 0U)
  {
    (void)osSemaphoreAcquire(edfTasks[id].sem, osWaitForever);
  }
}

static void EdfFinishJob(uint32_t id)
{
  EdfTask *task = &edfTasks[id];

  if (DeadlineMissed(task->activeReleaseCycle, task->relativeDeadlineUs, task->responseUs, task->responseMaxUs))
  {
    (*task->deadlineMiss)++;
  }

  task->active = 0U;
  EdfUpdatePriorities();
}

static void EdfRunJob(uint32_t id)
{
  EdfTask *task = &edfTasks[id];

  EdfWaitForJob(id);
  HAL_GPIO_WritePin(GPIOD, task->ledPin, GPIO_PIN_SET);
  busy_delay(task->execTimeUs);
  HAL_GPIO_WritePin(GPIOD, task->ledPin, GPIO_PIN_RESET);
  EdfFinishJob(id);
}

static void ActivateTaskA(void)
{
  releaseTaskA = DWT->CYCCNT;
  EdfActivate(EDF_TASK_A, osKernelGetTickCount());
}

static void ActivateTaskB(void)
{
  releaseTaskB = DWT->CYCCNT;
  EdfActivate(EDF_TASK_B, osKernelGetTickCount());
}

static void ActivateTaskC(void)
{
  releaseTaskC = DWT->CYCCNT;
  EdfActivate(EDF_TASK_C, osKernelGetTickCount());
}

static void ActivateTaskD(void)
{
  releaseTaskD = DWT->CYCCNT;
  EdfActivate(EDF_TASK_D, osKernelGetTickCount());
}

static void CheckAndActivateTasks_1MS(void)
{
  static uint32_t ms_cnt = 0U;

  if ((ms_cnt % TaskA_PERIOD_MS) == 0U) { ActivateTaskA(); }
  if ((ms_cnt % TaskB_PERIOD_MS) == 0U) { ActivateTaskB(); }
  if ((ms_cnt % TaskC_PERIOD_MS) == 0U) { ActivateTaskC(); }
  if ((ms_cnt % TaskD_PERIOD_MS) == 0U) { ActivateTaskD(); }

  ms_cnt++;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartTaskA */
/**
  * @brief  Function implementing the TaskA thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTaskA */
void StartTaskA(void *argument)
{
  /* USER CODE BEGIN 5 */
  (void)argument;

  for (;;)
  {
    EdfRunJob(EDF_TASK_A);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTaskB */
/**
* @brief Function implementing the TaskB thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskB */
void StartTaskB(void *argument)
{
  /* USER CODE BEGIN StartTaskB */
  (void)argument;

  for (;;)
  {
    EdfRunJob(EDF_TASK_B);
  }
  /* USER CODE END StartTaskB */
}

/* USER CODE BEGIN Header_StartTaskC */
/**
* @brief Function implementing the TaskC thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskC */
void StartTaskC(void *argument)
{
  /* USER CODE BEGIN StartTaskC */
  (void)argument;

  for (;;)
  {
    EdfRunJob(EDF_TASK_C);
  }
  /* USER CODE END StartTaskC */
}

/* USER CODE BEGIN Header_StartTaskD */
/**
* @brief Function implementing the TaskD thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskD */
void StartTaskD(void *argument)
{
  /* USER CODE BEGIN StartTaskD */
  (void)argument;

  for (;;)
  {
    EdfRunJob(EDF_TASK_D);
  }
  /* USER CODE END StartTaskD */
}

/* USER CODE BEGIN Header_StartSchedTask */
/**
* @brief Function implementing the SchedTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSchedTask */
void StartSchedTask(void *argument)
{
  /* USER CODE BEGIN StartSchedTask */
  (void)argument;

  uint32_t next1MS = osKernelGetTickCount();

  for (;;)
  {
    next1MS += 1U;
    CheckAndActivateTasks_1MS();
    osDelayUntil(next1MS);
  }
  /* USER CODE END StartSchedTask */
}

/* USER CODE BEGIN Header_startDefaultTask */
/**
* @brief Function implementing the startDefaultTas thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startDefaultTask */
void startDefaultTask(void *argument)
{
  /* USER CODE BEGIN startDefaultTask */
  (void)argument;

  uint32_t startCycles = DWT->CYCCNT;
  uint32_t idleTimeUs = 0U;
  const uint32_t cyclesPerUs = SystemCoreClock / 1000000U;

  for(;;)
  {
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
  /* USER CODE END startDefaultTask */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
