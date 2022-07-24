/**
******************************************************************************
* @file button.c
* @author Reiter Roman
* @brief Source file of BUTTON module.
* Diese Datei bietet Funktionen um den Button-Status abzufragen
*
@verbatim
 ===============================================================================
                      ##### How to use this driver #####
 ===============================================================================
   [..]
	Der Button-Treiber kann wie folgt verwendet werden:

	(#) Deklarieren einer BUTTON_HandleTypeDef handle structure
    (#) Initialisieren des Buttons mit der Methode BUTTON_Init().
		(++) Es müssen die Pins an welche der Button elektrisch verbunden ist übergeben werden.

	(#) Mit den Funktionen BUTTON_Short_or_Long_Press() kann ermittelt werden ob der Button kurz oder lange gedrückt worden ist.

  @endverbatim
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "button.h"

/** @addtogroup BUTTON BUTTON
  * @brief BUTTON Module Driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/** @addtogroup BUTTON_Exported_Functions BUTTON Exported Functions
  * @{
  */

/** @addtogroup BUTTON_Exported_Functions_Group1 Initialization Functions
 *  @brief   Initialization Functions
 *
@verbatim
 ===============================================================================
                      ##### Initialization Functions  #####
 ===============================================================================
  [..]
    Dienen um den Button zu initialisieren
@endverbatim
  * @{
  */

/**
  * @brief  Dient zum initialisieren des Button Handle
  * @param  hbutton		BUTTON handle
  * @param  port_button	Port des pin_button's
  * @param  pin_button 	Pin mit dem der Button elektr. verbunden ist
  * @retval HAL status
  */
uint8_t BUTTON_Init(BUTTON_HandleTypeDef * hbutton, GPIO_TypeDef * port_button, uint16_t pin_button){

	if (hbutton == NULL)
	{
		return HAL_ERROR;
	}
	hbutton->Button.Port=port_button;
	hbutton->Button.Pin=pin_button;

	return HAL_OK;

}
/**
  * @}
  */

/** @addtogroup BUTTON_Exported_Functions_Group2 IO Operation Functions
 *  @brief   IO Operation Functions
 *
@verbatim
 ===============================================================================
                      #####  IO Operation Functions  #####
 ===============================================================================
  [..]
    Dienen dazu, um mit dem Button zu interagieren.

	(+) Es kann ermittelt werden ober der Button kurz oder lange gedrückt worden ist.
@endverbatim
  * @{
  */

/**
  * @brief  Ermittelt ob der Button kurz oder lange gedrückt ist.
  * @param  hbutton  	BUTTON handle
  * @retval button-state
  */
uint8_t BUTTON_Short_or_Long_Press(BUTTON_HandleTypeDef * hbutton){

	static int count=0;
	static uint8_t last_state =0;
	static uint8_t current_state=0;
	static uint8_t long_press=0;
	uint8_t retval=0;

	/* Status des Buttons auslesen */
	current_state = !HAL_GPIO_ReadPin (hbutton->Button.Port, hbutton->Button.Pin);

	if(last_state == 1 && current_state == 1)       // button is pressed
	{
		count++;

		if(count>=250 && long_press !=1)  // button is released
		{
			/* button lange gedrückt */
			long_press=1;
			retval=1;
		}
	}
	else if(last_state == 0 && current_state == 1)  // button is released
	{
		/* Button gedrückt */
		retval=2;

	}
	else if(last_state == 1 && current_state == 0)  // button is released
	{
		if(count<250)
		{
			/* Button nach einem kurzem Tastendruck losgelassen */
			retval=3;
		}else{
			retval=4;
		}

		/* Button nach einem langen Tastendruck losgelassen */
		count=0;
		long_press=0;

	}

	/* letzen State speichern */
	last_state = current_state;

	return retval;
}

/* Private functions----------------------------------------------------------*/

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
