/*
 *
 *
 */
//#define DEVELOP_VERSION

/* project definition */
/* select only one from PROJECT_NAVY, PROJECT_C3 and PROJECT_ID4
 * if PROJECT_ID4 is selected, select only one from PROJECT_CAN_ID4, PROJECT_CAN_BZ4X and PROJECT_CAN_G3 */
//#define PROJECT_NAVY
//#define PROJECT_C3
#define PROJECT_ID4
#ifdef PROJECT_ID4				   /* ID4, BZ4X and G3 all are based on PROJECT_ID4, the differences are CAN messages */
#define PROJECT_CAN_ID4
//#define PROJECT_CAN_BZ4X
//#define PROJECT_CAN_G3
#endif

/* debug configurations */
#define MAIN_LOOP_DISPLAY_DOT           0           /* display only . */
#define MAIN_LOOP_DISPLAY_DEVICE        1           /* display device running status */
#define MAIN_LOOP_DISPLAY_INPUT         2           /* display all input values */
#define MAIN_LOOP_DISPLAY_STATUS        3           /* display system running status */
#define MAIN_LOOP_DISPLAY_CAN           11          /* display CAN incoming messages */

/* for CANFD communication */
#define COMMAND_P						'p'
#define COMMAND_C						'c'
#define COMMAND_F						'f'
#define COMMAND_W						'w'
#define COMMAND_D						'd'
#define COMMAND_S						's'

/* For SPI-CAN devices */
#if 0
#define SPICAN_DEVICE_NODE				"/dev/spidev0.0"
#endif
#if 1
#define SPICAN_DEVICE_NODE				"/dev/spican"
#endif
#define SPI_BIT_RATE                    6000000     // 6M Hz, it affects how fast CAN msg gets processed
#define SPI_DEFAULT_BUFFER_LENGTH       96          // SPI buffer sizes
#define CANFD_MSG_LEN_ID4               8           // all messages are actually 8 bytes (save processing time)
//#define CAN_RX_MODE_INT                             // defined to use interrupt mode; otherwise polling mode

/* For data log */
//#define LOG_IN_FRAM
#ifndef LOG_IN_FRAM									// log data to FRAM for controller
#define LOG_IN_FILE									// log data to file for farview
#endif
#define FRAM_SIZE                   	0x20000     // one page 0x10000 bytes, totally two pages (0x10000 words)
#define FRAM_ADDR_P1                    0x00000     // page #1 start address
#define FRAM_ADDR_P2                    0x10000     // page #2 start address
#define LOG_BUFF_SIZE                   2700

/* for architecture */
#define MEM_ALIGNMENT                   4               /* aligned on 4 bytes */
#define MEM_ALIGN_SIZE(size)            (((size) + MEM_ALIGNMENT - 1U) & ~(MEM_ALIGNMENT-1U))


