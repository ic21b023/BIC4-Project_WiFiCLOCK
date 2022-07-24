/**
******************************************************************************
* @file serialprotocol.c
* @author Reiter Roman
* @brief Source file of SERIALPROTOCOL module.
* Diese Datei bietet Funktionen für die Kommunikation über UART
*
@verbatim
 ===============================================================================
                      ##### How to use this driver #####
 ===============================================================================
   [..]
	Der Serialprotocol-Treiber kann wie folgt verwendet werden:

	(#) Deklarieren einer SERIALPROTOCOL_HandleTypeDef handle structure
    (#) Initialisieren des Serialprotocol's mit der Methode SERIALPROTOCOL_Init.

	(#) Mit SERIALPROTOCOL_X_Change werden die Eingaben eingelesen.

	(#) Mit SERIALPROTOCOL_Transmit() werden die Eingaben ausgegeben

	(#) SERIALPROTOCOL_ReceiveAdapter() muss die Empfangsmethode des Mikrocontrollers implementiert werden.

	(#) SERIALPROTOCOL_TransmitAdapter() muss die Sendemethode des Mikrocontrollers implementiert werden.

	(#) SERIALPROTOCOL_Command_Callback() dient als Schnittstelle wo die Commandos abgearbeitet werden bzw. den Kommandos
		Werte übergeben werden können (set) und Werte angefordert werden können (get)


  @endverbatim
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stddef.h"
#include "serialprotocol.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

/** @addtogroup SERIALPROTOCOL SERIALPROTOCOL
  * @brief SerialProtocol Module Driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Private_Defines SERIALPROTOCOL Private Defines
  * @{
  */

#define STM32_ACK_STR 				"STM32-ACK -> "		/*!< Nachricht OK */
#define STM32_NACK_STR 				"STM32-NACK -> " 	/*!< Nachricht falsch oder nicht erkannt */
#define NEW_LINE_STR 				"\n\r"				/*!< Neue Zeile */

#define CMD_SOF_STR 				"#"					/*!< Kommandobeginn */
#define CMD_SET_STR 				"="					/*!< Set-Kommando */
#define CMD_SEP_PAR_STR 			","					/*!< Seperator */
#define CMD_SEP_DEF_STR 			":"					/*!< Seperator */
#define CMD_SEP_PAR_TYP_STR 		"*"					/*!< Seperator */
#define CMD_EOF_STR 				"\r"				/*!< Kommando-Ende */
#define CMD_GET_STR 				"?"					/*!< Get-Kommando */

#define CMD_CMD_GETSET_STR 			"x"					/*!< Set oder Get-Kommando */
#define CMD_CMD_SET_STR 			"s"					/*!< Set-Kommando */
#define CMD_CMD_GET_STR 			"g"					/*!< Get-Kommando */

#define CMD_PAR_DIGIT_CHAR 			'd'					/*!< Nummer-Parameter */
#define CMD_PAR_ALPHA_CHAR 			'a'					/*!< Text-Parameter */
#define CMD_PAR_SOMETH_CHAR 		'x'					/*!< Nummer oder Text-Parameter */

#define COMMANDLIST_AMOUNT 			15					/*!< Kommandoanzahl */
#define COMMANDLIST_COMMANDSIZE		20					/*!< Kommandolänge */

#define FIELD_MAX					65					/*!< Neue Zeile */
#define MAX_PARAMETERS  			7					/*!< Maximale Parameteranzahl */
/**
  * @}
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Private_Variables SERIALPROTOCOL Private Variables
  * @{
  */

