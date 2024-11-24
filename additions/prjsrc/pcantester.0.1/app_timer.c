/*
 * app_timer.c
 *
 *  Created on: Jan 25, 2024
 *      Author: crane
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "utility.h"
#include "app_canfd.h"

//#define TEST_USE_SIG_STOP

// Custom data structure to pass arguments to the handler
struct TimerHandlerArgs {
	msg_opt_t (*msgid)[CAN_VEH_MSG_NUM];		/* the message array pointer */
    int index;					/* the message index for a specific message */
    int timer_total;			/* the message array size */
    int mode[CAN_VEH_MSG_NUM];
    int interval[CAN_VEH_MSG_NUM];
    int num[CAN_VEH_MSG_NUM];
};
static struct TimerHandlerArgs args;

void timer_handler(int signo, siginfo_t *info, void *context) {
    // periodic task goes here, using the passed arguments
	/* get the argument the structure array pointer */
	struct TimerHandlerArgs *temp = &args;
    timer_t *tidp;
    tidp = info->si_value.sival_ptr;
    int msgindex;
    for(int i=0; i<temp->timer_total; i++) {
    	/* search for the msg index according to the timerid of the expired timer */
    	ndPrintf("%d %d ", (int)(*tidp), (int)((*temp->msgid)[i].timer_id));
    	if(*tidp == (*temp->msgid)[i].timer_id) {
    		msgindex = i;
    		dPrintf("%d |", msgindex);
    		break;
    	}
    }

	for(int i=0; i<temp->timer_total; i++) {
		ndPrintf("%d \t%5d \t%5d \t%d | ", i, (*temp->msgid)[i].mode, (*temp->msgid)[i].interval, (*temp->msgid)[i].num);
	}							/* This way it works to get the correct value */
	ndPrintf("\n");

	struct canfdData *candata = msg_canfd_getData();
	msg_canfd_send_veh((uint8_t)(*temp->msgid)[msgindex].mode, (*temp->msgid)[msgindex].val);
	for(int i=0; i<temp->timer_total; i++) {
		ndPrintf("%d \t%5d \t%5d \t%d | ", i, (*temp->msgid)[i].mode, (*temp->msgid)[i].interval, (*temp->msgid)[i].num);
#ifdef PROJECT_ID4
		iPrintf("%s: %3d | ", candata->id4_vehData[(*temp->msgid)[i].mode].name, (*temp->msgid)[i].timer_count);
#endif
#ifdef PROJECT_C3
		iPrintf("%s: %X |", candata->c3canDataInfo[(*temp->msgid)[i].mode].name, (*temp->msgid)[i].timer_count);
#endif
	}
	(*temp->msgid)[msgindex].timer_count++;

	static int count = 0;
#if 1	/* display in the same line */
	iPrintf("%X\r", count++);	fflush(stdout);		/* It is relevant to the screen width */
#else	/* change line to display */
	iPrintf("[%d]\n", count++);
#endif
}

void start_timer(msg_opt_t (*appid)[CAN_VEH_MSG_NUM], int index, int total) {
    struct sigevent sev;
    int sec, ms;

    ndPrintf("Start_timer: %p %p %p %d %d\n", timerid, appid, &appid[index]->timer_id, index, total);
	for(int i=0; i<total; i++) {
		ndPrintf("%d \t%d \t%d \t%d | ", i, (*appid)[i].mode, (*appid)[i].interval, (*appid)[i].num);
	}
	ndPrintf("\n");

    sec = (*appid)[index].interval / 1000;
    ms = (*appid)[index].interval % 1000;
    ndPrintf("\nstart_timer: %d %d %d", (*appid)[index]->interval, sec, ms);

    // Set up the timer handler function
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timer_handler;
    sigaction(SIGALRM, &sa, NULL);

    // Create a timer
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;

    // Create and initialize custom data structure to pass arguments
    args.msgid = appid;
    args.index = index;
    args.timer_total = total;
    sev.sigev_value.sival_ptr = &((*appid)[index].timer_id);

    ndPrintf("Create timer: %p %p %p\n", &sev, timerid, &msgdata[index]->timer_id);
    if (1 == timer_create(CLOCK_REALTIME, &sev, &((*appid)[index].timer_id))) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    struct itimerspec its;
    its.it_value.tv_sec = sec;
    its.it_value.tv_nsec = ms * 1000000;
    its.it_interval.tv_sec = sec;
    its.it_interval.tv_nsec = ms * 1000000;

    if (-1 == timer_settime((*appid)[index].timer_id, 0, &its, NULL)) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}

void start_singletimer(timer_t *timerid, int interval, void (*handler)(void)) {
    struct sigevent sev;
    int sec, ms;

    ndPrintf("\nStart single timer: %p %p %d", timerid, handler, interval);

    sec = interval / 1000;
    ms = interval % 1000;
    ndPrintf("\nStart single timer: %d %d %d", interval, sec, ms);

    // Set up the timer handler function
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGALRM, &sa, NULL);

    // Create a timer
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGVTALRM;

    // Create and initialize custom data structure to pass arguments
    sev.sigev_value.sival_ptr = handler;

    ndPrintf("\nCreate timer: %p %p %p\n", &sev, timerid, handler);
    if (1 == timer_create(CLOCK_REALTIME, &sev, timerid)) {
        perror("timer_create");
        exit(EXIT_FAILURE);
    }

    struct itimerspec its;
    its.it_value.tv_sec = sec;
    its.it_value.tv_nsec = ms * 1000000;
    its.it_interval.tv_sec = sec;
    its.it_interval.tv_nsec = ms * 1000000;

    if (-1 == timer_settime(*timerid, 0, &its, NULL)) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}

#ifdef TEST_USE_SIG_STOP
void stop_timer(int signo) {
	timer_t timerid;

	if(SIGUSR2 == signo) {
		timerid = timerSend;
	}

    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    dPrintf("\nstop_timer: timerid %d, its %p\n", (int)timerid, &its);
    //while(1) { ;}
    if (-1 == timer_settime(timerid, 0, &its, NULL)) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}
#else
void stop_timer(timer_t *timerid) {
    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    ndPrintf("\nstop_timer: timerid %p, its %p\n", timerid, &its);
    if (-1 == timer_settime(*timerid, 0, &its, NULL)) {
        perror("timer_settime");
        exit(EXIT_FAILURE);
    }
}
#endif	/* #ifdef TEST_USE_SIG_STOP */

void delete_timer(timer_t *timerid) {
    // Clean up the timer
    if (-1 == timer_delete(*timerid)) {
        perror("timer_delete");
        exit(EXIT_FAILURE);
    }
}
