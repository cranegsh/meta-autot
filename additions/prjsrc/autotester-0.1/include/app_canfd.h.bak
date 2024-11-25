/*
 * canfdComm.h
 *
 *  Created on: Dec. 13, 2022
 *  Modified on Aug 15, 2023 to be ported to Raspberry Pi
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CANFDCOMM_H_
#define APPLICATION_CANFDCOMM_H_

#include <time.h>

#include "sysconfig.h"
#include "app_config.h"

#define PCAN_DEVICE                 PCAN_USBBUS2//PCAN_USBBUS1//

#define CAN_BUS_TYPE_CAN            0
#define CAN_BUS_TYPE_CANFD          1
#define CAN_BUS_TYPE                1//CAN_BUS_TYPE_CAN//FD
#define CANFD_BIT_RATE              "f_clock_mhz=80, nom_brp=2, nom_tseg1=63, nom_tseg2=16, nom_sjw=16, data_brp=2, data_tseg1=15, data_tseg2=4, data_sjw=4" //500K-2M, copied from PCAN Explorer config

#define CANFD_IDMASK_SID			0x1FFC0000
#define CANFD_IDMASK_EID			0x3FFFF
#define CANFD_EID_BITS				18
#define MAX_DATA_BYTES 				64

typedef enum {
    CAN_DLC_0,
    CAN_DLC_1,
    CAN_DLC_2,
    CAN_DLC_3,
    CAN_DLC_4,
    CAN_DLC_5,
    CAN_DLC_6,
    CAN_DLC_7,
    CAN_DLC_8,
    CAN_DLC_12,
    CAN_DLC_16,
    CAN_DLC_20,
    CAN_DLC_24,
    CAN_DLC_32,
    CAN_DLC_48,
    CAN_DLC_64
} CAN_DLC;

#ifdef PROJECT_ID4
#define CAN_VEH_MSG_LEN				8
#define CAN_VEH_MSG_NUM				7								/* basic CAN messages for ID4 */
#define FILTER_TOTAL                (CAN_VEH_MSG_NUM + 1 + 2 + 7)	/* 1: system ID; 2:two general filters; 7: debugger control */
#endif
#ifdef PROJECT_C3
#define CAN_VEH_MSG_LEN				8
#define CAN_VEH_MSG_NUM				10								/* basic CAN messages for C3, ID4 + 2: current and op mode for now. */
#define FILTER_TOTAL                (CAN_VEH_MSG_NUM + 2 + 7)		/* 1: system ID; 2:two general filters; 7: debugger control */
#endif

#ifdef PROJECT_ID4
/* The sequence of below enumeration should be the same as the sequence of data in id4_vehData!!! */
typedef enum {
	APP_OPT_DEV_SEND_FSH,
	APP_OPT_DEV_SEND_ATEMP,
	APP_OPT_DEV_SEND_VOLTAGE,
	APP_OPT_DEV_SEND_SOC,
	APP_OPT_DEV_SEND_SPEED,
	APP_OPT_DEV_SEND_CTEMP,
	APP_OPT_DEV_SEND_HUMIDITY,
	APP_OPT_UNKNOWN
} msg_mode_t;
#endif

#ifdef PROJECT_C3
/* The sequence of below enumeration should be the same as the sequence of data in ic3canDataInfo!!! */
typedef enum {
	APP_OPT_DEV_SEND_SOC,
	APP_OPT_DEV_SEND_FSH,
	APP_OPT_DEV_SEND_ATEMP,
	APP_OPT_DEV_SEND_VOLTAGE,
	APP_OPT_DEV_SEND_SPEED,
	APP_OPT_DEV_SEND_CTEMP,
	APP_OPT_DEV_SEND_HUMIDITY,
	APP_OPT_DEV_SEND_SYSID,
	APP_OPT_DEV_SEND_CURRENT,
	APP_OPT_DEV_SEND_OPMODE,
	APP_OPT_UNKNOWN
} msg_mode_t;
#endif

