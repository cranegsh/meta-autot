/*
 * canfdComm.c
 *
 *  Created on: Dec. 13, 2022
 *  modified on Aug 15, 2023 to be ported to Raspberry Pi
 *      Author: Crane Shao
 */
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#include <PCANBasic.h>

#include "app_canfd.h"
#include "utility.h"
#include "sysconfig.h"
#include "app_config.h"

#define STB_LOW                     //gpio_write("/dev/gpiochip0", 7, 0)
#define STB_HIGH                    //gpio_write("/dev/gpiochip0", 7, 1)

#define CANFD_DATA_LEN_BYTE_MAX     64
#define SID_MASK                    0x7FF           /* all bits care */
#define EID_MASK                    0x3FFFF         /* all bits care */
#define EID_BITS                    18

/* CAN Message IDs for ID4 project */
#ifdef PROJECT_ID4
#define FILTER_NUMBER				FILTER_TOTAL
#endif
/* ID4 messages */
#define BMS_20                      0x0CF			/* Voltage */
//#define BMS_22                      0x12DD54D1		/* State of Charge */
#define LiSi_01                     0x16A954BB		/* SW on dashboard */
#define ESP_21                      0x0FD			/* Speed */
#define KLIMA_03                    0x66E			/* Inside Temp. */
#define KLIMA_16                    0x16A95493		/* FSH Status */
#define KLIMA_S_01                  0x6B0			/* Humidity */
#define TEMP_01                     0x1A5555A6		/* Outside Temp. */
#define SYSTEMINFO_01               0x585			/* Bus Identification */

/* BZ4X/G3 messages */
#define TEMP_AMB_SIG				0x3B0
#define TEMP_AMB_RAW				0x380
#define TEMP_CABIN					0x407
#define SPEED_VEH					0x610
#define HUMIDITY_CABIN				0x480
#define DEFROST_SIG					0x381
/* BZ4X messages */
#ifdef PROJECT_CAN_BZ4X
#define TEMP_AMB					TEMP_AMB_RAW//TEMP_AMB_SIG
#define HV_READY					0x3B6
#define HV_SOC						0x3B6
#endif
/* G3 messages */
#ifdef PROJECT_CAN_G3
#define TEMP_AMB					TEMP_AMB_RAW
#define HV_READY					0x51E
#define HV_SOC						0x356
#endif

/* CAN Message IDs for C3 project */
#ifdef PROJECT_C3
#define FILTER_NUMBER				(FILTER_TOTAL)	/* same as ID4 */
#endif
/* C3 messages */
/* first 8 are the same as ID4 for  now */			/* total 8 messages */
#define CAN_CUR                     0x587U          /* temp for test */
#define CAN_OP_MODE                 0x588U          /* temp for test */

#define CAN_VEHICLE_MSG_NUMBER		3
#define CAN_VEHICLE_MSG_LEN			8

// CAN Message IDs for Farview project
#ifdef PROJECT_NAVY
#define FILTER_NUMBER				3	/* make sure to change this number accordingly */
#endif

/* receive message ID for debugger control */
#define ID_RCV_DATA		       		0x201
#define ID_RCV_DATA_CFG	       		0x211
#define ID_RCV_DATA_CTL	       		0x221
#define ID_RCV_DATA_ENV	       		0x231
#define ID_RCV_DATA_RES	       		0x241
#define ID_RCV_ERROR				0x202
#define ID_RCV_LOG      			0x203

/* transmit message ID for debugger control*/
#define ID_SEND_DATA      			0x301	/* This message combines ID_SEND_CONFIG and IC_SEND_COMMAND
												and use a structure to prepare the data */
#define ID_SEND_COMMAND	    		0x311
#define ID_SEND_CFG_PETD			0x321
#define ID_SEND_CFG_PWM				0x331

//#define DEBUG_DISPLAY_CAN_MSG                       // defined to display CAN frames
#ifdef DEBUG_DISPLAY_CAN_MSG
#define debugPrintf_canfd(...)  debugPrintf(__VA_ARGS__);
#define dbgPrintf_canfd(...)  dPrintf(__VA_ARGS__);
#define dPrintf_canfd(...)  dPrintf(__VA_ARGS__);
#else
#define debugPrintf_canfd(...)  ndebugPrintf(__VA_ARGS__);
#define dbgPrintf_canfd(...)  ndPrintf(__VA_ARGS__);
#define dPrintf_canfd(...)  ndPrintf(__VA_ARGS__);
#endif

/* CANFD data should be interpreted according to the below DBC information:
 * BO_ 207 BMS_20: 8
 * SG_ BMS_Spannung : 52|12@1+ (0.25,0) [0|1000] "Unit_Volt" Vector__XXX
 * BO_ 2463978705 BMS_22:
 * 8 SG_ BMS_Ladezustand : 17|11@1+ (0.05,0) [0|100] "Unit_PerCent" Vector__XXX
 * BO_ 253 ESP_21: 8
 * SG_ ESP_v_Signal : 32|16@1+ (0.01,0) [0|655.32] "Unit_KiloMeterPe" Vector__XXX
 * SG_ ESP_QBit_v_Signal : 55|1@1+ (1,0) [0|1] "" Vector__XXX
 * BO_ 1646 Klima_03: 8
 * SG_ KL_Innen_Temp : 32|8@1+ (0.5,-50) [-50|76] "Unit_DegreCelsi" Vector__XXX
 * BO_ 2527679635 Klima_16: 8
 * SG_ FSH_Status : 28|2@1+ (1,0) [0|3] "" Vector__XXX
 * BO_ 1712 Klima_Sensor_01: 8
 * SG_ FS_Luftfeuchte_rel : 40|8@1+ (0.5,-0.5) [0|100] "Unit_PerCent" Vector__XXX
 * BO_ 2589283750 Temperaturen_01: 8
 * SG_ KBI_QBit_Aussen_Temp_gef : 12|1@1+ (1,0) [0|1] "" Vector__XXX
 * SG_ KBI_Aussen_Temp_gef : 16|8@1+ (0.5,-50) [-50|75] "Unit_DegreCelsi" Vector__XXX
 */
