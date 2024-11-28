/*
 * configSet.c
 *
 *  Created on: Aug. 23, 2022
 *      Author: Crane Shao
 */
#include <stdbool.h>
#include <stdio.h>

#include "app_config.h"
#include "sysconfig.h"
#include "utility.h"
#include "app_log.h"
#include "app_canfd.h"

#define INVALID_INPUT               (-10000)

/* operation definitions */
#define MODE_OP_DEFAULT                         0
#define POWER_OFF_TEMP                          20              /* temperature in °C to turn off AC switch */
#define POWER_ON_TEMP                           10              /* temperature in °C to turn off AC switch */

/* boundary of errors */
#define TH_VOLTAGE_IN_PEAK                      660             /* in V */
#define TH_VOLTAGE_IN_RMS                       440             /* in V */
#define TH_CURRENT_IN_PEAK                      15              /* in A */
#define TH_CURRENT_IN_RMS                       10              /* in A */
#define TH_RES_WINDSHIELD                       52              /* in Ohms */

/* definitions */
#define MODE_OP_DEICE               true
#define MODE_OP_DEFOG               false
#define MODE_CTRL_CLOSE_LOOP        true
#define MODE_CTRL_OPEN_LOOP         false
/* control configurations for ID4 project */
#define DEFAULT_OPERATION_MODE      MODE_OP_DEICE
#define DEFAULT_CONTROL_MODE        MODE_CTRL_CLOSE_LOOP//MODE_CTRL_OPEN_LOOP//
#define DEFAULT_VOUTPUT_TARGET      80                      /* volts, for close loop control */
#define DEFAULT_VOUTPUT_START       VOLTAGE_OUTPUT_MIN      /* volts, for close loop control */
#define DEFAULT_PHASESHIFT_TARGET   120                     /* degree, for open loop control */
#define PHASESHIFT_START            PHASESHIFT_MIN          /* degree, for open loop control */
#define PHASESHIFT_END              50                      /* degree, for open loop control */
#define DEFAULT_RUN_TIME            20                    	/* in 1s, 20s */
#define DEFAULT_ADD_RUNTIME         10						/* in 1s, 10s */
#define DEFAULT_CHECK_TIME          100                     /* in 10ms, 1s */
#define DEFAULT_RAMPUP_TIME         500                     /* in 10ms, 5s */
/* range of control parameters */
#define PHASESHIFT_MIN              10                      /* in degree */
#define PHASESHIFT_MAX              160//90//               /* in degree */
#define VOLTAGE_OUTPUT_MIN          20                      /* in volts */
#define VOLTAGE_OUTPUT_MAX          95                      /* in volts */
#define RUN_TIME_MIN                0
#define RUN_TIME_MAX                600//281///* in seconds, if more than 600, the timing variable size needs to be long */
#define ADD_RUNTIME_MAX             30						/* in 1s, 30s */
#define TEMP_TRANSFO_MIN            -10                     /* in °C */
#define TEMP_TRANSFO_MAX            30                      /* in °C */
#define WINDSHIELD_RES_LOW_MIN      0
#define WINDSHIELD_RES_LOW_MAX      2500                    /* in mOhms */
#define WINDSHIELD_RES_HIGH_MIN     1500                    /* in mOhms */
#define WINDSHIELD_RES_HIGH_MAX     5000                    /* in mOhms */
#define CURRETNT_OUTPUT_MIN         0
#define CURRETNT_OUTPUT_MAX         80                      /* in A */
/* boundary of errors */
#define TEMP_TRANSFORMER_RUN         0.0f
#define WINDSHIELD_RES_MIN           1000                   /* in mOhms */
#define WINDSHIELD_RES_MAX           2500                   /* in mOhms */
/* dead band config (*10ns) */
#define DEADBAND_MIN                    0   // *10ns
#define DEADBAND_MAX                    100 // *10ns
#define DEADBAND_RED_PWM1_EN            true
#define DEADBAND_FED_PWM1_EN            true
#define DEADBAND_RED_PWM2_EN            true
#define DEADBAND_FED_PWM2_EN            true
#define DEADBAND_RED_PWM1               10  // realization is *10ns (irrelevant to PWM freq)
#define DEADBAND_FED_PWM1               10  // realization is *10ns (irrelevant to PWM freq)
#define DEADBAND_RED_PWM2               20  // realization is *10ns (irrelevant to PWM freq)
#define DEADBAND_FED_PWM2               20  // realization is *10ns (irrelevant to PWM freq)
/* adjustments */
#define COMPENSATION_LIK_DEFAULT        6090U       // pH
#define COMPENSATION_LIK_MIN            1000U
#define COMPENSATION_LIK_MAX            8000U
#define COMPENSATION_N_DEFAULT          20U
#define COMPENSATION_N_MIN              20U
#define COMPENSATION_N_MAX              30U
#define COMPENSATION_RLOAD_DEFAULT      1450U       // mOhms
#define COMPENSATION_RLOAD_MIN          100U
#define COMPENSATION_RLOAD_MAX          10000U
#define PWM34_ADJUSTMENT                1 /* adjust PWM3/4 */
#if (1 != PWM34_ADJUSTMENT)
#define ADJUSTMENT_3_RISING             0
#define ADJUSTMENT_3_FALLING            0
#define ADJUSTMENT_4_RISING             0
#define ADJUSTMENT_4_FALLING            0
#else
#define ADJUSTMENT_3_RISING             0
#define ADJUSTMENT_3_FALLING            30
#define ADJUSTMENT_4_RISING             0
#define ADJUSTMENT_4_FALLING            30
#endif
/* PWM initial frequency */
#define FREQ_INIT                       80000
#define FREQ_INIT_MIN                   50000//100000
#define FREQ_INIT_MAX                   150000

#ifdef PROJECT_NAVY
struct powerConfig powerUartConfig = {
  .mode = MODE_OP_DEFAULT,
  .tempOff = POWER_OFF_TEMP,
  .thVoltagePeak = TH_VOLTAGE_IN_PEAK,
  .thVoltageRms = TH_VOLTAGE_IN_RMS,
  .thCurrentPeak = TH_CURRENT_IN_PEAK,
  .thCurrentRms  = TH_CURRENT_IN_RMS,
  .thRes = TH_RES_WINDSHIELD,
  .checkVoltage = true,
  .checkCurrent = true,
  .checkRes = true,
  .configUpdated = false,
};
#endif

