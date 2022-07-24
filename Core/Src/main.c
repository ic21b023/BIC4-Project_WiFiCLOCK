/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "SerialProtocol/serialprotocol.h"
#include "WiFiProtocol/wifiprotocol.h"
#include "Rotary/rotary.h"
#include "Button/button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */
#define BRIGHTNESSTEST
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

typedef enum
{
	STATE_RUNNING_SBY = 0x01, 	/* State-Standby */
	STATE_SET_CLK_HRS = 0x02, 	/* State-Uhrzeit Stunden setzen */
	STATE_SET_CLK_MIN = 0x03, 	/* State-Uhrzeit Minuten setzen */
	STATE_SET_CLK_SEC = 0x04, 	/* State-Uhrzeit Sekunden setzen */
	STATE_SET_ALT_HRS = 0x05, 	/* State-Alarmzeit Stunden setzen */
	STATE_SET_ALT_MIN = 0x06, 	/* State-Alarmzeit Minuten setzen */
	STATE_SET_ALT_SEC = 0x07, 	/* State-Alarmzeit Sekunden setzen */
	STATE_RUNNING_ALT = 0x08, 	/* State-Standby mit Alarm aktiviert */
	STATE_ALT = 0x09 			/* State-Alarm */

} State_t;

typedef enum
{
	EV_ENC_LONG = 0x10, 		/* Encoder lange gedrückt */
	EV_ENC_SHORT = 0x11, 		/* Encoder kurz gedrückt */
	EV_ENC_PLUS = 0x12, 		/* Encoder Rechtsdreh */
	EV_ENC_MINUS = 0x13, 		/* Encoder Linksdreh */
	EV_BTN_LONG = 0x14, 		/* Button lange gedrückt */
	EV_BTN_SHORT = 0x15, 		/* Button kurz gedrückt */
	EV_ALT = 0x16, 				/* Alarm */
	EV_ENC_SHORT_RMT = 0x17, 	/* Encoder kurz gedrückt - über UART-Kommando */
	EV_ENC_LONG_RMT = 0x18, 	/* Encoder lange gedrückt - über UART-Kommando */
	EV_NOEVENT = 0x19 			/* kein Event */

} Event_t;

typedef enum
{
	CMD_CLK_HRS_PLUS = 0x20, 	/* Uhrzeit- Stunden um eins erhöhen */
	CMD_CLK_HRS_MINUS = 0x21, 	/* Uhrzeit- Stunden um eins senken */
	CMD_CLK_MIN_PLUS = 0x22, 	/* Uhrzeit- Minuten um eins erhöhen */
	CMD_CLK_MIN_MINUS = 0x23, 	/* Uhrzeit- Minuten um eins senken */
	CMD_CLK_SEC_PLUS = 0x24, 	/* Uhrzeit- Sekunden um eins erhöhen */
	CMD_CLK_SEC_MINUS = 0x25, 	/* Uhrzeit- Sekunden um eins senken */

	CMD_ALT_HRS_PLUS = 0x26, 	/* Alarmzeit- Stunden um eins erhöhen */
	CMD_ALT_HRS_MINUS = 0x27, 	/* Alarmzeit- Stunden um eins senken */
	CMD_ALT_MIN_PLUS = 0x28, 	/* Alarmzeit- Minuten um eins erhöhen */
	CMD_ALT_MIN_MINUS = 0x29, 	/* Alarmzeit- Minuten um eins senken */
	CMD_ALT_SEC_PLUS = 0x2A, 	/* Alarmzeit- Sekunden um eins erhöhen */
	CMD_ALT_SEC_MINUS = 0x2B, 	/* Alarmzeit- Sekunden um eins senken */

	CMD_ALT_ON = 0x2C, 		/* Alarm aktivieren */
	CMD_ALT_OFF = 0x2D, 	/* Alarm deaktivieren */
	CMD_ALARM = 0x2E, 		/* Alarm */
	CMD_ALT_QUIT = 0x2F, 	/* Alarm quittieren */

	CMD_ALT_ON_RMT = 0x30, 		/* Alarm über UART aktivieren */
	CMD_ALT_OFF_RMT = 0x31, 	/* Alarm über UART deaktivieren */
	CMD_ALT_QUIT_RMT = 0x32, 	/* Alarm über UART quittieren */

	CMD_UART_PRINT_TIME = 0x33, 		/* Uhrzeit über UART ausgeben */
	CMD_UART_PRINT_CLK_HRS = 0x34, 		/* Uhrzeit- Stunden über UART ausgeben */
	CMD_UART_PRINT_CLK_MIN = 0x35, 		/* Uhrzeit- Minuten über UART ausgeben */
	CMD_UART_PRINT_CLK_SEC = 0x36, 		/* Uhrzeit- Sekunden über UART ausgeben */
	CMD_UART_PRINT_ALARM = 0x37, 		/* Alarmzeit über UART ausgeben */
	CMD_UART_PRINT_ALT_HRS = 0x38, 		/* Alarmzeit- Stunden über UART ausgeben */
	CMD_UART_PRINT_ALT_MIN = 0x39, 		/* Alarmzeit- Minuten über UART ausgeben */
	CMD_UART_PRINT_ALT_SEC = 0x3A, 		/* Alarmzeit- Sekunden über UART ausgeben */
	CMD_UART_PRINT_ALT_STATE = 0x3B, 	/* Alarmstatus über UART ausgeben */

	CMD_CLK_SET_HRS = 0x3C, /* Rotary-Status / Alarmzeit- Stunden setzen */
	CMD_CLK_SET_MIN = 0x3D, /* Rotary-Status / Alarmzeit- Minuten setzen */
	CMD_CLK_SET_SEC = 0x3E, /* Rotary-Status / Alarmzeit- Sekunden setzen */
	CMD_ALT_SET_HRS = 0x3F, /* Rotary-Status / Alarmzeit- Stunden setzen */
	CMD_ALT_SET_MIN = 0x40, /* Rotary-Status / Alarmzeit- Minuten setzen */
	CMD_ALT_SET_SEC = 0x41, /* Rotary-Status / Alarmzeit- Sekunden setzen */
} Command_t;

typedef enum
{
	RECEIVER_TASK_CORE = 0x01, 		/* State-Standby */
	RECEIVER_TASK_ROTARY = 0x02, 	/* State-Uhrzeit Stunden setzen */
	RECEIVER_TASK_CLOCK = 0x03, 	/* State-Uhrzeit Minuten setzen */
	RECEIVER_TASK_UART = 0x04, 		/* State-Uhrzeit Sekunden setzen */
	RECEIVER_TASK_WIFI = 0x05, 		/* State-Alarmzeit Stunden setzen */

} ReceiverTask_t;