//#define CAN_STANDARD_ONLY

static struct canfdData canfdio = {
#ifdef PROJECT_ID4
   .id4Dataveh = {0, },
   .id4DataIn = { {0, 0, }, },
   .id4DataIn_Cfg = { { 0, { }, { }, }, },
   .id4DataIn_Ctl = { {0, 0, }, },
   .id4DataIn_Env = { {0, 0, }, },
   .id4DataIn_Res = { {0, 0, }, },
   .id4DataLog = { {0, 0, 0, 0, 0, 0}, },
   .id4DataOut = { {0, 0, }, },
   .id4DataOut_command = { {0, 0 }, },
   .id4DataOut_cfgPetd = { {0, 0 }, },
   .id4DataOut_cfgPwm = { {0, 0 }, },
   .id4_vehData = {
	/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
#ifdef PROJECT_CAN_ID4
		{ KLIMA_16, "FSH status", 500, 0 },
		{ TEMP_01, "Amb. Temp.", 1000, 0 },
		{ BMS_20<<CANFD_EID_BITS, "Voltage", 200, 0 },
//		{ BMS_22, "S. of Charge", 1000, 0 },
		{ LiSi_01, "SW on Dashboard", 100, 0 },
		{ ESP_21<<CANFD_EID_BITS, "Veh. Speed", 100, 0 },
		{ KLIMA_03<<CANFD_EID_BITS, "Cab. Temp.", 1000, 0 },
		{ KLIMA_S_01<<CANFD_EID_BITS, "Humidity", 1000, 0 }
#endif	/* PROJECT_CAN_ID4 */
#if defined PROJECT_CAN_BZ4X || defined PROJECT_CAN_G3
		{ DEFROST_SIG<<CANFD_EID_BITS, "Defrost S.", 500, 0 },
		{ TEMP_AMB<<CANFD_EID_BITS, "Amb. Temp. S", 1000, 0 },
		{ HV_READY<<CANFD_EID_BITS, "HV Ready", 500, 0 },
		{ HV_SOC<<CANFD_EID_BITS, "S. of Charge", 1000, 0 },
		{ SPEED_VEH<<CANFD_EID_BITS, "Veh. Speed", 100, 0 },
		{ TEMP_CABIN<<CANFD_EID_BITS, "Cab. Temp.", 1000, 0 },
		{ HUMIDITY_CABIN<<CANFD_EID_BITS, "Humidity", 1000, 0 }
#endif	/* PROJECT_CAN_BZ4X or PROJECT_CAN_G3 */
   }
#endif

#ifdef PROJECT_C3
   .c3canDataInfo = {
		/* The sequence of the members in the array must follow the sequence in msg_mode_t enum !!! */
		{ BMS_22, "S.Charge", 1000, 0 },
		{ KLIMA_16, "FSH Sts", 1000, 0 },
		{ TEMP_01, "Amb.Temp.", 2000, 0 },
		{ BMS_20<<CANFD_EID_BITS, "Voltage", 10, 0 },
		{ ESP_21<<CANFD_EID_BITS, "V.Speed", 10, 0 },
		{ KLIMA_03<<CANFD_EID_BITS, "Cab.Temp.", 2000, 0 },
		{ KLIMA_S_01<<CANFD_EID_BITS, "Humidity", 2000, 0 },
		{ SYSTEMINFO_01<<CANFD_EID_BITS, "Sys. ID", 100, 0 },
		{ CAN_CUR<<CANFD_EID_BITS, "Current", 10, 0 },
		{ CAN_OP_MODE<<CANFD_EID_BITS, "Op.mode", 1000, 0 },
   }
#endif	/* PROJECT_C3 */

#ifdef PROJECT_NAVY
   .navyOut = { {0, 0, 0, 0}, },
   .navyIn = { {0, 0, 0, 0, 0, 0, 0, 0}, },
   .updated = false,
#endif
};

union CANMSG_BMS20 {
    struct {
        uint32_t void_word;
        struct {
            uint32_t void_bits:16;
            uint32_t low:8;
            uint32_t high:8;
        } bms20_vol;
    } bF;
    struct {
        uint32_t void_word;
        struct {
            uint32_t void_bits:20;
            uint32_t low:4;
            uint32_t high:8;
        } bms20_vol;
    } bitsF;
    uint16_t hword[4];
    uint8_t byte[8];
};

union CANMSG_BMS22 {
    struct {
        struct{
            uint32_t void_bits:16;
            uint32_t low:8;
            uint32_t high:8;
        } bms22_soc;
        uint32_t void_word;
    } bF;
    struct {
        struct{
            uint32_t void_bits:17;
            uint32_t low:7;
            uint32_t high:4;
            uint32_t unimplmented;
        } bms22_soc;
        uint32_t void_word;
    } bitsF;
    uint16_t hword[4];
    uint8_t byte[8];
};