typedef struct {
	int16_t val;
	uint16_t num;			/* loop number */
	uint16_t interval;		/* interval in ms between each message submission */
	uint16_t option;
	msg_mode_t mode;
	timer_t timer_id;
	int timer_count;
	bool timer_mark;
} msg_opt_t;
/*
 * struct canfdFilter {
    int16_t sid;
    int16_t sid_mask;
    int16_t sid11;
    int16_t sid11_mask;
    int32_t eid;
    int32_t eid_mask;
    bool ide;
    bool ide_check;
    CAN_FIFO_CHANNEL fifo_chan;
    CAN_FILTER filter_num;
};
struct canfdnode {
    char* spi_device;              // SPI device channel
    struct canfdFilter vwmsgFilter[FILTER_TOTAL];
};
*/
/* This is the data structure for vehicle data submission */
typedef struct {
	uint32_t mid;						/* message ID */
	char *name;							/* message name */
	uint16_t interval;					/* interval time in ms of continuous submission of this message, 0 is infinite */
	uint16_t num;						/* number of continuous submission of this message */
	uint16_t value;						/* value of this message */
	uint8_t data[CAN_VEH_MSG_LEN];		/* data in frame of this message */
} canDataInfo_type;

#ifdef PROJECT_ID4
/* These are the data from the vehicle through CAN message */
struct id4DataVeh_type {
    float voltage;
    float stateCharge;
    float speed;
    float tempInside;
    float tempOutside;
    float humidity;
    uint16_t fsh;
    uint16_t countMsg;
    bool speedQbit;
    bool tempOutsideQbit;
};

/* These are the data sent from tester */
#define CAN_DATA_OUT_LEN     MEM_ALIGN_SIZE(sizeof(struct id4DataOut_type))
union dataOut_type {
    struct id4DataOut_type {
        long int freq;
        uint16_t deadband_red1;         // dead band rising edge delay for PWM1, in 10ns
        uint16_t deadband_fed1;         // dead band falling edge delay for PWM1, in 10ns
        uint16_t deadband_red2;         // dead band rising edge delay for PWM2, in 10ns
        uint16_t deadband_fed2;         // dead band falling edge delay for PWM2, in 10ns
        int16_t compensation_up3;       // compensation of rising edge for PWM3, in 10ns
        int16_t compensation_up4;       // compensation of rising edge for PWM4, in 10ns
        uint16_t compensation_lik;      // parameter for calculating compensation falling edge in pH (1000uH)
        uint16_t compensation_n;        // parameter for calculating compensation falling edge
        uint16_t compensation_rload;    // parameter for calculating compensation falling edge in mOhms
        int16_t modeControl;            /* true: close loop control; false: open loop control */
        uint16_t VoutTarget;            /* Desired target voltage in volts */
        uint16_t runtime;               /* Forced run time in seconds */
        int16_t thTempTransfo;          /* Transformer temperature check threshold in degree */
        uint16_t thReslow;              /* Windshield resistance check threshold low in mOhms */
        uint16_t thReshigh;             /* Windshield resistance check threshold high in mOhms */
        uint16_t thCout;                /* Output current protection check threshold in A */
        uint16_t checkHV;               /* true: check high voltage input; false: force high voltage input */
        uint16_t psTarget;              /* final phase shift for open loop control in degree */
        uint16_t addRuntime;            /* Forced extra run time in seconds based on calculated run time */
    } data;
    uint16_t word[CAN_DATA_OUT_LEN/2U];
    uint8_t byte[CAN_DATA_OUT_LEN];
};

#define CAN_DATA_OUT_COMMAND_LEN     MEM_ALIGN_SIZE(sizeof(struct id4DataOut_command_type))
union dataOut_command_type {
    struct id4DataOut_command_type {
        uint16_t debugValue;
        uint16_t addrStart;
        uint16_t addrEnd;
        uint16_t dataNum;
    } data;
    uint16_t word[CAN_DATA_OUT_COMMAND_LEN/2U];
    uint8_t byte[CAN_DATA_OUT_COMMAND_LEN];
};

#define CAN_DATA_OUT_CFG_PETD_LEN     MEM_ALIGN_SIZE(sizeof(struct petdConfig))
union dataOut_cfgPetd_type {
	struct petdConfig data;
    uint16_t word[CAN_DATA_OUT_CFG_PETD_LEN/2U];
    uint8_t byte[CAN_DATA_OUT_CFG_PETD_LEN];
};

