#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#include "sysconfig.h"
#include "utility.h"
#include "app_canfd.h"
#include "app_config.h"
#include "app_log.h"

#define DEFAULT_RUN_TIME			60		/* default running time seconds */
#define DEFAULT_MSG_INTERVAL		1000	/* default interval between same messages in ms; not useful any more as each message has its own */

typedef struct {
	char *dev;
	uint32_t addr;
	int16_t val;
	uint16_t num;			/* loop number */
	uint16_t interval;		/* interval in ms between each message submission */
	uint16_t period;		/* total running time in seconds */
	uint16_t option;
	msg_mode_t mode;
} app_opt_t;

static void help(const char *app) {
	fprintf(stderr,
		"Usage: %s options <param>\n"
		"Options:\n"
		"\t -l <number>: loop numbers (must option, 0 for infinite loop).\n"
		"\t -i <number>: interval time (millisecond, default in source code). \n"
		"\t -p <number>: total running time (seconds, default 60, 0 for continuous run).\n"
		"\t -c <number>: Send Start of Charge (percentage).\n"
		"\t -f <number>: Send FSH Status (0 or 1).\n"
		"\t -a <number>: Send Ambient Temperature (celsius degree).\n"
		"\t -v <number>: Send Battery Voltage (V).\n"
		"\t -s <number>: Send Speed (Km/h).\n"
		"\t -t <number>: Send Cabin Temperature (celsius degree).\n"
		"\t -h <number>: Send Humidity (percentage).\n"
#ifdef PROJECT_C3
		"\t -d <number>: Send System ID.\n"
		"\t -r <number>: Send Current (V).\n"
		"\t -o <number>: Send Op Mode (0 or 1).\n"
#endif
		"\t -u: display usage information.\n",
		app);
}

volatile sig_atomic_t print_flag = false;
timer_t timer_display;
int timer_display_ori = 0;

msg_opt_t msg[CAN_VEH_MSG_NUM] = {
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
#ifdef PROJECT_C3
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
	{ .timer_count = 0, .timer_mark = true },
#endif
};
int timer_num = 0;

struct sysData {
	struct canfdData *dataCan;
};

struct sysData farview_data;
int canfd_status = -1;

int app_commandP(void);
int app_commandD_log(void);
int app_commandD_error(void);
void app_displayData(struct sysData *sysdata, int count);
void app_displayLog(struct sysData *sysdata);
void display_handler(int signo, siginfo_t *info);

void enable_display(int signum) {
	print_flag = true;
}

void start_timer(msg_opt_t (*appid)[CAN_VEH_MSG_NUM], int index, int total);
void start_singletimer(timer_t *timerid, int interval, void (*handler)());
void stop_timer(timer_t *timerid);
void delete_timer(timer_t *timerid);


/**************************************************************************************
 * 						main program
 **************************************************************************************/