union CANMSG_ESP21 {
    struct {
        uint32_t void_word;
        struct {
            uint32_t low: 8;
            uint32_t high: 8;
            uint32_t unplemented1: 7;
            uint32_t Qbit: 1;
            uint32_t unplemented2: 8;
        } esp21_speed;
    } bitF;
    uint16_t hword[4];
    uint8_t byte[8];
};
/*
static struct canfdnode canNode = {
    .spi_device = SPICAN_DEVICE_NODE, // /dev/spidev0.0", -> not used for now!!!
    // Filter: SID, SID mask, SID11, SID11 mask, EID, EID mask, IDE, IDE check, FIFO ch, Filter # /
    .vwmsgFilter = {
#ifdef PROJECT_ID4
    { BMS_20,                 SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER0 },
//    { (BMS_22 >> EID_BITS),   SID_MASK, 0, 0, (BMS_22 & EID_MASK), EID_MASK,   true,  true, CAN_RX_FIFO, CAN_FILTER1 },
    { (LiSi_01 >> EID_BITS),   SID_MASK, 0, 0, (LiSi_01 & EID_MASK), EID_MASK,   true,  true, CAN_RX_FIFO, CAN_FILTER1 },
    { ESP_21,                 SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER2 },
    { KLIMA_03,               SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER3 },
    { (KLIMA_16 >> EID_BITS), SID_MASK, 0, 0, (KLIMA_16 & EID_MASK), EID_MASK, true,  true, CAN_RX_FIFO, CAN_FILTER4 },
    { KLIMA_S_01,             SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER5 },
    { (TEMP_01 >> EID_BITS),  SID_MASK, 0, 0, (TEMP_01 & EID_MASK), EID_MASK,  true,  true, CAN_RX_FIFO, CAN_FILTER6 },
    { SYSTEMINFO_01,          SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER7 },
#ifdef CAN_STANDARD_ONLY
    { 0, 0, 0, 0, 0, EID_MASK, false, true,  CAN_RX_FIFO, CAN_FILTER8 },  // receive only standard frames, no extended frames /
#else
    { 0, 0, 0, 0, 0, 0,        true,  true,  CAN_RX_FIFO, CAN_FILTER8 },  // receive only extended frames, no standard frames /
#endif
    { 0, 0, 0, 0, 0, 0,        false, false, CAN_RX_FIFO, CAN_FILTER9 },  // receive all frames /
#endif
	{ ID_RCV_DATA,            SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER10 },
	{ ID_RCV_LOG,             SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER11 },
	{ ID_RCV_ERROR,           SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER12 },
	{ ID_RCV_DATA_CFG,        SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER13 },
	{ ID_RCV_DATA_CTL,        SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER14 },
	{ ID_RCV_DATA_ENV,        SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER15 },
	{ ID_RCV_DATA_RES,        SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER16 },
#ifdef PROJECT_C3
	{ CAN_CUR,        		  SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER17 },
	{ CAN_OP_MODE,        	  SID_MASK, 0, 0, 0, 0,                            false, true, CAN_RX_FIFO, CAN_FILTER18 },
#endif
    }
};
*/
uint32_t canfd_DlcToDataBytes(CAN_DLC dlc)
{
    uint32_t dataBytesInObject = 0;

    if (dlc < CAN_DLC_12) {
        dataBytesInObject = dlc;
    } else {
        switch (dlc) {
            case CAN_DLC_12:
                dataBytesInObject = 12;
                break;
            case CAN_DLC_16:
                dataBytesInObject = 16;
                break;
            case CAN_DLC_20:
                dataBytesInObject = 20;
                break;
            case CAN_DLC_24:
                dataBytesInObject = 24;
                break;
            case CAN_DLC_32:
                dataBytesInObject = 32;
                break;
            case CAN_DLC_48:
                dataBytesInObject = 48;
                break;
            case CAN_DLC_64:
                dataBytesInObject = 64;
                break;
            default:
                break;
        }
    }

    return dataBytesInObject;
}

CAN_DLC canfd_DataBytesToDlc(uint8_t n)
{
	CAN_DLC dlc = CAN_DLC_0;

    if (n <= 4) {
        dlc = CAN_DLC_4;
    } else if (n <= 8) {
        dlc = CAN_DLC_8;
    } else if (n <= 12) {
        dlc = CAN_DLC_12;
    } else if (n <= 16) {
        dlc = CAN_DLC_16;
    } else if (n <= 20) {
        dlc = CAN_DLC_20;
    } else if (n <= 24) {
        dlc = CAN_DLC_24;
    } else if (n <= 32) {
        dlc = CAN_DLC_32;
    } else if (n <= 48) {
        dlc = CAN_DLC_48;
    } else if (n <= 64) {
        dlc = CAN_DLC_64;
    }

    return dlc;
}

static int16_t canfd_messageReceive(uint32_t *mid, uint8_t *data, uint16_t *num)
{
    int16_t status = -100;
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    TPCANMsg Message;
    TPCANTimestamp ts;
    TPCANTimestamp ts_prev;
    TPCANTimestamp ts_diff;
#else
    TPCANMsgFD Message;
    TPCANTimestampFD ts;
    TPCANTimestampFD ts_prev;
    TPCANTimestampFD ts_diff;
#endif
    TPCANStatus Status;

#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    Status = CAN_Read(PCAN_DEVICE, &Message, &ts);
#else
    Status = CAN_ReadFD(PCAN_DEVICE, &Message, &ts);
#endif

    if(PCAN_ERROR_OK == Status)
    {   // Copy the message
    	status = 0;
		*mid = (int)Message.ID;
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
		*num = (uint16_t)(Message.LEN);
#else
		*num = (uint16_t)(canfd_DlcToDataBytes((CAN_DLC)Message.DLC));
#endif
		int16_t i;
		for(i=0; i<*num; i++)
			data[i] = Message.DATA[i];
    }

    return status;
}

/* Function to send a message by calling another function to request send after loading message */
static int16_t canfd_messageSend(uint32_t mid, uint8_t *data, uint16_t num)
{
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    TPCANMsg Message;
#else
    TPCANMsgFD Message;
#endif
    TPCANStatus Status;

    // Initialize ID and control
    if(0 == (mid & CANFD_IDMASK_EID)) {
    	Message.ID = (mid & CANFD_IDMASK_SID) >> CANFD_EID_BITS;   			// set Standard ID (first 11 bits)
    	Message.MSGTYPE = PCAN_MESSAGE_STANDARD;          				// set Standard type
    }
    else {
        Message.ID = mid;   								// set Extended ID (total 29 bits)
        Message.MSGTYPE = PCAN_MESSAGE_EXTENDED;					// set Extended type
    }
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
    Message.LEN = (BYTE)(canfd_DataBytesToDlc((uint8_t)num));
    ndPrintf("\r\nmid %8X, DLC %02d", Message.ID, Message.LEN);
#else
    Message.DLC = (BYTE)(canfd_DataBytesToDlc((uint8_t)num));
    Message.MSGTYPE |= PCAN_MESSAGE_FD;
    Message.MSGTYPE |= PCAN_MESSAGE_BRS;
    ndPrintf("\r\nmid %8X, DLC %02d", Message.ID, Message.DLC);
#endif

    // Initialize transmit data
    uint8_t i;
    for(i=0; i<num; i++) {
        Message.DATA[i] = data[i];
    }

	// transmit message
	uint16_t count = 0;
	do {
#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
		Status = CAN_Write(PCAN_DEVICE, &Message);
#else
		Status = CAN_WriteFD(PCAN_DEVICE, &Message);
#endif
		if(PCAN_ERROR_OK != Status) {
			usleep(100);                  					// Check every 100us
			count++;
		}
	}
	while((PCAN_ERROR_OK != Status) && (200 > count));       			// Checking if message is sent for 20ms

	/* check if the data are submitted successfully */
	if (PCAN_ERROR_OK == Status) {
		ndPrintf("\t[Sent] %X | 0x%02X", Message.ID, Message.DATA[0]);
	}
	else {
		ndPrintf("\t[Failed to send] %X | 0x%02X", Message.ID, Status);
	}

    return (int16_t)Status;
}

