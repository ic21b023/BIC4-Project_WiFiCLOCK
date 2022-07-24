/**
******************************************************************************
* @file rotary.c
* @author Reiter Roman
* @brief Source file of ROTARY module.
* Diese Datei bietet Funktionen um den Rotary-Status abzufragen
*
@verbatim
 ===============================================================================
                      ##### How to use this driver #####
 ===============================================================================
   [..]
	Der Rotary-Treiber kann wie folgt verwendet werden:

	(#) Deklarieren einer ROTARY_HandleTypeDef handle structure
    (#) Initialisieren des Rotary mit der Methode ROTARY_Init().
		(++) Es müssen die Pins an welche der Rotary elektrisch verbunden ist übergeben werden.

	(#) Mit ROTARY_Encoder_Switch_Short_or_Long_Press() kann ermittelt werden ob der Button kurz oder lange gedrückt worden ist.

	(#) Mit ROTARY_Encoder_Switch_State() kann ermittelt werden ob der Rotary links oder rechts gedreht wurde

  @endverbatim
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rotary.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static uint8_t ring_rot[17][16]={
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}};
/* Private function prototypes -----------------------------------------------*/

static void ROTARY_SPI_tx(ROTARY_HandleTypeDef * hrotary, uint8_t * value);
static void ROTARY_Delay(TIM_HandleTypeDef *htim ,uint16_t us);
static uint16_t get_value_bitpositions(uint16_t zahl, uint16_t stelle);
/* Exported functions --------------------------------------------------------*/
/** @addtogroup ROTARY_Exported_Functions ROTARY Exported Functions
  * @{
  */

/** @addtogroup ROTARY_Exported_Functions_Group1 Initialization Functions
 *  @brief   Initialization Functions
 *
@verbatim
 ===============================================================================
                      ##### Initialization Functions  #####
 ===============================================================================
  [..]
    Dienen um den Rotary zu initialisieren
@endverbatim
  * @{
  */

/**
  * @brief  Dient zum initialisieren des Rotary Handle
  * @param  hrotary		ROTARY handle
  * @param  port_enc_a	Port des Pins des Encoders A
  * @param  pin_enc_a 	Pin des Pins des Encoders A
  * @param  port_enc_b	Port des Pins des Encoders B
  * @param  pin_enc_b 	Pin des Pins des Encoders B
  * @param  port_switch	Port des Pins des Schalters
  * @param  pin_switch 	Pin des Pins des Schalters
  * @retval HAL status
  */
uint8_t ROTARY_Init(ROTARY_HandleTypeDef * hrotary, GPIO_TypeDef * port_enc_a, uint16_t pin_enc_a, GPIO_TypeDef * port_enc_b, uint16_t pin_enc_b, GPIO_TypeDef * port_switch, uint16_t pin_switch){

	if (hrotary == NULL)
	{
		return HAL_ERROR;
	}
	hrotary->Encoder_Pin_A.Port=port_enc_a;
	hrotary->Encoder_Pin_A.Pin=pin_enc_a;
	hrotary->Encoder_Pin_B.Port=port_enc_b;
	hrotary->Encoder_Pin_B.Pin=pin_enc_b;
	hrotary->Encoder_Switch.Port=port_switch;
	hrotary->Encoder_Switch.Pin=pin_switch;

	return HAL_OK;
}


uint8_t ROTARY_Init_with_LEDs(ROTARY_HandleTypeDef * hrotary ,TIM_HandleTypeDef * htim,GPIO_TypeDef* port_mosi, uint16_t pin_mosi,GPIO_TypeDef* port_sck, uint16_t pin_sck,GPIO_TypeDef* port_cs, uint16_t pin_cs,GPIO_TypeDef* port_enc_a, uint16_t pin_enc_a,GPIO_TypeDef* port_enc_b, uint16_t pin_enc_b,GPIO_TypeDef* port_switch, uint16_t pin_switch){

	if (hrotary == NULL)
		{
			return HAL_ERROR;
		}

	hrotary->Encoder_Pin_A.Port=port_enc_a;
	hrotary->Encoder_Pin_A.Pin=pin_enc_a;
	hrotary->Encoder_Pin_B.Port=port_enc_b;
	hrotary->Encoder_Pin_B.Pin=pin_enc_b;
	hrotary->Encoder_Switch.Port=port_switch;
	hrotary->Encoder_Switch.Pin=pin_switch;
	hrotary->Encoder_Pin_CS.Port=port_cs;
	hrotary->Encoder_Pin_CS.Pin=pin_cs;
	hrotary->Encoder_Pin_MOSI.Port=port_mosi;
	hrotary->Encoder_Pin_MOSI.Pin=pin_mosi;
	hrotary->Encoder_Pin_SCK.Port=port_sck;
	hrotary->Encoder_Pin_SCK.Pin=pin_sck;
	hrotary->htim=htim;
	HAL_TIM_Base_Start(htim);

	/* prüfen ob Encoderlevel High oder Low */
	//enc_last=HAL_GPIO_ReadPin (GPIO_ENCA_Port, GPIO_ENCA_Pin);

	return HAL_OK;

}
/**
  * @}
  */