#define CAN_DATA_OUT_CFG_PWM_LEN     MEM_ALIGN_SIZE(sizeof(struct pwmConfig))
union dataOut_cfgPwm_type {
	struct pwmConfig data;
    uint16_t word[CAN_DATA_OUT_CFG_PWM_LEN/2U];
    uint8_t byte[CAN_DATA_OUT_CFG_PWM_LEN];
};

/* These are the data sent out to tester */
#define CAN_DATA_IN_LEN    MEM_ALIGN_SIZE(sizeof(struct id4DataIn_type))
union dataIn_type {
    struct id4DataIn_type {
        bool updated;
        uint16_t mode;
        uint16_t hitCount;
        uint16_t vin;
        uint16_t vout;
        uint16_t cout;
        uint16_t res;
        int16_t tempTro;
        int16_t temp1;
        int16_t temp2;
        int16_t temp3;
        int16_t temp4;
        uint16_t petdRuntime;
        uint16_t pwmCompen;
        uint16_t veh_fsh;
        uint16_t veh_voltage;
        uint16_t veh_stateCharge;
        uint16_t veh_speed;
        uint16_t veh_tempInside;
        uint16_t veh_tempOutside;
        uint16_t veh_humidity;
    } data;
    uint16_t word[CAN_DATA_IN_LEN/2U];
    uint8_t byte[CAN_DATA_IN_LEN];
};

#define CAN_DATA_IN_CFG_LEN    MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Cfg_type))
union dataIn_Cfg_type {
    struct id4DataIn_Cfg_type {
        bool updated;
        struct petdConfig petdCanConfig;
        struct pwmConfig pwmCanConfig;
    } data;
    uint16_t word[CAN_DATA_IN_CFG_LEN/2U];
    uint8_t byte[CAN_DATA_IN_CFG_LEN];
};

#define CAN_DATA_IN_CTL_LEN    MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Ctl_type))
union dataIn_Ctl_type {
    struct id4DataIn_Ctl_type {
        bool updated;
        uint16_t mode;
        uint16_t hitCount;
        uint16_t veh_fsh;
    } data;
    uint16_t word[CAN_DATA_IN_CTL_LEN/2U];
    uint8_t byte[CAN_DATA_IN_CTL_LEN];
};

#define CAN_DATA_IN_ENV_LEN    MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Env_type))
union dataIn_Env_type {
    struct id4DataIn_Env_type {
        bool updated;
        uint16_t vin;
        uint16_t veh_voltage;
        uint16_t veh_stateCharge;
        uint16_t veh_speed;
        uint16_t veh_tempInside;
        uint16_t veh_tempOutside;
        uint16_t veh_humidity;
    } data;
    uint16_t word[CAN_DATA_IN_ENV_LEN/2U];
    uint8_t byte[CAN_DATA_IN_ENV_LEN];
};

#define CAN_DATA_IN_RES_LEN    MEM_ALIGN_SIZE(sizeof(struct id4DataIn_Res_type))
union dataIn_Res_type {
    struct id4DataIn_Res_type {
        bool updated;
        int16_t tempTro;
        uint16_t vout;
        uint16_t cout;
        uint16_t res;
    } data;
    uint16_t word[CAN_DATA_IN_RES_LEN/2U];
    uint8_t byte[CAN_DATA_IN_RES_LEN];
};

/* These are the logged data sent out to tester */
#define CAN_DATA_LOG_LEN    MEM_ALIGN_SIZE(sizeof(struct id4DataLog_type))
union dataLog_type {
    struct id4DataLog_type {
        uint16_t value1;
        uint16_t value2;
        uint16_t value3;
        uint16_t value4;
        uint16_t address;
        uint16_t index;
    } data;
    uint16_t word[CAN_DATA_LOG_LEN/2U];
    uint8_t byte[CAN_DATA_LOG_LEN];
};
#endif	/* PROJECT_ID4 */