struct canfdData *msg_canfd_getData(void)   { return &canfdio; }

#ifdef PROJECT_NAVY
/* Function to prepare data for CAN submission - Navy project */
static int msg_canfd_prepare_navy(int option, uint32_t *mid, uint8_t *data, uint16_t *num)
{
	uint32_t temp = 0;
	int ret = 0, i;

	switch(option)
	{
		case 'c': //ID_SEND_CONFIG:
			temp = ID_SEND_CONFIG;
			*mid = temp << EID_BITS;
			*num = 8;
			//for(i=0; i<*num; i++)	data[i] = (uint8_t)canfdOutput.config + i;
		    data[0] = ( powerConfig_get()->thRes ) & 0xFF;
		    data[1] = ( powerConfig_get()->thRes ) >> 8;
		    data[2] = ( powerConfig_get()->thVoltagePeak ) & 0xFF;
		    data[3] = ( powerConfig_get()->thVoltagePeak ) >> 8;
		    data[4] = ( powerConfig_get()->thCurrentPeak ) & 0xFF;
		    data[5] = ( powerConfig_get()->thCurrentPeak ) >> 8;
			ndebugPrintf("0x%X - %d", temp, data[0]);
			break;
		case 'f': //ID_SEND_COMMAND:
			temp = ID_SEND_COMMAND;
			*mid = temp << EID_BITS;
			*num = 1;
			if(3 >= dData.number) {
				data[0] = dData.number;
			}
			ndebugPrintf("0x%X - %d", temp, data[0]);
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}


/* Function to interpret the CANFD data for Farview project */
static void msg_canfd_interpret_navy(uint32_t mid, uint8_t *data, uint16_t num)
{
    uint8_t i, temp;
    float value;

    dbgPrintf_canfd("\n");
	switch(mid >> EID_BITS)
	{
		case ID_RCV_DATA:
			dbgPrintf_canfd("Params: %d | ", num);
			for(i=0; i<num; i++)
			{
				dbgPrintf_canfd(" %02X", *(data+i));
				if((0 == (i+1)%16)) dbgPrintf_canfd("\n\t\t");
			}
			if(CAN_DATA_IN_LEN <= num) {
				for(i=0; i<CAN_DATA_IN_LEN; i++)
				{
					canfdio.navyIn.byte[i] = *(data+i);
				}
			}
			ndPrintf(" | %d\n", canfdio.navyIn.data.resWindshield);
			break;
		case ID_RCV_LOG:
			debugPrintf_canfd("Log:\t");
			for(i=0; i<num; i++)
			{
				dPrintf_canfd(" %02X", *(data+i));
				if((0 == (i+1)%16)) dPrintf_canfd("\n\t\t");
			}
			if(CAN_DATA_LOG_LEN <= num) {
				for(i=0; i<CAN_DATA_LOG_LEN; i++)
				{
					canfdio.navyLog.byte[i] = *(data+i);
				}
			}
			break;
		default:
			dbgPrintf_canfd("Invalid data! - ID%X\t", mid);
			for(i=0; i<num; i++)
			{
				dbgPrintf_canfd(" %02X", *(data+i));
				if((0 == (i+1)%16)) dbgPrintf_canfd("\n\t\t");
			}
			break;
	}
}
#endif

#ifdef PROJECT_ID4

void msg_canfd_print(void)
{
    iPrintf("\n");
    for(int i=0; i<CAN_DATA_IN_LEN; i++) {
    	iPrintf("%d ", canfdio.id4DataIn.byte[i]);
    }
}

#ifdef PROJECT_CAN_ID4
/* Function to prepare CANFD data for ID4 vehicle messages */
static int msg_canfd_prepare_id4Veh(uint8_t msgno, int16_t value, uint8_t *data)
{
	switch(msgno) {
		case APP_OPT_DEV_SEND_FSH:
			if(0 != value) {
				data[3] = 0x10;
			}
			break;
		case APP_OPT_DEV_SEND_ATEMP:
			data[2] = (value + 50 ) * 2;
			break;
		case APP_OPT_DEV_SEND_VOLTAGE:
			value = value * 4;
			data[7] = (value >> 4) & 0xFF;			/* take the higher 8 bits of total 12 bits and right shift 4 bits */
			data[6] = (value & 0x0F) << 4;			/* take the lower 4 bits and left shift 4 bits */
			break;
		case APP_OPT_DEV_SEND_SOC:
#if 0	// change SOC to LiSi_01
			value = value * 20;
			data[3] = (value >> 7 ) & 0x0F;			/* take the higher 4 bits of total 11 bits and right shift 7 bits */
			data[2] = (value & 0x7F) << 1;			/* take the lower 7 bits and left shift one bit */
#else
			if(0 != value) {
				data[6] = 0x40;
			}
#endif
			break;
		case APP_OPT_DEV_SEND_SPEED:
			value = value * 100;
			data[5] = (value >> 8 ) & 0xFF;			/* take the higher 8 bits of total 16 bits and right shift 8 bits */
			data[4] = value & 8;					/* take the lower 8 bits */
			break;
		case APP_OPT_DEV_SEND_CTEMP:
			data[4] = (value + 50 ) * 2;
			// When setting Cabin temperature as 1, FSH_Auto is set at the same time!
			if(1 == value) {
				data[6] = 0x08;
			}
			break;
		case APP_OPT_DEV_SEND_HUMIDITY:
//			data[5] = value * 2 + 1;
			value = value * 10 + 396;
			data[2] = value & 0xFF;
			data[3] = (value >> 8) & 0x03;
			break;
		default:
			return -1;
	}
	return 0;
}
#endif
#if defined PROJECT_CAN_G3 || defined PROJECT_CAN_BZ4X
/* Function to prepare CANFD data for G3 and BZ4X vehicle messages */
#ifdef PROJECT_CAN_G3
static int msg_canfd_prepare_g3Veh(uint8_t msgno, int16_t value, uint8_t *data)
#else
static int msg_canfd_prepare_bz4xVeh(uint8_t msgno, int16_t value, uint8_t *data)
#endif
{
	float temp;
	switch(msgno) {
		case APP_OPT_DEV_SEND_FSH:
			if(0 != value) {
				data[1] |= 0x20;
			}
			else {
				data[1] &= (~0x20);
			}
			break;
		case APP_OPT_DEV_SEND_ATEMP:
			temp = (float)value / 160 * 256;
			value = (int16_t)temp;
			if(0 <= value) {
				data[6] = value;
			}
			else {
				data[6] = 0x100 + value;				/* two's complement */
			}
			break;
		case APP_OPT_DEV_SEND_VOLTAGE:
			if(0 != value) {
				data[0] |= 0x80;
			}
			else {
				data[0] &= (~0x80);
			}
			break;
		case APP_OPT_DEV_SEND_SOC:
			//not confirmed yet
			break;
		case APP_OPT_DEV_SEND_SPEED:
			data[2] = value;
			break;
		case APP_OPT_DEV_SEND_CTEMP:
			data[2] = value * 4 + 26;				/* (value + 6.5 ) / 0.25 */
			break;
		case APP_OPT_DEV_SEND_HUMIDITY:
			// not confirmed yet
			break;
		default:
			return -1;
	}
	return 0;
}
#endif

/* Function to prepare CANFD data for ID4 project */
static int msg_canfd_prepare_id4(int option, uint32_t *mid, uint8_t *data, uint16_t *num)
{
	int ret = 0;
	int i;

	switch(option) {
		case COMMAND_C:
		    *mid = ID_SEND_CFG_PETD << CANFD_EID_BITS;
		    *num = CAN_DATA_OUT_CFG_PETD_LEN;
		    ndPrintf("\n len %d | ", *num);
		    for(i=0; i<*num; i++)
		    {
		    	data[i] = canfdio.id4DataOut_cfgPetd.byte[i];
		    	ndPrintf("%02X ", data[i]);
		    }
		    ndPrintf("\n Send petd config!");
			break;
		case COMMAND_F:
		    *mid = ID_SEND_CFG_PWM << CANFD_EID_BITS;
		    *num = CAN_DATA_OUT_CFG_PWM_LEN;
		    ndPrintf("\n len %d | ", *num);
		    for(i=0; i<*num; i++)
		    {
		    	data[i] = canfdio.id4DataOut_cfgPwm.byte[i];
		    	ndPrintf("%02X ", data[i]);
		    }
		    ndPrintf("\n Send pwm config!");
			break;
		case COMMAND_P:
		case COMMAND_D:
		    *mid = ID_SEND_COMMAND << CANFD_EID_BITS;
		    *num = CAN_DATA_OUT_COMMAND_LEN;
		    ndPrintf("\n len %d | ", *num);
		    for(i=0; i<*num; i++)
		    {
		    	data[i] = canfdio.id4DataOut_command.byte[i];
		    	ndPrintf("%02X ", data[i]);
		    }
		    ndPrintf("\n Send command!");
			break;
		case COMMAND_S:
			*num = CAN_VEHICLE_MSG_LEN;
	        if(1 == dData.display) {
			    *mid = KLIMA_16;
	        }
	        else if(2 == dData.display) {
			    *mid = KLIMA_16;
			    data[3] = 0x10;				/* FSH = ON */
	        }
	        else if(3 == dData.display) {
			    *mid = TEMP_01;
			    data[2] = (dData.number + 50 ) * 2;		/* convert temperature to CAN data */
	        }
		    ndPrintf("\n Send CAN message %d with %d!", dData.display, dData.number);
			break;
		default:
			ndPrintf("\n Send nothing!");
			ret = -1;
			break;
	}

	return ret;
}

static void msg_canfd_copyData(int8_t number, uint8_t length, uint8_t *source, uint8_t *dest)
{
	uint8_t i;
	for(i=0; i<number; i++)
	{
		dbgPrintf_canfd(" %02X", *(source+i));
		ndPrintf(" %02X", *(source+i));
		if((0 == (i+1)%16)) dbgPrintf_canfd("\n\t\t");
	}
	ndPrintf("\nReceived %d for %d", number, length);
	if(length <= number)
	{
		ndPrintf("\n Copying ...")
		for(i=0; i<length; i++)
		{
			dest[i] = *(source+i);
			ndPrintf(" %02X", dest[i]);
		}
	}
}

/* Function to interpret the CANFD data for ID4 project */
/* This is to interpret the messages from the tester */
static void msg_canfd_interpret_id4(uint32_t mid, uint8_t *data, uint16_t num)
{
    uint8_t i, temp;
    float value;

    dbgPrintf_canfd("\n");
	switch(mid >> EID_BITS)
	{
		case ID_RCV_DATA:
			dbgPrintf_canfd("Params: %d | ", num);
			msg_canfd_copyData(num, CAN_DATA_IN_LEN, data, (uint8_t *)&canfdio.id4DataIn.byte);
			break;
		case ID_RCV_DATA_CFG:
			debugPrintf_canfd("Data cfg:\t");
			//debugPrintf("Data cfg:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_CFG_LEN, data, (uint8_t *)&canfdio.id4DataIn_Cfg.byte);
			//dPrintf("\n");  for(i=0; i<CAN_DATA_IN_CFG_LEN; i++) dPrintf("%02X ", canfdio.id4DataIn_Cfg.byte[i]);
			break;
		case ID_RCV_DATA_CTL:
			debugPrintf_canfd("Data ctl:\t");
			//debugPrintf("Data ctl:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_CTL_LEN, data, (uint8_t *)&canfdio.id4DataIn_Ctl.byte);
			//dPrintf("\n");  for(i=0; i<CAN_DATA_IN_CFG_LEN; i++) dPrintf("%02X ", canfdio.id4DataIn_Ctl.byte[i]);
			break;
		case ID_RCV_DATA_ENV:
			debugPrintf_canfd("Data env:\t");
			//debugPrintf("Data env:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_ENV_LEN, data, (uint8_t *)&canfdio.id4DataIn_Env.byte);
			break;
		case ID_RCV_DATA_RES:
			debugPrintf_canfd("Data res:\t");
			//debugPrintf("Data res:\t"); for(i=0; i<num; i++) dPrintf("%02X ", data[i]);
			msg_canfd_copyData(num, CAN_DATA_IN_RES_LEN, data, (uint8_t *)&canfdio.id4DataIn_Res.byte);
			break;
		case ID_RCV_LOG:
			debugPrintf_canfd("Log:\t");
			msg_canfd_copyData(num, CAN_DATA_LOG_LEN, data, (uint8_t *)&canfdio.id4DataLog.byte);
			break;
		default:
			dbgPrintf_canfd("Invalid data! - ID%X\t", mid);
			for(i=0; i<num; i++)
			{
				dbgPrintf_canfd(" %02X", *(data+i));
				if((0 == (i+1)%16)) dbgPrintf_canfd("\n\t\t");
			}
			break;
	}
}

/* Function to interpret the CANFD data for ID4 project */
#if 0	/* This is to interpre the messages from the vehicle */
static bool msg_canfd_interpret_id4(uint32_t mid, uint8_t *data, uint16_t num)
{
    bool status = true;
    uint16_t i, temp;
    float value;
    union CANMSG_BMS20 canmsgBMS20;
    union CANMSG_BMS22 canmsgBMS22;
    union CANMSG_ESP21 canmsgESP21;

	dbgPrintf_canfd("\n");
    if(mid & EID_MASK)
    {   // extended frame
        switch(mid)
        {
            case BMS_22:
                dbgPrintf_canfd("BMS_22:\t");
                for(i=0; i<num/2; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS22.hword[i] = *data++;
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS22.hword[i] += (*data++) << 8;
                }
                temp = canmsgBMS22.bF.bms22_soc.high & 0x0F;
                temp <<= 7;
                temp += (canmsgBMS22.bF.bms22_soc.low >> 1);
                value = (float)temp/20;
                dPrintf_canfd("\tState of Charge: %4.2f %%", value);
                canfdio.stateCharge = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("BMS_22:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tState of Charge: %4.2f %%", value);
                }
                break;
            case KLIMA_16:
                dbgPrintf_canfd("KLIMA_16:\t");
                temp = data[3];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                temp = (temp >> 4) & 0x3;
                dPrintf_canfd("\tFSH status: %d", temp);
                canfdio.fsh = temp;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("KLIMA_16:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tFSH status: %u", temp);
                }
                break;
            case TEMP_01:
                dbgPrintf_canfd("TEMP_01:\t");
                temp = data[1];
                value = data[2];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                value = value/2 - 50;
                dPrintf_canfd("\tOutside temperature: %5.2f �C", value);
                temp = (temp>>4) & 0x01;
                dPrintf_canfd(" | Qbit: %d", temp);
                canfdio.tempOutside = value;
                canfdio.tempOutsideQbit = (0 == temp) ? false : true;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("TEMP_01:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tOutside temperature: %3.2f �C", value);
                }
                break;
            default:
                dbgPrintf_canfd("Invalid data!\t");
                dbgPrintf_canfd("E:%X -", mid);
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                break;
        }
    }
    else
    {   // standard frame
        switch(mid >> EID_BITS)
        {
            case BMS_20:
                dbgPrintf_canfd("BMS_20:\t");
                for(i=0; i<num/2; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS20.hword[i] = *data++;
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgBMS20.hword[i] += (*data++) << 8;
                }
                temp = canmsgBMS20.bF.bms20_vol.high;
                temp <<= 4;
                temp += (canmsgBMS20.bF.bms20_vol.low >> 4);
                value = (float)temp/4;
                dPrintf_canfd("\tBMS_Spannung: %4.2f V", value);
                canfdio.voltage = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("BMS_20:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tHigh Voltage: %3.2f V", value);
                }
                break;
            case ESP_21:
                dbgPrintf_canfd("ESP_21:\t");
                for(i=0; i<num/2; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgESP21.hword[i] = *data++;
                    dbgPrintf_canfd(" %02X", *data);
                    canmsgESP21.hword[i] += (*data++) << 8;
                }
                temp = canmsgESP21.bitF.esp21_speed.high;
                value = (temp << 8) + canmsgESP21.bitF.esp21_speed.low;
                value = value / 100;
                dPrintf_canfd("\tSpeed: %4.2f KM/h", value);
                temp = canmsgESP21.bitF.esp21_speed.Qbit;
                dPrintf_canfd(" | Qbit: %d", temp);
                canfdio.speed = value;
                canfdio.speedQbit = (0 == temp) ? false : true;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("ESP_21:\n");
                    print_array_byte(data - num, num);
                    iPrintf("\tVehicle Speed: %3.2f KM/h", value);
                }
                break;
            case KLIMA_03:
                dbgPrintf_canfd("KLIMA_03:\t");
                value = data[4];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                value = value/2 - 50;
                dPrintf_canfd("\tInside temperature: %4.2f �C", value);
                canfdio.tempInside = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("KLIMA_03:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tInside temperature: %3.2f �C", value);
                }
                break;
            case KLIMA_S_01:
                dbgPrintf_canfd("KLIMA_S_01:\t");
                value = data[5];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                value = value /2 - 0.5;
                dPrintf_canfd("\tHumidity: %4.2f %%", value);
                canfdio.humidity = value;
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("KLIMA_S_01:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tHumidity: %3.2f %%", value);
                }
                break;
            case SYSTEMINFO_01:
                dbgPrintf_canfd("SYSTEMINFO_01:\t");
                temp = data[4];
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                dPrintf_canfd("\tSI_Bus_Identifikation: %d", temp&0xFF);
                if(MAIN_LOOP_DISPLAY_CAN == dData.display)
                {
                    iPrintf("SYSTEMINFO_01:\t");
                    print_array_byte(data - num, num);
                    iPrintf("\tCAN bus identification: 0x%X", temp&0xFF);
                }

                break;
            default:
                dbgPrintf_canfd("Invalid data!\t");
                dbgPrintf_canfd("S:%X -", (int)(mid >> EID_BITS));
                for(i=0; i<num; i++)
                {
                    dbgPrintf_canfd(" %02X", *data);
                    data++;
                }
                break;
        }
    }

    return status;
}
//#endif