#ifdef PROJECT_ID4
struct petdConfig petdUartConfig = {
  .configUpdated = false,
  .VoutTarget = DEFAULT_VOUTPUT_TARGET,
  .runtime = DEFAULT_RUN_TIME,
  .forceRuntime = false,
  .thTempTransfo = (uint16_t)TEMP_TRANSFORMER_RUN,
  .checkTempransfo = true,
  .thReshigh = WINDSHIELD_RES_MAX,
  .thReslow = WINDSHIELD_RES_MIN,
  .checkRes = true,
  .checkHV = true,
  .checkCout = true,
  .modeControl = DEFAULT_CONTROL_MODE,
  .psTarget = DEFAULT_PHASESHIFT_TARGET,
  .addRuntime = DEFAULT_ADD_RUNTIME,
};

static struct pwmConfig pwmUartConfig = {
  .deadband_red1 = DEADBAND_RED_PWM1,
  .deadband_fed1 = DEADBAND_FED_PWM1,
  .deadband_red2 = DEADBAND_RED_PWM2,
  .deadband_fed2 = DEADBAND_FED_PWM2,
  .compensation_up3 = ADJUSTMENT_3_RISING,
//  .compensation_down3 = ADJUSTMENT_3_FALLING,
  .compensation_up4 = ADJUSTMENT_4_RISING,
//  .compensation_down4 = ADJUSTMENT_4_FALLING,
  .freq = FREQ_INIT,
  .compensation_lik = COMPENSATION_LIK_DEFAULT,
  .compensation_n = COMPENSATION_N_DEFAULT,
  .compensation_rload = COMPENSATION_RLOAD_DEFAULT,
};
#endif	/* #ifdef PROJECT_NAVY */

void clearStdin()
{
    // keep reading 1 more char as long as the end of the stream, indicated by the newline char,
    // has NOT been reached
    while (true)
    {
        int c = getc(stdin);
        if (c == EOF || c == '\n')
        {
            break;
        }
    }
}

//int UART_Read_Number(const char *msg)
//{
//	int number = 0;
//
//    iPrintf("%s: ", msg);
//    scanf("%d", &number);
//
//    return number;
//}
/* Function to input an integer allowing negative value
 * return INVALID_INPUT if failed to acquire a number
 * */
int UART_Read_Number(const char *msg)
{
    unsigned char hitkey = 0;
    unsigned int digit = 0;
    int number = 0;
    unsigned int count = 0;
    int sign = 1;

    printf("\r\n Please input %s: ", msg);
    hitkey = getc(stdin);
    if('\n' == hitkey)
    {
        return INVALID_INPUT;
    }
    else if ('-' == hitkey)
    {
        sign = -1;
    }
    else if(('0'<=hitkey) && ('9'>=hitkey))
    {
        digit = hitkey - '0';
        number = digit;
    }

    do{
          hitkey = getc(stdin);
          count++;
          if(('0'<=hitkey) && ('9'>=hitkey))
          {
              digit = hitkey - '0';
              number = number*10 + digit;
          }
    } while(('\n' != hitkey) && (1000000 > number));

    return (number * sign);
}

unsigned char UART_Read_Char(void)
{
	unsigned char ch;

	scanf("%c", &ch);

	/* clean the last ENTER after inputting a number. Any API to clear std io? -> write own */
	if('\r' == ch) scanf("%c", &ch);

	return ch;
}

/* Function to get config from Stdio inputs */
int configInput(const char *msg)
{
    int number;

    number = UART_Read_Number(msg);
    iPrintf("You input %d\r\n", number);

    return number;
}

#ifdef PROJECT_NAVY
void powerConfig_updateCan(void)
{
	struct canfdData *temp;
	temp = msg_canfd_getData();

	temp->navyOut.data.thRes = powerUartConfig.thRes;
	temp->navyOut.data.thVoltagePeak = powerUartConfig.thVoltagePeak;
	temp->navyOut.data.thVoltageRms = powerUartConfig.thVoltageRms;
	temp->navyOut.data.thCurrentPeak = powerUartConfig.thCurrentPeak;
	temp->navyOut.data.thCurrentRms = powerUartConfig.thCurrentRms;
	temp->navyOut.data.tempOff = powerUartConfig.tempOff;
	temp->navyOut.data.tempOn = powerUartConfig.tempOn;
	temp->navyOut.data.mode = powerUartConfig.mode;

	temp->updated = true;
}

bool powerConfig_input(void)
{
    struct powerConfig temp_cfg;

	iPrintf("\r\n\nNavy Configuration");
    iPrintf("\r\nPress ENTER to pass!\r\n");

    clearStdin();
    temp_cfg.mode = UART_Read_Number("\r\nOperation mode (0 ~ 3)");
    temp_cfg.tempOff = UART_Read_Number("\r\nAC switch off windshield temp (-20 - 20°C)");
    temp_cfg.tempOn = UART_Read_Number("\r\nAC switch on windshield temp (-20 - 20°C)");               /* Windshield temperature to turn off AC switch */
    temp_cfg.thRes = UART_Read_Number("\r\nWindshield resistance threshold (0 - 52Ohms)");
    temp_cfg.thVoltagePeak = UART_Read_Number("\r\nVoltage peak check threshold (0 - 660V)");
    temp_cfg.thVoltageRms = UART_Read_Number("\r\nVoltage rms check threshold (0 - 440V)");
    temp_cfg.thCurrentPeak = UART_Read_Number("\r\nCurrent peak check threshold (0 - 15A)\0");
    temp_cfg.thCurrentRms = UART_Read_Number("\r\nCurrent rms check threshold (0 - 10A)\0");

    // confirm the selections
    uint16_t hitkey;
    iPrintf("\r\n\r\nPlease confirm y or n:");
    hitkey = getc(stdin);
    while(('y' != hitkey) && ('n' != hitkey) && ('Y' != hitkey) && ('N' != hitkey))
    {   // invalid input. Need input again
    	iPrintf("\r\nInvalid input! Please input again!");
        hitkey = getc(stdin);
    }

    /* check and copy the updated configuration */
    if(('y' == hitkey) || ('Y' == hitkey))
    {   /* copy the input */
        powerConfig_check(&temp_cfg, &powerUartConfig, true);
        iPrintf("\r\nConfiguration confirmed!\r\n");
        petdUartConfig.configUpdated = true;
        return true;
    }

    return false;
}
#endif	/* #ifdef PROJECT_NAVY */