/** @addtogroup ROTARY_Exported_Functions_Group2 IO Operation Functions
 *  @brief   IO Operation Functions
 *
@verbatim
 ===============================================================================
                      #####  IO Operation Functions  #####
 ===============================================================================
  [..]
    Dienen dazu, um mit dem Rotary zu interagieren.

	(+) Es kann ermittelt werden ober der Rotary-Switch kurz oder lange gedrückt worden ist und ob dieser links oder rechts gedreht wurde.
@endverbatim
  * @{
  */

/**
  * @brief  Ermittelt ob der Rotary kurz oder lange gedrückt worden ist
  * @param  hrotary  ROTARY handle
  * @retval Rotary-State
  */
uint8_t ROTARY_Encoder_Switch_Short_or_Long_Press(ROTARY_HandleTypeDef * hrotary){

	static uint32_t count=0;
	static uint8_t last_state =0;
	static uint8_t current_state=0;
	static uint8_t long_press=0;
	uint8_t retval=0;

	/* Status des Rotary auslesen */
	current_state = HAL_GPIO_ReadPin (hrotary->Encoder_Switch.Port, hrotary->Encoder_Switch.Pin);

	if(last_state == 1 && current_state == 1)
	{
		count++;

		if(count>=250 && long_press !=1)
		{
			/* button lange gedrückt */
			long_press=1;
			retval=1;
		}
	}
	else if(last_state == 0 && current_state == 1)
	{
		/* Button gedrückt */
		retval=2;

	}
	else if(last_state == 1 && current_state == 0)
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

/**
  * @brief  Ermittelt ob der Rotary links oder rechts gedreht worden ist.
  * @param  hrotary  ROTARY handle
  * @retval Rotary-State
  */
uint8_t ROTARY_Encoder_Switch_State(ROTARY_HandleTypeDef * hrotary){

	uint8_t retval=0;
	static uint8_t encA_now;
	static uint8_t encB_now;
	static uint8_t encA_last;
	static uint8_t encB_last;

	/* Status des Rotary auslesen */
	encA_now = HAL_GPIO_ReadPin (hrotary->Encoder_Pin_A.Port, hrotary->Encoder_Pin_A.Pin);
	encB_now = HAL_GPIO_ReadPin (hrotary->Encoder_Pin_B.Port, hrotary->Encoder_Pin_B.Pin);

	/* Ermitteln ob links oder rechts gedreht wurde */
	if(encA_now != encB_now)
	{
		if(encB_now == encA_last)
		{
			retval= 1;
		}
		else if(encA_now == encB_last)
		{
			retval= 2;
		}
	}

	/* letzen State speichern */
	encA_last=HAL_GPIO_ReadPin (hrotary->Encoder_Pin_A.Port, hrotary->Encoder_Pin_A.Pin);
	encB_last=HAL_GPIO_ReadPin (hrotary->Encoder_Pin_B.Port, hrotary->Encoder_Pin_B.Pin);

	return retval;
}


/**
  * @brief  Funktion 					Setzt die LED's des Encoders
  * @param  hrotary 					rotary handle
  * @param  bit_array_of_leds 			Jedes Bit steht für eine LED / 0=aus, 1=ein
  * @param  bit_array_of_brightness		Helligkeit jeder LED 0-100%
  */
void ROTARY_set_LEDs(ROTARY_HandleTypeDef * hrotary, uint16_t bit_array_of_leds[], uint8_t bit_array_of_brightness[]){
 //16000 MAX


	for(int i=0;i<16;i++){

		ROTARY_SPI_tx(hrotary,ring_rot[0]);
		ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[i])*10);
		ROTARY_SPI_tx(hrotary,(ring_rot[get_value_bitpositions(*bit_array_of_leds,(uint16_t)i)]));
		ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[i]);
	}