/*!< Nachrichtenstruktur der Queues */
typedef struct {
	ReceiverTask_t ReceiverTask; 	/*!< Empfängertask */
	char Message[30];     			/*!< Nachricht */
	int  Value; 					/*!< Wert */
	State_t State; 					/*!< State */
	Command_t Command; 				/*!< Kommando */
} QueueItem_t;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;

/* Definitions for TaskCORE */
osThreadId_t TaskCOREHandle;
uint32_t TaskCOREBuffer[ 512 ];
osStaticThreadDef_t TaskCOREControlBlock;
const osThreadAttr_t TaskCORE_attributes = {
  .name = "TaskCORE",
  .cb_mem = &TaskCOREControlBlock,
  .cb_size = sizeof(TaskCOREControlBlock),
  .stack_mem = &TaskCOREBuffer[0],
  .stack_size = sizeof(TaskCOREBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskRotary */
osThreadId_t TaskRotaryHandle;
uint32_t TaskRotaryBuffer[ 512 ];
osStaticThreadDef_t TaskRotaryControlBlock;
const osThreadAttr_t TaskRotary_attributes = {
  .name = "TaskRotary",
  .cb_mem = &TaskRotaryControlBlock,
  .cb_size = sizeof(TaskRotaryControlBlock),
  .stack_mem = &TaskRotaryBuffer[0],
  .stack_size = sizeof(TaskRotaryBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskClock */
osThreadId_t TaskClockHandle;
uint32_t TaskClockBuffer[ 700 ];
osStaticThreadDef_t TaskClockControlBlock;
const osThreadAttr_t TaskClock_attributes = {
  .name = "TaskClock",
  .cb_mem = &TaskClockControlBlock,
  .cb_size = sizeof(TaskClockControlBlock),
  .stack_mem = &TaskClockBuffer[0],
  .stack_size = sizeof(TaskClockBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskUART2 */
osThreadId_t TaskUART2Handle;
uint32_t TaskUART2Buffer[ 800 ];
osStaticThreadDef_t TaskUART2ControlBlock;
const osThreadAttr_t TaskUART2_attributes = {
  .name = "TaskUART2",
  .cb_mem = &TaskUART2ControlBlock,
  .cb_size = sizeof(TaskUART2ControlBlock),
  .stack_mem = &TaskUART2Buffer[0],
  .stack_size = sizeof(TaskUART2Buffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskWiFi */
osThreadId_t TaskWiFiHandle;
uint32_t TaskWiFiBuffer[ 512 ];
osStaticThreadDef_t TaskWiFiControlBlock;
const osThreadAttr_t TaskWiFi_attributes = {
  .name = "TaskWiFi",
  .cb_mem = &TaskWiFiControlBlock,
  .cb_size = sizeof(TaskWiFiControlBlock),
  .stack_mem = &TaskWiFiBuffer[0],
  .stack_size = sizeof(TaskWiFiBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskRotaryLed */
osThreadId_t TaskRotaryLedHandle;
uint32_t TaskRotaryLedBuffer[ 256 ];
osStaticThreadDef_t TaskRotaryLedControlBlock;
const osThreadAttr_t TaskRotaryLed_attributes = {
  .name = "TaskRotaryLed",
  .cb_mem = &TaskRotaryLedControlBlock,
  .cb_size = sizeof(TaskRotaryLedControlBlock),
  .stack_mem = &TaskRotaryLedBuffer[0],
  .stack_size = sizeof(TaskRotaryLedBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for QueueFromRotary */
osMessageQueueId_t QueueFromRotaryHandle;
uint8_t QueueFromRotaryBuffer[ 16 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueFromRotaryControlBlock;
const osMessageQueueAttr_t QueueFromRotary_attributes = {
  .name = "QueueFromRotary",
  .cb_mem = &QueueFromRotaryControlBlock,
  .cb_size = sizeof(QueueFromRotaryControlBlock),
  .mq_mem = &QueueFromRotaryBuffer,
  .mq_size = sizeof(QueueFromRotaryBuffer)
};
/* Definitions for QueueToRotary */
osMessageQueueId_t QueueToRotaryHandle;
uint8_t QueueToRotaryBuffer[ 16 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueToRotaryControlBlock;
const osMessageQueueAttr_t QueueToRotary_attributes = {
  .name = "QueueToRotary",
  .cb_mem = &QueueToRotaryControlBlock,
  .cb_size = sizeof(QueueToRotaryControlBlock),
  .mq_mem = &QueueToRotaryBuffer,
  .mq_size = sizeof(QueueToRotaryBuffer)
};
/* Definitions for QueueFromClock */
osMessageQueueId_t QueueFromClockHandle;
uint8_t QueueFromClockBuffer[ 16 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueFromClockControlBlock;
const osMessageQueueAttr_t QueueFromClock_attributes = {
  .name = "QueueFromClock",
  .cb_mem = &QueueFromClockControlBlock,
  .cb_size = sizeof(QueueFromClockControlBlock),
  .mq_mem = &QueueFromClockBuffer,
  .mq_size = sizeof(QueueFromClockBuffer)
};
/* Definitions for QueueToClock */
osMessageQueueId_t QueueToClockHandle;
uint8_t QueueToClockBuffer[ 16 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueToClockControlBlock;
const osMessageQueueAttr_t QueueToClock_attributes = {
  .name = "QueueToClock",
  .cb_mem = &QueueToClockControlBlock,
  .cb_size = sizeof(QueueToClockControlBlock),
  .mq_mem = &QueueToClockBuffer,
  .mq_size = sizeof(QueueToClockBuffer)
};
/* Definitions for QueueFromUART2 */
osMessageQueueId_t QueueFromUART2Handle;
uint8_t QueueFromUART2Buffer[ 16 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueFromUART2ControlBlock;
const osMessageQueueAttr_t QueueFromUART2_attributes = {
  .name = "QueueFromUART2",
  .cb_mem = &QueueFromUART2ControlBlock,
  .cb_size = sizeof(QueueFromUART2ControlBlock),
  .mq_mem = &QueueFromUART2Buffer,
  .mq_size = sizeof(QueueFromUART2Buffer)
};
/* Definitions for QueueToUART2 */
osMessageQueueId_t QueueToUART2Handle;
uint8_t QueueToUART2Buffer[ 16 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueToUART2ControlBlock;
const osMessageQueueAttr_t QueueToUART2_attributes = {
  .name = "QueueToUART2",
  .cb_mem = &QueueToUART2ControlBlock,
  .cb_size = sizeof(QueueToUART2ControlBlock),
  .mq_mem = &QueueToUART2Buffer,
  .mq_size = sizeof(QueueToUART2Buffer)
};
/* Definitions for QueueFromWiFi */
osMessageQueueId_t QueueFromWiFiHandle;
uint8_t QueueFromWiFiBuffer[ 16 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueFromWiFiControlBlock;
const osMessageQueueAttr_t QueueFromWiFi_attributes = {
  .name = "QueueFromWiFi",
  .cb_mem = &QueueFromWiFiControlBlock,
  .cb_size = sizeof(QueueFromWiFiControlBlock),
  .mq_mem = &QueueFromWiFiBuffer,
  .mq_size = sizeof(QueueFromWiFiBuffer)
};
/* Definitions for QueueToWifi */
osMessageQueueId_t QueueToWifiHandle;
uint8_t QueueToWifiBuffer[ 1 * sizeof( QueueItem_t ) ];
osStaticMessageQDef_t QueueToWifiControlBlock;
const osMessageQueueAttr_t QueueToWifi_attributes = {
  .name = "QueueToWifi",
  .cb_mem = &QueueToWifiControlBlock,
  .cb_size = sizeof(QueueToWifiControlBlock),
  .mq_mem = &QueueToWifiBuffer,
  .mq_size = sizeof(QueueToWifiBuffer)
};
/* USER CODE BEGIN PV */
WIFIPROTOCOL_HandleTypeDef hwifiprotocol_uart1;
SERIALPROTOCOL_HandleTypeDef hserialprotocol_uart2;
ROTARY_HandleTypeDef hrotary;
BUTTON_HandleTypeDef hbutton;
uint16_t rotary_volume=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM6_Init(void);
void FunctionCORETask(void *argument);
void FunctionRotaryTask(void *argument);
void FunctionClockTask(void *argument);
void FunctionUART2Task(void *argument);
void FunctionWiFiTask(void *argument);
void FunctionTaskRotaryLed(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  /* UART1-EmpfangsInterrupt-Flag setzen */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1,(uint8_t*) hwifiprotocol_uart1.Communication.Rx,140);
  __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

  /* UART2-EmpfangsInterrupt-Flag setzen */
  HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t *) hserialprotocol_uart2.inputMessageBuffer, SERIALPROTOCOL_MessageBuffer_SIZE);

  /* RGB-LED ausschalten */
  HAL_GPIO_WritePin(RGB_GN_GPIO_Port, RGB_GN_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RGB_RT_GPIO_Port, RGB_RT_Pin, GPIO_PIN_SET);

  /* Rotary initialisieren */
  ROTARY_Init_with_LEDs(&hrotary,&htim6,MOSI_GPIO_Port, MOSI_Pin, SCK_GPIO_Port, SCK_Pin, CS_2_GPIO_Port, CS_2_Pin,ENC_A_GPIO_Port,ENC_A_Pin,ENC_B_GPIO_Port,ENC_B_Pin,SWITCH_GPIO_Port,SWITCH_Pin);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of QueueFromRotary */
  QueueFromRotaryHandle = osMessageQueueNew (16, sizeof(QueueItem_t), &QueueFromRotary_attributes);

  /* creation of QueueToRotary */
  QueueToRotaryHandle = osMessageQueueNew (16, sizeof(QueueItem_t), &QueueToRotary_attributes);

  /* creation of QueueFromClock */
  QueueFromClockHandle = osMessageQueueNew (16, sizeof(QueueItem_t), &QueueFromClock_attributes);

  /* creation of QueueToClock */
  QueueToClockHandle = osMessageQueueNew (16, sizeof(QueueItem_t), &QueueToClock_attributes);

  /* creation of QueueFromUART2 */
  QueueFromUART2Handle = osMessageQueueNew (16, sizeof(QueueItem_t), &QueueFromUART2_attributes);

  /* creation of QueueToUART2 */
  QueueToUART2Handle = osMessageQueueNew (16, sizeof(QueueItem_t), &QueueToUART2_attributes);

  /* creation of QueueFromWiFi */
  QueueFromWiFiHandle = osMessageQueueNew (16, sizeof(QueueItem_t), &QueueFromWiFi_attributes);

  /* creation of QueueToWifi */
  QueueToWifiHandle = osMessageQueueNew (1, sizeof(QueueItem_t), &QueueToWifi_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskCORE */
  TaskCOREHandle = osThreadNew(FunctionCORETask, NULL, &TaskCORE_attributes);

  /* creation of TaskRotary */
  TaskRotaryHandle = osThreadNew(FunctionRotaryTask, NULL, &TaskRotary_attributes);

  /* creation of TaskClock */
  TaskClockHandle = osThreadNew(FunctionClockTask, NULL, &TaskClock_attributes);

  /* creation of TaskUART2 */
  TaskUART2Handle = osThreadNew(FunctionUART2Task, NULL, &TaskUART2_attributes);

  /* creation of TaskWiFi */
  TaskWiFiHandle = osThreadNew(FunctionWiFiTask, NULL, &TaskWiFi_attributes);

  /* creation of TaskRotaryLed */
  TaskRotaryLedHandle = osThreadNew(FunctionTaskRotaryLed, NULL, &TaskRotaryLed_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 16;
  sTime.Minutes = 1;
  sTime.Seconds = 20;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 16-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65536-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RGB_RT_Pin|RGB_GN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SCK_Pin|MOSI_Pin|CS_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ENC_B_Pin SWITCH_Pin ENC_A_Pin */
  GPIO_InitStruct.Pin = ENC_B_Pin|SWITCH_Pin|ENC_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RGB_RT_Pin RGB_GN_Pin */
  GPIO_InitStruct.Pin = RGB_RT_Pin|RGB_GN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SCK_Pin MOSI_Pin CS_2_Pin */
  GPIO_InitStruct.Pin = SCK_Pin|MOSI_Pin|CS_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Command_Callback(SERIALPROTOCOL_HandleTypeDef *hserialprot)
{
		QueueItem_t queueItem;

		/* Queueitem an Clock-Task senden */
		queueItem.ReceiverTask=RECEIVER_TASK_CLOCK;

		/* Uhrzeit- ausgeben */
		if (!strcmp(hserialprot->Command.Name, "time"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){

			}else{
				queueItem.Command= CMD_UART_PRINT_TIME;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		} /* Uhrzeit- Stunden setzen oder ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "time_hours"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){

				if(atoi(hserialprot->Command.Parameter[0].Value)<24){
					queueItem.Value=atoi(hserialprot->Command.Parameter[0].Value);
					queueItem.Command=CMD_CLK_SET_HRS;
					osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
					return SERIALPROTOCOL_STATUS_OK;
				}else{
					return SERIALPROTOCOL_STATUS_ERROR;
				}
			}else{
				queueItem.Command= CMD_UART_PRINT_CLK_HRS;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		} /* Uhrzeit- Minuten setzen oder ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "time_minutes"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){
				if(atoi(hserialprot->Command.Parameter[0].Value)<60){
					queueItem.Value=atoi(hserialprot->Command.Parameter[0].Value);
					queueItem.Command=CMD_CLK_SET_MIN;
					osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
					return SERIALPROTOCOL_STATUS_OK;
				}else{
					return SERIALPROTOCOL_STATUS_ERROR;
				}
			}else{
				queueItem.Command= CMD_UART_PRINT_CLK_MIN;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		} /* Uhrzeit- Sekunden setzen oder ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "time_seconds"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){
				if(atoi(hserialprot->Command.Parameter[0].Value)<60){
					queueItem.Value=atoi(hserialprot->Command.Parameter[0].Value);
					queueItem.Command=CMD_CLK_SET_SEC;
					osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
					return SERIALPROTOCOL_STATUS_OK;
				}else{
					return SERIALPROTOCOL_STATUS_ERROR;
				}
			}else{
				queueItem.Command= CMD_UART_PRINT_CLK_SEC;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		}/* Alarm- ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "alarm"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){

			}else{
				queueItem.Command= CMD_UART_PRINT_ALARM;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		}/* Alarm- Stunden setzen oder ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "alarm_hours"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){

				if(atoi(hserialprot->Command.Parameter[0].Value)<24){
					queueItem.Value=atoi(hserialprot->Command.Parameter[0].Value);
					queueItem.Command=CMD_ALT_SET_HRS;
					osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
					return SERIALPROTOCOL_STATUS_OK;
				}else{
					return SERIALPROTOCOL_STATUS_ERROR;
				}

			}else{
				queueItem.Command= CMD_UART_PRINT_ALT_HRS;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		}/* Alarm- Minuten setzen oder ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "alarm_minutes"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){

				if(atoi(hserialprot->Command.Parameter[0].Value)<60){
					queueItem.Value=atoi(hserialprot->Command.Parameter[0].Value);
					queueItem.Command=CMD_ALT_SET_MIN;
					osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
					return SERIALPROTOCOL_STATUS_OK;
				}else{
					return SERIALPROTOCOL_STATUS_ERROR;
				}
			}else{
				queueItem.Command= CMD_UART_PRINT_ALT_MIN;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		}/* Alarm- Sekunden setzen oder ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "alarm_seconds"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){
				if(atoi(hserialprot->Command.Parameter[0].Value)<60){
					queueItem.Value=atoi(hserialprot->Command.Parameter[0].Value);
					queueItem.Command=CMD_ALT_SET_SEC;
					osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
					return SERIALPROTOCOL_STATUS_OK;
				}else{
					return SERIALPROTOCOL_STATUS_ERROR;
				}
			}else{
				queueItem.Command= CMD_UART_PRINT_ALT_SEC;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		}/* Alarm- Status setzen oder ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "alarm_state"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){

				if(atoi(hserialprot->Command.Parameter[0].Value)==0){
					queueItem.ReceiverTask=RECEIVER_TASK_ROTARY;
					  queueItem.Command=CMD_ALT_OFF;
					  osMessageQueuePut(QueueFromClockHandle, &queueItem, 0U, 50U );
					  return SERIALPROTOCOL_STATUS_OK;

				}else if(atoi(hserialprot->Command.Parameter[0].Value)==1){
					queueItem.ReceiverTask=RECEIVER_TASK_ROTARY;
					  queueItem.Command=CMD_ALT_ON;
					  osMessageQueuePut(QueueFromClockHandle, &queueItem, 0U, 50U );
					  return SERIALPROTOCOL_STATUS_OK;
				}
				else{
					  return SERIALPROTOCOL_STATUS_ERROR;
				}

			}else{
				queueItem.Command= CMD_UART_PRINT_ALT_STATE;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}

		}/* Alarm- quitieren oder Status ausgeben */
		else if (!strcmp(hserialprot->Command.Name, "alarm_quit"))
		{
			if(hserialprot->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_EXECUTE){
				queueItem.ReceiverTask=RECEIVER_TASK_ROTARY;
				  queueItem.Command=CMD_ALT_QUIT_RMT;
				  osMessageQueuePut(QueueFromClockHandle, &queueItem, 0U, 50U );
				  return SERIALPROTOCOL_STATUS_OK;
			}else{
				queueItem.Command= CMD_UART_PRINT_ALT_STATE;
				osMessageQueuePut(QueueFromUART2Handle, &queueItem, 0U, 50U);
				return SERIALPROTOCOL_STATUS_OK;
			}
		}

	return SERIALPROTOCOL_STATUS_ERROR;
}

SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_TransmitAdapter(uint8_t * buffer, uint16_t buffersize)
{
	/* Eingaben an die Konsole senden */
	return HAL_UART_Transmit(&huart2,  buffer,buffersize, 100);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	/* Nachrichten empfangen, Empfangsevent setzen */
	if(huart->Instance==USART1){
		hwifiprotocol_uart1.Communication.Event.MessageReceived=1;
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,(uint8_t*) hwifiprotocol_uart1.Communication.Rx,140);
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
	}

	/* Nachrichten empfangen, Empfangsevent setzen */
	if(huart->Instance==USART2){
		hserialprotocol_uart2.Event.MessageReceived=1;
		HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t *) hserialprotocol_uart2.inputMessageBuffer, SERIALPROTOCOL_MessageBuffer_SIZE);
	}
}

WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TransmitAdapter(uint8_t * buffer, uint16_t buffersize)
{
	/* Nachrichten zu WiFi-Modul senden */
	return HAL_UART_Transmit(&huart1, buffer, buffersize,50);
}

WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_ReceiveAdapter(uint8_t * buffer, uint16_t buffersize)
{
	/* Nachrichten vom WiFi-Modul empfangen */
	WIFIPROTOCOL_StatusTypeDef ret = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, buffer,buffersize);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
	return ret;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_FunctionCORETask */
/**
  * @brief  Function implementing the TaskCORE thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_FunctionCORETask */
void FunctionCORETask(void *argument)
{
  /* USER CODE BEGIN 5 */
	static QueueItem_t queueItem={0};
  /* Infinite loop */
  for(;;)
  {
	  /* QueueItem vom Ratoary-Task an Receiver-Task weiterreichen */
	  if (osMessageQueueGet(QueueFromRotaryHandle, &queueItem, NULL, 1U)==osOK)
	  {
		  switch ((int)queueItem.ReceiverTask)
		  {
			case RECEIVER_TASK_CLOCK:
				osMessageQueuePut(QueueToClockHandle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_UART:
				osMessageQueuePut(QueueToUART2Handle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_WIFI:
				osMessageQueuePut(QueueToWifiHandle, &queueItem, 0U, 50U);
			break;
		  }
	  }

	  /* QueueItem vom Clock-Task an Receiver-Task weiterreichen */
	  if (osMessageQueueGet(QueueFromClockHandle, &queueItem, NULL, 1U)==osOK)
	  {
		  switch ((int)queueItem.ReceiverTask)
		  {
			case RECEIVER_TASK_ROTARY:
				osMessageQueuePut(QueueToRotaryHandle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_UART:
				osMessageQueuePut(QueueToUART2Handle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_WIFI:
				osMessageQueuePut(QueueToWifiHandle, &queueItem, 0U, 50U);
			break;
		  }
	  }

	  /* QueueItem vom UART-Task an Receiver-Task weiterreichen */
	  if (osMessageQueueGet(QueueFromUART2Handle, &queueItem, NULL, 1U)==osOK)
	  {
		  switch ((int)queueItem.ReceiverTask)
		  {
			case RECEIVER_TASK_ROTARY:
				osMessageQueuePut(QueueToRotaryHandle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_CLOCK:
				osMessageQueuePut(QueueToClockHandle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_WIFI:
				osMessageQueuePut(QueueToWifiHandle, &queueItem, 0U, 50U);
			break;
		  }
	  }

	  /* QueueItem vom Wifi-Task an Receiver-Task weiterreichen */
	  if (osMessageQueueGet(QueueFromWiFiHandle, &queueItem, NULL, 1U)==osOK)
	  {
		  switch ((int)queueItem.ReceiverTask)
		  {
			case RECEIVER_TASK_ROTARY:
				osMessageQueuePut(QueueToRotaryHandle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_CLOCK:
				osMessageQueuePut(QueueToClockHandle, &queueItem, 0U, 50U);
			break;
			case RECEIVER_TASK_UART:
				osMessageQueuePut(QueueToUART2Handle, &queueItem, 0U, 50U);
			break;
		  }
	  }

    osDelay(1);
  }

  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_FunctionRotaryTask */
/**
* @brief Function implementing the TaskRotary thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_FunctionRotaryTask */
void FunctionRotaryTask(void *argument)
{
  /* USER CODE BEGIN FunctionRotaryTask */

/* Startzustände setzen */
uint8_t state = STATE_RUNNING_SBY;
uint8_t event = EV_NOEVENT;

/* Button initialisieren */
BUTTON_Init(&hbutton, BUTTON_GPIO_Port, BUTTON_Pin);

/* Events bzw. Pinstates des Rotarys und des Buttons für die FSM einlesen und auswerten */
uint8_t readEvent(){

	/* Ermitteln ob der Rotary nach links oder rechts gedreht wurde */
	switch (ROTARY_Encoder_Switch_State(&hrotary))
	{
		case 1:
			rotary_volume++;
			return EV_ENC_PLUS;

		break;
		case 2:
			rotary_volume--;
			return EV_ENC_MINUS;

		break;
	}

	/* Ermitteln ob der Rotary-Button gedrückt wurde */
	switch (ROTARY_Encoder_Switch_Short_or_Long_Press(&hrotary))
	{
		case 1:
			return EV_ENC_LONG;
		break;
		case 2:
			return EV_NOEVENT;
		break;

		case 3:
			return EV_ENC_SHORT;
		break;

		case 4:
			return EV_NOEVENT;
		break;
	}

/* Ermitteln ob der Click-Board-Button gedrückt wurde */
	switch (BUTTON_Short_or_Long_Press(&hbutton))
	{
		case 1:
			return EV_BTN_LONG;
		break;
		case 2:
			return EV_NOEVENT;
		break;

		case 3:
			return EV_BTN_SHORT;
		break;

		case 4:
			return EV_NOEVENT;
		break;
	}

	QueueItem_t queueItem = {0};

	/* Clock-Ereignisse auslesen und demnach das Event setzen */
	if (osMessageQueueGet(QueueToRotaryHandle, &queueItem, NULL, 1U)==osOK)
	{
		switch ((uint8_t)queueItem.Command) {
			case CMD_ALARM:
				  return EV_ALT;
			break;
			case CMD_ALT_OFF:
				  return EV_ENC_LONG_RMT;
			break;
			case CMD_ALT_ON:
				  return EV_ENC_SHORT_RMT;
			break;
			case CMD_ALT_QUIT_RMT:
				  return EV_ENC_SHORT_RMT;
			break;
		}
	}
  return EV_NOEVENT;
}

/* je nach FSM-State permanent ausführen */
void PerformDoAction(unsigned char state){

	switch (state)
	{
		/* Standbay und Alarm eingeschaltet */
		case STATE_RUNNING_ALT:
			HAL_GPIO_WritePin(RGB_GN_GPIO_Port, RGB_GN_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(RGB_RT_GPIO_Port, RGB_RT_Pin, GPIO_PIN_SET);
		break;

		/* Alarm */
		case STATE_ALT:
		 HAL_GPIO_TogglePin(RGB_RT_GPIO_Port, RGB_RT_Pin);
		 HAL_GPIO_WritePin(RGB_GN_GPIO_Port, RGB_GN_Pin, GPIO_PIN_SET);
		break;

		/* Wenn kein Alarm eingeschaltet und kein Alarm ansteht */
		default:
			HAL_GPIO_WritePin(RGB_GN_GPIO_Port, RGB_GN_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(RGB_RT_GPIO_Port, RGB_RT_Pin, GPIO_PIN_SET);
		break;
	}
}

/* FSM zum Einstellen der Uhrzeit, der Alarmzeit und aktivieren/deaktivieren des Alarms */
uint8_t statemachine(unsigned char event)
{
	QueueItem_t queueItem={0};

	/* zum ClockTast senden */
	queueItem.ReceiverTask=RECEIVER_TASK_CLOCK;
	switch (state)
	{
		/* Standby und Alarm ausgeschaltet */
		case STATE_RUNNING_SBY:
			 switch (event)
			 {
			 	 /* Encoder lange gedrückt */
				 case EV_ENC_LONG:
					 state = STATE_SET_ALT_HRS;
					 queueItem.State=STATE_SET_ALT_HRS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Button lange gedrückt */
				 case EV_BTN_LONG:
					 state = STATE_SET_CLK_HRS;
					 queueItem.State=STATE_SET_CLK_HRS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder kurz gedrückt */
				 case EV_ENC_SHORT:
					 state = STATE_RUNNING_ALT;
					 queueItem.State=STATE_RUNNING_ALT;
					 queueItem.Command=CMD_ALT_ON;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder kurz gedrückt über UART */
				 case EV_ENC_SHORT_RMT:
					 state = STATE_RUNNING_ALT;
					 queueItem.State=STATE_RUNNING_ALT;
					 queueItem.Command=CMD_ALT_ON_RMT;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;
			 }
		break;

		/* Alarm- Stunden einstellen */
		case STATE_SET_ALT_HRS:
			 switch (event)
			 {
			 	 /* Encoder kurz gedrückt */
				 case EV_ENC_SHORT:
					 state = STATE_SET_ALT_MIN;
					 queueItem.State=STATE_SET_ALT_MIN;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Rechtsdreh */
				 case EV_ENC_PLUS:
					 queueItem.Command=CMD_ALT_HRS_PLUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Linksdreh */
				 case EV_ENC_MINUS:
					 queueItem.Command=CMD_ALT_HRS_MINUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;
			 }
		break;

		/* Alarm- Minuten einstellen */
		case STATE_SET_ALT_MIN:
			 switch (event)
			 {
			 	 /* Encoder kurz gedrückt */
				 case EV_ENC_SHORT:
					 state = STATE_SET_ALT_SEC;
					 queueItem.State=STATE_SET_ALT_SEC;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Rechtsdreh */
				 case EV_ENC_PLUS:
					 queueItem.Command=CMD_ALT_MIN_PLUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Linksdreh */
				 case EV_ENC_MINUS:
					 queueItem.Command=CMD_ALT_MIN_MINUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;
			 }
		break;

		/* Alarm- Sekunden einstellen */
		case STATE_SET_ALT_SEC:
			 switch (event)
			 {
			 	 /* Encoder kurz gedrückt */
				 case EV_ENC_SHORT:
					 state = STATE_RUNNING_SBY;
					 queueItem.State=STATE_RUNNING_SBY;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Rechtsdreh */
				 case EV_ENC_PLUS:
					 queueItem.Command=CMD_ALT_SEC_PLUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Linksdreh */
				 case EV_ENC_MINUS:
					 queueItem.Command=CMD_ALT_SEC_MINUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;
			 }
		break;

		/* Uhrzeit- Stunden einstellen */
		case STATE_SET_CLK_HRS:
			 switch (event)
			 {
			 	 /* Encoder kurz gedrückt */
				 case EV_ENC_SHORT:
					 state = STATE_SET_CLK_MIN;
					 queueItem.State=STATE_SET_CLK_MIN;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Rechtsdreh */
				 case EV_ENC_PLUS:
					 queueItem.Command=CMD_CLK_HRS_PLUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Linksdreh */
				 case EV_ENC_MINUS:
					 queueItem.Command=CMD_CLK_HRS_MINUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;
			 }
		break;

		/* Uhrzeit- Minuten einstellen */
		case STATE_SET_CLK_MIN:
			 switch (event)
			 {
			 	 /* Encoder kurz gedrückt */
				 case EV_ENC_SHORT:
					 state = STATE_SET_CLK_SEC;
					 queueItem.State=STATE_SET_CLK_SEC;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Rechtsdreh */
				 case EV_ENC_PLUS:
					 queueItem.Command=CMD_CLK_MIN_PLUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Linksdreh */
				 case EV_ENC_MINUS:
					 queueItem.Command=CMD_CLK_MIN_MINUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;
			 }
		break;

		/* Uhrzeit- Sekunden einstellen */
		case STATE_SET_CLK_SEC:
			 switch (event)
			 {
			 	 /* Encoder kurz gedrückt */
				 case EV_ENC_SHORT:
					 state = STATE_RUNNING_SBY;
					 queueItem.State=STATE_RUNNING_SBY;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Rechtsdreh */
				 case EV_ENC_PLUS:
					 queueItem.Command=CMD_CLK_SEC_PLUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;

				 /* Encoder Linksdreh */
				 case EV_ENC_MINUS:
					 queueItem.Command=CMD_CLK_SEC_MINUS;
					 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
				 break;
			 }
		break;

		/* Standby und Alarm aktiviert */
		case STATE_RUNNING_ALT:
				 switch (event)
				 {
				 	 /* Encoder lange gedrückt */
					 case EV_ENC_LONG:
						 state = STATE_RUNNING_SBY;
						 queueItem.State=STATE_RUNNING_SBY;
						 queueItem.Command=CMD_ALT_OFF; /* Alarm deaktivieren und zurück in den Standby */
						 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
					 break;
					 /* Encoder lange gedrückt über UART */
					 case EV_ENC_LONG_RMT:
						 state = STATE_RUNNING_SBY; /* Alarm deaktivieren und zurück in den Standby */
						 queueItem.State=STATE_RUNNING_SBY;
						 queueItem.Command=CMD_ALT_OFF_RMT;
						 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
					 break;

					 /* ALARM */
					 case EV_ALT:
						 state = STATE_ALT;
						 queueItem.State=STATE_ALT;
						 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
					 break;
				 }
		break;

		/* ALARM */
		case STATE_ALT:
				 switch (event)
				 {
				 	 /* Encoder kurz gedrückt */
					 case EV_ENC_SHORT:
						 state = STATE_RUNNING_SBY; /* Alarm resitieren und zurück in den Standby */
						 queueItem.State=STATE_RUNNING_SBY;
						 queueItem.Command=CMD_ALT_QUIT;
						 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
					 break;

					 /* Encoder kurz gedrückt über UART */
					 case EV_ENC_SHORT_RMT:
						 state = STATE_RUNNING_SBY; /* Alarm resitieren und zurück in den Standby */
						 queueItem.State=STATE_RUNNING_SBY;
						 queueItem.Command=CMD_ALT_QUIT_RMT;
						 osMessageQueuePut(QueueFromRotaryHandle, &queueItem, 0U, 50U);
					 break;
				 }
		break;
	}

  event = EV_NOEVENT;
  return state;
}
	 /* Infinite loop */
  for(;;)
  {
	  event = readEvent(); /* Ermitteln ob ein Event stattfand */
	  state = statemachine(event); /* nach dem Event den jeweiligen Status setzen */
	  PerformDoAction(state); /* Funktionen je nach Status ausführen */
	osDelay(1);
  }
  /* USER CODE END FunctionRotaryTask */
}

/* USER CODE BEGIN Header_FunctionClockTask */
/**
* @brief Function implementing the TaskClock thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_FunctionClockTask */
void FunctionClockTask(void *argument)
{
  /* USER CODE BEGIN FunctionClockTask */

static QueueItem_t queueItem = {0};
static uint32_t ticker = {0};
static const char * str_print_3d ="%02d:%02d:%02d\r\n";
static const char * str_print_1d ="%02d\r\n";
uint8_t state=1;
  /* Infinite loop */
  for(;;)
  {
	   RTC_DateTypeDef sDate = {0};
	   RTC_TimeTypeDef sTime = {0};
	   RTC_AlarmTypeDef sAlarm = {0};
	   char buffer[20] = {0};

	   /* Prüfen ob ein Alarm ausgelöst wurde */
	if(!HAL_RTC_PollForAlarmAEvent(&hrtc, 100)){
		queueItem.ReceiverTask=RECEIVER_TASK_ROTARY;
		queueItem.Command=CMD_ALARM;
		osMessageQueuePut(QueueFromClockHandle, &queueItem, 0U, 50U );
	}

	/* Elemente der Clock-Queue auslesen */
	if (osMessageQueueGet(QueueToClockHandle, &queueItem, NULL, 1U)==osOK)
	{
		 /* Status des Rotary's übernehmen */
		if(queueItem.State>=1){
			state=queueItem.State;
		}

		/* Empfängertask festlegen */
		queueItem.ReceiverTask=RECEIVER_TASK_UART;

		/* Empfange Nachricht der Clock-Queue abarbeiten */
		switch ((uint8_t)queueItem.Command)
		{
			/* Uhrzeit- Stunden erhöhen */
			case CMD_CLK_HRS_PLUS:
			HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
			sTime.Hours=(sTime.Hours+1)%24;
			HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
			sprintf(buffer,str_print_3d,sTime.Hours, sTime.Minutes, sTime.Seconds);
			break;

			/* Uhrzeit- Stunden verringern */
			case CMD_CLK_HRS_MINUS:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Hours -=1;
				if(sTime.Hours==255){
					sTime.Hours=23;
				}
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sTime.Hours, sTime.Minutes, sTime.Seconds);
			break;

			/* Uhrzeit- Minuten erhöhen */
			case CMD_CLK_MIN_PLUS:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Minutes=(sTime.Minutes+1)%60;
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sTime.Hours, sTime.Minutes, sTime.Seconds);
			break;

			/* Uhrzeit- Minuten verringern */
			case CMD_CLK_MIN_MINUS:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Minutes -=1;
				if(sTime.Minutes==255){
					sTime.Minutes=59;
				}
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sTime.Hours, sTime.Minutes, sTime.Seconds);
			break;

			/* Uhrzeit- Sekunden erhöhen */
			case CMD_CLK_SEC_PLUS:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Seconds=(sTime.Seconds+1)%60;
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sTime.Hours, sTime.Minutes, sTime.Seconds);
			break;

			/* Uhrzeit- Sekunden verringern */
			case CMD_CLK_SEC_MINUS:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Seconds -=1;
				if(sTime.Seconds==255){
					sTime.Seconds=59;
				}
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sTime.Hours, sTime.Minutes, sTime.Seconds);
			break;

			/* Alarmzeit- Stunden erhöhen */
			case CMD_ALT_HRS_PLUS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.Alarm = RTC_ALARM_A;
				/*
				if(sAlarm.AlarmTime.Hours>=sTime.Hours){
					sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				}else{
					sAlarm.AlarmDateWeekDay = sDate.WeekDay+1;
				}
				*/
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				sAlarm.AlarmTime.Hours = (sAlarm.AlarmTime.Hours+1)%24;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
			break;

			/* Alarmzeit- Stunden verringern */
			case CMD_ALT_HRS_MINUS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.Alarm = RTC_ALARM_A;
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				sAlarm.AlarmTime.Hours -=1;

				if(sAlarm.AlarmTime.Hours==255)
				{
					sAlarm.AlarmTime.Hours=23;
				}

				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
			break;

			/* Alarmzeit- Minuten erhöhen */
			case CMD_ALT_MIN_PLUS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.AlarmTime.Minutes=(sAlarm.AlarmTime.Minutes+1)%60;
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
			break;

			/* Alarmzeit- Minuten verringern */
			case CMD_ALT_MIN_MINUS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.AlarmTime.Minutes -=1;
				if(sAlarm.AlarmTime.Minutes==255)
				{
					sAlarm.AlarmTime.Minutes=59;
				}
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
			break;

			/* Alarmzeit- Sekunden erhöhen */
			case CMD_ALT_SEC_PLUS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				sAlarm.AlarmTime.Seconds=(sAlarm.AlarmTime.Seconds+1)%60;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
			break;

			/* Alarmzeit- Sekunden verringern */
			case CMD_ALT_SEC_MINUS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.AlarmTime.Seconds -=1;
				if(sAlarm.AlarmTime.Seconds==255)
				{
					sAlarm.AlarmTime.Seconds=59;
				}
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
			break;

			/* Alarm- deaktivieren */
			case CMD_ALT_OFF:
				HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
				sprintf(buffer,str_print_1d,(READ_BIT(hrtc.Instance->CR,RTC_CR_ALRAE_Msk))>>RTC_CR_ALRAE_Pos);
			break;

			/* Alarm- aktivieren */
			case CMD_ALT_ON:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK){Error_Handler();}
				sprintf(buffer,str_print_1d,(READ_BIT(hrtc.Instance->CR,RTC_CR_ALRAE_Msk))>>RTC_CR_ALRAE_Pos);
			break;

			/* Alarm- deaktivieren über UART */
			case CMD_ALT_OFF_RMT:
				HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
				sprintf(buffer,str_print_1d,(READ_BIT(hrtc.Instance->CR,RTC_CR_ALRAE_Msk))>>RTC_CR_ALRAE_Pos);
			break;

			/* Alarm- über UART aktivieren */
			case CMD_ALT_ON_RMT:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK){Error_Handler();}
				sprintf(buffer,str_print_1d,(READ_BIT(hrtc.Instance->CR,RTC_CR_ALRAE_Msk))>>RTC_CR_ALRAE_Pos);
			break;

			/* Alarm- resitieren */
			case CMD_ALT_QUIT:
				HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
				strcpy(buffer,"1\r\n");
			break;

			/* Alarm- resitieren */
			case CMD_ALT_QUIT_RMT:
				HAL_RTC_DeactivateAlarm(&hrtc,RTC_ALARM_A);
				strcpy(buffer," OK\r\n");
			break;

			/* Uhrzeit- am UART ausgeben */
			case CMD_UART_PRINT_TIME:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sTime.Hours, sTime.Minutes, sTime.Seconds);
			break;

			/* Uhrzeit- Stunden am UART ausgeben */
			case CMD_UART_PRINT_CLK_HRS:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sTime.Hours);
			break;

			/* Uhrzeit- Minuten am UART ausgeben */
			case CMD_UART_PRINT_CLK_MIN:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sTime.Minutes);
			break;

			/* Uhrzeit- Sekunden am UART ausgeben */
			case CMD_UART_PRINT_CLK_SEC:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sTime.Seconds);
			break;

			/* Alarmzeit- am UART ausgeben */
			case CMD_UART_PRINT_ALARM:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_3d,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds);
			break;

			/* Alarmzeit- Stunden am UART ausgeben */
			case CMD_UART_PRINT_ALT_HRS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sAlarm.AlarmTime.Hours);
			break;

			/* Alarmzeit- Minuten am UART ausgeben */
			case CMD_UART_PRINT_ALT_MIN:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sAlarm.AlarmTime.Minutes);
			break;

			/* Alarmzeit- Sekunden am UART ausgeben */
			case CMD_UART_PRINT_ALT_SEC:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sAlarm.AlarmTime.Seconds);
			break;

			/* Alarm- Status am UART ausgeben */
			case CMD_UART_PRINT_ALT_STATE:
				sprintf(buffer,str_print_1d,(READ_BIT(hrtc.Instance->CR,RTC_CR_ALRAE_Msk))>>RTC_CR_ALRAE_Pos);
			break;

			/* Uhrzeit- Stunden setzen und am UART ausgeben */
			case CMD_CLK_SET_HRS:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Hours=queueItem.Value;
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sTime.Hours);
			break;

			/* Uhrzeit- Minuten setzen und am UART ausgeben */
			case CMD_CLK_SET_MIN:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Minutes=queueItem.Value;
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sTime.Minutes);
			break;

			/* Uhrzeit- Sekunden setzen und am UART ausgeben */
			case CMD_CLK_SET_SEC:
				HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sTime.Seconds=queueItem.Value;
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sTime.Seconds);
			break;

			/* Alarmzeit- Stunden setzen und am UART ausgeben */
			case CMD_ALT_SET_HRS:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.Alarm = RTC_ALARM_A;
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				sAlarm.AlarmTime.Hours=queueItem.Value;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sAlarm.AlarmTime.Hours);
			break;

			/* Alarmzeit- Minuten setzen und am UART ausgeben */
			case CMD_ALT_SET_MIN:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.Alarm = RTC_ALARM_A;
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				sAlarm.AlarmTime.Minutes=queueItem.Value;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sAlarm.AlarmTime.Minutes);
			break;

			/* Alarmzeit- Sekunden setzen und am UART ausgeben */
			case CMD_ALT_SET_SEC:
				HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
				sAlarm.Alarm = RTC_ALARM_A;
				sAlarm.AlarmDateWeekDay = sDate.WeekDay;
				sAlarm.AlarmTime.Seconds=queueItem.Value;
				HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BIN);
				sprintf(buffer,str_print_1d,sAlarm.AlarmTime.Seconds);
			break;
		}
		if(queueItem.Command!=0){
			strcpy(queueItem.Message,buffer);
			osMessageQueuePut(QueueFromClockHandle, &queueItem, 0U, 50U );
		}
	}

	 	 /* Uhrzeit und Alarmzeit permanen dem WiFi-Task senden */
		if(ticker+250 <= osKernelGetTickCount())
		{
			/* Empfängertask festlegen */
			queueItem.ReceiverTask=RECEIVER_TASK_WIFI;

			 /* Uhrzeit und Alarmzeit auslesen */
			HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BIN);
			HAL_RTC_GetAlarm(&hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN);

			/* Rotary-Task Status überprüfen */
			switch (state)
			{
			 	 /* Nachricht für das andere Wifi-Modul je nach Status erstellen */
				case STATE_RUNNING_SBY:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,0,0);
				break;
				case STATE_RUNNING_ALT:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,0,1);
				break;
				case STATE_ALT:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,0,2);
				break;
				case STATE_SET_CLK_HRS:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,1,0);
				break;
				case STATE_SET_CLK_MIN:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,2,0);
				break;
				case STATE_SET_CLK_SEC:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,3,0);
				break;
				case STATE_SET_ALT_HRS:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,4,0);
				break;
				case STATE_SET_ALT_MIN:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,5,0);
				break;
				case STATE_SET_ALT_SEC:
					sprintf(queueItem.Message,"#time=%02d:%02d:%02d,%02d:%02d:%02d,%02d,%02d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds,sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmTime.Seconds,6,0);
				break;
			}

			osMessageQueuePut(QueueFromClockHandle, &queueItem, 0U, 50U );
			ticker = osKernelGetTickCount();
		}

    osDelay(1);
  }
  /* USER CODE END FunctionClockTask */
}

