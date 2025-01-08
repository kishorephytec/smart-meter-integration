/*
 * command_id.h
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */

#ifndef COMMAND_ID_H_
#define COMMAND_ID_H_

typedef struct cmd_id
{
	unsigned int cmdid;
	char *cmd_info; 
}cmd_id_t;

static  cmd_id_t command_id[] = {
	/* Procubed Stack Validation Tool Commands*/
	{ 0xC0 , "SWITCH_OPERATIONAL_MODE"},
	{ 0xC1 , "SWITCH_OPERATIONAL_MODE_CONF"},
	{ 0xC2 , "GET_OPERATIONAL_MODE"},
	{ 0xC3, "SEND_OPERATINAL_MODE"},
	{ 0xC4 , "SET_TX_PKT_CONFIG"},
	{ 0xC5 , "SET_TX_PKT_CONFIG_CONF"},
	{ 0xC6 , "START_PACKET_TX_REQ"},
	{ 0xC7, "START_PACKET_TX_REQ_CONF"},
	{0xC8, "TX_PACKET_CONF"},
	{0xCB, "START_PACKET_RX_REQ"},
	{0xCC, "START_PACKET_RX_CONF"},
	{0xCD , "STOP_PACKET_RX_REQ"},
	{0xCE, "STOP_PACKET_RX_REQ_CONF"},
	{0xCF, "RX_FRAME_IND"},
	{0xD0, "RX_FRAME_IND_DISPLAY_OFF"},
	{0xD1 , "START_START_CONTINUOUS_TX_REQ"},
	{0xD2, "START_START_CONTINUOUS_TX_REQ_CONF"},
	{0xD3, "STOP_CONTINUOUS_TX_REQ"},
	{0xD4, "STOP_CONTINUOUS_TX_CONF"},
	{0xD5, "START_CONTINUOUS_RX_REQ"},
	{0xD6, "START_CONTINUOUS_RX_CONF"},
	{0xD7, "STOP_CONTINUOUS_RX_REQ"},
	{0xD8, "STOP_CONTINUOUS_RX_CONF"},
	{0xD9, "GET_RX_COUNT_DETAILS"},
	{0xDA, "SEND_RX_COUNT_DETAILS"},
	{0xDB , "PHY_ENC_TEST_TX"},
	//{0xDC , "ACTIVE_CMD_NOT_SUPPORTED"},
	{0xDD, "SET_RSSI_THRESHOLD"},
	{ 0xDE, "SET_RSSI_THRESHOLD_CONF" },
	{ 0xDF, "SET_ADJUST_FREQ" },
	{ 0xE0, "SET_ADJUST_FREQ_CONF" },
	{ 0xE1, "SET_SERIAL_BAUDRATE" },
	{ 0xE2, "SET_SERIAL_BAUDRATE_CONF" },
	{ 0xE3, "GET_CONFIG_INFO_REQ" },
	{ 0xE4, "GET_CONFIG_INFO_RESP" },

		/* Internal Commands Commands*/

	{ 0x10 , "SEND_SELF_MAC_ADDR" },
	{ 0x11 , "RECIVED_SELF_MAC_ADDR_CONF" },
	{ 0x12 , "SEND_GTK_HASH" },
	{ 0x13, "RECIVED_GTK_HASH_CONF" },
	{ 0x14 , "SEND_START_CMD_TO_HOST_APD" },
	{ 0x15 , "RECIVED_START_CMD_TO_HOST_APD_CONF" },
	{ 0x16 , "SEND_EAPOL_PACKT_TRANSMIT" },
	{ 0x17 ,"SEND_EAPOL_PACKET_RECIEVED" },
	{ 0x18 , "RECIVED_EAPOL_AUTH_SUCESS_CONF" },
	{ 0x19 , "RECEIVED_GTK_UPDATE_IND" },
	{ 0x1A , "SEND_GTK_UPDATE_CONF" },
	{ 0x1B, "RECEIVED_MAC_SEC_SET_REQUEST" },
	{ 0x1C , "SEND_MAC_SEC_SET_CONF" },
	{ 0x1D , "SEND_LIFETIME" },
	{0x1E	, "SEND_REVOCATION_KEY " },
	{0x1F	, "SEND_EAPOL_PACKET_ACK" },
	{0x20	, "SEND_REVOKE_STA_REQ " },
	{0x21	, "RECV_REVOKE_STA_CONF   " },                 


		/*Swagger Hub Commands*/

	{ 0x30	 , "NODE_START_STOP (Swagger Hub Commands)" },
	{ 0x31	 , "NODE_START_STOP_CONF(Swagger Hub Commands)" },
	{ 0x32 , "SET_PHY (Swagger Hub Commands)" },
	{ 0x33 , "CONF_SET_PHY" },
	{ 0x34	 , "SET_MAC_CHANNEL_PLAN_REG_OP_API (Swagger Hub Commands)" },
	{ 0x35	 , "SET_MAC_CHANNEL_PLAN_REG_OP_API_CONF (Swagger Hub Commands)" },
	{ 0x36	 , "SET_MAC_CHANNEL_PLAN_EXPLICIT_API (Swagger Hub Commands)" },
	{ 0x37	 , "SET_MAC_CHANNEL_PLAN_EXPLICIT_API_CONF (Swagger Hub Commands)" },
	{ 0x38	 , "SET_MAC_UNICAST_API (Swagger Hub Commands)" },
	{ 0x39	 , "SET_MAC_UNICAST_API_CONF (Swagger Hub Commands)" },
	{ 0x3A	 , "SET_MAC_BROADCAST_API (Swagger Hub Commands)" },
	{ 0x3B	 , "SET_MAC_BROADCAST_API_CONF (Swagger Hub Commands)" },
	{ 0x3C	 , "SET_MAC_CHAN_PLAN_FIXED (Swagger Hub Commands)" },
	{ 0x3D	 , "SET_MAC_CHAN_PLAN_FIXED_CONF (Swagger Hub Commands)" },
	{ 0x3E	 , "SET_LBR_MAC_CONFIG (Swagger Hub Commands)" },
	{ 0x40	 , "SET_LBR_MAC_GTKS_CONFIG (Swagger Hub Commands)" },
	{ 0x41	 , "SET_LBR_MAC_GTKS_CONFIG_CONF (Swagger Hub Commands)" },
	{ 0x5C	 , "SET_LBR_MAC_PMK_PTK_GTK_LIFETIME_CONFIG (Swagger Hub Commands)" },
	{ 0x5D	 , "SET_LBR_MAC_PMK_PTK_GTK_LIFETIME_CONFIG_CONF (Swagger Hub Commands)" },
	{ 0x44	 , "RESET_RPL_MSG_RATE (Swagger Hub Commands)" },
	{ 0x45	 , "RESET_RPL_MSG_RATE_CONF (Swagger Hub Commands)" },
	{ 0x4E	 , "GET_SEC_KEYS (Swagger Hub Commands)" },
	{ 0x4F	 , "GET_SEC_KEYS_CONF (Swagger Hub Commands)" },
	{ 0x4C	 , "GET_IP_ADDRESSES (Swagger Hub Commands)" },
	{ 0x4D	 , "SEND_IPv6_ADDRESS (Swagger Hub Commands)" },
	//{ 0x48	 , "SEND_UDP (Swagger Hub Commands)" },
	//{ 0x49	 , "SEND_UDP_ECHO_RESP (Swagger Hub Commands)" },
	{ 0x4A	 , "SEND_ICMPv6 (Swagger Hub Commands)" },
	{ 0x4B	 , "SEND_ICMPv6_CONF (Swagger Hub Commands)" },
	{ 0x46	 , "SUBSCRIBE_PACKETS (Swagger Hub Commands)" },
	{ 0x47	 , "SUBSCRIBE_PACKETS_CONF (Swagger Hub Commands)" },
	{ 0x50	 , "API_GET_DODAG_ROUTES (Swagger Hub Commands)" },
	{ 0x52	 , "API_GET_NEIGHBOR_TABLE (Swagger Hub Commands)" },
	{ 0x55	 , "API_GET_JOIN_STATE (Swagger Hub Commands)" },
	{ 0x42	 , "SET_ROUTER_CONFIG (Swagger Hub Commands)" },

		/*Tool Command*/

	{ 0x8B	, "SET_BASIC_CONFIG" },
	{ 0x8C	, "SET_BASIC_CONFIG_CONF" },
	{ 0x82	, "DEVICE_NODE_CONFIG" },
	{ 0x83	, "DEVICE_NODE_CONFIG_CONF" },
	{ 0x8F	, "SYSTEM_RESET" },
	{ 0x8D	, "START_FAN_NETWORK" },
	{ 0x8E	, "START_FAN_NETWORK_CONF" },
	{ 0x9F	, "CONTROLLER_APP_START_REQ" },
	{ 0xA0	, "CONTROLLER_APP_START_CONF" },
	{ 0x90	, "APP_2_ENET_ECHO_REQ" },
	{ 0x91	, "APP_2_ENET_ECHO_RESP" },
	{ 0x92	, "APP_2_FAN_UDP_REQ" },
	{ 0x93	, "APP_2_FAN_UDP_RCV_CB" },
	{ 0xA1	, "FAN_BROADCAST_TIMING_SET_REQUEST" },
	{ 0xA2	, "FAN_BROADCAST_TIMING_SET_CONFIRM" },
	{ 0xA3	, "FAN_NETNAME_IE_SET_REQUEST" },
	{ 0xA4	, "FAN_NETNAME_IE_SET_CONFIRM" },
	{ 0xAB	, "FAN_BROADCAST_IE_SET_REQUEST" },
	{ 0xAC	, "FAN_BROADCAST_IE_SET_CONFIRM" },
	{ 0xAD	, "FAN_UNICAST_IE_SET_REQUEST" },
	{ 0xAE	, "FAN_UNICAST_IE_SET_CONFIRM" },
	{ 0xA5	, "FAN_PAN_IE_SET_REQUEST" },
	{ 0xA6	, "FAN_PAN_IE_SET_CONFIRM" },
	{ 0xA7	, "FAN_GTK_HASH_IE_SET_REQUEST" },
	{ 0xA8	, "FAN_GTK_HASH_IE_SET_CONFIRM" },
	{ 0xA9	, "FAN_PAN_VER_IE_SET_REQUEST" },
	{ 0xAA	, "FAN_PAN_VER_IE_SET_CONFIRM" },
	{ 0x9D	, "TX_SECHEDULE_END" },
	{ 0x9E	, "TX_SECHEDULE_START" },
	{ 0x86	, "UDP_PORT_REGISTER" },
	{ 0x87	, "UDP_PORT_REGISTER_CONF" },
	{ 0x80	, "TELEC_SET_PA_LEVEL_MCR" },
	{ 0x81	, "TELEC_SET_PA_LEVEL_MCR_CONF" },
	
//	{0xDC 	, "MLME_SET_FAN_MAC_CHANNEL_INFO_REQ" },
	{0xDD	, "MAC_MLME_SET_MAC_CHANNEL_INFO_CONFIRM" },
	{0xDE	, "FAN_MAC_MLME_SET_REQUEST" },
	{0xDF	, "FAN_MAC_MLME_SET_CONFIRM" },
	{0x78	, "MLME_WS_ASYNC_FRAME_REQ" },
	{0xEE	 , "WS_ASYNC_FRAME_REQ_CONF" },
	{0xEF	 , "WS_ASYNC_FRAME_INDICATION " },
	{0x88	 , "MLME_EAPOL_FRAME_REQ " },
	{0x89	, "MLME_EAPOL_INDICATION  " },  
	{0x8A	 , "MLME_REQ_CONF " },
	{0x7B	, "FAN_MAC_MCPS_DATA_REQUEST  " },                     
	{0x7C	, "FAN_MAC_MCPS_DATA_CONFIRM " },                      
	{0x7D	, "FAN_MAC_MCPS_DATA_INDICATION" },
	{0x7E 	, "FAN_MAC_ACK_INDICATION" },
	{0x7F	, "FAN_MAC_NO_ACK_INDICATION   " },         

	{ 0x00, NULL }



};



#endif /* COMMAND_ID_H_ */