#ifdef PROJECT_C3
#define CAN_DATA_IN_LEN    sizeof(struct C3DataVeh_type)
typedef union dataIn_type {
    struct C3DataVeh_type {
        uint16_t voltage;
        uint16_t stateCharge;
        uint16_t speed;
        int16_t tempInside;
        int16_t tempOutside;
        uint16_t humidity;
        uint16_t current;
        uint16_t fsh;
        uint16_t countMsg;
        uint16_t systemId;
        uint16_t opMode;
    } data;
    uint16_t word[CAN_DATA_IN_LEN/2U];
    uint8_t byte[CAN_DATA_IN_LEN];
} dataIn_type;
#endif	/* PROJECT_C3 */

#ifdef PROJECT_NAVY
#define CAN_DATA_OUT_LEN     sizeof(struct navyDataOut) //8//
union dataOut {
    struct navyDataOut {
        uint16_t mode;
        int16_t tempOff;
        int16_t tempOn;
        uint16_t thRes;
        uint16_t thVoltagePeak;
        uint16_t thVoltageRms;
        uint16_t thCurrentPeak;
        uint16_t thCurrentRms;
    } data;
    uint16_t word[CAN_DATA_OUT_LEN/2];
    uint8_t byte[CAN_DATA_OUT_LEN];
};

#define CAN_DATA_IN_LEN    sizeof(struct navyDataIn) //16
union dataIn {
    struct navyDataIn {
        uint16_t mode;
        int16_t state;
        int16_t tempOff;
        uint16_t thRes;
        uint16_t thVoltage;
        uint16_t thCurrent;
        int16_t temp01;
        int16_t voltagePeak;
        uint16_t voltageRms;
        int16_t currentPeak;
        uint16_t currentRms;
        uint16_t resWindshield; 
    } data;
    uint16_t word[CAN_DATA_IN_LEN/2];
    uint8_t byte[CAN_DATA_IN_LEN];
};

#define CAN_DATA_LOG_LEN    sizeof(struct navyDataLog)
union dataLog {
    struct navyDataLog {
        bool updated;
        uint16_t count;
        int16_t voltagePeak;
        uint16_t voltageRms;
        int16_t currentPeak;
        uint16_t currentRms;
        uint16_t resWindshield;
        int16_t temp01;
        int16_t temp02;
        int16_t temp03;
    } data;
    uint16_t word[(CAN_DATA_LOG_LEN+1)/2];
    uint8_t byte[CAN_DATA_LOG_LEN];
};
#endif	/* PROJECT_NAVY */

struct canfdData {
#ifdef PROJECT_ID4
	struct id4DataVeh_type id4Dataveh;
	canDataInfo_type id4_vehData[CAN_VEH_MSG_NUM];
	union dataIn_type id4DataIn;
	union dataIn_Cfg_type id4DataIn_Cfg;
	union dataIn_Ctl_type id4DataIn_Ctl;
	union dataIn_Env_type id4DataIn_Env;
	union dataIn_Res_type id4DataIn_Res;
	union dataLog_type id4DataLog;
	union dataOut_type id4DataOut;
	union dataOut_command_type id4DataOut_command;
	union dataOut_cfgPetd_type id4DataOut_cfgPetd;
	union dataOut_cfgPwm_type id4DataOut_cfgPwm;
#endif
#ifdef PROJECT_C3
	canDataInfo_type c3canDataInfo[CAN_VEH_MSG_NUM];
	dataIn_type c3dataIn;
#endif
#ifdef PROJECT_NAVY
    union dataIn navyIn;
    union dataLog navyLog;
    union dataOut navyOut;
#endif
    bool updated;
};

struct canfdnode *msg_canfd_getNode(void);

/* Function to pass data pointer */
struct canfdData *msg_canfd_getData(void);
void msg_canfd_print(void);

/* Functions to implement CANFD tasks */
void msg_canfd_send_veh(uint8_t msgno, uint16_t value);
void msg_canfd_send_tester(int cmd);
int16_t msg_canfd_receive_isr(void);
int16_t msg_canfd_rcvCanLog(void);
int16_t msg_canfd_rcvCanConfigs(void);
void msg_canfd_receive(void);

/* Function to initialize CAN FD device */
bool msg_canfd_init(void);

#endif /* APPLICATION_CANFDCOMM_H_ */