#ifdef PROJECT_ID4
void petdConfig_updateCan(void)
{
	struct canfdData *temp;
	temp = msg_canfd_getData();

//	temp->id4DataOut_cfgPetd.data.modeControl = petdUartConfig.modeControl;
//	temp->id4DataOut_cfgPetd.data.VoutTarget = petdUartConfig.VoutTarget;
//	temp->id4DataOut_cfgPetd.data.runtime = petdUartConfig.runtime;
//	temp->id4DataOut_cfgPetd.data.thTempTransfo = petdUartConfig.thTempTransfo;
//	temp->id4DataOut_cfgPetd.data.thReslow = petdUartConfig.thReslow;
//	temp->id4DataOut_cfgPetd.data.thReshigh = petdUartConfig.thReshigh;
//	temp->id4DataOut_cfgPetd.data.thCout = petdUartConfig.thCout;
//	temp->id4DataOut_cfgPetd.data.checkHV = petdUartConfig.checkHV;
//	temp->id4DataOut_cfgPetd.data.psTarget = petdUartConfig.psTarget;
//	temp->id4DataOut_cfgPetd.data.addRuntime = petdUartConfig.addRuntime;
	temp->id4DataOut_cfgPetd.data = petdUartConfig;

	temp->updated = true;
}

bool petdConfig_input(void)
{
    iPrintf("\r\n\nPETD Configuration:");
    //iPrintf("\r\nPress ENTER to pass!\r\n");

    struct petdConfig temp_cfg;

    temp_cfg.modeControl = UART_Read_Number("\r\nControl mode (1 for CL, 0 for OL)");
    if(INVALID_INPUT != temp_cfg.modeControl)
    {
        if(0 == temp_cfg.modeControl)
        {
            temp_cfg.modeControl = 0;
        }
        else if(1 == temp_cfg.modeControl)
        {
            temp_cfg.modeControl = 1;
        }
        else
        {   /* assign the default value */
            temp_cfg.modeControl = petdUartConfig.modeControl;
        }
    }
    else
    {
        temp_cfg.modeControl = petdUartConfig.modeControl;
    }

    if(MODE_CTRL_OPEN_LOOP == temp_cfg.modeControl)
    {
        temp_cfg.psTarget = UART_Read_Number("\r\nDesired Final Phase Shift (10 - 160) °");
    }
    else
    {
        temp_cfg.VoutTarget = UART_Read_Number("\r\nDesired Output Voltage (20 - 95)V");
    }

    temp_cfg.runtime = UART_Read_Number("\r\nDesired Run Time (0 - 600s)");
    if(INVALID_INPUT != temp_cfg.runtime) {
        /* invalid input, get the last value */
        temp_cfg.runtime = petdUartConfig.runtime;
    }
    if(0 == temp_cfg.runtime) {
        temp_cfg.addRuntime = UART_Read_Number("\r\nAdditional run time (0 - 30s)\0");
    }

    temp_cfg.thTempTransfo = UART_Read_Number("\r\nTransfo Temp. threshold (-10°C - 30°C)");
    temp_cfg.checkHV = UART_Read_Number("\r\nHigh voltage enable/disable (1 to enable, 0 to disable)");
    temp_cfg.thReslow = UART_Read_Number("\r\nResistance threshold low (0 - 2500 mOhms)");
    temp_cfg.thReshigh = UART_Read_Number("\r\nResistance threshold high (1500 - 5000) mOhms");
    temp_cfg.thCout = UART_Read_Number("\r\nCurrent Output threshold (0 - 80A)");

    // confirm the selections
    uint16_t hitkey;
    iPrintf("\r\n\r\nPlease confirm y or n:");
    hitkey = UART_Read_Char();
    while(('y' != hitkey) && ('n' != hitkey) && ('Y' != hitkey) && ('N' != hitkey))
    {   // invalid input. Need input again
        iPrintf("\r\nInvalid input! Please input again!");
        hitkey = UART_Read_Char();
    }

    /* assign the updated configuration */
    if(('y' == hitkey) || ('Y' == hitkey))
    {   /* copy the input */
        petdConfig_check(&temp_cfg, &petdUartConfig, true);
        iPrintf("\r\nConfiguration confirmed!\r\n");
        petdUartConfig.configUpdated = true;
        return true;
    }

    return false;
}

void pwmConfig_updateCan(void)
{
	struct canfdData *temp;
	temp = msg_canfd_getData();

	temp->id4DataOut_cfgPwm.data.freq = pwmUartConfig.freq;
	temp->id4DataOut_cfgPwm.data.deadband_red1 = pwmUartConfig.deadband_red1;
	temp->id4DataOut_cfgPwm.data.deadband_red2 = pwmUartConfig.deadband_red2;
	temp->id4DataOut_cfgPwm.data.deadband_fed1 = pwmUartConfig.deadband_fed1;
	temp->id4DataOut_cfgPwm.data.deadband_fed2 = pwmUartConfig.deadband_fed2;
	temp->id4DataOut_cfgPwm.data.compensation_up3 = pwmUartConfig.compensation_up3;
	temp->id4DataOut_cfgPwm.data.compensation_up4 = pwmUartConfig.compensation_up4;
	temp->id4DataOut_cfgPwm.data.compensation_lik = pwmUartConfig.compensation_lik;
	temp->id4DataOut_cfgPwm.data.compensation_n = pwmUartConfig.compensation_n;
	temp->id4DataOut_cfgPwm.data.compensation_rload = pwmUartConfig.compensation_rload;

	temp->updated = true;
}

