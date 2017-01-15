#ifndef MAIN_MESH_H
#define MAIN_MESH_H

#include	<stdarg.h>
#include	<stdio.h>



#include "nrf51.h"
#include "nrf51_bitfields.h"
#include "nrf_esb.h"
#include "my_nrf_error.h"
#include "ringbuf.h"

#include "radio-platform.h"
#include	"node.h"

#define MY_DEBUG						1

#define UART_TX_BUF_SIZE			1024
#define	UART_RX_PIN					11UL //unused
#define	UART_TX_PIN					12UL
#define UART_BAUDRATE				UART_BAUDRATE_BAUDRATE_Baud115200  

#define ESB_RX_BUF_SIZE				1024


/******RTC defines********************/

#define RTC_TIME_CMP			(32768*10)		//RTC_time=(1/RTC_freq)*RTC_TIME_CONST (compare is 32-bit register)
#define	RTC_PRESCALER				0U				//RTC_freq=32768/(PRESCALER+1)(PRESCALER is 12-bit register)	
/*************************************/


#define TIC		do{NRF_TIMER1->TASKS_START =0x01;}while(0);

#define TOC		do{NRF_TIMER1->TASKS_CAPTURE[0] =0x01;\
								NRF_TIMER1->TASKS_SHUTDOWN=0x01;\
								if(NRF_TIMER1->CC[0] > 50)dbg_print("Timer: %d us", NRF_TIMER1->CC[0]);\
								NRF_TIMER1->TASKS_CLEAR = 0x01;}while(0);





void clocks_start(void);
void RTC_Init(void);
void dbg_print(const char  *ucFormat, ...);
void UART_Init(void);
void timer_delay(uint32_t delay_us);
void	Timer0_Init(void);
uint32_t esb_rx_init(uint8_t rf_ch);
uint32_t esb_tx_init(uint8_t rf_ch);
static void TIC_TOC_timer_set(void);

#endif

