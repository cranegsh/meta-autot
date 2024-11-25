/*
 * Utility.c
 *
 *  Created on: Jul. 27, 2022
 *      Author: Crane Shao
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "utility.h"
#include "sysconfig.h"

struct debugData dData = {
    .mark = 0,
    .buffer = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .number = 0x3FFFF,
    .display = MAIN_LOOP_DISPLAY_STATUS
};

union time_bcd_t {
    struct {
        uint16_t unit: 4;
        uint16_t ten: 3;
        uint16_t os: 1;
        uint16_t unused: 8;
    } bF;
    uint16_t hword;
};

void print_array_byte(uint8_t *dat, uint16_t num)
{
    uint16_t i;

    for(i=0; i<num; i++)
    {
        printf(" %02X", dat[i] & 0xFF);
    }

    return;
}

void print_array(uint16_t *dat, uint16_t num)
{
    uint16_t i;

    for(i=0; i<num; i++)
    {
        printf(" %02X", dat[i] & 0xFF);
    }

    return;
}

float round_float(float x)
{
    uint32_t temp;
    float value;

    temp = (uint32_t)x;
    value = x - temp;

    if(0.5 <= value)
        return (temp+1);
    else
        return temp;
}

/* Function to convert 32-bit integer to hex string */
bool dec2hex(uint32_t n, char *ans)
{
    // ch variable to store character temporarily
    char ch;
    // i variable to count
    uint16_t i = 0;

    if(0 == n)
    {
        ans[0] = '0'; ans[1]='\0';
        return true;
    }

    while (n != 0) {
        // remainder variable to store remainder
        uint16_t rem = 0;

        // storing remainder in rem variable.
        rem = n % 16;

        // check if temp < 10
        if (rem < 10) {
            ch = rem + '0';
        }
        else {
            ch = rem - 10 + 'A';
        }

        // updating the ans string with the character variable
        ans[i++] = ch;
        n = n / 16;
    }

    // reversing the ans string to get the final result
    i = 0;
    uint16_t j;
    j = strlen(ans) - 1;
    while(i < j)
    {
      // swap ans[i] and ans[j]
      ch = ans[i];
      ans[i] = ans[j];
      ans[j] = ch;
      i++;
      j--;
    }

    return true;
}

/* Function to convert 32-bit integer to hex string */
char* dec2hex_word(uint32_t n, char *ans)
{
    dec2hex(n, ans);
    return ans;
}

/* Function to convert 8-bit integer to hex string */
char* dec2hex_byte(uint16_t n, char *ans)
{
    uint16_t a, a16;
    char cha, cha16;

    n = n&0xFF;

    a = n%16;
    a16 = n/16;

    if (a < 10) {
        cha = a + '0';
    }
    else {
        cha = a - 10 + 'A';
    }
    if (a16 < 10) {
        cha16 = a16 + '0';
    }
    else {
        cha16 = a16 - 10 + 'A';
    }

    ans[0] = cha16;
    ans[1] = cha;
    ans[2] = '\0';

    return ans;
}

uint16_t bcd2bin(uint16_t value)
{
    union time_bcd_t temp;

    temp.hword = value;
    return temp.bF.ten * 10 + temp.bF.unit;
}

uint16_t bin2bcd(uint16_t value)
{
    union time_bcd_t temp;

    temp.bF.ten = value/10;
    temp.bF.unit = value % 10;

    return temp.hword;
}