/* USER CODE BEGIN Header_FunctionUART2Task */
/**
* @brief Function implementing the TaskUART2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_FunctionUART2Task */
void FunctionUART2Task(void *argument)
{
  /* USER CODE BEGIN FunctionUART2Task */
SERIALPROTOCOL_Init(&hserialprotocol_uart2);
//int run=0;

  /* Infinite loop */
  for(;;)
  {
	  SERIALPROTOCOL_X_Change(&hserialprotocol_uart2);

	  QueueItem_t queueItem;
	  if (osMessageQueueGet(QueueToUART2Handle, &queueItem, 0U, 1U)==osOK)
	  {
		  if(HAL_UART_Transmit(&huart2, (uint8_t *) queueItem.Message,(uint16_t)strlen(queueItem.Message), 100)!= HAL_OK){Error_Handler();}
	  }

    osDelay(1);
  }
  /* USER CODE END FunctionUART2Task */
}

/* USER CODE BEGIN Header_FunctionWiFiTask */
/**
* @brief Function implementing the TaskWiFi thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_FunctionWiFiTask */
void FunctionWiFiTask(void *argument)
{
  /* USER CODE BEGIN FunctionWiFiTask */

/* Wifi-Modul initialisieren */
WIFIPROTOCOL_Init(&hwifiprotocol_uart1,WIFIPROTOCOL_AS_SERVER);

  /* Infinite loop */
  for(;;)
  {
	  QueueItem_t queueItem;

	  /* Wifi-Nachricht auslesen und an das Wifi-Modul senden */
	  if (osMessageQueueGet(QueueToWifiHandle, &queueItem, NULL, 1U)==osOK)
	  {
		  /* Nachricht über TCP versenden */
		  WIFIPROTOCOL_Send_Message_AS_SERVER(&hwifiprotocol_uart1, "0", queueItem.Message);
	  }

    osDelay(1);
  }
  /* USER CODE END FunctionWiFiTask */
}

