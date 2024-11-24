/*
 * dataLog.h
 *
 *  Created on: Apr. 8, 2023
 *      Author: Crane Shao
 */

#ifndef APPLICATION_DATALOG_H_1_
#define APPLICATION_DATALOG_H_1_

#include <stdint.h>

#include "sysconfig.h"
#include "app_config.h"

#define LOG_ERROR_SIZE                  sizeof(struct errorInfo)//17 /* number of 16bit words for one error info */
#define LOG_ERROR_NUMBER                10//0               /* number of error info to be saved */
//#define DATA_LOG_BYTE
//#define DATA_LOG_TIME

/* parameters to log when error occurs */
struct errorParameters {
    bool modeCtrl;
    bool modeOp;
    uint16_t voutSet;
    uint16_t tempAmbient;
    uint16_t runtime;           /* in 10ms */
    uint16_t vin;
    uint16_t vout;
    uint16_t cout;
    uint16_t res;
    int16_t tempTransfo;
    int16_t tempT1;
    int16_t tempT2;
    int16_t tempT3;
    int16_t tempT4;
};

/* error information to log when error occurs */
struct errorInfo {
    uint16_t errno;
    uint16_t runningtime;       /* in 10ms */
    struct errorParameters logParameters;
};

union errInfoUnion {
    struct errorInfo logerrorInfo;
    uint16_t word_16bits[LOG_ERROR_SIZE];
};

struct errInfoLog {
    union errInfoUnion errorLog;
    uint16_t errorCount;            /* total error number (0 - ...) */
    bool writeEnabled;
    uint16_t logPosi;               /* log position (0-9) in error log space of LOG_ERROR_NUMBER */
};

/* Function to write data to FRAM */
void dataLogfram_write(void);

/* Functions to operate log buff: write and read/print */
void dataLog_buff(uint16_t *data);
void dataLogbuff_enable();
void dataLogbuff_disable();
void dataLogbuff_clearfilled();
bool dataLogbuff_print_word(uint16_t addr, uint16_t num);
bool dataLogbuff_print_byte(uint16_t addr, uint16_t num);
uint16_t dataLogbuff_read(void);

/* Functions to log configs and error info to FRAM
 * and read log data from FRAM*/
bool dataLogfram_configsWrite(void);
bool dataLogfram_configDisplay(void);
void dataLogfram_writeEnable_configs(void);
struct errInfoLog *logBuf_error_get(void);
bool dataLogfram_errorWrite(void);
bool dataLogfram_errorDisplay(void);
void dataLogfram_writeEnable_error(void);

/* Functions to read log buff and write to FRAM or vice versa */
void dataLogfram_writeEnable_buff(void);
bool dataLogfram_write2buff_word(void);
bool dataLogfram_readfmbuff_word(uint32_t addr, uint16_t num_No);
bool dataLogfram_readfmbuff_wordn(uint32_t addr, uint16_t num_No);

/* Functions to operate FRAM: write and read/print */
bool dataLogger_writeByte(uint32_t addr, uint16_t num, uint16_t *data);
bool dataLogger_writeWord(uint32_t addr, uint16_t num, uint16_t *data);
bool dataLogger_readWord(uint32_t addr, uint16_t num, uint16_t *data);
bool dataLogger_print_wordn(uint32_t addr, uint16_t num);
bool dataLogger_print_word(uint32_t addr, uint16_t nums);
bool dataLogger_print_byte(uint32_t addr, uint16_t num);

void dataLog_init(void);

#endif /* APPLICATION_DATALOG_H_1_ */
