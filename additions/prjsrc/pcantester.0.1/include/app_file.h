#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

bool fileWrite_Word(FILE *dev, uint16_t addr, uint16_t value);
bool fileRead_Word(FILE *dev, uint16_t addr, uint16_t *value);
bool fileWrite_Byte(FILE *dev, uint16_t addr, uint8_t value);
bool fileRead_Byte(FILE *dev, uint16_t addr, uint8_t *value);
