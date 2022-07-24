/**
  ******************************************************************************
  * @file    rotary.h
  * @author  Reiter R.
  * @brief   Header file of ROTARY module.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ROTARY_H_
#define ROTARY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32l4xx_hal.h"

/** @addtogroup ROTARY ROTARY
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @addtogroup ROTARY_Exported_Types ROTARY Exported Types
  * @{
  */

/**
  * @brief  ROTARY Port und Pin Structure definition
  */
typedef struct
{
	GPIO_TypeDef* Port; /*!< Port */

	uint16_t Pin; /*!< Pin */

}ROTARY_GpioTypeDef;

/**
  * @brief  ROTARY Handle Structure definition
  */
typedef struct
{
	uint8_t IsInit; /*!< gibt an ob der Button initialisiert ist */

	ROTARY_GpioTypeDef Encoder_Switch; /*!< Port und Pin des Rorary-Schalters */

	ROTARY_GpioTypeDef Encoder_Pin_A; /*!< Port und Pin des Rorary-Links-Rechts-Dreh */

	ROTARY_GpioTypeDef Encoder_Pin_B; /*!< Port und Pin des Rorary-Links-Rechts-Dreh */

	ROTARY_GpioTypeDef Encoder_Pin_MOSI; /*!< Port und Pin des Rorary-Schalters */

	ROTARY_GpioTypeDef Encoder_Pin_SCK; /*!< Port und Pin des Rorary-Links-Rechts-Dreh */

	ROTARY_GpioTypeDef Encoder_Pin_CS; /*!< Port und Pin des Rorary-Links-Rechts-Dreh */

	TIM_HandleTypeDef * htim;

}ROTARY_HandleTypeDef;

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup ROTARY_Exported_Functions ROTARY Exported Functions
  * @{
  */

/** @addtogroup ROTARY_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
/* Initialization and de-initialization functions *****************************/

uint8_t ROTARY_Init(ROTARY_HandleTypeDef * hrotary, GPIO_TypeDef * port_enc_a, uint16_t pin_enc_a, GPIO_TypeDef * port_enc_b, uint16_t pin_enc_b, GPIO_TypeDef * port_switch, uint16_t pin_switch);
uint8_t ROTARY_Init_with_LEDs(ROTARY_HandleTypeDef * hrotary ,TIM_HandleTypeDef * htim,GPIO_TypeDef* port_mosi, uint16_t pin_mosi,GPIO_TypeDef* port_sck, uint16_t pin_sck,GPIO_TypeDef* port_cs, uint16_t pin_cs,GPIO_TypeDef* port_enc_a, uint16_t pin_enc_a,GPIO_TypeDef* port_enc_b, uint16_t pin_enc_b,GPIO_TypeDef* port_switch, uint16_t pin_switch);
/**
  * @}
  */

/** @addtogroup ROTARY_Exported_Functions_Group2 IO Operation Functions
  * @{
  */
/* IO operation functions *****************************************************/
uint8_t ROTARY_Encoder_Switch_Short_or_Long_Press(ROTARY_HandleTypeDef * hrotary);
uint8_t ROTARY_Encoder_Switch_State(ROTARY_HandleTypeDef * hrotary);
void ROTARY_set_LEDs(ROTARY_HandleTypeDef * hrotary, uint16_t  bit_array_of_leds[], uint8_t bit_array_of_brightness[]);
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* ROTARY_H_ */