bool pwmConfig_input(void)
{
    iPrintf("\r\n\nPWM Configuration");
    iPrintf("\r\nPress ENTER to pass!\r\n");

    int32_t temp = 0;

    struct pwmConfig temp_cfg;

    /* set frequency of PWM1 and PWM2 */
    temp = UART_Read_Number("\r\nPWM1/2/3/4 Frequency (*KHz, 50~150)");
    temp *= 1000;
    temp_cfg.freq = temp;

    /* set dead band for PWM1 and PWM2 RED and FED, ns */
    temp = UART_Read_Number("\r\nDead Band PWM1 RED (0~100 of 10ns)");
    temp_cfg.deadband_red1 = temp/10;
    temp = UART_Read_Number("\r\nDead Band PWM1 FED (0~100 of 10ns)");
    temp_cfg.deadband_fed1 = temp/10;
    temp = UART_Read_Number("\r\nDead Band PWM2 RED (0~100 of 10ns)");
    temp_cfg.deadband_red2 = temp/10;
    temp = UART_Read_Number("\r\nDead Band PWM2 FED (0~100 of 10ns)");
    temp_cfg.deadband_fed2 = temp/10;

    /* set dead band for PWM1 and PWM2 RED and FED, ns */
//    temp = UART_Read_Number("\r\nCompensation PWM3A DOWN (0~100 of 10ns)");
//    temp_cfg.compensation_down3 = temp/10;
//    temp = UART_Read_Number("\r\nCompensation PWM4A DOWN (0~100 of 10ns)");
//    temp_cfg.compensation_down4 = temp/10;
    temp = UART_Read_Number("\r\nCompensation PWM3A UP (0~100 of 10ns)");
    temp_cfg.compensation_up3 = temp/10;
    temp = UART_Read_Number("\r\nCompensation PWM4A UP (0~100 of 10ns)");
    temp_cfg.compensation_up4 = temp/10;
    temp = UART_Read_Number("\r\nCompensation Lik (1000~8000 pH )\0");
    temp_cfg.compensation_lik = temp;
    temp = UART_Read_Number("\r\nCompensation N (20~30) *0.1\0");
    temp_cfg.compensation_n = temp;
    temp = UART_Read_Number("\r\nCompensation Rload (100~10000)mOhms\0");
    temp_cfg.compensation_rload = temp;

    // confirm the selections
    int hitkey;
    iPrintf("\r\n\r\nPlease confirm y or n:");
    hitkey = UART_Read_Char();
    while(('y' != hitkey) && ('n' != hitkey) && ('Y' != hitkey) && ('N' != hitkey))
    {   // invalid input. Need input again
    	iPrintf("\r\nInvalid input! Please input again!");
        hitkey = UART_Read_Char();
    }
    if(('y' == hitkey) || ('Y' == hitkey))
    {
        iPrintf("\r\nConfiguration confirmed!");
        pwmConfig_check(&temp_cfg, &pwmUartConfig, true);
    }
    else
    {
        iPrintf("\r\nConfiguration ignored!");
        return false;
    }

    return true;
}
#endif	/* #ifdef PROJECT_ID4 */

