/**
******************************************************************************
* @file wifiprotocol.c
* @author Reiter Roman
* @brief Source file of WIFIPROTOCOL module.
* Diese Datei bietet Funktionen für die Kommunikation über UART
*
@verbatim
 ===============================================================================
                      ##### How to use this driver #####
 ===============================================================================
   [..]
	Der Wifiprotocol-Treiber kann wie folgt verwendet werden:

	(#) Deklarieren einer WIFIPROTOCOL_HandleTypeDef handle structure
    (#) Initialisieren des Wifiprotocol's mit der Methode WIFIPROTOCOL_Init.

	(#) Mit WIFIPROTOCOL_Reveive_Message() werden Nachrichten empfangen

	(#) Mit WIFIPROTOCOL_Send_Message_AS_SERVER() werden serverseitige Nachrichten versendet

	(#) Mit WIFIPROTOCOL_Send_Message_AS_CLIENT() werden clientseitige Nachrichten versendet

	(#) Mit WIFIPROTOCOL_TCP_Close_AS_Server() wird die TCP-Verbindung vom Server beendet.

	(#) Mit WIFIPROTOCOL_TCP_Close_AS_Client() wird die TCP-Verbindung vom Client beendet.

	(#) Mit WIFIPROTOCOL_TCP_Open_AS_Client() wird eine TCP-Verbindung vom Client zum Server aufgebaut

	(#) Mit WIFIPROTOCOL_Connect_To_AP_AS_Client() kann sich eine Station zu einem Soft-AP verbinden

	(#) WIFIPROTOCOL_TransmitAdapter() muss die Sendemethode des Mikrocontrollers implementiert werden.

	(#) WIFIPROTOCOL_ReceiveAdapter() muss die Empfangsmethode des Mikrocontrollers implementiert werden.

  @endverbatim
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "wifiprotocol.h"
#include <stdlib.h>

/** @addtogroup WIFIPROTOCOL WIFIPROTOCOL
  * @brief WiFiProtocol Module Driver
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Private_Defines WIFIPROTOCOL Private Defines
  * @{
  */
