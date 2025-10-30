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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
__IO uint32_t BspButtonState = BUTTON_RELEASED;
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
typedef struct {
	ADC_HandleTypeDef *hadc;
	uint32_t channel;
} adc;

static const adc adc_parameters[] = {
		{&hadc1, ADC_CHANNEL_6},
		{&hadc1, ADC_CHANNEL_7},
		{&hadc1, ADC_CHANNEL_8},
		{&hadc1, ADC_CHANNEL_9},
		{&hadc1, ADC_CHANNEL_1},
		{&hadc1, ADC_CHANNEL_2},
		{&hadc1, ADC_CHANNEL_15},
		{&hadc1, ADC_CHANNEL_12}
};

typedef struct {
	TIM_HandleTypeDef *htim;
	uint32_t channel;
} tim;

static const tim tim_parameters[] = {
		{&htim2, TIM_CHANNEL_1},
		{&htim2, TIM_CHANNEL_3},
		{&htim3, TIM_CHANNEL_1},
		{&htim3, TIM_CHANNEL_2},
		{&htim3, TIM_CHANNEL_3},
		{&htim3, TIM_CHANNEL_4}
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint16_t ADC_Convert(ADC_HandleTypeDef *hadc, uint32_t channel, uint32_t sample_time)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = sample_time;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) Error_Handler();

    HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, 100);
    uint16_t v = (uint16_t)HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);
    return v;
}
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
  // LDR parameters
  uint16_t ldr[8];
  uint16_t ref[8]; // ref base
  float cal[8]; // calibration ratio

  static uint16_t num_sensors = 8;
  static uint16_t num_servos = 6;
  static float threshold = 0.2;
  static uint32_t cal_t0 = 0;
  static uint32_t cal_ms = 2000; // calibration cycle time
  uint8_t player_column_num = 0;
  char alg_column_num[2];
  uint8_t reset = 0;
  uint8_t first_move = 0;

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

  cal_t0 = HAL_GetTick();
  for (int k = 0; k < num_sensors; k++) ldr[k] = ADC_Convert(adc_parameters[k].hadc, adc_parameters[k].channel, ADC_SAMPLETIME_247CYCLES_5);
  for (int k = 0; k < num_sensors; k++) ref[k] = ldr[k];
  for (int k = 0; k < num_sensors; k++) cal[k] = (float)ldr[k] / (float)ref[k];

  /* USER CODE END 2 */

  /* Initialize led */
  BSP_LED_Init(LED_GREEN);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN BSP */
  /* -- Sample board code to switch on led ---- */
  BSP_LED_On(LED_GREEN);

  // servo check
  for (int k = 0; k < num_servos; k++)
  {
	  __HAL_TIM_SET_COMPARE(tim_parameters[k].htim, tim_parameters[k].channel, 1000); HAL_Delay(2000);
	  __HAL_TIM_SET_COMPARE(tim_parameters[k].htim, tim_parameters[k].channel, 2000); HAL_Delay(2000);
	  __HAL_TIM_SET_COMPARE(tim_parameters[k].htim, tim_parameters[k].channel, 1000); HAL_Delay(2000);
  }

  // wait for rasb pi
  HAL_Delay(10000);
  uint8_t ready = 'r';
  uint8_t received = 'i';
  printf("waiting...\r\n");
  for (;;)
  {
	  HAL_UART_Transmit(&huart3, &ready, 1, HAL_MAX_DELAY);
	  if (HAL_UART_Receive(&huart3, (uint8_t*)&alg_column_num, 2, 100) == HAL_OK)
	  {
		  printf("playing opening %d\r\n", alg_column_num[0] - '0');
		  HAL_UART_Transmit(&huart3, &received, 1, HAL_MAX_DELAY);
		  break;
	  }
	  HAL_Delay(100);
  }

  // close all servos except opening move
  for (int k = 0; k < num_servos; k++)
  {
	  if (k == alg_column_num[0] - '0') continue;
	  __HAL_TIM_SET_COMPARE(tim_parameters[k].htim, tim_parameters[k].channel, 2000);
	  HAL_Delay(200);
	  printf("closing %d\r\n", k + 1);
  }

  first_move = 1; // first move set

  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	for (int k = 0; k < num_sensors; k++) ldr[k] = ADC_Convert(adc_parameters[k].hadc, adc_parameters[k].channel, ADC_SAMPLETIME_247CYCLES_5);

	if (HAL_GetTick() - cal_t0 >= cal_ms)
	{
	  //printf("ldr0 value: %u, cal0 value: %0.2f\n",(unsigned)ldr[0],cal[0]);
	  if (cal[0] < ((float)ldr[0] / (float)ref[0]) - threshold || cal[0] > ((float)ldr[0] / (float)ref[0]) + threshold)
	  {
		 printf("recalibrating\r\n");
		 for (int k = 0; k < num_sensors; k++) ref[k] = ADC_Convert(adc_parameters[k].hadc, adc_parameters[k].channel, ADC_SAMPLETIME_247CYCLES_5);
		 for (int k = 0; k < num_sensors; k++) cal[k] = (float)ldr[k] / (float)ref[k];
	  }
	  cal_t0 = HAL_GetTick();
	}

	/*
	for (int k = 1; k < num_sensors; k++)
		printf("ldr%d value: %u\r\n",k,(unsigned)ldr[k]);
	*/

	if (first_move)
	{
		// check if opening move arrived at column
		if (cal[4] > ((float)ldr[4] / (float)ref[4]) + threshold)
		{
			printf("confirming opening move...\r\n");
			HAL_Delay(2000);
			ldr[4] = ADC_Convert(adc_parameters[4].hadc, adc_parameters[4].channel, ADC_SAMPLETIME_247CYCLES_5);
			if (cal[4] > ((float)ldr[4] / (float)ref[4]) + threshold) break; // full column
			// otherwise successful play
			// open all servos
			for (int k = 0; k < num_servos; k++) __HAL_TIM_SET_COMPARE(tim_parameters[k].htim, tim_parameters[k].channel, 1000);
			// end first move
			first_move = 0;
			break;
		}
		HAL_Delay(100);
	}
	else
	{
		for (int k = 1; k < num_sensors; k++)
		{
			if (cal[k] > ((float)ldr[k] / (float)ref[k]) + threshold)
			{
				printf("checking %d\r\n", k);
				HAL_Delay(2000); // check if full column
				ldr[k] = ADC_Convert(adc_parameters[k].hadc, adc_parameters[k].channel, ADC_SAMPLETIME_247CYCLES_5);
				if (cal[k] > ((float)ldr[k] / (float)ref[k]) + threshold) continue; // full column
				// otherwise successful play

				player_column_num = (uint8_t)('0' + k);
				HAL_UART_Transmit(&huart3, &player_column_num, 1, HAL_MAX_DELAY);
				printf("success\r\n");
				HAL_Delay(2000);
				for (;;)
				{
					if (HAL_UART_Receive(&huart3, (uint8_t*)&alg_column_num, 2, 100) == HAL_OK && alg_column_num[0] <= '7' && alg_column_num[0] >= '1')
					{
						printf("playing %d\r\n", alg_column_num[0] - '0');
						HAL_UART_Transmit(&huart3, &received, 1, HAL_MAX_DELAY);
						// close all columns except algorithm move
						for (int k = 0; k < num_servos; k++)
						{
							if (k == alg_column_num[0] - '0') continue;
							__HAL_TIM_SET_COMPARE(tim_parameters[k].htim, tim_parameters[k].channel, 2000);
						}
						break;
					}
					HAL_Delay(100);
				}

				if (alg_column_num[1] == 'w')
				{
					printf("win detected...\r\n");
					reset = '0';
					HAL_UART_Transmit(&huart3, &reset, 1, HAL_MAX_DELAY);
					printf("resetting!\r\n");
					HAL_Delay(10000);

					printf("waiting...\r\n");
					for (;;)
					{
						HAL_UART_Transmit(&huart3, &ready, 1, HAL_MAX_DELAY);
						if (HAL_UART_Receive(&huart3, (uint8_t*)&alg_column_num, 2, 100) == HAL_OK)
						{
							printf("playing opening %d\r\n", alg_column_num[0] - '0');
							HAL_UART_Transmit(&huart3, &received, 1, HAL_MAX_DELAY);
							break;
						}
						HAL_Delay(100);
					}
					HAL_Delay(2000);
				}
				break;
			}
		}
	}
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 21;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitTypeDef io = {0};
  io.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
  io.Mode = GPIO_MODE_ANALOG;
  io.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &io);
  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 84-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 19999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief BSP Push Button callback
  * @param Button Specifies the pressed button
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_USER)
  {
    BspButtonState = BUTTON_PRESSED;
  }
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
