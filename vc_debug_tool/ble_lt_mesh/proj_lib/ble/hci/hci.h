/********************************************************************************************************
 * @file	hci.h
 *
 * @brief	for TLSR chips
 *
 * @author	telink
 * @date	Sep. 30, 2010
 *
 * @par     Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *          All rights reserved.
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#pragma  once


#include "../ble_common.h"

typedef int (*blc_hci_rx_handler_t) (void);
typedef int (*blc_hci_tx_handler_t) (void);
typedef int (*blc_hci_handler_t) (unsigned char *p, int n);
typedef int (*blc_hci_app_handler_t) (unsigned char *p);

extern blc_hci_handler_t			blc_master_handler;


#define			HCI_FLAG_EVENT_TLK_MODULE					(1<<24)
#define			HCI_FLAG_EVENT_BT_STD						(1<<25)
#define			HCI_FLAG_EVENT_STACK						(1<<26)
#define			HCI_FLAG_ACL_BT_STD							(1<<27)

#define			TLK_MODULE_EVENT_STATE_CHANGE				0x0730
#define			TLK_MODULE_EVENT_DATA_RECEIVED				0x0731
#define			TLK_MODULE_EVENT_DATA_SEND					0x0732
#define			TLK_MODULE_EVENT_BUFF_AVAILABLE				0x0733




#define			HCI_MAX_ACL_DATA_LEN              			27

#define 		HCI_MAX_DATA_BUFFERS_SALVE              	8
#define 		HCI_MAX_DATA_BUFFERS_MASTER              	8


#define 		HCI_FIRST_NAF_PACKET             			0x00
#define 		HCI_CONTINUING_PACKET             			0x01
#define 		HCI_FIRST_AF_PACKET               			0x02


/*********************************************************************
 * ENUMS
 */


/**
 *  @brief  Definition for HCI request type
 */
typedef enum hci_type_e {
	//-------- BLE stack
	HCI_TYPE_CMD = 0x01,
	HCI_TYPE_ACL_DATA,
	HCI_TYPE_SCO_DATA,
	HCI_TYPE_EVENT,
	HCI_TYPE_TLK_MODULE = 0xff,
	//-------- mesh
	HCI_RSP_USER_START 			= 0x10,
	HCI_RSP_USER 				= HCI_RSP_USER_START,	// line feeds
	HCI_RSP_USER_END 			= 0x2F,
	TSCRIPT_MESH_TX				= 0x30,
	TSCRIPT_PROVISION_SERVICE	= 0x31,
	TSCRIPT_PROXY_SERVICE		= 0x32,
	TSCRIPT_END 				= 0x36,
	HCI_LOG 					= 0x3A,	// ":"
	DONGLE_REPORT_SPP_DATA 		= 0x55,
	DONGLE_REPORT_PROVISION_UUID= 0x56,
	DONGLE_REPORT_PROXY_UUID	= 0x57,
	DONGLE_REPORT_ATT_MTU		= 0x58,
    DONGLE_REPORT_ONLINE_ST_UUID= 0x59,
    DONGLE_REPORT_ONLINE_ST_DATA= 0x5a,
    DONGLE_REPORT_READ_RSP		= 0x5b,
    DONGLE_REPORT_FW_VERSION	= 0x5c,
    MESH_CMD_RSP 				= 0x70,
    MESH_ADV_PAYLOAD 			= 0x71,
    MESH_PROV 					= 0x72,	// provision parameters
	MESH_ADV_BEAR_GATT 			= 0x73,
	MESH_ADV_BLE_ST 			= 0x74,
	MESH_MONITOR_DATA 			= 0x75,
	MESH_ADV_ONE_PKT_COMPLETED 	= 0x76,
	MESH_CONNECTION_STS_REPORT 	= 0x77,
	MESH_TX_CMD_RUN_STATUS		= 0x78,
	MESH_GATT_OTA_STATUS 	    = 0x79,
	// can't use 0x7f,  because of HCI_TYPE_TLK_MODULE

	//-------- mesh cmd receive
	TSCRIPT_MESH_RX 			= 0x80,
	TSCRIPT_MESH_RX_NW 			= 0x90,
	
	TSCRIPT_GATEWAY_DIR_RSP 	= 0x91,
	HCI_GATEWAY_CMD_SAR_MSG		= 0x92, 
	TSCRIPT_CMD_VC_DEBUG 		= 0xfa,
	// can't use 0xff,	because of HCI_TYPE_TLK_MODULE
} hci_type_t;



// hci event
extern u32		hci_eventMask;
extern u32		hci_le_eventMask;
extern u32		hci_tlk_module_eventMask;
ble_sts_t 		blc_hci_setEventMask_cmd(u32 evtMask);      //eventMask: BT/EDR
ble_sts_t 		blc_hci_le_setEventMask_cmd(u32 evtMask);   //eventMask: LE
ble_sts_t 		bls_hci_mod_setEventMask_cmd(u32 evtMask);  //eventMask: module special



// Controller event handler
typedef int (*hci_event_handler_t) (u32 h, u8 *para, int n);
extern hci_event_handler_t		blc_hci_event_handler;
void 	blc_hci_registerControllerEventHandler (hci_event_handler_t  handler);


int 		blc_hci_sendACLData2Host (u16 handle, u8 *p);



int blc_hci_send_data (u32 h, u8 *para, int n);
void blc_enable_hci_master_handler ();




int blc_acl_from_btusb ();

void blc_register_hci_handler (void *prx, void *ptx);
int blc_hci_rx_from_usb (void);
int blc_hci_tx_to_usb (void);
int blc_hci_tx_to_btusb (void);

int blc_hci_handler (u8 *p, int n);
int blm_hci_handler (u8 *p, int n);
int blc_hci_send_event (u32 h, u8 *para, int n);

int blc_hci_proc (void);
void set_blc_hci_flag_fun(unsigned char flag) ;