#define STATE_SEND_RECEIVE 1
#define STATE_CHECK_RESPONSE 2
#define	STATE_SEND_MESSAGE 3
#define	STATE_EXTRACT_MESSAGE 4
#define STATE_BREAK 99
/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Private_Variables WIFIPROTOCOL Private Variables
  * @{
  */

static char * init_commands_server[7]={

		CMD_AT_RESTORE,
		CMD_AT_ATE_OFF,
		AT_CWAUTOCONN_OFF,
		CMD_AT_CWSAP_SET(ESP32_CLK,1234567890,5,3),
		CMD_AT_CWMODE_SET(CWMODE_SOFTAP_MODE),
		CMD_AT_CIPMUX_SET(CIPMUXMODE_MULTIPLE_CONN),
		CMD_AT_CIPSERVER_SET(CIPSERVERMODE_CREATE_SERVER),
		//CMD_AT_CIPSTO_SET(10)
};

static char * init_commands_client[4]={

		CMD_AT_RESTORE,
		CMD_AT_ATE_OFF,
		AT_CWAUTOCONN_OFF,
		CMD_AT_CWMODE_SET(CWMODE_STATION_MODE),
};

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Private_Functions WIFIPROTOCOL Private Functions
  * @{
  */

static int check_message_content(char * rx_message, int message_buff_size, char * set, int set_strlen, int ab);
static void rxtx_calibration(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tx_message, uint16_t tx_buffer_size, char * rx_message, uint16_t rx_buffer_size);
static int find_char_pos_left(char * rx_message, int Size,char set);
static int menge(char * rx_message, int Size);
static void extract_message_ipd(char * rx_message,int Size, char * corr_message);
static WIFIPROTOCOL_StatusTypeDef send(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tx_message, char * rx_message, char * content_string, int attempts);

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Exported_Functions WIFIPROTOCOL Exported Functions
  * @{
  */

/** @addtogroup WIFIPROTOCOL_Exported_Functions_Group1 Initialization Functions
 *  @brief   Initialization Functions
 *
@verbatim
 ===============================================================================
             ##### Initialization Functions  #####
 ===============================================================================
    [..] Dienen fürs initialisieren des Wifiprotocl Modules

@endverbatim
  * @{
  */

/**
  * @brief  verarbeitet die einzel Eingegebenen Zeichen von RxBuffer und gibt dementsprechend die Antwort im TXBuffer zurück
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @param  initmode 	 Initialisieren als Client oder Serer
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Init(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, WIFIPROTCOL_InitTypeDef initmode){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;
	char ** run_coms;
	int commands=0;
	if(initmode==WIFIPROTOCOL_AS_CLIENT){
		run_coms= init_commands_client;
		commands=4;
	}else if(initmode==WIFIPROTOCOL_AS_SERVER){
		run_coms= init_commands_server;
		commands=7;
	}

	for(int i=0;i<commands;i++)
	{
		if(send(hwifiprotocol,run_coms[i],hwifiprotocol->Communication.Rx,"OK",SEND_ATTEMPTS_COMMANDS)){
			hwifiprotocol->Status=WIFIPROTOCOL_READY;
			return WIFIPROTOCOL_ERROR;
		}

	}
	hwifiprotocol->Status=WIFIPROTOCOL_READY;
	return WIFIPROTOCOL_OK;
}

/**
  * @}
  */

/** @addtogroup WIFIPROTOCOL_Exported_Functions_Group2 IO Operation Functions
 *  @brief   IO Operation Functions
 *
@verbatim
 ===============================================================================
                      #####  IO Operation Functions  #####
 ===============================================================================
    [..]  Dienen dazu, um die Kommunikation zwischen MCU und Wifi-Modul zu handeln

@endverbatim
  * @{
  */

/**
  * @brief  verarbeitet Empfangene Nachrichten
  * @param  hwifiprotocol  		WIFIPROTOCOL handle
  * @param  receivedMessage		empfangene Nachtricht
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Reveive_Message(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * receivedMessage)
{

	if(hwifiprotocol->Status!=WIFIPROTOCOL_BUSY){
		WIFIPROTOCOL_ReceiveAdapter((uint8_t*)hwifiprotocol->Communication.Rx, BUFFER_SIZE);
		if(hwifiprotocol->Communication.Event.MessageReceived==1)
		{
			//if(check_message_content(rx_message,BUFFER_SIZE,"WIFI CONNECTED",14)){
			if(check_message_content(hwifiprotocol->Communication.Rx,BUFFER_SIZE,"+IPD",4,0)){
				extract_message_ipd(hwifiprotocol->Communication.Rx,BUFFER_SIZE,receivedMessage);
				return WIFIPROTOCOL_OK;
			}
			memset(hwifiprotocol->Communication.Rx,0,BUFFER_SIZE);
			hwifiprotocol->Communication.Event.MessageReceived=0;
		}
	}
	return WIFIPROTOCOL_TIMEOUT;
}

/**
  * @brief  verarbeitet die einzel Eingegebenen Zeichen von RxBuffer und gibt dementsprechend die Antwort im TXBuffer zurück
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @param  channel 	 TCP-Kanal
  * @param  tx_text 	 Sendetext
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Send_Message_AS_SERVER(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol,char * channel, char * tx_text ){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;
	char cmd_at_chipsend_set[50] = AT_CIPSEND ;
	strcat(cmd_at_chipsend_set,AT_SET);
	strcat(cmd_at_chipsend_set,channel);
	strcat(cmd_at_chipsend_set,",");
	char chars[10];
	itoa(strlen(tx_text),chars,10);
	strcat(cmd_at_chipsend_set,chars);
	strcat(cmd_at_chipsend_set, CRLF);

	if(!send(hwifiprotocol, cmd_at_chipsend_set,hwifiprotocol->Communication.Rx,">",1)){
		if(!send(hwifiprotocol, tx_text,hwifiprotocol->Communication.Rx,"SEND OK",1)){
			hwifiprotocol->Status=WIFIPROTOCOL_READY;
			return WIFIPROTOCOL_OK;
		}
	}
	hwifiprotocol->Status=WIFIPROTOCOL_READY;
	return WIFIPROTOCOL_ERROR;
}

/**
  * @brief  verarbeitet die einzel Eingegebenen Zeichen von RxBuffer und gibt dementsprechend die Antwort im TXBuffer zurück
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @param  tx_text 	 Sendetext
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Send_Message_AS_CLIENT(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tx_text ){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;
	char cmd_at_chipsend_set[50] = AT_CIPSEND ;
	strcat(cmd_at_chipsend_set,AT_SET);
	char chars[10];
	itoa(strlen(tx_text),chars,10);
	strcat(cmd_at_chipsend_set,chars);
	strcat(cmd_at_chipsend_set, CRLF);

	if(!send(hwifiprotocol, cmd_at_chipsend_set,hwifiprotocol->Communication.Rx,">",SEND_ATTEMPTS_COMMANDS)){
		if(!send(hwifiprotocol, tx_text,hwifiprotocol->Communication.Rx,"SEND OK",SEND_ATTEMPTS_MESSAGE)){
			hwifiprotocol->Status=WIFIPROTOCOL_READY;
			return WIFIPROTOCOL_OK;
		}
	}
	hwifiprotocol->Status=WIFIPROTOCOL_READY;
	return WIFIPROTOCOL_ERROR;
}

/**
  * @brief  schließt serverseitig die TCP-Verbindung
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @param  channel 	 TCP-Kanal
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TCP_Close_AS_Server(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * channel){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;
	char cmd_at_cipclose_set[50] = AT_CIPCLOSE;
		strcat(cmd_at_cipclose_set,AT_SET);
		strcat(cmd_at_cipclose_set,channel);
		strcat(cmd_at_cipclose_set, CRLF);
	if(!send(hwifiprotocol,cmd_at_cipclose_set,hwifiprotocol->Communication.Rx, "OK",SEND_ATTEMPTS_COMMANDS)){
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_OK;
	}
	else{
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_ERROR;
	}
}

/**
  * @brief  schließt clientseitig die TCP-Verbindung
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TCP_Close_AS_Client(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;

	char cmd_at_cipclose_set[50] = AT_CIPCLOSE;
	strcat(cmd_at_cipclose_set,CRLF);

	if(!send(hwifiprotocol, cmd_at_cipclose_set,hwifiprotocol->Communication.Rx, "OK",SEND_ATTEMPTS_COMMANDS)){
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_OK;
	}else{
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_ERROR;
	}
}

/**
  * @brief  öffnet clientseitig eine TCP-Verbindung
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @param  con 	Verbindungstyp
  * @param  ip 	 	IP-Adresse
  * @param  port	Port
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TCP_Open_AS_Client(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tcp, char * ip, char * port){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;
	char cmd_at_cipstart_set[50] = AT_CIPSTART;
		strcat(cmd_at_cipstart_set,AT_SET);
		strcat(cmd_at_cipstart_set,H_KOMMA);
		strcat(cmd_at_cipstart_set,tcp);
		strcat(cmd_at_cipstart_set,H_KOMMA  );
		strcat(cmd_at_cipstart_set, KOMMA );
		strcat(cmd_at_cipstart_set,  H_KOMMA);
		strcat(cmd_at_cipstart_set,ip);
		strcat(cmd_at_cipstart_set,H_KOMMA );
		strcat(cmd_at_cipstart_set, KOMMA);
		strcat(cmd_at_cipstart_set,port);
		strcat(cmd_at_cipstart_set, CRLF);

	if(!send(hwifiprotocol,cmd_at_cipstart_set, hwifiprotocol->Communication.Rx, "CONNECT\r\n\r\nOK",SEND_ATTEMPTS_COMMANDS)){
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_OK;
	}
	else{
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_ERROR;
	}
}

/**
  * @brief  Client zu einem AP verbinden
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @param  ssid	 Ein-Zeichen-Empfangspuffer
  * @param  pw 	 Sendepuffer/Antwortpuffer
  * @retval WIFIPROTOCOL status
  */
WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_Connect_To_AP_AS_Client(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol,char * ssid, char * pw){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;
	char cmd_at_cwjap_set[50] = AT_CWJAP;
		strcat(cmd_at_cwjap_set,AT_SET);
		strcat(cmd_at_cwjap_set,H_KOMMA);
		strcat(cmd_at_cwjap_set,ssid);
		strcat(cmd_at_cwjap_set,H_KOMMA);
		strcat(cmd_at_cwjap_set,KOMMA);
		strcat(cmd_at_cwjap_set,H_KOMMA);
		strcat(cmd_at_cwjap_set,pw);
		strcat(cmd_at_cwjap_set,H_KOMMA);
		strcat(cmd_at_cwjap_set,CRLF);

	if(!send(hwifiprotocol,cmd_at_cwjap_set,hwifiprotocol->Communication.Rx ,"WIFI CONNECTED",SEND_ATTEMPTS_COMMANDS)){
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_OK;
	}
	else{
		hwifiprotocol->Status=WIFIPROTOCOL_READY;
		return WIFIPROTOCOL_ERROR;
	}
}

/**
  * @}
  */

/** @addtogroup WIFIPROTOCOL_Exported_Functions_Group3 Adapter Functions
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
  * @retval custom
  */
__weak WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_TransmitAdapter(uint8_t * buffer, uint16_t buffersize)
{
	/* Prevent unused argument(s) compilation warning */
	//UNUSED(hwifiprotocol);

	/* NOTE : This function should not be modified, when the callback is needed,
            	the SERIALPROT_Command_GPO_Callback could be implemented in the user file
	 */
	return 1;
}

/**
  * @brief  dient als Wrapper, wo die Empfangsfunktion des Mikrocontrollers eingebunden werden soll
  * @param  buffer 	Zeichenkette welche übertragen werden soll
  * @param  buffersize Länge der Zeichenkette
  * @retval custom
  */
__weak WIFIPROTOCOL_StatusTypeDef WIFIPROTOCOL_ReceiveAdapter(uint8_t * buffer, uint16_t buffersize)
{
	/* Prevent unused argument(s) compilation warning */
	//UNUSED(hwifiprotocol);

	/* NOTE : This function should not be modified, when the callback is needed,
            	the SERIALPROT_Command_GPO_Callback could be implemented in the user file
	 */
	return 1;
}

/**
  * @}
  */

/* Private functions----------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Private_Functions WIFIPROTOCOL Private Functions
  * @{
  */

/**
  * @brief  Inhalt einer Nachricht überprüfen
  * @param  rx_message 	Nachricht
  * @param  message_buff_size 	Nachrichtengröße
  * @param  set 	Zeichenkette
  * @param  set_strlen 	länge der Zeichenkette
  * @param  ab 		ab welcher Position soll gestartet werden
  * @retval result
  */
static int check_message_content(char * rx_message, int message_buff_size, char * set, int set_strlen, int ab){

	for (int i=ab; i<message_buff_size; i++)
	{
		if(rx_message[i]==set[0])
		{
			int count=0;
			for (int j=0; j<set_strlen; j++)
			{
				if(rx_message[(i+j)%(message_buff_size)] == set[j]){
					count++;
					if(count==set_strlen){
						return i+1;
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	return 0;
}

/**
  * @brief  zum Übertragen von Kommandos
  * @param  hwifiprotocol  WIFIPROTOCOL handle
  * @param  tx_message 	Sendenachricht
  * @param  tx_buffer_size 	Größe der Sendenachricht
  * @param  rx_message 	Empfangsnachricht
  * @param  rx_buffer_size 	Größe der Empfangsnachricht
  * @retval none
  */
static void rxtx_calibration(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tx_message, uint16_t tx_buffer_size, char * rx_message, uint16_t rx_buffer_size){

	for( int i =0;i<300000;i++){};
	WIFIPROTOCOL_TransmitAdapter((uint8_t*)tx_message ,tx_buffer_size);
	while(hwifiprotocol->Communication.Event.MessageReceived==0){}
	hwifiprotocol->Communication.Event.MessageReceived=0;
	for( int i =0;i<300000;i++){};
}

/**
  * @brief  gibt die Position einer Zeichenkette zurück
  * @param  rx_message 	Nachricht
  * @param  Size 	Größe der Nachricht
  * @param  set 	der zu überprüfende String
  * @retval result
  */
static int find_char_pos_left(char * rx_message, int Size,char set){

	for (int i=0; i<Size; i++)
	{
		if ((rx_message[i] == set) )
		{
			return i;
		}
	}

	return 0;
}

/**
  * @brief  überprüft ob ein String eine Zahl enthält
  * @param  string[] 	der zu überprüfende String
  * @retval result
  */
static int menge(char * rx_message, int Size){

    int start =find_char_pos_left(rx_message,Size,',');
    int ende =find_char_pos_left(rx_message,Size,':');
    char zahl_str[ende-start];
    memset(zahl_str,0,sizeof(zahl_str));
    memcpy(zahl_str,rx_message+start+1,ende-start-1);
    return atoi(zahl_str);
}

/**
  * @brief  überprüft ob ein String eine Zahl enthält
  * @param  string[] 	der zu überprüfende String
  * @retval result
  */
static void extract_message_ipd(char * rx_message,int Size, char * corr_message){

     int ende = find_char_pos_left(rx_message,Size,':');
     memcpy(corr_message,rx_message+ende+1,menge(rx_message,Size));
}

/**
  * @brief  überprüft ob ein String eine Zahl enthält
  * @param  hwifiprotocol 	der zu überprüfende String
  * @param  tx_message 	der zu überprüfende String
  * @param  rx_message 	der zu überprüfende String
  * @param  content_string 	der zu überprüfende String
  * @param  attempts 	der zu überprüfende String
  * @retval result
  */
static WIFIPROTOCOL_StatusTypeDef send(WIFIPROTOCOL_HandleTypeDef * hwifiprotocol, char * tx_message, char * rx_message, char * content_string, int attempts){
	hwifiprotocol->Status=WIFIPROTOCOL_BUSY;
	uint8_t state = STATE_SEND_RECEIVE;
	uint8_t attempt_counter=0;
	while(1)
	{
		switch (state)
		{
			case STATE_SEND_RECEIVE:
				rxtx_calibration(hwifiprotocol,tx_message,(uint16_t)strlen((const char *)tx_message),rx_message,BUFFER_SIZE);
				attempt_counter++;
				state = STATE_CHECK_RESPONSE;
			break;

			case STATE_CHECK_RESPONSE:

				if(check_message_content(rx_message,BUFFER_SIZE,content_string,strlen(content_string),0) ){
					memset(rx_message,0,BUFFER_SIZE);
					hwifiprotocol->Status=WIFIPROTOCOL_READY;
					return WIFIPROTOCOL_OK;
				}else if(attempt_counter<=attempts){
					memset(rx_message,0,BUFFER_SIZE);
					state = STATE_SEND_RECEIVE;
				}else{
					memset(rx_message,0,BUFFER_SIZE);
					hwifiprotocol->Status=WIFIPROTOCOL_READY;
					return WIFIPROTOCOL_ERROR;
				}

			break;
		}
	}
	hwifiprotocol->Status=WIFIPROTOCOL_READY;
	return WIFIPROTOCOL_ERROR;
}

/**
  * @}
  */

/**
  * @}
  */