int app_config()
{
	int ret = 0;

	dData.display = 0;
	dData.number = 0;

    //if(UART_Read_Passwd())                                     // using a protocol instead of just a keyboard hit
    {
        //iPrintf("\r\nPlease input command (p, c, f, d, s)");
        iPrintf("\r\nPlease input command (p, c, f, d)");
        iPrintf("\r\n p: get the config and print out them");
        iPrintf("\r\n c: config PETD");
        iPrintf("\r\n f: config PWM");
        iPrintf("\r\n d: get the log");
        //iPrintf("\r\n s: send CAN message");
        iPrintf("\r\n ->: ");

        char hitkey;
        hitkey = UART_Read_Char();
        while((COMMAND_P != hitkey) && ((COMMAND_P - 32) != hitkey)
                && (COMMAND_C != hitkey) && ((COMMAND_C - 32) != hitkey)
                && (COMMAND_F != hitkey) && ((COMMAND_F - 32) != hitkey)
                && (COMMAND_W != hitkey) && ((COMMAND_W - 32) != hitkey)
                && (COMMAND_D != hitkey) && ((COMMAND_P - 32) != hitkey)
				&& (COMMAND_S != hitkey) && ((COMMAND_S - 32) != hitkey))
        {   // invalid input. Need input again
        	iPrintf("\r\nInvalid input! Please input again!");
            hitkey = UART_Read_Char();
        }

        switch(hitkey) {
#ifdef PROJECT_ID4
			case COMMAND_P:
			case (COMMAND_P - 32):
				/* display some information */
        		msg_canfd_getData()->id4DataOut_command.data.debugValue = 999;
        		msg_canfd_getData()->updated = true;
        		//while(1)
        			{ msg_canfd_send_tester(COMMAND_P); }
				ret = COMMAND_P;
    		break;
        	case COMMAND_C:
        	case (COMMAND_C - 32):
				/* config PETD control parameters */
				getc(stdin);
				if(petdConfig_input())
				{
					petdConfig_print();
					petdConfig_updateCan();
				}
            	ret = COMMAND_C;
            	break;
          	case COMMAND_F:
        	case (COMMAND_F - 32):
				/* config PETD control parameters */
				getc(stdin);
				if(pwmConfig_input())
				{
					pwmConfig_print();          /* display the configurations */
					pwmConfig_updateCan();
				}
        		ret = COMMAND_F;
				break;
        	case COMMAND_D:
        	case (COMMAND_D - 32):
				/* process debug option */
				getc(stdin);		/* get rid of ENTER key */
				dData.display = configInput("number for debugging");
				if(444 == dData.display)
				{	/* request log data */
				    uint32_t position = 0;
				    uint16_t number = 0x10;
				    position = UART_Read_Number("\r\nRead FRAM from (0 ~ 16376/0x4000)");
				    number = UART_Read_Number("\r\nRead words of (1 ~ 17376)");
		            msg_canfd_getData()->id4DataOut_command.data.addrStart = position;
		            msg_canfd_getData()->id4DataOut_command.data.dataNum = number;
	            	msg_canfd_getData()->id4DataOut_command.data.debugValue = dData.display;
	            	msg_canfd_getData()->updated = true;
	            	ret = COMMAND_D;
				}
				else if (333 == dData.display)
				{	/* request error data */
	            	msg_canfd_getData()->id4DataOut_command.data.debugValue = dData.display;
	            	msg_canfd_getData()->updated = true;
	            	ret = COMMAND_D;
				}
				else {
					ret = dData.display;
				}
				break;
        	case COMMAND_S:
        	case (COMMAND_S - 32):
				/* process sending the CAN message */
				getc(stdin);		/* get rid of ENTER key */
        		iPrintf("\r\nPlease input number for debugging");
		        iPrintf("\r\n 1: FSH OFF");
		        iPrintf("\r\n 2: FSH ON");
		        iPrintf("\r\n 3: Ambient temperature");
		        dData.display = configInput("->");
		        if(3 == dData.display) {
				    dData.number = UART_Read_Number("\r\nAmbient Temp. (-30°C - 30°C)");
		        }
		        msg_canfd_getData()->updated = true;
				ret = COMMAND_S;
				break;
#endif
#ifdef PROJECT_NAVY
        	case COMMAND_P:
        	case (COMMAND_P - 32):
				powerConfig_print();
				ret = COMMAND_P;
        		break;
        	case COMMAND_C:
        	case (COMMAND_C - 32):
				if(powerConfig_input())
				{
					powerConfig_print();
					powerConfig_updateCan();
				}
				ret = COMMAND_C;
			     break;
        	case COMMAND_D:
        	case (COMMAND_D - 32):
				dData.number = configInput("operation mode(0 ~ 3)");
				if((0 > dData.number) || (3 < dData.number)) {
					iPrintf("Input out of range, ignored!");
				}
				else {
					msg_canfd_getData()->navyOut.data.mode = dData.number;
				}
				break;
#endif
        	case COMMAND_W:
        	case (COMMAND_W - 32):
				ret = 0;
				break;
        	default:
        		ret = 0;
        		break;
        }

#if 0
        if((COMMAND_P == hitkey) || ((COMMAND_P - 32) == hitkey))
        {   /* display some information */
#ifdef PROJECT_ID4
            pwmConfig_print();
            petdConfig_print();
#endif
#ifdef PROJECT_NAVY
            powerConfig_print();
#endif
            ret = COMMAND_P;
        }
        else if((COMMAND_C == hitkey) || ((COMMAND_C - 32) == hitkey))
        {   /* config PWM parameters */
#ifdef PROJECT_ID4
            if(petdConfig_input())
            {
                petdConfig_print();
            }
#endif
#ifdef PROJECT_NAVY
            if(powerConfig_input())
            {
                powerConfig_print();
                powerConfig_updateCan();
            }
#endif
            ret = COMMAND_C;
        }
        else if((COMMAND_F == hitkey) || ((COMMAND_F - 32) == hitkey))
        {   /* config PWM parameters */
 #ifdef PROJECT_ID4
        	if(pwmConfig_input())
            {
                pwmConfig_print();          /* display the configurations */
            }
#endif
#ifdef PROJECT_NAVY
        	dData.number = configInput("operation mode(0 ~ 3)");
        	if((0 > dData.number) || (3 < dData.number)) {
        		iPrintf("Input out of range, ignored!");
        	}
        	else {
        		msg_canfd_getData()->navyOut.data.mode = dData.number;
        	}
#endif
        	ret = COMMAND_F;
        }
        else if((COMMAND_W == hitkey) || ((COMMAND_W - 32) == hitkey))
        {   /* write to FRAM */
            writeLogging();
            ret = COMMAND_W;
        }
        else if((COMMAND_D == hitkey) || ((COMMAND_D - 32) == hitkey))
        {   /* input a number for debugging */
            dData.display = configInput("number for debugging");
//            dData.number = configInput("number for debugging");

#ifdef PROJECT_ID4
            if(444 == dData.display)
            {
                printLogging();
            }
            else if (333 == dData.display)
            {
                dataLogfram_errorDisplay();
            }
#endif
            ret = COMMAND_D;
        }
#endif
    }

    return ret;
}

void writeLogging(void)
{
    bool status;
    uint32_t address = 0;
    uint16_t number = 0x10, value = 0;
    char hitkey;

    address = UART_Read_Number("\r\nWrite FRAM from (0 ~ 131072/0x20000)");
    number = UART_Read_Number("\r\nWrite words of (1 ~ 65535)");
    value = UART_Read_Number("\r\nWrite value of (0 ~ 65535)");

    do{
        iPrintf("\r\nWrite %2d W @0x%5X:\t", number, address);
        status = dataLogger_writeWord(address, number, &value);
        if(!status)
        {
            iPrintf("\r\nError writing FRAM %d words at %d!", number, address);
        }

        address += number * 2;
        value += number;
        if(FRAM_SIZE <= address)
        {
            address = 0;
        }

        iPrintf("\r\nWriting FRAM %d words at %d successfully!", number, address);
        hitkey = UART_Read_Char();
    } while('\r' != hitkey);
}

void printLogging(void)
{
    bool status;
    uint32_t address = 0;
    uint16_t number = 0x10;
    char hitkey;

    address = UART_Read_Number("\r\nRead FRAM from (0 ~ 131072/0x20000)");
    number = UART_Read_Number("\r\nRead words of (1 ~ 65535)");

    if(555 == address)
    {   /* print values in log buff */
        address = 0;

        do{
#ifdef DATA_LOG_BYTE
            iPrintf("\r\nRead %2d bytes @%d:\n\r", number, (uint16_t)address);
            dataLogbuff_print_byte(address, number);
#else
            iPrintf("\r\nRead %2d words @%d:\n\r", number, (uint16_t)address);
            dataLogbuff_print_word(address, number);
#endif
            address += number;

            hitkey = UART_Read_Char();
        } while('\r' != hitkey);

        return;
    }

    /* printf values from Storage */
    do{
#ifdef DATA_LOG_BYTE
        iPrintf("\r\nRead %2d Bytes @%d:\t", number, address);

        status = dataLogger_print_byte(address, number);
        if(!status)
        {
            iPrintf("\r\nError reading FRAM %d bytes at %d!", number, address);
        }
        address += number;
#else
        iPrintf("\r\nRead %2d Words @%d:\t", number, address);

        /* read and display only one word */
//        status = dataLogger_print_word(address, number);     address += number * 2;
        /* read and display N (4 for now) words */
        status = dataLogger_print_wordn(address, number);    address += number * 8;
        if(!status)
        {
            iPrintf("\r\nError reading FRAM %d words at %d!", number, address);
        }
#endif

        if(FRAM_SIZE <= address)
        {
            address = 0;
        }

        hitkey = UART_Read_Char();
    } while('\r' != hitkey);
}