/*

    ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[0])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,0)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[0]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[1])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,1)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[1]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[2])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,2)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[2]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[3])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,3)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[3]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[4])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,4)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[4]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[5])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,5)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[5]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[6])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,6)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[6]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[7])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,7)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[7]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[8])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,8)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[8]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[9])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,9)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[9]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[10])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,10)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[10]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[11])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,11)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[11]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[12])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,12)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[12]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[13])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,13)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[13]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[14])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,14)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[14]);

  	ROTARY_SPI_tx(hrotary,ring_rot[0]);
  	ROTARY_Delay(hrotary->htim,(100-bit_array_of_brightness[15])*10);
  	ROTARY_SPI_tx(hrotary,ring_rot[get_value_bitpositions(bit_array_of_leds,15)]);
  	ROTARY_Delay(hrotary->htim,10*bit_array_of_brightness[15]);
*/
  	/* prüfen ob Encoderlevel High oder Low */
  //	enc_last = HAL_GPIO_ReadPin (GPIO_ENCA_Portx, GPIO_ENCA_Pinx);

}

/**
  * @brief  Read the specified input port pin.
  * @param  GPIOx where x can be (A..H) to select the GPIO peripheral for STM32L4 family
  * @param  GPIO_Pin specifies the port bit to read.
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @retval The input port pin value.
  */
static void ROTARY_SPI_tx(ROTARY_HandleTypeDef * hrotary, uint8_t * value){

	HAL_GPIO_WritePin(hrotary->Encoder_Pin_CS.Port, hrotary->Encoder_Pin_CS.Pin, GPIO_PIN_RESET);

	for(int i=15;i>=0;i--){
		HAL_GPIO_WritePin(hrotary->Encoder_Pin_MOSI.Port, hrotary->Encoder_Pin_MOSI.Pin, value[i]);
		HAL_GPIO_WritePin(hrotary->Encoder_Pin_SCK.Port, hrotary->Encoder_Pin_SCK.Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(hrotary->Encoder_Pin_SCK.Port, hrotary->Encoder_Pin_SCK.Pin, GPIO_PIN_RESET);
	}

	HAL_GPIO_WritePin(hrotary->Encoder_Pin_CS.Port,hrotary->Encoder_Pin_CS.Pin, GPIO_PIN_SET);

}

/**
  * @brief  Initialize the CLIPBOARDx peripheral according to the specified parameters in the GPIO_Init.
  * @param  GPIOx where x can be (A..H) to select the GPIO peripheral for STM32L4 family
  * @param  GPIO_Init pointer to a GPIO_InitTypeDef structure that contains
  *         the configuration information for the specified GPIO peripheral.
  * @retval None
  */

static void ROTARY_Delay(TIM_HandleTypeDef *htim ,uint16_t us){
 	  	//  uint32_t d = HAL_GetTickFreq();
 	  	//__HAL_TIM_SET_PRESCALER(&htim6, 16-1);
 	  	__HAL_TIM_SET_COUNTER(htim,0);  // set the counter value a 0
 	  	while (__HAL_TIM_GET_COUNTER(htim) < us);  // wait for the counter to reach the us input in the parameter

  }

/**
  * @brief  Set or clear the selected data port bit.
  *
  * @note   This function uses GPIOx_BSRR and GPIOx_BRR registers to allow atomic read/modify
  *         accesses. In this way, there is no risk of an IRQ occurring between
  *         the read and the modify access.
  *
  * @param  GPIOx where x can be (A..H) to select the GPIO peripheral for STM32L4 family
  * @param  GPIO_Pin specifies the port bit to be written.
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  * @param  PinState specifies the value to be written to the selected bit.
  *         This parameter can be one of the GPIO_PinState enum values:
  *            @arg GPIO_PIN_RESET: to clear the port pin
  *            @arg GPIO_PIN_SET: to set the port pin
  * @retval None
  */
static uint16_t get_value_bitpositions(uint16_t zahl, uint16_t stelle){
	     return (((zahl& ( 0b0000000000000001<<stelle))>>stelle)+(stelle*((zahl& ( 0b0000000000000001<<stelle))>>stelle)));

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
