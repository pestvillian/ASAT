/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ili9341.h"
#include "XPT2046_touch.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TS_LEFT 0
#define TS_RIGHT 320
#define TS_TOP 0
#define TS_BOT 240

#define MAX_LINE_LENGTH 32 //for linear movement it wont be more than 32 chars

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi4;

TIM_HandleTypeDef htim3;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

typedef enum {
	EMPTY = 0, NOT_EMPTY = 1
} ButtonStatus;

typedef struct {
	uint16_t x, y, w, h;
	char label[MAX_LINE_LENGTH];
	ButtonStatus status;
} Button;

Button backButton = { 10, 200, 60, 40, "Back" }; //x, y, w, h, label
Button nextButton = { 260, 200, 60, 40, "Next" };


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FMC_Init(void);
static void MX_TIM3_Init(void);
static void MX_SPI4_Init(void);
/* USER CODE BEGIN PFP */
void DrawMainPage(uint8_t page_num);
uint8_t handleTouch();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
	for (int i = 0; i < len; i++) {
		ITM_SendChar((*ptr++));  // Send the character via ITM
	}
	return len;
}

uint8_t count = 0;
static uint8_t touchFlag = 0;
static uint8_t page_num = 1;

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	printf("Hello\n");

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
	MX_FMC_Init();
	MX_TIM3_Init();
	MX_SPI4_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(LCD_NRST_GPIO_Port, LCD_NRST_Pin, GPIO_PIN_SET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(LCD_NRST_GPIO_Port, LCD_NRST_Pin, GPIO_PIN_SET);
	HAL_Delay(50);

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	htim3.Instance->CCR1 = 50;

	lcdInit();
	//lcdTest();
	lcdSetOrientation(LCD_ORIENTATION_LANDSCAPE);
	DrawMainPage(1);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if (touchFlag) {
			handleTouch();
			touchFlag = 0;
		}
		//HAL_Delay(100);

		//note: (0,0) is top left. (320,0) is top right
		//dont draw past x>300 or y>230

//	  lcdFillRGB(COLOR_WHITE);
//	  lcdDrawChar(300, 230, 'X', COLOR_BLACK, COLOR_WHITE);
//	  lcdDrawRect(0, 0, 100, 150, COLOR_RED);
//	  lcdDrawChar(120+5, 160+5, 'H', COLOR_BLACK, COLOR_WHITE);
//	  lcdDrawChar(126+5, 160+5, 'e', COLOR_BLACK, COLOR_WHITE);
//	  lcdDrawChar(132+5, 160+5, 'y', COLOR_BLACK, COLOR_WHITE);
//	  lcdSetCursor(120+5, 175+5);
//	  lcdSetTextColor(COLOR_BLACK, COLOR_WHITE);
//	  lcdPrintf("Next Line");
//	  HAL_Delay(10000);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief SPI4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI4_Init(void) {

	/* USER CODE BEGIN SPI4_Init 0 */

	/* USER CODE END SPI4_Init 0 */

	/* USER CODE BEGIN SPI4_Init 1 */

	/* USER CODE END SPI4_Init 1 */
	/* SPI4 parameter configuration*/
	hspi4.Instance = SPI4;
	hspi4.Init.Mode = SPI_MODE_MASTER;
	hspi4.Init.Direction = SPI_DIRECTION_2LINES;
	hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi4.Init.NSS = SPI_NSS_HARD_OUTPUT;
	hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
	hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi4.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi4) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI4_Init 2 */

	/* USER CODE END SPI4_Init 2 */

}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 168 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 100;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */
	HAL_TIM_MspPostInit(&htim3);

}