/* Function to receive CANFD message
 * FSH_Status is at byte #3 (0 - 7) in Klima16
 * @retVal  1:  FSH status
 *          0:  FSH status
 *          -1: not Klima16 message
 */
int16_t msg_canfd_receive_isr(void)
{
    uint16_t msgData;
    int16_t status;

    status = canfd_messageReceive_Klima16(&msgData);

    if(0 == status)
    {   /* message received */
        status = (msgData >> 4) & 0x3;
        if(status)
        {   /* FSH_status is 1 */
            canfdio.id4Dataveh.fsh = true;
        }
        else
        {   /* FSH_Status is 0 */
            canfdio.id4Dataveh.fsh = false;
        }
    }

    return status;
}
#endif	/* PROJECT_CAN_ID4 */

#endif	/* PROJECT_ID4 */

#ifdef PROJECT_C3
/* Function to prepare CANFD data for C3 vehicle messages */
static int msg_canfd_prepare_c3Veh(uint8_t msgno, int16_t value, uint8_t *data)
{
	switch(msgno) {
		case APP_OPT_DEV_SEND_FSH:
			if(0 != value) {
				data[3] = 0x10;
			}
			break;
		case APP_OPT_DEV_SEND_ATEMP:
			data[2] = (value + 50 ) * 2;
			break;
		case APP_OPT_DEV_SEND_VOLTAGE:
			value = value * 4;
			data[7] = (value >> 4) & 0xFF;			/* take the higher 8 bits of total 12 bits and right shift 4 bits */
			data[6] = (value & 0x0F) << 4;			/* take the lower 4 bits and left shift 4 bits */
			break;
		case APP_OPT_DEV_SEND_SOC:
			value = value * 20;
			data[3] = (value >> 7 ) & 0x0F;			/* take the higher 4 bits of total 11 bits and right shift 7 bits */
			data[2] = (value & 0x7F) << 1;			/* take the lower 7 bits and left shift one bit */
			break;
		case APP_OPT_DEV_SEND_SPEED:
			value = value * 100;
			data[5] = (value >> 8 ) & 0xFF;			/* take the higher 8 bits of total 16 bits and right shift 8 bits */
			data[4] = value & 8;					/* take the lower 8 bits */
			break;
		case APP_OPT_DEV_SEND_CTEMP:
			data[4] = (value + 50 ) * 2;
			break;
		case APP_OPT_DEV_SEND_HUMIDITY:
			data[5] = value * 2 + 1;
			break;
		case APP_OPT_DEV_SEND_SYSID:
			data[4] = (uint8_t)value;
			break;
		case APP_OPT_DEV_SEND_CURRENT:
			data[7] = (uint8_t)value;
			break;
		case APP_OPT_DEV_SEND_OPMODE:
			data[0] = (uint8_t)value;
			break;
		default:
			return -1;
	}
	return 0;
}
#endif	/* PROJECT_C3 */

