#ifndef __DEBUG_H__
#define __DEBUG_H__


/*****************************************************/
		/* config */
#define INIT_FROM_CH	0
#define INIT_CH_N		4
#define INIT_MASK		0xff 	//+MASK_BASE+MASK_DELTA)		//+MASK_PER+MASK_FREQ0+MASK_FREQ1+MASK_FREQ2)
#define INIT_KEY		255

/*****************************************************/
typedef struct{
	uint8_t startCH;
	uint8_t NofCH;
	uint8_t mask;
	uint8_t viewCH;
	uint32_t baudRate;
	struct{
		uint8_t busy 	:1;
		uint8_t fla_dbg	:1;
		uint8_t rsvd 	:6;
	} flag;
} str_debug;

#define DEBUG_STX	0x3A
#define DEBUG_CR	0x0D
#define DEBUG_LF	0x0A
#define DEBUG_SFR_N 42

#define MASK_SUM	0x01
#define MASK_REL	0x02
#define MASK_BASE	0x04
#define MASK_DELTA	0x08
#define MASK_PER	0x10
#define MASK_FREQ0	0x20
#define MASK_FREQ1	0x40
#define MASK_FREQ2	0x80

#define DEBUG_TC	0x01
#define DEBUG_LED	0x02
#define DEBUG_BUZ	0x04

extern str_debug t_dbg;
void Do_taskDebug(void);

#define RX_BUFF_SIZE	100
#define TX_BUFF_SIZE	200
#define RX_MSG_SIZE 	25
typedef struct{ 
	uint16_t rxW_i;
	uint16_t rxR_i;
	uint8_t	rxQ[RX_BUFF_SIZE];
	uint16_t txW_i;
	uint16_t txR_i;
	uint16_t txCS;
	uint8_t	txQ[TX_BUFF_SIZE];
} str_uart;
typedef enum 
{
	RX_STEP_STX = 0,
	RX_STEP_CMD,
	RX_STEP_LENGTH,
	RX_STEP_DATA,
	RX_STEPCS,
	RX_STEP_ETX,
	RX_STEP_ERROR = 0xff
} ExStatus_t;
#define RX_FRAME_STX	0x02
#define RX_FRAME_ETX	0x03
#define RX_INIT 	0
#define RX_OK		1
#define RX_ERROR	2

extern str_uart t_uart;
void putNstr (uint8_t* p_data, uint8_t size);
void uart_InitReg(void);
uint8_t uart_InitVar (void);
uint8_t uart_RxCheck(uint8_t** ptr);
void dbg_SendData(void);
void makePaketB(uint8_t dat);
void makePaketW(uint16_t dat);
void Do_taskDebug(void);
void dbg_Set_Baudrate(uint32_t dbgBaud);

#endif	// __DEBUG_H__