/*!< Kommando-Array */
/*!< <Kommandoname>:<(e)xecute,(g)et,(s)et><Parameteranzahl>*<Parameter1 - (d)igit, (a)chars oder (x) beides, Parameter2... > */
char commandlist[COMMANDLIST_AMOUNT][COMMANDLIST_COMMANDSIZE] = {
		"time:g:1*x",
		"time_hours:x:1*x",
		"time_minutes:x:1*x",
		"time_seconds:x:1*x",
		"alarm:g:1*x",
		"alarm_hours:x:1*x",
		"alarm_minutes:x:1*x",
		"alarm_seconds:x:1*x",
		"alarm_state:x:1*x",
		"alarm_quit:x:1*x"
};

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Private_Functions SERIALPROTOCOL Private Functions
  * @{
  */
static uint8_t is_number_string(char string[]);
static uint8_t is_alpha_string(char string[]);

static unsigned char cut_input(char * delimiter,char string [], char result [][SERIALPROTOCOL_MessageBuffer_SIZE]);
static uint8_t string_char_frequency(char * string, char * spanset);

static void SERIALPROTOCOL_Wrong_Message(SERIALPROTOCOL_HandleTypeDef * hserialprotocol);
static SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Check_Message(SERIALPROTOCOL_HandleTypeDef * hserialprotocol);

static void SERIALPROTOCOL_Create_Message(SERIALPROTOCOL_HandleTypeDef * hserialprotocol );
static SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Sort_Input(SERIALPROTOCOL_HandleTypeDef * hserialprotocol, char * RxBuffer );

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Exported_Functions SERIALPROTOCOL Exported Functions
  * @{
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group1 Initialization Functions
 *  @brief   Initialization Functions
 *
@verbatim
 ===============================================================================
                      ##### Initialization Functions  #####
 ===============================================================================
  [..]
    Dienen um das Serialprotocol zu initialisieren
@endverbatim
  * @{
  */

/**
  * @brief  Dient zum initialisieren des Serialprotocol Handle
  * @param  hserialprotocol		SERIALPROTOCOL handle
  * @retval SERIALPROTOCOL status
  */
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Init(SERIALPROTOCOL_HandleTypeDef * hserialprotocol){
	if (hserialprotocol == NULL)
	{
		return SERIALPROTOCOL_STATUS_ERROR;
	}

	return SERIALPROTOCOL_STATUS_OK;
}

/**
  * @}
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group2 IO Operation Functions
 *  @brief   IO Operation Functions
 *
@verbatim
 ===============================================================================
                      #####  IO Operation Functions  #####
 ===============================================================================
  [..]
    Dienen dazu, um mit dem Serialprotocol zu interagieren.

	(+) Es werden die Eingaben mit den Befehlen überprüft und demnach gehandelt.
@endverbatim
  * @{
  */

/**
  * @brief  verarbeitet die einzel Eingegebenen Zeichen
  * @param  hserialprotocol  SERIALPROTOCOL handle
  * @retval SERIALPROTOCOL status
  */
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_X_Change(SERIALPROTOCOL_HandleTypeDef * hserialprotocol){

			if(hserialprotocol->Event.MessageReceived==1)
			{
				hserialprotocol->Event.MessageReceived=0;
				return SERIALPROTOCOL_Transmit(hserialprotocol);
			}

	return SERIALPROTOCOL_STATUS_OK;
}

/**
  * @brief  gibt Anworten aufgrund der Eingaben aus
  * @param  hserialprotocol SERIALPROTOCOL handle
  * @retval SERIALPROTOCOL status
  */
SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Transmit(SERIALPROTOCOL_HandleTypeDef * hserialprotocol ){

	SERIALPROTCOL_StatusTypeDef status;

	/* Prüft ob die Nachricht ein Zeichen oder Zeichenkette ist */
	if(strlen(hserialprotocol->inputMessageBuffer)>1){

		/* Verabreiten der Zeichenkette */
		for(unsigned char i=0;i<strlen(hserialprotocol->inputMessageBuffer);i++){
			status = SERIALPROTOCOL_Sort_Input(hserialprotocol, hserialprotocol->inputMessageBuffer+i);
		}

	}else{
		/* Verabreiten des einzelnen Zeichens */
		status= SERIALPROTOCOL_Sort_Input(hserialprotocol,  hserialprotocol->inputMessageBuffer );
	}

	/* Ausgabe der Eingabe und Puffer löschen */
	SERIALPROTOCOL_TransmitAdapter((uint8_t*)hserialprotocol->outputMessageBuffer,(uint16_t)strlen(hserialprotocol->outputMessageBuffer));
	memset(hserialprotocol->outputMessageBuffer,0,sizeof(hserialprotocol->outputMessageBuffer));
	memset(hserialprotocol->inputMessageBuffer,0,sizeof(hserialprotocol->inputMessageBuffer));
	return status;
}

/**
  * @}
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group3 Adapter Functions
 *  @brief Adapter Functions
 *
@verbatim
 ===============================================================================
                      #####  Adapter Functions  #####
 ===============================================================================
  [..]
    Dienen dazu, um die spezifischen Übertragungsfuntkionen des Mikrocontrollers in das Serialprotocol zu integrieren

@endverbatim
  * @{
  */

/**
  * @brief  dient als Wrapper, wo die Übertragungsfunktion des Mikrocontrollers eingebunden werden soll
  * @param  buffer 	Zeichenkette welche übertragen werden soll
  * @param  buffersize Länge der Zeichenkette
  * @retval SERIALPROTOCOL status
  */
__weak SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_TransmitAdapter(uint8_t * buffer, uint16_t buffersize)
{
	/* NOTE : This function should not be modified,
            	the SERIALPROTOCOL_TransmitAdapter could be implemented in the user file
	 */
	return 1;
}

/**
  * @brief  dient als Wrapper, wo die Empfangsfunktion des Mikrocontrollers eingebunden werden soll
  * @param  buffer 	Zeichenkette welche übertragen werden soll
  * @param  buffersize Länge der Zeichenkette
  * @retval SERIALPROTOCOL status
  */
__weak SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_ReceiveAdapter(uint8_t * buffer, uint16_t buffersize)
{
	/* NOTE : This function should not be modified,
            	the SERIALPROTOCOL_ReceiveAdapter could be implemented in the user file
	 */
	return 1;
}

/**
  * @}
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group4 Callback Functions
 *  @brief   Callback Functions
 *
@verbatim
 ===============================================================================
                      #####  Callback Functions  #####
 ===============================================================================
  [..]
    Dienen dazu, um den Befehlen Werte zuzuweisen (set), Parameter mitzugeben (get) oder nur eine funktion erfüllen sollen (execute)

@endverbatim
  * @{
  */

/**
  * @brief  kann in die main.c kopiert werden kann um die Ein-/Ausgabe des Befehls festzulegen und auszugeben
  * @param  hserialprotocol SERIALPROTOCOL handle
  * @retval SERIALPROTOCOL status
  */
__weak SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Command_Callback(SERIALPROTOCOL_HandleTypeDef * hserialprotocol)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hserialprotocol);

	/* NOTE : This function should not be modified, when the callback is needed,
            	the SERIALPROT_Command_GPO_Callback could be implemented in the user file
	 */
	return 0;
}

/**
  * @}
  */

/**
  * @}
  */

/* Private functions----------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Private_Functions SERIALPROTOCOL Private Functions
  * @{
  */

/**
  * @brief  überprüft ob ein String eine Zahl enthält
  * @param  string[] 	der zu überprüfende String
  * @retval result
  */
static uint8_t is_number_string(char string[])
{
    for (uint8_t i = 0; string[i]!= '\0'; i++)
    {
        if (isdigit((int)string[i]) == 0)
              return 0;
    }
    return 1;
}

/**
  * @brief  überprüft ob ein String ein aus Kleinbuchstaben besteht
  * @param  string[] 	der zu überprüfende String
  * @retval result
  */
static uint8_t is_alpha_string(char string[])
{
    for (uint8_t i = 0; string[i]!= '\0'; i++)
    {
        if (isalpha((int)string[i]) == 0)
              return 0;
    }
    return 1;
}

/**
  * @brief  überprüft ob ein String ein aus Kleinbuchstaben besteht
  * @param  string[] 	der zu überprüfende String
  * @retval result
  */
uint8_t is_something(char string[])
{
    return 1;
}

/**
  * @brief  zerteilt den Inputstring "string" bei jedem Zeichen "delimiter" und speichert diese in "result"
  * @param  string		der zu zerteilende String
  * @param  delimiter	das Zeichen, nachdem der String geteilt werden soll
  * @param  result		zerteilte Strings
  * @retval amount of arrays
  */
static unsigned char cut_input(char * delimiter,char string [], char result [][SERIALPROTOCOL_MessageBuffer_SIZE]){

    char string_copy[strlen(string)+1];
    memcpy (string_copy, string, sizeof(string_copy));
    char *ptr;
    unsigned char i =0;
    ptr = strtok(string_copy, delimiter);
    {

        while(ptr != NULL)
        {
            strcpy(result[i++],ptr);
            ptr = strtok(NULL, delimiter);
        }
    }

    return i;
}

/**
  * @brief  zählt die die vorkommenden Zeichen "spanset" eines Strings "string" und retourniert die Anzahl
  * @param  string 		String, wo die Zeichen gezählt werden sollen
  * @param  spanset 	Zeichen, welches gezählt werden soll
  * @retval count		Anzahl der Zeichen in dem String
  */
static uint8_t string_char_frequency(char * string, char * spanset){

    char *ptr = string;

    uint8_t count=0;
		for(uint8_t i=0; i<strlen(string);i++)
        {
            ptr = strpbrk(ptr, spanset);

            if(ptr++)
            	count++;
            else
                break;
        }

	return count;
}

/**
  * @brief  fügt dem TxBuffer "falsche Nachricht" hinzu
  * @param  hserialprotocol SERIALPROTOCOL handle
  * @retval none
  */
static void SERIALPROTOCOL_Wrong_Message(SERIALPROTOCOL_HandleTypeDef * hserialprotocol){

	strcat(hserialprotocol->outputMessageBuffer, "\n\r");
	strcat(hserialprotocol->outputMessageBuffer, "STM32-NACK -> ");
	strcat(hserialprotocol->outputMessageBuffer, hserialprotocol->PrivateMembers.collectionBuffer);
	strcat(hserialprotocol->outputMessageBuffer, "\n\r");
}

/**
  * @brief  überprüft ob ein String ein aus Kleinbuchstaben besteht
  * @param  RxBuffer 	die zu überprüfende Nachricht
  * @retval SERIALPROTOCOL status
  */
static SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Sort_Input(SERIALPROTOCOL_HandleTypeDef * hserialprotocol, char * RxBuffer ){

	SERIALPROTCOL_StatusTypeDef status = SERIALPROTOCOL_STATUS_COLLECT;
	/* Überprüfen eingegebene Zeichen zwischen 32 und 127 oder Enter-Taste sind */
	if((RxBuffer[0]>=32 && RxBuffer[0]<=127) || RxBuffer[0]=='\r' )
	{
		/* Wenn Puffergröße überschritten, dann OV ausgeben */
		if(strlen(hserialprotocol->PrivateMembers.collectionBuffer)==SERIALPROTOCOL_MessageBuffer_SIZE)
		{
			memset(hserialprotocol->PrivateMembers.collectionBuffer,0,strlen(hserialprotocol->PrivateMembers.collectionBuffer));
			strcat(hserialprotocol->outputMessageBuffer, " -> OV\n\r");
			strcat(hserialprotocol->outputMessageBuffer, "Input> ");
		}
		else
		{
			/* Wenn Backspace-Taste gedrückt */
			if(RxBuffer[0]!='\177')
			{
				strncat(hserialprotocol->PrivateMembers.collectionBuffer, RxBuffer,1);
				strncat(hserialprotocol->outputMessageBuffer, RxBuffer,1);
			}
		}

		/* Wenn Backspace-Taste gedrückt */
		if(RxBuffer[0]=='\177')
		{
			/* Verhindern, dass "Input> " überschrieben wird */
			if(strlen(hserialprotocol->PrivateMembers.collectionBuffer)>0){
				hserialprotocol->PrivateMembers.collectionBuffer[strlen(hserialprotocol->PrivateMembers.collectionBuffer)-1]=0;
				strcat(hserialprotocol->outputMessageBuffer, "\177");
			}
			else
			{
				strcat(hserialprotocol->outputMessageBuffer, "\32");
			}
		}

		/* Wenn Enter-Taste gedrückt */
		if(RxBuffer[0]=='\r')
		{
			status=SERIALPROTOCOL_STATUS_READY;
			/* Eingabe überprüfen ob Kommando-Syntax */
			if(SERIALPROTOCOL_Check_Message(hserialprotocol) == SERIALPROTOCOL_STATUS_OK )
			{
				SERIALPROTOCOL_Create_Message(hserialprotocol);

			}else if(!strcmp(hserialprotocol->PrivateMembers.collectionBuffer,"\r"))
			{
				strcat(hserialprotocol->outputMessageBuffer, "\n\r");
			}else{
				SERIALPROTOCOL_Wrong_Message(hserialprotocol);
			}
				memset(hserialprotocol->PrivateMembers.collectionBuffer,0,strlen(hserialprotocol->PrivateMembers.collectionBuffer));
		}
	}
	else
	{
		strcat(hserialprotocol->outputMessageBuffer, "\32");
	}

	return status;
}

/**
  * @brief  überprüft ob das eingegebene Kommando set oder get ist und erstellt die Antwort
  * @param  hserialprotocol SERIALPROTOCOL handle
  * @retval none
  */
static void SERIALPROTOCOL_Create_Message(SERIALPROTOCOL_HandleTypeDef * hserialprotocol ){
	/* Erzeugen der Ausgangsnachricht falls Rückgabewerde der Callback 0 ist */
	if (!SERIALPROTOCOL_Command_Callback(hserialprotocol)){

		/* Nachrichtenarten abragen */
		if(hserialprotocol->Command.CommandKind == SERIALPROTOCOL_COMMANDKIND_SET){
			strcat(hserialprotocol->outputMessageBuffer, NEW_LINE_STR);
			strcat(hserialprotocol->outputMessageBuffer, STM32_ACK_STR);
			strcat(hserialprotocol->outputMessageBuffer, "#set=> ");
			strcat(hserialprotocol->outputMessageBuffer, hserialprotocol->Command.Name);
			strcat(hserialprotocol->outputMessageBuffer, "=");
		}else{
			strcat(hserialprotocol->outputMessageBuffer, NEW_LINE_STR);
			strcat(hserialprotocol->outputMessageBuffer, STM32_ACK_STR);
			strcat(hserialprotocol->outputMessageBuffer, "#get=> ");
			strcat(hserialprotocol->outputMessageBuffer, hserialprotocol->Command.Name);
			strcat(hserialprotocol->outputMessageBuffer, "=");
		}

	}else{
		SERIALPROTOCOL_Wrong_Message(hserialprotocol);
	}
}

/**
  * @brief  überprüft vorab ob die Eingabe eine "Kommando-Syntax" ist
  * @param  hserialprotocol SERIALPROTOCOL handle
  * @retval SERIALPROTOCOL status
  */
static SERIALPROTCOL_StatusTypeDef SERIALPROTOCOL_Check_Message(SERIALPROTOCOL_HandleTypeDef * hserialprotocol)
{
	uint8_t (*entscheide) (char []);
	memset(hserialprotocol->Command.Name,0,sizeof(hserialprotocol->Command.Name));
	hserialprotocol->Command.CommandKind=0;
	memset(hserialprotocol->Command.Parameter,0,sizeof(hserialprotocol->Command.Parameter));
	hserialprotocol->Command.ParameterAmount=0;
	char inputmessage[40] = {0};
	strcat(inputmessage, hserialprotocol->PrivateMembers.collectionBuffer);

	/* Überprüfen ob der Input ein Set-Kommando ist */
	if( string_char_frequency(inputmessage,CMD_SOF_STR) == 1 && string_char_frequency(inputmessage,CMD_GET_STR) == 0 && string_char_frequency(inputmessage,CMD_SET_STR) == 1 && string_char_frequency(inputmessage,CMD_EOF_STR) == 1 && strcspn( inputmessage, CMD_SOF_STR )==0 && (strcspn( inputmessage, CMD_EOF_STR )+1) == strlen(inputmessage)  )
	{
		inputmessage[strlen(inputmessage)-1]=0;
		hserialprotocol->Command.CommandKind = SERIALPROTOCOL_COMMANDKIND_SET;

		/* Input für die Überptüfung zerteilen */
		char split_cmd[2][SERIALPROTOCOL_MessageBuffer_SIZE]={0};
		char split_parameter[6][SERIALPROTOCOL_MessageBuffer_SIZE]={0};

		cut_input(CMD_SET_STR,inputmessage,split_cmd);

		strcpy(hserialprotocol->Command.Name, (split_cmd[0]+1));
		hserialprotocol->Command.ParameterAmount=(string_char_frequency(split_cmd[1],CMD_SEP_PAR_STR))+1;
	 	cut_input(CMD_SEP_PAR_STR,split_cmd[1],split_parameter);

	 	/* überprüfen ob das Kommando existiert */
		for(uint8_t i=0;i<(sizeof(commandlist)/sizeof(commandlist[0]));i++)
		{
			/* Input für die Überptüfung zerteilen */
			char split_stern[2][FIELD_MAX]={0};
	     	cut_input(CMD_SEP_PAR_TYP_STR,commandlist[i],split_stern);
	      	char split_dop[3][FIELD_MAX]={0};
	      	cut_input(CMD_SEP_DEF_STR,split_stern[0],split_dop);

	      	char strint[10]={0};
	      	 itoa(hserialprotocol->Command.ParameterAmount,strint,10);

	      	if(!strcmp(hserialprotocol->Command.Name,split_dop[0]) && !strcmp(strint,split_dop[2]))
	      	{
	      		/* Überprüfen ob es sich um ein Set-Kommando handelt */
	          	if(strcmp(CMD_CMD_GETSET_STR,split_dop[1]) && strcmp(CMD_CMD_SET_STR,split_dop[1]) ){
	    	      		return SERIALPROTOCOL_STATUS_ERROR;
	    		}

	          	/* Input für die Überptüfung zerteilen */
	      		char split_kom[MAX_PARAMETERS][FIELD_MAX]={0};
	      		cut_input(CMD_SEP_PAR_STR,split_stern[1],split_kom);

	      		/* überprüfen ob die Parameter stimmen */
	      		for(uint8_t j=0;strlen(split_kom[j])!=0;j++)
	      		{
	      			char x = split_kom[j][0];
	      			switch(x) {
		        		case CMD_PAR_DIGIT_CHAR: entscheide=is_number_string; break;
		        		case CMD_PAR_ALPHA_CHAR: entscheide=is_alpha_string; break;
		        		case CMD_PAR_SOMETH_CHAR: entscheide=is_something; break;
		                default: return SERIALPROTOCOL_STATUS_ERROR; break;
	      			}

	      			if(entscheide(split_parameter[j]))
	      			{
	      				hserialprotocol->Command.Parameter[j].ParameterKind=x;
	      				strcpy(hserialprotocol->Command.Parameter[j].Value,split_parameter[j]);
	      			}else
	      			{
	      				return SERIALPROTOCOL_STATUS_ERROR;
	      			}
	      		}
	      		return SERIALPROTOCOL_STATUS_OK;
	      	}
		}

		return SERIALPROTOCOL_STATUS_ERROR;

		/* Überprüfen ob der Input ein Get-Kommando ist */
	}else if(string_char_frequency(inputmessage,CMD_SOF_STR) == 1 && string_char_frequency(inputmessage,CMD_GET_STR) == 1 && string_char_frequency(inputmessage,CMD_SET_STR) == 0 && string_char_frequency(inputmessage,CMD_EOF_STR) == 1 && strcspn( inputmessage, CMD_SOF_STR )==0 && (strcspn( inputmessage, CMD_GET_STR )+2) == strlen(inputmessage) && (strcspn( inputmessage, CMD_EOF_STR )+1) == strlen(inputmessage) ){

		hserialprotocol->Command.CommandKind = SERIALPROTOCOL_COMMANDKIND_GET;

		strncpy(hserialprotocol->Command.Name, inputmessage+1,strlen(inputmessage)-3);

		/* Input für die Überptüfung zerteilen */
		char split_cmd[2][SERIALPROTOCOL_MessageBuffer_SIZE]={0};
		char split_parameter[6][SERIALPROTOCOL_MessageBuffer_SIZE]={0};
		cut_input(CMD_GET_STR,inputmessage,split_cmd);

		/* überprüfen ob das Kommando existiert */
		for(uint8_t i=0;i<(sizeof(commandlist)/sizeof(commandlist[0]));i++)
		{
			/* Input für die Überptüfung zerteilen */
			char split_stern[2][FIELD_MAX]={0};
			cut_input(CMD_SEP_PAR_TYP_STR,commandlist[i],split_stern);
			char split_dop[3][FIELD_MAX]={0};
			cut_input(CMD_SEP_DEF_STR,split_stern[0],split_dop);

			if(!strcmp(hserialprotocol->Command.Name,split_dop[0]) ){

				/* Überprüfen ob es sich um ein Get-Kommando handelt */
				if(strcmp(CMD_CMD_GETSET_STR,split_dop[1]) && strcmp(CMD_CMD_GET_STR,split_dop[1]) ){
					return SERIALPROTOCOL_STATUS_ERROR;
				}
				hserialprotocol->Command.ParameterAmount=atoi(split_dop[2]);

				/* Input für die Überptüfung zerteilen */
	      		char split_kom[MAX_PARAMETERS][FIELD_MAX]={0};
	      		cut_input(CMD_SEP_PAR_STR,split_stern[1],split_kom);

	      		/* überprüfen ob die Parameter stimmen */
	      		for(uint8_t j=0;strlen(split_kom[j])!=0;j++)
	      		{
	      			char x = split_kom[j][0];
	      			switch(x) {
		        		case CMD_PAR_DIGIT_CHAR: entscheide=is_number_string; break;
		        		case CMD_PAR_ALPHA_CHAR: entscheide=is_alpha_string; break;
		        		case CMD_PAR_SOMETH_CHAR: entscheide=is_something; break;
		        		default: return SERIALPROTOCOL_STATUS_ERROR; break;
	      			}

	      			if(entscheide(split_parameter[j]))
	      			{
	      				hserialprotocol->Command.Parameter[j].ParameterKind=x;

	      			}else
	      			{
	      				return SERIALPROTOCOL_STATUS_ERROR;
	      			}
	      		}
	      		return SERIALPROTOCOL_STATUS_OK;
			}
		}

		return SERIALPROTOCOL_STATUS_ERROR;

		/* Überprüfen ob der Input ein executable-Kommando ist */
	}else if( string_char_frequency(inputmessage,CMD_SOF_STR) == 1 && string_char_frequency(inputmessage,CMD_GET_STR) == 0 && string_char_frequency(inputmessage,CMD_SET_STR) == 0 && string_char_frequency(inputmessage,CMD_EOF_STR) == 1 && strcspn( inputmessage, CMD_SOF_STR )==0 && (strcspn( inputmessage, CMD_EOF_STR )+1) == strlen(inputmessage)){

		hserialprotocol->Command.CommandKind = SERIALPROTOCOL_COMMANDKIND_EXECUTE;

		strncpy(hserialprotocol->Command.Name, inputmessage+1,strlen(inputmessage)-2);
		char split_cmd[2][SERIALPROTOCOL_MessageBuffer_SIZE]={0};
		cut_input(CMD_GET_STR,inputmessage,split_cmd);

		/* überprüfen ob das Kommando existiert und die Parameter stimmen */
		for(uint8_t i=0;i<(sizeof(commandlist)/sizeof(commandlist[0]));i++)
		{
			/* Input für die Überptüfung zerteilen */
			char split_stern[2][FIELD_MAX]={0};
			cut_input(CMD_SEP_PAR_TYP_STR,commandlist[i],split_stern);
			char split_dop[3][FIELD_MAX]={0};
			cut_input(CMD_SEP_DEF_STR,split_stern[0],split_dop);

			/* Komandoinformationen speichern */
			if(!strcmp(hserialprotocol->Command.Name,split_dop[0]) ){

				hserialprotocol->Command.ParameterAmount=0;

				return SERIALPROTOCOL_STATUS_OK;
			}
		}

		return SERIALPROTOCOL_STATUS_ERROR;
	}else{

		return SERIALPROTOCOL_STATUS_ERROR;
	}
}

/**
  * @}
  */

/**
  * @}
  */