int main(int argc, char *argv[]) {
	int i, ret = 0;
    unsigned char input_value = 0;

    time_t time_ori;
	app_opt_t opt;
	opt.val = 0;
	opt.num = 0;
	opt.interval = DEFAULT_MSG_INTERVAL;
	opt.period = 0;
	opt.option = 0xFF;
	opt.mode = APP_OPT_UNKNOWN;

	/* get the option and parameters if needed (followed with :) */
//	dPrintf("\r\nGetting %d arguments and the option is %d\n", argc, ret);
	while(-1 != (ret = getopt(argc, argv, "l:i:p:f:a:v:c:s:t:h:d:r:o:u"))) {
		ndPrintf("\r\nGet %d arguments and the option is %c\n", argc, ret);
		switch(ret) {
			case 'l':
				opt.num = atoi(optarg);
				break;
			case 'i':
				opt.interval = atoi(optarg);
				break;
			case 'p':
				opt.period = atoi(optarg);
				break;
			case 'f':
				opt.mode = APP_OPT_DEV_SEND_FSH;
				opt.val = atoi(optarg);
				if(1 >= opt.val) {
					dData.display = opt.val + 1;
					dData.number = opt.val;
				}
				else {
					help(argv[0]);
				}
				ndPrintf("\r\nFSH Sts:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'a':
				opt.mode = APP_OPT_DEV_SEND_ATEMP;
				opt.val = atoi(optarg);
				dData.display = 3;
				dData.number = opt.val;
				ndPrintf("\r\nAmb.Temp.:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'v':
				opt.mode = APP_OPT_DEV_SEND_VOLTAGE;
				opt.val = atoi(optarg);
				ndPrintf("\r\nVoltage:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'c':
				opt.mode = APP_OPT_DEV_SEND_SOC;
				opt.val = atoi(optarg);
				ndPrintf("\r\nS.Charge:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 's':
				opt.mode = APP_OPT_DEV_SEND_SPEED;
				opt.val = atoi(optarg);
				ndPrintf("\r\nV. Speed:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 't':
				opt.mode = APP_OPT_DEV_SEND_CTEMP;
				opt.val = atoi(optarg);
				ndPrintf("\r\nCab Temp.:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'h':
				opt.mode = APP_OPT_DEV_SEND_HUMIDITY;
				opt.val = atoi(optarg);
				ndPrintf("\r\nHumidity:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
#ifdef PROJECT_C3
			case 'd':
				opt.mode = APP_OPT_DEV_SEND_SYSID;
				opt.val = atoi(optarg);
				ndPrintf("\r\nSystem ID:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'r':
				opt.mode = APP_OPT_DEV_SEND_CURRENT;
				opt.val = atoi(optarg);
				ndPrintf("\r\nCurrent:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
			case 'o':
				opt.mode = APP_OPT_DEV_SEND_OPMODE;
				opt.val = atoi(optarg);
				ndPrintf("\r\nOp.mode:\tNo.%d mode %d", timer_num, opt.mode);
				msg[timer_num].mode = opt.mode;
				msg[timer_num].val = opt.val;
				timer_num++;
				break;
#endif
			case 'u':
				help(argv[0]);
				return 0;
				//break;
			case '?':
				// unknown option or missing argument
				iPrintf("Unknown option or missing argument!\n");
				return 0;
				//break;
			default:							/* it won't come here if there is no argument at all, why? */
				opt.mode = APP_OPT_UNKNOWN;
				//help(argv[0]);
				//return -1;
				break;
		}
	}

#ifdef DEVELOP_VERSION
    dPrintf("RPI: uint8_t: %d | uint16_t: %d | uint32_t: %d\n", \
            (int)sizeof(uint8_t), (int)sizeof(uint16_t), (int)sizeof(uint32_t));
    dPrintf("RPI: bool: %d | short: %d | int: %d | long int: %d | float: %d\n", \
    		(int)sizeof(bool), (int)sizeof(short), (int)sizeof(int), (int)sizeof(long int), (int)sizeof(float));
    dPrintf("RPI: struct petdConfig: %d | struct pwmConfig: %d | struct id4DataIn_Cfg_type: %d\n", \
    		PETD_CONFIG_LEN, PWM_CONFIG_LEN, CAN_DATA_IN_CFG_LEN);
#endif

	ndPrintf("\r\nMsg No. %d - %d %d %d \r\n", opt.mode, opt.val, opt.interval, opt.num);
	ndPrintf("Mode: %d | %d * %d | Total msg #: %d\n", opt.mode, opt.interval, opt.num, timer_num);

	/* init devices */
	if(msg_canfd_init()) {
		canfd_status = 0;
		iPrintf("\r\nCAN functions are available!\n");
	}
	else {
		canfd_status = -1;//0;//
		iPrintf("\r\nCAN functions are NOT available!\n");
	}

	dataLog_init();

	/* init data */
	farview_data.dataCan = msg_canfd_getData();

	/* init timer for display */
	//start_singletimer(&timer_display, 1000, display_handler);
	timer_display_ori = time(NULL);

	/* get the function selection */
    if(APP_OPT_UNKNOWN == opt.mode) {
		input_value = app_config();

//    	if(2 == input_value) {
//    		/* init timer */
//    		signal(SIGALRM, enable_display);
//    		alarm(1);
//    	}

		/* send CAN messages */
		if((0 == canfd_status) && (farview_data.dataCan->updated)) {
			ndPrintf("\n Sending data '%c' to CAN ...", input_value);
			msg_canfd_send_tester(input_value);
			ndPrintf("\n data '%c' to CAN sent!", input_value);
			farview_data.dataCan->updated = false;
		}
    }
    else
    /* start the timer to control the submission of vehicle CAN messages */
    {
		dPrintf("\nTotal msg #: %d", timer_num);
		ndPrintf("\nMsg name\tNo.\tValue\t | interval\tnum\n");		/* when controlling loop number of every single message */
		iPrintf("\nMsg name\tNo.\tValue\t | interval(ms)\n");

#ifdef PROJECT_ID4
    	for(int i=0; i<timer_num; i++) {
			/* use the default interval and total number */
			msg[i].interval = farview_data.dataCan->id4_vehData[msg[i].mode].interval;
			msg[i].num = farview_data.dataCan->id4_vehData[msg[i].mode].num;
			/* use the command interval and total number */
			//msg[i].interval = opt.interval;
			msg[i].num = opt.num;

			/* start the timer */
			ndPrintf("Call to create timer: %p %p\n", &msg, &msg[i].timer_id);
			//start_timer(&msg, i, timer_num);

			ndPrintf("%s:\t%d\t%d\t | %d\t%d \n", farview_data.dataCan->id4_vehData[msg[i].mode].name, \
					msg[i].mode + 1, msg[i].val, \
					msg[i].interval, msg[i].num);					/* when controlling loop number of every single message */
			iPrintf("%s:\t%d\t%d\t | %d\n", farview_data.dataCan->id4_vehData[msg[i].mode].name, \
					msg[i].mode + 1, msg[i].val, msg[i].interval);
    	}
#endif
#ifdef PROJECT_C3
    	for(int i=0; i<timer_num; i++) {
			/* use the default interval and total number specified in message control data */
			msg[i].interval = farview_data.dataCan->c3canDataInfo[msg[i].mode].interval;
			msg[i].num = farview_data.dataCan->c3canDataInfo[msg[i].mode].num;
			/* use the interval and total number from command line */
			//msg[i].interval = opt.interval;
			msg[i].num = opt.num;

			/* start the timer */
			ndPrintf("Call to create timer: %p %p\n", &msg, &msg[i].timer_id);
			//start_timer(&msg, i, timer_num);

			iPrintf("%s:\t%d\t%d\t | %d\n", farview_data.dataCan->c3canDataInfo[msg[i].mode].name, msg[i].mode + 1, msg[i].val, msg[i].interval);
    	}
#endif
    	for(int i=0; i<timer_num; i++) {
    		start_timer(&msg, i, timer_num);
    	}

    	time_ori = time(NULL);
    }

	ndPrintf("\nStart the main loop... input value is %d", input_value);
	//for(;;) {}
	for(;;) {
		if((0 != opt.period) && (opt.period < (time(NULL) - time_ori))) {
			printf("\n");
			return 0;
		}

		if((COMMAND_C == input_value) || (COMMAND_F == input_value)) {
			/* just return */
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}

#ifdef PROJECT_ID4
		if(COMMAND_P == input_value) {
			app_commandP();
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}

		if((COMMAND_D == input_value) && (444 == dData.display)) {
			/* receive log data */
			app_commandD_log();
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}
#endif

		if((COMMAND_D == input_value) && (333 == dData.display)) {
			/* receive error data */
			app_commandD_error();
    		if(APP_OPT_UNKNOWN == opt.mode) { printf("\n"); return ret; }
		}

//		if(COMMAND_S == input_value) {
		if(APP_OPT_UNKNOWN != opt.mode) {
			static int timer_stopped = 0;
    		for(int i=0; i<timer_num; i++) {
    			/* search all the timer to check if any one completed the submission */
        		if( (msg[i].timer_count >= msg[i].num) && (0 != msg[i].num) ) {
        			if(msg[i].timer_mark) {
        				/* if this timer is not stopped, stop it */
        				stop_timer(&msg[i].timer_id);

        				/* mark it after it is stopped */
            			msg[i].timer_mark = false;

            			/* update the total number of the timer left */
            			timer_stopped++;
                		ndPrintf(" | stopped %d\n", timer_stopped);
        			}
        		}
    		}

    		if(timer_num == timer_stopped) {
    			/* all the submission is complete, return */
    			printf("\n");
    			return ret;
    		}
		}

		if(2 == input_value) {
			/* receive CAN messages */
			if(0 == canfd_status) {
				msg_canfd_receive();
			}

//			if(print_flag) {
//				/* display data */
//				app_displayData(&farview_data, i);
//				/* reset the flag */
//				print_flag = false;
//
//				/* reset the alarm in seconds */
//				alarm(1);
//			}
			if(1 < (time(NULL) - timer_display_ori)) {
				app_displayData(&farview_data, i);
				timer_display_ori = time(NULL);
			}
		}

		//debugPrintf("Cycle count: %d", i); if(70 == i++) for(;;) {;}
		if(60 == i++) dPrintf("\n"); // How does it affects the printing?
	}

	return ret;
}

#ifdef PROJECT_ID4
int app_commandP(void)
{
	ndPrintf("\r\n print command sent out");
	int status = msg_canfd_rcvCanConfigs();
		if(0 == status) {
			ndPrintf("\r\n Config received");
		struct canfdData *temp;
		temp = msg_canfd_getData();
		pwmConfig_check(&temp->id4DataIn_Cfg.data.pwmCanConfig, pwmConfig_get(), false);
		pwmConfig_get()->configUpdated = true;
		petdConfig_check(&temp->id4DataIn_Cfg.data.petdCanConfig, petdConfig_get(), false);
		petdConfig_get()->configUpdated = true;
		pwmConfig_print();
		petdConfig_print();
	}
		getc(stdin);

		return 0;
}

int app_commandD_log(void)
{
	ndPrintf("\r\n request log command sent out");
	iPrintf("\n log data (position, No., values ...):");
	uint16_t position, number;
	int index = -1;
    position = farview_data.dataCan->id4DataOut_command.data.addrStart;
    number = farview_data.dataCan->id4DataOut_command.data.dataNum;

	for(;;) {
		memset((void*)&farview_data.dataCan->id4DataLog.data, 0, CAN_DATA_LOG_LEN);
		//iPrintf("\n Start receiving log from %d of %d - %d", index, number, farview_data.dataCan->id4DataLog.data.index);
		int status = msg_canfd_rcvCanLog();
		if(0 == status) {
			/* check if it is a new data: temporary use,
			 * not good enough for the second round
			 * as there are residual data from CAN bus */
			if(index < farview_data.dataCan->id4DataLog.data.index) {
				/* new data */
				app_displayLog(&farview_data);
				index = farview_data.dataCan->id4DataLog.data.index;
				iPrintf("\t%d of %d", index, number);

				/* check if all data are received */
    			if(number-1 <= index) {
    				/* all data are received */
    				iPrintf("\n");
    				//iPrintf("\t break check: %d of %d", index, number);
    				return 0;
    			}
			}
		}
	}
}

void app_displayLog(struct sysData *sysdata)
{
	iPrintf("\n %d\t%d\t%d\t%d\t%d\t%d", \
			sysdata->dataCan->id4DataLog.data.address, \
			sysdata->dataCan->id4DataLog.data.index, \
			sysdata->dataCan->id4DataLog.data.value1, \
			sysdata->dataCan->id4DataLog.data.value2, \
			sysdata->dataCan->id4DataLog.data.value3, \
			sysdata->dataCan->id4DataLog.data.value4 \
			);
}

#endif	/* #ifdef PROJECT_ID4 */

int app_commandD_error(void)
{
	ndPrintf("\r\n request error command sent out");
	iPrintf("\n error data (Error No., parameters ...):");

	return 0;
}

void app_displayData(struct sysData *sysdata, int count)
{
#ifdef PROJECT_ID4
#if 0
    iPrintf(" | %d", sysdata->dataCan->id4Dataveh.fsh);
    iPrintf(" | %4.1fKM/h", sysdata->dataCan->id4Dataveh.speed);
    iPrintf(" | %4.1fV", sysdata->dataCan->id4Dataveh.voltage);
    iPrintf(" | %4.1f%%", sysdata->dataCan->id4Dataveh.stateCharge);
    iPrintf(" | tI %4.1fÂ°C", sysdata->dataCan->id4Dataveh.tempInside);
    iPrintf(" | tO %4.1fÂ°C", sysdata->dataCan->id4Dataveh.tempOutside);
    iPrintf(" | H %4.1f%%", sysdata->dataCan->id4Dataveh.humidity);
#endif
#if 0
    struct canfdData *temp;
    temp = msg_canfd_getData();
    iPrintf("\n");
    for(int i=0; i<CAN_DATA_IN_LEN; i++) {
    	iPrintf("%d ", sysdata->dataCan->id4DataIn.byte[i]);
    	//iPrintf("%d ", temp->id4DataIn.byte[i]);
    }
    //msg_canfd_print();
#endif
#if 0
    iPrintf("\n%s", sysdata->dataCan->id4DataIn.data.mode ? "deIce" : "deFog");
    iPrintf(" | %d", sysdata->dataCan->id4DataIn.data.hitCount);
    iPrintf(" | %d", sysdata->dataCan->id4DataIn.data.veh_fsh);
    iPrintf(" | %3.1fKM/h", (float)sysdata->dataCan->id4DataIn.data.veh_speed / 10);
    iPrintf(" | %3.1fV", (float)sysdata->dataCan->id4DataIn.data.veh_voltage / 10);
    iPrintf(" | %3.1f%", (float)sysdata->dataCan->id4DataIn.data.veh_stateCharge / 10);
    iPrintf(" | tI%3.1fC", (float)sysdata->dataCan->id4DataIn.data.veh_tempInside / 100);
    iPrintf(" | tO%3.1fC", (float)sysdata->dataCan->id4DataIn.data.veh_tempOutside / 100);
    iPrintf(" | H%3.1f%", (float)sysdata->dataCan->id4DataIn.data.veh_humidity / 100);
    iPrintf(" | vI%3.1fV", (float)sysdata->dataCan->id4DataIn.data.vin / 10);
    iPrintf(" | vO%3.1fV", (float)sysdata->dataCan->id4DataIn.data.vout / 10);
    iPrintf(" | cO%3.1fA", (float)sysdata->dataCan->id4DataIn.data.cout / 100);
    iPrintf(" | R%5.3fOhms", (float)sysdata->dataCan->id4DataIn.data.res / 1000);
    iPrintf(" | Tru%ds", sysdata->dataCan->id4DataIn.data.petdRuntime);
    iPrintf(" | %dns", sysdata->dataCan->id4DataIn.data.pwmCompen);
    iPrintf(" | tT%3.1f", (float)sysdata->dataCan->id4DataIn.data.tempTro / 100);
#endif
#if 1
    iPrintf("\n%s", sysdata->dataCan->id4DataIn_Ctl.data.mode ? "deIce" : "deFog");
    iPrintf(" | %d", sysdata->dataCan->id4DataIn_Ctl.data.hitCount);
    iPrintf(" | %d", sysdata->dataCan->id4DataIn_Ctl.data.veh_fsh);
    iPrintf(" | %3.1fKM/h", (float)sysdata->dataCan->id4DataIn_Env.data.veh_speed / 10);
    iPrintf(" | %3.1fV", (float)sysdata->dataCan->id4DataIn_Env.data.veh_voltage / 10);
    iPrintf(" | %3.1f%%", (float)sysdata->dataCan->id4DataIn_Env.data.veh_stateCharge / 10);
    iPrintf(" | tI%3.1fC", (float)sysdata->dataCan->id4DataIn_Env.data.veh_tempInside / 100);
    iPrintf(" | tO%3.1fC", (float)sysdata->dataCan->id4DataIn_Env.data.veh_tempOutside / 100);
    iPrintf(" | H%3.1f%%", (float)sysdata->dataCan->id4DataIn_Env.data.veh_humidity / 100);
    iPrintf(" | vI%3.1fV", (float)sysdata->dataCan->id4DataIn_Env.data.vin / 10);
    iPrintf(" | vO%3.1fV", (float)sysdata->dataCan->id4DataIn_Res.data.vout / 10);
    iPrintf(" | cO%3.1fA", (float)sysdata->dataCan->id4DataIn_Res.data.cout / 100);
    iPrintf(" | R%5.3fOhms", (float)sysdata->dataCan->id4DataIn_Res.data.res / 1000);
    iPrintf(" | tT%3.1f", (float)sysdata->dataCan->id4DataIn_Res.data.tempTro / 100);
    iPrintf(" | Tru%ds", sysdata->dataCan->id4DataIn_Cfg.data.petdCanConfig.runtime);
    iPrintf(" | %dns\n", sysdata->dataCan->id4DataIn_Cfg.data.pwmCanConfig.compensation_up3);
#endif
#endif	/* #ifdef PROJECT_ID4 */
#ifdef PROJECT_NAVY
    iPrintf("\nSetup: ");
    iPrintf("mode %u, ", sysdata->dataCan->navyIn.data.mode);
    iPrintf("Vp %5.1fV, ", (float)sysdata->dataCan->navyIn.data.voltagePeak / 10);
    iPrintf("Vr %5.1fV, ", (float)sysdata->dataCan->navyIn.data.voltageRms / 100);
    iPrintf("Cp %4.1fA, ", (float)sysdata->dataCan->navyIn.data.currentPeak / 100);
    iPrintf("Cr %4.1fA, ", (float)sysdata->dataCan->navyIn.data.currentRms / 100);
    iPrintf("Rw %4.1fOhms, ", (float)sysdata->dataCan->navyIn.data.resWindshield / 100);
    iPrintf("Tw %4.1f°C | ", (float)sysdata->dataCan->navyIn.data.temp01 / 100);
    iPrintf("State %d, ", sysdata->dataCan->navyIn.data.state);
    iPrintf("%d", count);
#endif	/* #ifdef PROJECT_NAVY */
}

void display_handler(int signo, siginfo_t *info)
{
	ndPrintf(" | display_handler:");
	app_displayData(&farview_data, 0);
}