/* Function to send data as from vehicle for test */
void msg_canfd_send_veh(uint8_t msgno, uint16_t value)
{
    uint32_t messageID = 0;
    uint8_t messageData[CAN_VEH_MSG_NUM];
    uint16_t dataNumber = CAN_VEH_MSG_NUM;
    int16_t i, status = 0;

    /* clear the data buffer */
    for(i=0; i<dataNumber; i++) { messageData[i] = 0;	}

    /* prepare the data frame */
#ifdef PROJECT_CAN_ID4
    status = msg_canfd_prepare_id4Veh(msgno, value, messageData);
#endif
#ifdef PROJECT_CAN_G3
    status = msg_canfd_prepare_g3Veh(msgno, value, messageData);
#endif
#ifdef PROJECT_CAN_BZ4X
    status = msg_canfd_prepare_bz4xVeh(msgno, value, messageData);
#endif
#ifdef PROJECT_C3
    status = msg_canfd_prepare_c3Veh(msgno, value, messageData);
#endif
    if(0 != status)
    {
    	ndPrintf("\nData not ready!");
    	return;
    }

#ifdef PROJECT_ID4
    messageID = canfdio.id4_vehData[msgno].mid;
#endif
#ifdef PROJECT_C3
    messageID = canfdio.c3canDataInfo[msgno].mid;
#endif
    status = canfd_messageSend(messageID, messageData, dataNumber);
    ndPrintf("\n Status %d, Sent message: 0x%X, %d | ", status, messageID >> EID_BITS, dataNumber);

    if(0 != status)
    {	/* submission failed. Resend the data! */
    	// TODO: Need to find out why sometimes there are messages with ID of 0x1FFFFFFF and all data are 0xFF
    	ndPrintf("\nSubmission failed: ID-0x%X | %d\t", messageID >> EID_BITS, dataNumber);
    	ndPrintf("F.");
    }
    else
    {	/* submission done. Display the message! */
    	ndPrintf(" / Sent message: 0x%X, %d | ", messageID >> EID_BITS, dataNumber);
		for(int i=0; i<dataNumber; i++)
		{
			ndPrintf(" %02X", messageData[i]);
			if((0 == i%16) && (0 < i)) ndPrintf("\n\t");
		}
		ndPrintf("\n");
    }
}

