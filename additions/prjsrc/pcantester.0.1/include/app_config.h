/*
 * configSet.h
 *
 *  Created on: Aug. 23, 2022
 *      Author: Crane Shao
 */

#ifndef APPLICATION_CONFIGSET_H_
#define APPLICATION_CONFIGSET_H_

#include <stdint.h>

/* Power configuration
 * must be in the same sequence as the firmware in the control board
 * as the data passing is handled as a whole
 */
struct powerConfig {
	uint16_t mode;
	int16_t tempOff;                /* Windshield temperature to turn off AC switch */
	int16_t tempOn;                 /* Windshield temperature to turn on AC switch */
	uint16_t thVoltagePeak;         /* Voltage input threshold for error */
	uint16_t thVoltageRms;          /* Voltage input threshold for error */
	uint16_t thCurrentPeak;         /* Current input threshold for error */
	uint16_t thCurrentRms;          /* Current input threshold for error */
	uint16_t thRes;                 /* Windshield resistance threshold for error */
    bool checkRes;
    bool checkVoltage;
    bool checkCurrent;
    bool configUpdated;
};

/* PWM configuration
 * must be in the same sequence as the firmware in the control board
 * as the data passing is handled as a whole
 */
#define PWM_CONFIG_LEN				MEM_ALIGN_SIZE(sizeof(struct pwmConfig))
struct pwmConfig {
	bool configUpdated;
	bool filer1;
	uint32_t freq;
    uint16_t deadband_red1;         // dead band rising edge delay for PWM1, in 10ns
    uint16_t deadband_fed1;         // dead band falling edge delay for PWM1, in 10ns
    uint16_t deadband_red2;         // dead band rising edge delay for PWM2, in 10ns
    uint16_t deadband_fed2;         // dead band falling edge delay for PWM2, in 10ns
    int16_t compensation_up3;       // compensation of rising edge for PWM3, in 10ns
//    int16_t compensation_down3;     // compensation of falling edge for PWM3, in 10ns
    int16_t compensation_up4;       // compensation of rising edge for PWM4, in 10ns
//    int16_t compensation_down4;     // compensation of falling edge for PWM4, in 10ns
    uint16_t compensation_lik;      // parameter for calculating compensation falling edge in pH (1000uH)
    uint16_t compensation_n;        // parameter for calculating compensation falling edge
    uint16_t compensation_rload;    // parameter for calculating compensation falling edge in mOhms
};

/* PETD configuration
 * must be in the same sequence as the firmware in the control board
 * as the data passing is handled as a whole
 */
#define PETD_CONFIG_LEN				MEM_ALIGN_SIZE(sizeof(struct petdConfig))
struct petdConfig {
    bool configUpdated;
    bool filter1;					/* add filters to be align with the sender structure */
    int16_t VoutTarget;             /* Desired target voltage in volts */
    int16_t runtime;                /* Forced run time in seconds */
    int16_t thTempTransfo;          /* Transformer temperature check threshold in degree */
    int16_t thReslow;               /* Windshield resistance check threshold low in mOhms */
    int16_t thReshigh;              /* Windshield resistance check threshold high in mOhms */
    int16_t thCout;                 /* Output current protection check threshold in A */
    bool forceRuntime;              /* true: force run time; false: compute run time */
    bool filter2;
    bool checkTempransfo;           /* true: check transformer temperature; false: force transformer temperature */
    bool filter3;
    int16_t checkHV;                /* true: check high voltage input; false: force high voltage input */
    bool checkRes;                  /* true: check windshield resistance; false: force windshield resistance */
    bool filter4;
    bool checkCout;                 /* true: check output current; false: force output current */
    bool filter5;
    int16_t modeControl;            /* true: close loop control; false: open loop control */
    int16_t psTarget;               /* final phase shift for open loop control in degree */
    uint16_t addRuntime;            /* Forced extra run time in seconds based on calculated run time */
    int16_t filer0;					/* for alignment when combining with another structure */
};

int app_config();

void writeLogging(void);
void printLogging(void);

/* Functions to config power control - Navy */
struct powerConfig *powerConfig_get(void);
void powerConfig_updateCan(void);
void powerConfig_print(void);
bool powerConfig_check(struct powerConfig *configIn, struct powerConfig *configOut, bool ignore);

/* Functions to config PETD */
void petdConfig_print(void);
struct petdConfig *petdConfig_get(void);
bool petdConfig_check(struct petdConfig *configIn, struct petdConfig *configOut, bool ignore);

/* Functions to config PWM */
void pwmConfig_print(void);
struct pwmConfig* pwmConfig_get(void);
bool pwmConfig_check(struct pwmConfig *configIn, struct pwmConfig *configOut, bool ignore);

#endif /* APPLICATION_CONFIGSET_H_ */