/* USER CODE BEGIN Header_FunctionTaskRotaryLed */
/**
* @brief Function implementing the TaskRotaryLed thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_FunctionTaskRotaryLed */
void FunctionTaskRotaryLed(void *argument)
{
  /* USER CODE BEGIN FunctionTaskRotaryLed */


	ROTARY_Init_with_LEDs(&hrotary,&htim6,MOSI_GPIO_Port, MOSI_Pin, SCK_GPIO_Port, SCK_Pin, CS_2_GPIO_Port, CS_2_Pin,ENC_A_GPIO_Port,ENC_A_Pin,ENC_B_GPIO_Port,ENC_B_Pin,SWITCH_GPIO_Port,SWITCH_Pin);


  /* Infinite loop */
  for(;;)
  {

#ifndef BRIGHTNESSTEST
	/* kalkulierte LED's des Encoders Ein- bzw. Ausschalten und die Helligkeit einstellen  */
	  uint8_t rotaty_brightness_collection[16]={1,7,13,19,25,31,37,43,49,55,61,67,73,79,85,91};
	  uint16_t rotaty_bits=0b1111111111111111;
	ROTARY_set_LEDs(&hrotary, ring_rotation_led[(rotary_volume%2)], rotaty_brightness_collection);
#endif
#ifdef BRIGHTNESSTEST
	  uint8_t rotaty_brightness_collection[16]={1,7,13,19,25,31,37,43,49,55,61,67,73,79,85,91};
		  uint16_t rotaty_bits= (uint16_t)0b1111111111111111;
	ROTARY_set_LEDs(&hrotary,&(rotaty_bits), rotaty_brightness_collection);
#endif
    osDelay(1);
  }
  /* USER CODE END FunctionTaskRotaryLed */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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

#ifdef  USE_FULL_ASSERT
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