/* Function to send data - Farview project */
void msg_canfd_send_tester(int cmd)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint16_t dataNumber = 0;
    int16_t i, status = 0;

    /* clear the data buffer */
    for(i=0; i<MAX_DATA_BYTES; i++) { messageData[i] = 0;	}

    /* prepare the data frame */
#ifdef PROJECT_ID4
    status = msg_canfd_prepare_id4(cmd, &messageID, messageData, &dataNumber);
#endif
#ifdef PROJECT_NAVY
    //status = msg_canfd_prepare_navy(cmd, &messageID, messageData, &dataNumber);
    messageID = ID_SEND_DATA;
    dataNumber = CAN_DATA_OUT_LEN;
    ndPrintf("\n len %d | ", dataNumber);
    for(i=0; i<dataNumber; i++)
    {
    	messageData[i] = canfdio.navyOut.byte[i];
    	ndPrintf("%02X ", messageData[i]);
    }
#endif
    if(0 != status)
    {
    	ndPrintf("\nData not ready!");
    	return;
    }

    status = canfd_messageSend(messageID, messageData, dataNumber);
    ndPrintf("\n Status %d, Sent message: 0x%X, %d | ", status, messageID >> EID_BITS, dataNumber);

    if(0 != status)
    {	/* submission failed. Resend the data! */
    	// TODO: Need to find out why sometimes there are messages with ID of 0x1FFFFFFF and all data are 0xFF
    	ndPrintf("\nSubmission failed: ID-0x%X | %d\t", messageID >> EID_BITS, dataNumber);
    	ndPrintf("F.");
    }
    else
    {	/* submission done. Display the message! */
    	ndPrintf(" / Sent message: 0x%X, %d | ", messageID >> EID_BITS, dataNumber);
		for(int i=0; i<dataNumber; i++)
		{
			ndPrintf(" %02X", messageData[i]);
			if((0 == i%16) && (0 < i)) ndPrintf("\n\t");
		}
		ndPrintf("\n");
    }
}