#ifdef PROJECT_NAVY
struct powerConfig *powerConfig_get(void)
{
    return &powerUartConfig;
}

void powerConfig_print(void)
{
    iPrintf("\r\nNavy configuration:");

    iPrintf("\r\nOperation mode \t\t [ %d ]", powerUartConfig.mode);
    iPrintf("\r\nAC Switch off windshield temp. \t[ %d ]°C", powerUartConfig.tempOff);
    iPrintf("\r\nAC Switch on windshield temp. \t[ %d ]°C", powerUartConfig.tempOn);
    iPrintf("\r\nWindshield res. check: \t[ %d ] Ohms,\t%s", powerUartConfig.thRes, (powerUartConfig.checkRes)?"Enabled\0":"Disabled");
    iPrintf("\r\nVoltage peak check: \t[ %d ] V, \t%s", powerUartConfig.thVoltagePeak, powerUartConfig.checkVoltage?"Enabled":"Disabled");
    iPrintf("\r\nVoltage rms check: \t[ %d ] V, \t%s", powerUartConfig.thVoltageRms, powerUartConfig.checkVoltage?"Enabled":"Disabled");
    iPrintf("\r\nCurrent peak check: \t[ %d ] A, \t%s", powerUartConfig.thCurrentPeak, powerUartConfig.checkCurrent?"Enabled":"Disabled");
    iPrintf("\r\nCurrent rms check: \t[ %d ] A, \t%s", powerUartConfig.thCurrentRms, powerUartConfig.checkCurrent?"Enabled":"Disabled");
}