/* FMC initialization function */
static void MX_FMC_Init(void) {

	/* USER CODE BEGIN FMC_Init 0 */

	/* USER CODE END FMC_Init 0 */

	FMC_NORSRAM_TimingTypeDef Timing = { 0 };

	/* USER CODE BEGIN FMC_Init 1 */

	/* USER CODE END FMC_Init 1 */

	/** Perform the SRAM1 memory initialization sequence
	 */
	hsram1.Instance = FMC_NORSRAM_DEVICE;
	hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
	/* hsram1.Init */
	hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
	hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
	hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
	hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
	hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
	hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram1.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
	hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
	hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
	hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
	hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
	hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
	hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
	hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
	/* Timing */
	Timing.AddressSetupTime = 3;
	Timing.AddressHoldTime = 15;
	Timing.DataSetupTime = 2;
	Timing.BusTurnAroundDuration = 2;
	Timing.CLKDivision = 16;
	Timing.DataLatency = 17;
	Timing.AccessMode = FMC_ACCESS_MODE_A;
	/* ExtTiming */

	if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK) {
		Error_Handler();
	}

	/* USER CODE BEGIN FMC_Init 2 */

	/* USER CODE END FMC_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LCD_NRST_GPIO_Port, LCD_NRST_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : PB12 PB13 PB14 PB15 */
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : DEBUG_LED_Pin */
	GPIO_InitStruct.Pin = DEBUG_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DEBUG_LED_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : LCD_NRST_Pin */
	GPIO_InitStruct.Pin = LCD_NRST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LCD_NRST_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : T_IRQ_Pin */
	GPIO_InitStruct.Pin = T_IRQ_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(T_IRQ_GPIO_Port, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief: Draw main page and handle button label and status updates
 * @param: page_num: determines which page should be drawn from 1-10
 * @retval: none
 */
void DrawMainPage(uint8_t page_num) {
	//Draw "Protocol Title" box
	lcdFillRGB(COLOR_WHITE);
	char empty[20] = "Empty\n"; //strings from flash memory come with newline at end
	char pageTitle[20] = "";
	sprintf(pageTitle, "Page %d", page_num);
	lcdSetCursor(10, 10);
	lcdSetTextColor(COLOR_BLACK, COLOR_WHITE);
	lcdSetTextFont(&Font20);
	//BSP_LCD_SetFont(&Font20);
	lcdPrintf(pageTitle);
	lcdSetTextFont(&Font12);

//	//get button label info from flash memory
//	char readBuffer1[MAX_LINE_LENGTH] = ""; //initialize with nulls
//	char readBuffer2[MAX_LINE_LENGTH] = ""; //initialize with nulls
//	char readBuffer3[MAX_LINE_LENGTH] = ""; //initialize with nulls
//
//	//if the protocol exists, read from it. else, name it empty
//	if (read_from_flash(readBuffer1, get_sector_address(page_num))) {
//		strcpy(buttons[0].label, readBuffer1);
//		buttons[0].status = NOT_EMPTY;
//	} else {
//		strcpy(buttons[0].label, empty);
//		buttons[0].status = EMPTY;
//	}
//	if (read_from_flash(readBuffer2,
//			get_sector_address(page_num) + PROTOCOL_SIZE)) {
//		strcpy(buttons[1].label, readBuffer2);
//		buttons[1].status = NOT_EMPTY;
//	} else {
//		strcpy(buttons[1].label, empty);
//		buttons[1].status = EMPTY;
//	}
//	if (read_from_flash(readBuffer3,
//			get_sector_address(page_num) + 2 * PROTOCOL_SIZE)) {
//		strcpy(buttons[2].label, readBuffer3);
//		buttons[2].status = NOT_EMPTY;
//	} else {
//		strcpy(buttons[2].label, empty);
//		buttons[2].status = EMPTY;
//	}
//
//	// Draw buttons
//	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
//	for (int i = 0; i < NUM_BUTTONS; i++) {
//		BSP_LCD_FillRect(buttons[i].x, buttons[i].y, buttons[i].w,
//				buttons[i].h);
//		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
//		BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
//		if (buttons[i].label[10] == NEWLINE_CHAR) {
//		}
//		buttons[i].label[strlen(buttons[i].label) - 1] = ' '; //dont display null terminator
//		BSP_LCD_DisplayStringAt(buttons[i].x + 10, buttons[i].y + 10,
//				(uint8_t*) buttons[i].label, LEFT_MODE);
//		BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
//	}
//
	// Draw "Next" button
	if (page_num != 10) {
		//BSP_LCD_SetTextColor(LCD_COLOR_RED);
		lcdDrawRect(nextButton.x, nextButton.y, nextButton.w, nextButton.h,
				COLOR_BLACK);
		lcdSetCursor(nextButton.x + 5, nextButton.y + 5);
		lcdPrintf(nextButton.label);
	}

	// Draw "Back" button
	if (page_num != 1) {
		lcdDrawRect(backButton.x, backButton.y, backButton.w, backButton.h,
				COLOR_BLACK);
		lcdSetCursor(backButton.x + 5, backButton.y + 5);
		lcdPrintf(backButton.label);
	}
//
//	//draw "Queue" button on page 1
//	if (page_num == 1) {
//		BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
//		BSP_LCD_FillRect(queueButton.x, queueButton.y, queueButton.w,
//				queueButton.h);
//		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
//		BSP_LCD_SetBackColor(LCD_COLOR_YELLOW);
//		BSP_LCD_DisplayStringAt(queueButton.x + 10, queueButton.y + 10,
//				(uint8_t*) queueButton.label, LEFT_MODE);
//	}
}

uint8_t handleTouch() {
	uint16_t x = 0, y = 0;
	if (!XPT2046_TouchGetCoordinates(&x, &y)) {
		return 0;
	}
	//map the x coordinate to be left is 0
	x = TS_RIGHT - x;
	if ((x > nextButton.x) && (x < nextButton.x + nextButton.w) &&
			(y > nextButton.y) && (y < nextButton.y + nextButton.h) && (page_num != 10)) {
		printf("touched\n");
		page_num++;
		DrawMainPage(page_num);
	}
	if ((x > backButton.x) && (x < backButton.x + backButton.w) &&
			(y > backButton.y) && (y < backButton.y + backButton.h)  && (page_num != 1)) {
		printf("touched\n");
		page_num--;
		DrawMainPage(page_num);
	}

	HAL_Delay(25); //debouncing delay
	return 1;
}

/*** Touchscreen Interrupt Handler ****/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == T_IRQ_Pin) {
		if (XPT2046_TouchPressed()) {
			touchFlag = 1;
		}
	}
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
