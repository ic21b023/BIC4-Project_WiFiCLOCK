/**
  ******************************************************************************
  * @file    button.h
  * @author  Reiter R.
  * @brief   Header file of BUTTON module.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BUTTON_H_
#define BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup BUTTON BUTTON
  * @{
  */

/* Exported macros ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/** @addtogroup BUTTON_Exported_Types BUTTON Exported Types
  * @{
  */

/**
  * @brief  BUTTON Port und Pin Structure definition
  */
typedef struct
{

	GPIO_TypeDef* Port; /*!< Port */

	uint16_t Pin; /*!< Pin */


}BUTTON_GpioTypeDef;

/**
  * @brief  BUTTON Handle Structure definition
  */
typedef struct
{

	uint8_t IsInit; /*!< gibt an ob der Button initialisiert ist */

	BUTTON_GpioTypeDef Button; /*!< Port und Pin des Buttons */

}BUTTON_HandleTypeDef;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup BUTTON_Exported_Functions BUTTON Exported Functions
  * @{
  */

/** @addtogroup BUTTON_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
/* Initialization and de-initialization functions *****************************/


uint8_t BUTTON_Init(BUTTON_HandleTypeDef * hbutton, GPIO_TypeDef * port_button, uint16_t pin_button);

/**
  * @}
  */

/** @addtogroup BUTTON_Exported_Functions_Group2 IO Operation Functions
  * @{
  */
/* IO operation functions *****************************************************/
uint8_t BUTTON_Short_or_Long_Press(BUTTON_HandleTypeDef * hbutton);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_H_ */
