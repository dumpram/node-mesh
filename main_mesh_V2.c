#include "main_mesh_V2.h"

tRingBufObject rbo_uart;
tRingBufObject rbo_esb_rx;


uint8_t uart_rx_buf[UART_TX_BUF_SIZE];
uint8_t esb_rx_buf[ESB_RX_BUF_SIZE];

uint8_t uart_ok_to_send = 1;
bool	timer_done = false;

extern	bool rtc_wake_up;
extern node_t parent;

extern void node_loop(void);

/******Handle UART Incoming interrupt**************/
__irq void  UART0_IRQHandler(void){
	
	NVIC_DisableIRQ(UART0_IRQn);
	NVIC_ClearPendingIRQ(UART0_IRQn);
	
	if(NRF_UART0->EVENTS_TXDRDY == 0x01){
		NRF_UART0->TASKS_STOPTX;
		NRF_UART0->EVENTS_TXDRDY = 0x00;
		uart_ok_to_send = 1;
		if(!RingBufEmpty(&rbo_uart)){
			uart_ok_to_send = 0;
			NRF_UART0->TXD = RingBufReadOne(&rbo_uart);	
			NRF_UART0->TASKS_STARTTX = 0x01UL;
		}
	}

	NVIC_EnableIRQ(UART0_IRQn);
}
/*****************************************************/
/********RTC handler***************************/
__irq	void RTC0_IRQHandler(void){

	NVIC_DisableIRQ(RTC0_IRQn);
	NVIC_ClearPendingIRQ(RTC0_IRQn);
	NRF_RTC0->TASKS_STOP = 0x01;
	NRF_RTC0->TASKS_CLEAR =0x01;
	
	rtc_wake_up = true;
	//provjeri node_status, ako je ok, radi dalje, inace reset
	
	NRF_RTC0->EVENTS_COMPARE[0] = 0x00;
	NRF_RTC0->TASKS_START = 0x01;	
	
	NVIC_EnableIRQ(RTC0_IRQn);		
}
/****************************************************/

__irq void	TIMER0_IRQHandler(void){
	
		NVIC_DisableIRQ(TIMER0_IRQn);
		NVIC_ClearPendingIRQ(TIMER0_IRQn);
		NRF_TIMER0->EVENTS_COMPARE[0] = 0x00;
		timer_done = true;
	
		NVIC_EnableIRQ(TIMER0_IRQn);
}



int main(void){
	
//	bool temp_bool = false;
//	probe_t			incoming_probe;
	node_t			temp_node;
	int	temp_cnt=0;
	int i, temp_data;
	unsigned char temp_buff[100];
	
	clocks_start();	
	RingBufInit(&rbo_uart, uart_rx_buf, UART_TX_BUF_SIZE);
	RingBufInit(&rbo_esb_rx, esb_rx_buf, ESB_RX_BUF_SIZE);
	TIC_TOC_timer_set();

	UART_Init();
	RTC_Init();
	Timer0_Init();
	
	parent.id = 0xe287a6a4;
	temp_node.id = 0x05364061;
	dbg_print("Init finished! ID: %#X\r\n", get_nrf_id());

	while(1){
		
		sleep_until_next_interval();
#if 0
		get_node_data(temp_buff, &temp_cnt);
		for(i=0; i<temp_cnt; i++) dbg_print("%c", temp_buff[i]);
		dbg_print("Length: %d\r\n", temp_cnt);
#else		
		memcpy(temp_buff, &parent.id, 4);
		temp_data = get_my_packet_data();
		temp_cnt = sprintf(&temp_buff[4], "ID: %X \t Temperatura: %d C \r\n",get_nrf_id(), temp_data);
		TIC;
		set_node_data(&temp_node, (uint8_t *)temp_buff, temp_cnt+4);
		clock_monotonic_reset();
		TOC;
		dbg_print("Poslano: %d bytea\r\n", temp_cnt+8);
#endif
		
		
	}
}	


__inline void clocks_start( void ){
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
	
		NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
	  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
	
		NRF_POWER->TASKS_LOWPWR = 0x01;
}

__inline void RTC_Init(void){
	
	NRF_RTC0->POWER = (RTC_POWER_POWER_Enabled << RTC_POWER_POWER_Pos);
		
	NVIC_SetPriority(RTC0_IRQn, 0x02);
	NVIC_ClearPendingIRQ(RTC0_IRQn);	
	
	NRF_RTC0->PRESCALER = RTC_PRESCALER;
	NRF_RTC0->CC[0] = RTC_TIME_CMP;
	NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
	NRF_RTC0->EVENTS_COMPARE[0] = 0x00;
	NRF_RTC0->TASKS_START = 0x01;
	NVIC_EnableIRQ(RTC0_IRQn);
}



