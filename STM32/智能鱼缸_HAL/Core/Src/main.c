/*
 * main.c
 *
 *  说明：STM32F103C8T6 智能鱼缸主程序
 *        功能：DHT11温湿度采集 + 水位检测 + 自动排水 + OLED显示 + ESP8266 TCP上报
 *
 *  硬件连接：
 *    PA0  → 水位传感器ADC输入
 *    PA1  → 水泵继电器控制（高电平启动）
 *    PB6  → OLED I2C SCL
 *    PB7  → OLED I2C SDA
 *    PB9  → DHT11数据线
 *    PA2  → ESP8266 USART2 TX
 *    PA3  → ESP8266 USART2 RX
 *    PC13 → 板载LED（TIM1中断翻转，心跳指示）
 *
 *  控制逻辑：
 *    - 每500ms读取一次温湿度和水位
 *    - 水位>10mm启动排水，水位<1mm停止排水（迟滞控制，防止频繁启停）
 *    - ESP8266作为TCP服务器（端口8080），手机连接后每500ms推送传感器数据
 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 智能鱼缸主程序
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Headfile.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ESP_TCP_PORT   8080   /* ESP8266 TCP服务器端口 */
#define ESP_CONN_NUM   0      /* ESP8266连接号（手机连接后默认为0） */

/* 水泵控制阈值（单位：mm，与ADC_Water_GetHeight返回值一致） */
#define PUMP_ON_LEVEL   10    /* 水位高于此值启动排水 */
#define PUMP_OFF_LEVEL  1     /* 水位低于此值停止排水（迟滞区间1~10mm保持原状态） */
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/**
 * @brief  启动水泵（PA1输出高电平，继电器吸合）
 */
void WaterPump_Start(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
}

/**
 * @brief  停止水泵（PA1输出低电平，继电器断开）
 */
void WaterPump_Stop(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

/**
 * @brief  向ESP8266发送AT指令并等待响应
 * @param  cmd  AT指令字符串（必须以\\r\\n结尾）
 * @note   固定等待500ms让模块处理并返回响应
 */
void ESP8266_SendCmd(char *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 100);
    HAL_Delay(500);
}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  应用程序入口
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    /* MCU初始化 */
    HAL_Init();
    SystemClock_Config();

    /* 外设初始化（CubeMX生成） */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM1_Init();
    MX_ADC1_Init();
    MX_USART2_UART_Init();  /* USART2：115200波特率，连接ESP8266 */

    /* USER CODE BEGIN 2 */

    /* ESP8266上电配置（AT指令为临时配置，断电丢失，每次上电需重新配置） */
    HAL_Delay(3000);                      /* 等待ESP8266硬件启动（必须≥3s） */
    ESP8266_SendCmd("AT\r\n");                  /* 测试模块是否正常（返回OK） */
    ESP8266_SendCmd("AT+CIPMUX=1\r\n");         /* 开启多连接模式 */
    ESP8266_SendCmd("AT+CIPSERVER=1,8080\r\n"); /* 启动TCP服务器，端口8080 */

    /* OLED初始化 */
    OLED_Init();
    OLED_Clear();

    /* 启动TIM1更新中断（用于PC13 LED心跳翻转） */
    HAL_TIM_Base_Start_IT(&htim1);

    /* 传感器数据变量 */
    uint8_t temperature = 1;   /* 温度（℃） */
    uint8_t humidity    = 1;   /* 湿度（%） */
    uint8_t water_level = 0;   /* 水位（mm） */
    char buff[20];

    /* OLED固定标签显示 */
    OLED_ShowCHinese(20, 1, 0);  /* "温" */
    OLED_ShowCHinese(37, 1, 2);  /* "度" */
    OLED_ShowCHinese(20, 4, 3);  /* "水" */
    OLED_ShowCHinese(37, 4, 4);  /* "位" */

    /* USER CODE END 2 */

    /* 主循环 */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* 1. 读取DHT11温湿度 */
        DHT11_Read_Data(&temperature, &humidity);
        sprintf(buff, "=%d C", temperature);
        OLED_ShowString(60, 1, (uint8_t *)buff, 16);

        /* 2. 读取水位（ADC转换） */
        water_level = ADC_Water_GetHeight();
        sprintf(buff, "=%d mm", water_level);
        OLED_ShowString(60, 4, (uint8_t *)buff, 16);

        /* 3. 通过ESP8266 TCP推送传感器数据 */
        char sensor_data[50];
        sprintf(sensor_data, "温度:%d°C,湿度:%d%%,水位:%dmm\r\n",
                temperature, humidity, water_level);

        /* 发送AT+CIPSEND指令通知模块准备接收数据 */
        char tcp_send_cmd[30];
        sprintf(tcp_send_cmd, "AT+CIPSEND=%d,%d\r\n",
                ESP_CONN_NUM, (int)strlen(sensor_data));
        ESP8266_SendCmd(tcp_send_cmd);

        /* 发送实际传感器数据 */
        HAL_UART_Transmit(&huart2, (uint8_t*)sensor_data,
                          strlen(sensor_data), 100);
        HAL_Delay(100);

        /* 4. 水泵自动控制（迟滞逻辑） */
        if (water_level > PUMP_ON_LEVEL)
        {
            WaterPump_Start();   /* 水位过高，启动排水 */
        }
        else if (water_level < PUMP_OFF_LEVEL)
        {
            WaterPump_Stop();    /* 水位过低，停止排水 */
        }
        /* 水位在1~10mm之间时保持水泵当前状态（迟滞，防止频繁启停） */

        HAL_Delay(500);  /* 500ms控制周期 */
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief  系统时钟配置（HSE 8MHz × PLL9 = 72MHz）
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** 配置振荡器：HSE外部8MHz晶振 + PLL×9 = 72MHz */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** 配置总线时钟：SYSCLK=72MHz, AHB=72MHz, APB1=36MHz, APB2=72MHz */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    /** ADC时钟：PCLK2/6 = 12MHz（ADC最大14MHz） */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/**
 * @brief  TIM定时器更新中断回调函数
 * @param  htim  触发中断的定时器句柄
 * @note   TIM1定时中断触发时翻转PC13（板载LED），作为系统心跳指示
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);  /* 翻转板载LED */
    }
}
/* USER CODE END 4 */

/**
  * @brief  错误处理函数（HAL库初始化失败时调用）
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  断言失败回调
  * @param  file  源文件名
  * @param  line  行号
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
