/**
  ******************************************************************************
  * @file    wifiprotocol.h
  * @author  Reiter R.
  * @brief   Header file of WIFIPROTOCOL module.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef WIFIPROTOCOL_H_
#define WIFIPROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "at_commands.h"

/** @addtogroup WIFIPROTOCOL WIFIPROTOCOL
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Exported_Constants WiFiProtocol Exported Constants
  * @{
  */
#define BUFFER_SIZE  140	/*!< Puffergröße */
#define SEND_ATTEMPTS_COMMANDS  5 /*!< Anzahl wie oft der Sendeversuch von Kommandos durchgeführt werden soll */
#define SEND_ATTEMPTS_MESSAGE  2 /*!< Anzahl wie oft der Sendeversuch von Nachrichten durchgeführt werden soll */

/**
  * @}
  */

/* Exported macros ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Exported_Types WIFIPROTOCOL Exported Types
  * @{
  */

/**
  * @brief  WIFIPROTOCOL Status Structure definition
  */
typedef enum
{
  WIFIPROTOCOL_OK		= 0x00, 	/*!< Nachricht OK */
  WIFIPROTOCOL_ERROR	= 0x01,		/*!< Nachricht nicht OK */
  WIFIPROTOCOL_BUSY		= 0x02,		/*!< Beschäftigt */
  WIFIPROTOCOL_READY	= 0x03, 	/*!< Breit */
  WIFIPROTOCOL_TIMEOUT  = 0x03		/*!< Zeitüberschreitung */
} WIFIPROTOCOL_StatusTypeDef;

/**
  * @brief  WIFIPROTOCOL Message Initalisation Structure definition
  */
typedef enum
{
  WIFIPROTOCOL_AS_SERVER = 0x01,	/*!< Als Server */
  WIFIPROTOCOL_AS_CLIENT = 0x02		/*!< Als Client */

} WIFIPROTCOL_InitTypeDef;

/**
  * @brief  WIFIPROTOCOL Message-Event Structure definition
  */
typedef struct
{
	char MessageReceived;	/*!< MessageReceived Event */

}WIFIPROTOCOL_Event_TypeDef;

/**
  * @brief  WIFIPROTOCOL Communication Status Structure definition
  */
typedef struct
{
	WIFIPROTOCOL_Event_TypeDef Event;

	char Rx [BUFFER_SIZE];	/*!< Ausgabepuffer */

	char Tx [BUFFER_SIZE];	/*!< Eingabepuffer */

	char includedMessage [BUFFER_SIZE];	/*!< Empfangene Nachricht */

}WIFIPROTOCOL_Communication_TypeDef;

/**
  * @brief  WIFIPROTOCOL Handle Structure definition
  */
typedef struct
{
	WIFIPROTCOL_InitTypeDef As ;	/*!< Module initialisiert als */

	WIFIPROTOCOL_StatusTypeDef Status;	/*!< aktueller Status des Moduls */

	WIFIPROTOCOL_Communication_TypeDef Communication;	/*!< Kummunikations-Typen */

}WIFIPROTOCOL_HandleTypeDef;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Exported_Functions WIFIPROTOCOL Exported Functions
  * @{
  */

/** @addtogroup WIFIPROTOCOL_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
/* Initialization and de-initialization functions *****************************/

WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Init(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, WIFIPROTCOL_InitTypeDef initmode);
/**
  * @}
  */

/** @addtogroup WIFIPROTOCOL_Exported_Functions_Group2 IO Operation Functions
  * @{
  */
/* IO operation functions *****************************************************/
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Reveive_Message(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * receivedMessage);
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Send_Message_AS_SERVER(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol,char * channel, char * tx_text );
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Send_Message_AS_CLIENT(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tx_text );
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TCP_Close_AS_Server(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol,char * channel);
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TCP_Close_AS_Client(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol);
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TCP_Open_AS_Client(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tcp, char * ip, char * port);
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Connect_To_AP_AS_Client(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol,char * ssid, char * pw);

/**
  * @}
  */

/** @addtogroup WIFIPROTOCOL_Exported_Functions_Group3 Adapter Functions
  * @{
  */
/* Adapter  functions *****************************************************/
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TransmitAdapter(uint8_t * buffer, uint16_t buffersize);
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_ReceiveAdapter(uint8_t * buffer, uint16_t buffersize);

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

#endif /* WIFIPROTOCOL_H_ */