bool powerConfig_check(struct powerConfig *configIn, struct powerConfig *configOut, bool ignore)
{
    bool status;

    if(INVALID_INPUT != configIn->mode)
    {
        if((0 <= configIn->mode) && (3 >= configIn->mode))
        {
            configOut->mode = configIn->mode;
        }
        else
        {
            iPrintf("\r\nOperation mode is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->mode = MODE_OP_DEFAULT;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->tempOff)
    {
        if((-20 <= configIn->tempOff) && (20 >= configIn->tempOff))
        {
            configOut->tempOff = configIn->tempOff;
        }
        else
        {
            iPrintf("\r\nTemp winshield is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->tempOff = POWER_OFF_TEMP;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->tempOn)
    {
        if((-20 <= configIn->tempOn) && (20 >= configIn->tempOn)
        		&& (configIn->tempOn <= configOut->tempOff))
        {
            configOut->tempOn = configIn->tempOn;
        }
        else
        {
            iPrintf("\r\nTemp winshield is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->tempOn = POWER_ON_TEMP;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thRes)
    {
        if( 0 == configIn->thRes)
        {
            configOut->thRes = 0;
            configOut->checkRes = false;
        }
        else if((0 <= configIn->thRes) && (TH_RES_WINDSHIELD >= configIn->thRes))
        {
            configOut->thRes = configIn->thRes;
            configOut->checkRes = true;
        }
        else
        {
            iPrintf("\r\nWindshield resistance is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->thRes = TH_RES_WINDSHIELD;
                configOut->checkRes = true;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thVoltagePeak)
    {
        if( 0 == configIn->thVoltagePeak)
        {
            configOut->thVoltagePeak = 0;
            configOut->checkVoltage = false;
        }
        else if((0 <= configIn->thVoltagePeak) && (TH_VOLTAGE_IN_PEAK >= configIn->thVoltagePeak))
        {
            configOut->thVoltagePeak = configIn->thVoltagePeak;
            configOut->checkVoltage = true;
        }
        else
        {
            iPrintf("\r\nVoltage peak is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->thVoltagePeak = TH_VOLTAGE_IN_PEAK;
                configOut->checkVoltage = true;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thVoltageRms)
    {
        if( 0 == configIn->thVoltageRms)
        {
            configOut->thVoltageRms = 0;
            configOut->checkVoltage = false;
        }
        else if((0 <= configIn->thVoltageRms) && (TH_VOLTAGE_IN_RMS >= configIn->thVoltageRms))
        {
            configOut->thVoltageRms = configIn->thVoltageRms;
            configOut->checkVoltage = true;
        }
        else
        {
            iPrintf("\r\nVoltage rms is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->thVoltageRms = TH_VOLTAGE_IN_RMS;
                configOut->checkVoltage = true;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thCurrentPeak)
    {
        if( 0 == configIn->thCurrentPeak)
        {
            configOut->thCurrentPeak = 0;
            configOut->checkCurrent = false;
        }
        else if((0 <= configIn->thCurrentPeak) && (TH_CURRENT_IN_PEAK >= configIn->thCurrentPeak))
        {
            configOut->thCurrentPeak = configIn->thCurrentPeak;
            configOut->checkCurrent = true;
        }
        else
        {
            iPrintf("\r\nCurrent peak is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->thCurrentPeak = TH_CURRENT_IN_PEAK;
                configOut->checkVoltage = true;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thCurrentRms)
    {
        if( 0 == configIn->thCurrentRms)
        {
            configOut->thCurrentRms = 0;
            configOut->checkCurrent = false;
        }
        else if((0 <= configIn->thCurrentRms) && (TH_CURRENT_IN_RMS >= configIn->thCurrentRms))
        {
            configOut->thCurrentRms = configIn->thCurrentRms;
            configOut->checkCurrent = true;
        }
        else
        {
            iPrintf("\r\nCurrent rms is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->thCurrentRms = TH_CURRENT_IN_RMS;
                configOut->checkVoltage = true;
            }
            status = false;
        }
    }

    return status;
}
#endif	/* #ifdef PROJECT_NAVY */

#ifdef PROJECT_ID4
struct petdConfig *petdConfig_get(void)
{
    return &petdUartConfig;
}

void petdConfig_print(void)
{
    iPrintf("\r\nPETD configuration:");

    iPrintf("\r\nControl mode:\t\t[ %s ]", petdUartConfig.modeControl ? "Close" : "Open");
    if(MODE_CTRL_OPEN_LOOP == petdUartConfig.modeControl)
    {
        iPrintf("\r\nFinal phase shift: \t[ %d ]°", petdUartConfig.psTarget);
    }
    else
    {
        iPrintf("\r\nDesired Output Voltage: [ %d ] V", petdUartConfig.VoutTarget);
    }
    iPrintf("\r\nDesired Run Time: \t[ %d ] s,\t%s", petdUartConfig.runtime, (petdUartConfig.forceRuntime)?"Enabled": "Disabled");
    if(0 == petdUartConfig.runtime) {
        iPrintf("\r\nAdditional run Time: \t[ %d ] s", petdUartConfig.addRuntime);
    }
    iPrintf("\r\nTransfo Temp. threshold: [ %d ] °C,\t%s", petdUartConfig.thTempTransfo, (petdUartConfig.checkTempransfo)?"Enabled":"Disabled");
    iPrintf("\r\nHigh Voltage check (250V-470V): \t%s", petdUartConfig.checkHV?"Enabled":"Disabled");
    iPrintf("\r\nResistance check: [ %d-%d ] mOhms, \t%s", petdUartConfig.thReslow, petdUartConfig.thReshigh, petdUartConfig.checkRes?"Enabled":"Disabled");
    iPrintf("\r\nCurrent output check: \t[ %d ] A, \t%s", petdUartConfig.thCout, petdUartConfig.checkCout?"Enabled":"Disabled");
    iPrintf("\r\n");
}

/* Function to check the config and assigned it accordingly
 * @ configIn: config inputs
 * @ configOut: config acquired
 * @ ignore: if true, ignore the input and keep the original value; if false, assign the default value
 * if the config value is 0, this parameter control is disabled.
 */
bool petdConfig_check(struct petdConfig *configIn, struct petdConfig *configOut, bool ignore)
{
    bool status;

    configOut->modeControl = configIn->modeControl;

    if(MODE_CTRL_OPEN_LOOP == configOut->modeControl)
    {
        if(INVALID_INPUT != configIn->psTarget)
        {
            if((PHASESHIFT_MIN <= configIn->psTarget) && (PHASESHIFT_MAX >= configIn->psTarget))
            {
                configOut->psTarget = configIn->psTarget;
            }
            else
            {
                iPrintf("\r\nPhase shift is out of range, ignored!");
                if(!ignore)
                {
                    configOut->psTarget = DEFAULT_PHASESHIFT_TARGET;
                }
            }
        }
    }
    else
    {
        if(INVALID_INPUT != configIn->VoutTarget)
        {
            if((VOLTAGE_OUTPUT_MIN <= configIn->VoutTarget) && (VOLTAGE_OUTPUT_MAX >= configIn->VoutTarget))
            {
                configOut->VoutTarget = configIn->VoutTarget;
            }
            else
            {
                iPrintf("\r\nOutput Voltage is out of range, ignored!");
                if(!ignore)
                {
                    configOut->VoutTarget = DEFAULT_VOUTPUT_TARGET;
                }
                status = false;
            }
        }
    }

    if(INVALID_INPUT != configIn->runtime)
    {
        if (0 == configIn->runtime)
        {
            configOut->runtime = 0;
            configOut->forceRuntime = false;
        }
        else if((RUN_TIME_MIN < configIn->runtime) && (RUN_TIME_MAX >= configIn->runtime))
        {
            configOut->runtime = configIn->runtime;
            configOut->forceRuntime = true;
        }
        else
        {
            iPrintf("\r\nRun Time is out of range, ignored!");
            if(!ignore)
            {
                configOut->runtime = DEFAULT_RUN_TIME;
                configOut->forceRuntime = false;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->addRuntime)
    {
        if((0 < configIn->addRuntime) && (ADD_RUNTIME_MAX >= configIn->addRuntime))
        {
            configOut->addRuntime = configIn->addRuntime;
        }
        else
        {
            iPrintf("\r\nAdditional Run Time is out of range, ignored!\r\n");
            if(!ignore)
            {
                configOut->addRuntime = DEFAULT_ADD_RUNTIME;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thTempTransfo)
    {
        if( 0 == configIn->thTempTransfo)
        {
            configOut->thTempTransfo = 0;
            configOut->checkTempransfo = false;
        }
        else if((TEMP_TRANSFO_MIN <= configIn->thTempTransfo) && (TEMP_TRANSFO_MAX >= configIn->thTempTransfo))
        {
            configOut->thTempTransfo = configIn->thTempTransfo;
            configOut->checkTempransfo = true;
        }
        else
        {
            iPrintf("\r\nTemp transfo is out of range, ignored!");
            if(!ignore)
            {
                configOut->thTempTransfo = TEMP_TRANSFORMER_RUN;
                configOut->checkTempransfo = true;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->checkHV)
    {
        if(0 == configIn->checkHV)
        {
            configOut->checkHV = false;
        }
        else if(1 == configIn->checkHV)
        {
            configOut->checkHV = true;
        }
        else
        {
            iPrintf("\r\nHV input invalid, ignored!");
            if(!ignore)
            {
                configOut->checkHV = true;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thReslow)
    {
        if(0 == configIn->thReslow)
        {
            configOut->thReslow = 0;
            configOut->checkRes = false;
        }
        else if((WINDSHIELD_RES_LOW_MIN < configIn->thReslow) && (WINDSHIELD_RES_LOW_MAX > configIn->thReslow))
        {
            configOut->thReslow = configIn->thReslow;
            configOut->checkRes = true;
        }
        else
        {
            iPrintf("\r\nRes low threshold out of range, ignored!");
            if(!ignore)
            {
                configOut->thReslow = WINDSHIELD_RES_MIN;
                configOut->checkRes = true;
            }
            status = false;
        }
    }

    if(INVALID_INPUT != configIn->thReshigh)
    {
        if((WINDSHIELD_RES_HIGH_MAX > configIn->thReshigh) && (WINDSHIELD_RES_HIGH_MIN < configIn->thReshigh))
        {
            configOut->thReshigh = configIn->thReshigh;
        }
        else
        {
            iPrintf("\r\nRes high threshold out of range, ignored!");
            if(!ignore)
            {
                configOut->thReslow = WINDSHIELD_RES_MAX;
            }
            status = false;
        }
    }

    if((configOut->checkRes) && (configOut->thReshigh <= configOut->thReslow))
    {
        configOut->thReslow = WINDSHIELD_RES_MIN;
        configOut->thReslow = WINDSHIELD_RES_MAX;
    }

    if(INVALID_INPUT != configIn->thCout)
    {
        if(0 == configIn->thCout)
        {
            configOut->thCout = 0;
            configOut->checkCout = false;
        }
        else if((CURRETNT_OUTPUT_MAX > configIn->thCout) && (CURRETNT_OUTPUT_MIN < configIn->thCout))
        {
            configOut->thCout = configIn->thCout;
            configOut->checkCout = true;
        }
        else
        {
            iPrintf("\r\nCurrent threshold out of range, ignored!");
            if(!ignore)
            {
                configOut->thCout = CURRETNT_OUTPUT_MAX;
                configOut->checkCout = true;
            }
            status = false;
        }
    }

    return status;
}

void pwmConfig_print(void)
{
    iPrintf("\r\nPWM configuration:");
    iPrintf("\r\nFreq: %dKHz", (int)(pwmUartConfig.freq/1000));
    iPrintf("\r\nDeadband RED1: %dns", pwmUartConfig.deadband_red1 * 10);
    iPrintf("\r\nDeadband FED1: %dns", pwmUartConfig.deadband_fed1 * 10);
    iPrintf("\r\nDeadband RED2: %dns", pwmUartConfig.deadband_red2 * 10);
    iPrintf("\r\nDeadband FED2: %dns", pwmUartConfig.deadband_fed2 * 10);
//    iPrintf("\r\nCompensation PWM3A DOWN: %dns", pwmUartConfig.compensation_down3 * 10);
//    iPrintf("\r\nCompensation PWM4A DOWN: %dns", pwmUartConfig.compensation_down4 * 10);
    iPrintf("\r\nCompensation PWM3A UP: %dns", pwmUartConfig.compensation_up3 * 10);
    iPrintf("\r\nCompensation PWM4A UP: %dns", pwmUartConfig.compensation_up4 * 10);
    iPrintf("\r\nCompensation PWM3A Lik: \t%4.2f uH", (float)pwmUartConfig.compensation_lik / 1000);
    iPrintf("\r\nCompensation PWM4A n: \t\t%3.1f", (float)pwmUartConfig.compensation_n / 10);
    iPrintf("\r\nCompensation PWM3A Rload: \t%4.2f Ohms\r\n", (float)pwmUartConfig.compensation_rload /1000);
}

struct pwmConfig* pwmConfig_get(void)
{
    return &pwmUartConfig;
}

bool pwmConfig_check(struct pwmConfig *configIn, struct pwmConfig *configOut, bool ignore)
{
    if((FREQ_INIT_MIN <= configIn->freq) && (FREQ_INIT_MAX >= configIn->freq))
    {
        configOut->freq = configIn->freq;
    }
    else
    {
        if(!ignore)
        {
            configOut->freq = FREQ_INIT;
        }
    }

    if((DEADBAND_MIN <= configIn->deadband_red1) && (DEADBAND_MAX >= configIn->deadband_red1))
    {
        configOut->deadband_red1 = configIn->deadband_red1;
    }
    else
    {
        if(!ignore)
        {
            configOut->deadband_red1 = DEADBAND_RED_PWM1;
        }
    }

    if((DEADBAND_MIN <= configIn->deadband_fed1) && (DEADBAND_MAX >= configIn->deadband_fed1))
    {
        configOut->deadband_fed1 = configIn->deadband_fed1;
    }
    else
    {
        if(!ignore)
        {
            configOut->deadband_fed1 = DEADBAND_FED_PWM1;
        }
    }

    if((DEADBAND_MIN <= configIn->deadband_red2) && (DEADBAND_MAX >= configIn->deadband_red2))
    {
        configOut->deadband_red2 = configIn->deadband_red2;
    }
    else
    {
        if(!ignore)
        {
            configOut->deadband_red2 = DEADBAND_RED_PWM2;
        }
    }

    if((DEADBAND_MIN <= configIn->deadband_fed2) && (DEADBAND_MAX >= configIn->deadband_fed2))
    {
        configOut->deadband_fed2 = configIn->deadband_fed2;
    }
    else
    {
        if(!ignore)
        {
            configOut->deadband_fed2 = DEADBAND_FED_PWM2;
        }
    }

    if((COMPENSATION_LIK_MIN <= configIn->compensation_lik) && (COMPENSATION_LIK_MAX >= configIn->compensation_lik))
    {
        configOut->compensation_lik = configIn->compensation_lik;
    }
    else
    {
        if(!ignore)
        {
            configOut->compensation_lik = COMPENSATION_LIK_DEFAULT;
        }
    }

    if((COMPENSATION_N_MIN <= configIn->compensation_n) && (COMPENSATION_N_MAX >= configIn->compensation_n))
    {
        configOut->compensation_n = configIn->compensation_n;
    }
    else
    {
        if(!ignore)
        {
            configOut->compensation_n = COMPENSATION_N_DEFAULT;
        }
    }

    if((COMPENSATION_RLOAD_MIN <= configIn->compensation_rload) && (COMPENSATION_RLOAD_MAX >= configIn->compensation_rload))
    {
        configOut->compensation_rload = configIn->compensation_rload;
    }
    else
    {
        if(!ignore)
        {
            configOut->compensation_rload = COMPENSATION_RLOAD_DEFAULT;
        }
    }

    if((DEADBAND_MIN <= configIn->compensation_up3) && (DEADBAND_MAX >= configIn->compensation_up3))
    {
        configOut->compensation_up3 = configIn->compensation_up3;
    }
    else
    {
        if(!ignore)
        {
            configOut->compensation_up3 = ADJUSTMENT_3_RISING;
        }
    }

    if((DEADBAND_MIN <= configIn->compensation_up4) && (DEADBAND_MAX >= configIn->compensation_up4))
    {
        configOut->compensation_up4 = configIn->compensation_up4;
    }
    else
    {
        if(!ignore)
        {
            configOut->compensation_up4 = ADJUSTMENT_4_RISING;
        }
    }

    return true;
}

#endif	/* #ifdef PROJECT_ID4 */