__inline void UART_Init(void){
	
	NRF_UART0->POWER = UART_POWER_POWER_Enabled << UART_POWER_POWER_Pos;		

	NRF_UART0->PSELCTS = 0xFFFFFFFF; //CTS line disconnected
	NRF_UART0->PSELRTS = 0xFFFFFFFF; //RTS line disconnected
	NRF_UART0->PSELRXD = 0xFFFFFFFF; //RX line disconnected
	NRF_UART0->PSELTXD = UART_TX_PIN; //TX line connected to pin UART_TX_PIN
	NRF_UART0->CONFIG = (UART_CONFIG_HWFC_Disabled << UART_CONFIG_HWFC_Pos) |
											(UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos);	
	NRF_UART0->BAUDRATE = UART_BAUDRATE;
	
	NVIC_SetPriority(UART0_IRQn , 0x05);
	NVIC_ClearPendingIRQ(UART0_IRQn );

	
	NRF_UART0->INTENSET = UART_INTENSET_TXDRDY_Enabled << UART_INTENSET_TXDRDY_Pos;
	NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos;	
	NVIC_EnableIRQ(UART0_IRQn);
	
	NRF_UART0->EVENTS_RXDRDY = 0x00;
	NRF_UART0->TASKS_STARTRX = 0x01;
}

__inline void dbg_print(const char  *ucFormat, ...){
	#if MY_DEBUG
			int32_t cnt = 0;
			char temp_buff[128];
			va_list args;

			va_start(args, ucFormat);
			
			cnt = vsprintf(temp_buff, ucFormat, args);
			
			va_end(args);
			
			RingBufWrite(&rbo_uart, (uint8_t *)temp_buff, cnt);
			if(uart_ok_to_send == 1){
					uart_ok_to_send = 0;
					NRF_UART0->TXD = RingBufReadOne(&rbo_uart);	
					NRF_UART0->TASKS_STARTTX = 0x01UL;
			}
	#endif	
}

__inline void	Timer0_Init(void){
	NRF_TIMER0->POWER = 0x01;
	NVIC_SetPriority(TIMER0_IRQn , 0x03);
	NVIC_ClearPendingIRQ(TIMER0_IRQn );

	
	NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
	NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
	NRF_TIMER0->INTENSET  = TIMER_INTENSET_COMPARE0_Set << TIMER_INTENSET_COMPARE0_Pos;
	NRF_TIMER0->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled <<TIMER_SHORTS_COMPARE0_CLEAR_Pos |
												TIMER_SHORTS_COMPARE0_STOP_Enabled << TIMER_SHORTS_COMPARE0_STOP_Pos;
	NRF_TIMER0->PRESCALER = 4; //MAX value is 9;; f(timer)=16MHz/2^(PRESCALER)
}

void timer_delay(uint32_t delay_us){
	
	NRF_TIMER0->CC[0] = delay_us;
	NRF_TIMER0->EVENTS_COMPARE[0] = 0x00;
	NRF_TIMER0->TASKS_CLEAR = 0x01;
	timer_done = false;
	NVIC_EnableIRQ(TIMER0_IRQn);
	NRF_TIMER0->TASKS_START =0x01;
	
	while(timer_done == false) 	__WFI(); //sleep while waiting for timer interrupt
	
	NRF_TIMER0->TASKS_SHUTDOWN=0x01;	
	NVIC_DisableIRQ(TIMER0_IRQn);	
}

void timer_start(uint32_t wait_for){
	NRF_TIMER0->CC[0] = wait_for;
	NRF_TIMER0->EVENTS_COMPARE[0] = 0x00;
	NRF_TIMER0->TASKS_CLEAR = 0x01;
	timer_done = false;
	NVIC_EnableIRQ(TIMER0_IRQn);
	NRF_TIMER0->TASKS_START =0x01;
}
void TIC_TOC_timer_set(void){
	
	NRF_TIMER1->POWER = 0x01;
	NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
	NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
	NRF_TIMER1->PRESCALER = 4; //MAX value is 9;; f(timer)=16MHz/2^(PRESCALER)
	
	NRF_TIMER1->TASKS_CLEAR = 0x01;
}





