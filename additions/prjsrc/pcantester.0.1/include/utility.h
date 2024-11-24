/*
 * Utility.h
 *
 *  Created on: Jul. 27, 2022
 *      Author: Crane Shao
 */

#ifndef MISC_UTILITY_H_
#define MISC_UTILITY_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* For storing the debug information */
#define DEBUG_BUFF_SIZE         100
struct debugData {
    uint32_t mark;
    uint32_t buffer[DEBUG_BUFF_SIZE];
    uint32_t number;
    uint16_t display; /* main loop display: 0 for nothing; 1 for INT counts; 2 for inputs; */
};
extern struct debugData dData;

#define ENABLE_DEBUG_PRINT_DRIVER
#define ENABLE_DEBUG_PRINT_APP
#define ENABLE_INFO_PRINT

#ifdef ENABLE_DEBUG_PRINT_DRIVER
#define drvdebugPrintf(...)  { printf("\r\ndrvDbg - "); printf(__VA_ARGS__); }
#define drvdPrintf(...)  printf(__VA_ARGS__);
#else
#define drvdebugPrintf(...)    // Do nothing
#define drvdPrintf(...)
#endif

#ifdef ENABLE_DEBUG_PRINT_APP
#define debugPrintf(...)  { printf("\r\nDebug - "); printf(__VA_ARGS__); }
#define dPrintf(...)  printf(__VA_ARGS__);
#else
#define debugPrintf(...)    // Do nothing
#define dPrintf(...)
#endif

/* disable printing */
#define ndrvdebugPrintf(...)
#define ndrvdPrintf(...)
#define ndebugPrintf(...)
#define ndPrintf(...)

#ifdef ENABLE_INFO_PRINT
#define infoPrintf(...)  { printf("\r\nInfo - "); printf(__VA_ARGS__); }
#define iPrintf(...)  printf(__VA_ARGS__);
#else
#define infoPrintf(...)    // Do nothing
#define iPrintf(...)
#endif

/* Function to print an array of integers */
void print_array_byte(uint8_t *dat, uint16_t num);
void print_array(uint16_t *dat, uint16_t num);

/* Functions to convert integer and float to a string */
void itostr(char* str, int32_t number, bool flag);
void ftostr(char* str, double number, float bit_num);

/* Functions to calculate */
uint32_t powerof10(uint8_t num);
float round_float(float x);

/* Functions to convert decimal to hex string */
bool dec2hex(uint32_t n, char *ans);
char* dec2hex_word(uint32_t n, char *ans);
char* dec2hex_byte(uint16_t n, char *ans);

uint16_t bcd2bin(uint16_t value);
uint16_t bin2bcd(uint16_t value);

#endif /* MISC_UTILITY_H_ */
