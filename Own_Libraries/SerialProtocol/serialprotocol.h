/**
  ******************************************************************************
  * @file    serialprotocol.h
  * @author  Reiter R.
  * @brief   Header file of SERIALPROTOCOL module.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SERIALPROTOCOL_H_
#define SERIALPROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/** @addtogroup SERIALPROTOCOL SERIALPROTOCOL
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Exported_Constants SERIALPROTOCOL Exported Constants
  * @{
  */
#define SERIALPROTOCOL_MessageBuffer_SIZE 65 	/*!< Größe des Messagebuffers */

/**
  * @}
  */

/* Exported macros ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Exported_Types SERIALPROTOCOL Exported Types
  * @{
  */

/**
  * @brief  SERIALPROTOCOL Status Structure definition
  */
typedef enum
{
	SERIALPROTOCOL_STATUS_OK	= 0x00,		/*!< Nachricht OK */
	SERIALPROTOCOL_STATUS_ERROR	= 0x01,		/*!< Nachricht nicht OK */
	SERIALPROTOCOL_STATUS_COLLECT	= 0x02,	/*!< Eingaben werden im Buffer gesammelt */
	SERIALPROTOCOL_STATUS_READY	= 0x03		/*!< Buffer bereit zu Ausgabe */
} SERIALPROTCOL_StatusTypeDef;

/**
  * @brief  SERIALPROTOCOL Commando Kind Structure definition
  */
typedef enum
{
	SERIALPROTOCOL_COMMANDKIND_SET	= 0x00,		/*!< Set-Kommando */
	SERIALPROTOCOL_COMMANDKIND_GET	= 0x01, 	/*!< Get-Kommando */
	SERIALPROTOCOL_COMMANDKIND_EXECUTE	= 0x02, /*!< Execute-Kommando */
} SERIALPROTOCOL_CommandKindTypeDef;


/**
  * @brief  SERIALPROTOCOL Parameter Kind Structure definition
  */
typedef enum
{
	 SERIALPROTOCOL_PARAMETERKIND_ALPHA	= 0x61,		/*!< Text-Parameter */
	 SERIALPROTOCOL_PARAMETERKIND_DIGIT = 0x64, 	/*!< Nummer-Parameter */
	 SERIALPROTOCOL_PARAMETERKIND_SOME 	= 0x78, 	/*!< Text und Nummer-Parameter */

} SERIALPROTOCOL_ParameterkindTypeDef;

/**
  * @brief  SERIALPROTOCOL Parameter Structure definition
  */
typedef struct
{
  SERIALPROTOCOL_ParameterkindTypeDef ParameterKind;
  char Value[15];       /*!< Wert des Parameters */

}SERIALPROTOCOL_ParameterTypeDef;

/**
  * @brief  SERIALPROTOCOL Command Structure definition
  */
typedef struct
{
  char Name[15];        /*!< Kommandoname */

  SERIALPROTOCOL_CommandKindTypeDef CommandKind; /*!< Parameterart des Kommandos */

  char ParameterAmount; /*!< Parameteranzahl des Kommandos */

  SERIALPROTOCOL_ParameterTypeDef Parameter[15];       /*!< Parameter des Kommandos */

}SERIALPROTOCOL_CommandTypeDef;

/**
  * @brief  SERIALPROTOCOL Private-Member Structure definition
  */
typedef struct{

	char collectionBuffer[SERIALPROTOCOL_MessageBuffer_SIZE]; 	/*!< Buffer sammelt die eingegebenen Zeichen */

}SERIALPROTOCOL_PrivateMembers;

/**
  * @brief  SERIALPROTOCOL Message-Event Structure definition
  */
typedef struct
{
	char MessageReceived; /*!< MessageReceived Event */

}SERIALPROTOCOL_Event_TypeDef;

/**
  * @brief  SERIALPROTOCOL Handle Structure definition
  */
typedef struct
{
	unsigned char IsInit; /*!< Module initialisiert */

	SERIALPROTOCOL_Event_TypeDef Event; /*!< Event */

	SERIALPROTOCOL_CommandTypeDef Command; /*!< Kommando */

	char outputMessageBuffer[SERIALPROTOCOL_MessageBuffer_SIZE]; /*!< Ausgabepuffer */

	char inputMessageBuffer [SERIALPROTOCOL_MessageBuffer_SIZE]; /*!< Eingabepuffer */

	SERIALPROTOCOL_PrivateMembers PrivateMembers; /*!< Private Members */

}SERIALPROTOCOL_HandleTypeDef;

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Exported_Functions SERIALPROTOCOL Exported Functions
  * @{
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group1 Initialization and de-initialization functions
  * @{
  */
/* Initialization and de-initialization functions *****************************/

SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Init(SERIALPROTOCOL_HandleTypeDef * hserialprotocol);
/**
  * @}
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group2 IO Operation Functions
  * @{
  */
/* IO operation functions *****************************************************/
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_X_Change(SERIALPROTOCOL_HandleTypeDef * hserialprotocol);
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Transmit(SERIALPROTOCOL_HandleTypeDef * hserialprotocol );

/**
  * @}
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group3 Adapter Functions
  * @{
  */
/* Adapter  functions *****************************************************/
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_TransmitAdapter(uint8_t * buffer, uint16_t buffersize);
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_ReceiveAdapter(uint8_t * buffer, uint16_t buffersize);

/**
  * @}
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group4 Callback Functions
  * @{
  */
/* Callback  functions *****************************************************/
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Command_Callback(SERIALPROTOCOL_HandleTypeDef * hserialprotocol);

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

#endif /* SERIALPROTOCOL_H_ */