#ifdef PROJECT_ID4
int16_t msg_canfd_rcvCanConfigs(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint16_t dataNumber;
	int16_t status;

	do {
		status = canfd_messageReceive(&messageID, messageData, &dataNumber);
	} while ((0 != status) || ( ID_RCV_DATA_CFG != (messageID >> EID_BITS)));

	ndebugPrintf("Received 0x%X | %d\t", messageID, dataNumber);
	msg_canfd_copyData(dataNumber, CAN_DATA_IN_CFG_LEN, messageData, (uint8_t *)&canfdio.id4DataIn_Cfg.byte);

	return status;
}

int16_t msg_canfd_rcvCanLog(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint16_t dataNumber;
	int16_t status;

	do {
		status = canfd_messageReceive(&messageID, messageData, &dataNumber);
	} while ((0 != status) || ( ID_RCV_LOG != (messageID >> EID_BITS)));

	ndebugPrintf("Received 0x%X | %d\t", messageID, dataNumber);
	msg_canfd_copyData(dataNumber, CAN_DATA_LOG_LEN, messageData, (uint8_t *)&canfdio.id4DataLog.byte);

	return status;
}
#endif	/* #ifdef PROJECT_ID4 */

void msg_canfd_receive(void)
{
    uint32_t messageID = 0;
    uint8_t messageData[MAX_DATA_BYTES];
    uint16_t dataNumber;
    int16_t status;
    static uint32_t timer_sec = 0;
    time_t now;
    struct tm *systime;
    int i;

    time( &now );
    systime = localtime( &now );

    status = canfd_messageReceive(&messageID, messageData, &dataNumber);
    /* TODO: investigate Why there is garbage messages with message ID of 0? */

    if((0 == status) && ( 0 != messageID))
    {
		ndebugPrintf("Received 0x%X | %d\t", messageID, dataNumber);
		//print_array_byte(messageData, dataNumber); dPrintf("\n");

		/* check data: wrong data if the number of data is odd or larger than max CANFD data length */
		if((0 == (dataNumber % 2 )) && (CANFD_DATA_LEN_BYTE_MAX >= dataNumber))
		{	 /* valid, so process the message */
#ifdef PROJECT_ID4
			 msg_canfd_interpret_id4(messageID, messageData, dataNumber);
#endif
#ifdef PROJECT_NAVY
			 msg_canfd_interpret_navy(messageID, messageData, dataNumber);
#endif
		}

		timer_sec = systime->tm_sec;
    }
    else if (1 < systime->tm_sec - timer_sec) {
    	/* didn't receive any message after 1 second, set all data 0 */
		for(i=0; i<CAN_DATA_IN_LEN; i++)
		{
#ifdef PROJECT_ID4
			canfdio.id4DataIn.byte[i] = 0;
#endif
#ifdef PROJECT_C3
			canfdio.c3dataIn.byte[i] = 0;
#endif
#ifdef PROJECT_NAVY
			canfdio.navyIn.byte[i] = 0;
#endif
		}
		timer_sec = systime->tm_sec;
    }

    return;
}

bool msg_canfd_init(void)
{
	 bool retVal = false;
     TPCANStatus Status;

#if (CAN_BUS_TYPE_CAN == CAN_BUS_TYPE)
     Status = CAN_Initialize(PCAN_DEVICE, PCAN_BAUD_500K, 0, 0, 0);
     printf("CAN_Initialize(%xh): Status=0x%x\n", PCAN_DEVICE, (int)Status);
#else
     Status = CAN_InitializeFD(PCAN_DEVICE, CANFD_BIT_RATE);
     printf("CANFD_Initialize(%xh): Status=0x%x\n", PCAN_DEVICE, (int)Status);
#endif

	 if(PCAN_ERROR_OK == Status)
	 {
		 //canfd_configCheck();
#ifdef DEVELOP_VERSION
		 infoPrintf("CAN bus is initialized successfully!\r\n");
#endif
		 retVal = true;
	 }
	 else
	 {
		 infoPrintf("CAN bus initialization failed!\n");
	 }

	 return retVal;
}
