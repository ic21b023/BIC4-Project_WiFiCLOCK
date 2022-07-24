/**
  ******************************************************************************
  * @file    at_commands.h
  * @author  Reiter R.
  * @brief   Header file of AT-COMMANDS for the WIFIPROTOCOL module.
  ******************************************************************************
  */

#ifndef INC_AT_COMMANDS_H_
#define INC_AT_COMMANDS_H_

#include "string.h"


/** @addtogroup WIFIPROTOCOL WIFIPROTOCOL
  * @{
  */

/* Exported constants --------------------------------------------------------*/
/** @addtogroup WIFIPROTOCOL_Exported_Constants WiFiProtocol Exported Constants
  * @{
  */


#define TOSTR(s) #s
#define AT_SET "="
#define AT_GET "?"
#define CRLF "\r\n"
#define H_KOMMA "\""
//#define GF  """
#define KOMMA ","
#define AT_OFF "0"
#define AT_ON "1"
/*  Basic AT Commands */

inline int mul(int a, int b){return(a*b);
}


 /* Wi-Fi AT Commands */

#define CWMODE_DISABLE_WIFI 0 		    			/*!< Null mode. Wi-Fi RF will be disabled */
#define CWMODE_STATION_MODE 1      		    		/*!< Station mode  */
#define CWMODE_SOFTAP_MODE  2         				/*!< SoftAP mode    */
#define CWMODE_SOFTAP_AND_STATION_MODE  3  			/*!< SoftAP+Station mode */

#define AT_CWMODE 									"AT+CWMODE"
#define CMD_AT_CWMODE_SET(cwmode)           		AT_CWMODE AT_SET TOSTR(cwmode) CRLF /* Set the Wi-Fi mode (Station/SoftAP/Station+SoftAP) */
#define CMD_AT_CWMODE_GET               			AT_CWMODE AT_GET CRLF

#define AT_CIPSTO									"AT+CIPSTO"
#define CMD_AT_CIPSTO_SET(timeout)					AT_CIPSTO AT_SET TOSTR(timeout) CRLF /* Enable/disable the multiple connections mode */
#define CMD_AT_CIPSTO_GET               			AT_CIPSTO AT_GET CRLF

#define AT_CWJAP 									"AT+CWJAP"
#define CMD_AT_CWJAP_SET(ssid,pwd) 	        		AT_CWJAP AT_SET H_KOMMA #ssid H_KOMMA KOMMA H_KOMMA #pwd H_KOMMA CRLF  /* Connect to an AP */
#define CMD_AT_CWJAP_GET							AT_CWJAP AT_GET CRLF

#define AT_CIPSTART									"AT+CIPSTART"
#define CMD_AT_CIPSTART_SET(ssid,pwd,port) 	    	AT_CIPSTART AT_SET H_KOMMA #ssid H_KOMMA KOMMA H_KOMMA #pwd H_KOMMA KOMMA #port CRLF  /* Connect to an AP */


#define CIPMUXMODE_SINGLE_CONN              		0 		    	/*!< single connection */
#define CIPMUXMODE_MULTIPLE_CONN            		1      		    /*!< multiple connections */
#define AT_CIPMUX                            		"AT+CIPMUX"
#define CMD_AT_CIPMUX_SET(cipmux)					AT_CIPMUX AT_SET TOSTR(cipmux) CRLF /* Enable/disable the multiple connections mode */
#define CMD_AT_CIPMUX_GET               			AT_CIPMUX AT_GET CRLF

#define AT_CWAUTOCONN								"AT+CWAUTOCONN"
#define AT_CWAUTOCONN_OFF							AT_CWAUTOCONN AT_SET AT_OFF CRLF
#define AT_CWAUTOCONN_ON							AT_CWAUTOCONN AT_SET AT_ON CRLF

#define AT_CWSAP                            		"AT+CWSAP"
#define CMD_AT_CWSAP_SET(ssid,pwd,chl,ecn) 			AT_CWSAP AT_SET H_KOMMA #ssid H_KOMMA KOMMA H_KOMMA #pwd H_KOMMA KOMMA #chl KOMMA #ecn CRLF  /* Query/Set the configuration of an ESP32 SoftAP */
#define CMD_AT_CWSAP_GET							AT_CWSAP AT_GET CRLF

#define AT_CIPSTATUS                            	"AT+CIPSTATUS"


#define AT_CIPAP                            		"AT+CIPAP"
#define CMD_AT_CIPAP_SET(ip,gateway,netmask) 		AT_CIPAP AT_SET H_KOMMA #ip H_KOMMA KOMMA H_KOMMA #gateway KOMMA H_KOMMA #netmask H_KOMMA CRLF  /* Query/Set the IP address of an ESP32 SoftAP */
#define CMD_AT_CIPAP_GET							AT_CIPAP AT_GET CRLF


#define CIPSERVERMODE_DELETE_SERVER             	0 		    	/*!< delete a server */
#define CIPSERVERMODE_CREATE_SERVER            		1      		    /*!< create a server */

#define AT_CIPSERVER                           		"AT+CIPSERVER"
#define CMD_AT_CIPSERVER_SET(cipserver)				AT_CIPSERVER AT_SET TOSTR(cipserver) CRLF /* Delete/create a TCP/SSL server */
#define CMD_AT_CIPSERVER_SET_PORT(cipserver,port)		AT_CIPSERVER AT_SET TOSTR(cipserver) "," TOSTR(port) CRLF /* Delete/create a TCP/SSL server */
#define CMD_AT_CIPSERVER_GET               			AT_CIPSERVER AT_GET CRLF


#define AT_CIPSEND                              	"AT+CIPSEND"
#define CMD_AT_CIPSEND_SERVER(linkid,length)		AT_CIPSEND AT_SET TOSTR(linkid) KOMMA TOSTR(length) CRLF /* Delete/create a TCP/SSL server */
#define CMD_AT_CIPSEND_CLIENT(length)				AT_CIPSEND AT_SET TOSTR(length) CRLF /* Delete/create a TCP/SSL server */

#define AT_CWQIF                            		"AT+CWQIF"
#define CMD_AT_CWQIF                            	"AT+CWQIF"

#define AT_CIPCLOSE  							 	"AT+CIPCLOSE"
#define CMD_AT_CIPCLOSE(send)						AT_CIPCLOSE AT_SET TOSTR(send) CRLF /* Delete/create a TCP/SSL server */

#define AT_RESTORE 									"AT+RESTORE"
#define CMD_AT_RESTORE 								AT_RESTORE CRLF

#define AT_ATE 									    "ATE"
#define CMD_AT_ATE_OFF								AT_ATE "0" CRLF
#define CMD_AT_ATE_ON								AT_ATE "1" CRLF

#define AT_AP_REVEIVE_DATA							"+IPD"
#define AT_STATION_CONNECTED						"+STA_CONNECTED:"
#define AT_STATION_DISCONNECTED						"+STA_DISCONNECTED:"
#define AT_AP_DISTRIBUTES_IP						"+DIST_STA_IP:"

#define RESP_AT_OK									"OK"
#define RESP_AT_ERROR								"ERROR"
#define RESP_AT_SEND_OK								"SEND OK"


 /**
   * @}
   */

 /**
   * @}
   */
#endif /* INC_AT_COMMANDS_H_ */
