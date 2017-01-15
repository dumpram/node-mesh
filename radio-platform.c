#include "radio-platform.h"

#define MASK_SIGN           (0x00000200UL)		//for chip temp conversion
#define MASK_SIGN_EXTENSION (0xFFFFFC00UL)		//for chip temp conversion

extern node_status_t node_status;
extern node_state_t node_state;

extern bool					timer_done;
																									
extern tRingBufObject rbo_esb_rx;																	
																									
extern void dbg_print(const char  *ucFormat, ...);
extern void timer_delay(uint32_t delay_us);
extern void timer_start(uint32_t wait_for);
		


node_t parent;//izbrisati kod konačne implenentacije!!

nrf_esb_payload_t	        probe_payload;
uint8_t										esb_probe_status = 0xFF; //0xFF-wait for event; 0xAA-RX received; 0xEE-TX failed of TX success
probe_t										global_temp_probe;

const uint8_t 						probe_magic[4] = {PROBE_M0, PROBE_M1, PROBE_M2, PROBE_M3};
const uint8_t							config_ack_magic[4] = {0xde, 0xad, 0xbe, 0xef};
const uint8_t 						start_beacon_magic[4] = {0xfe, 0xed, 0xba, 0xbe};
const uint8_t 						data_magic[4] = {0xb0, 0x0b, 0x50, 0x00};
const uint8_t						data_end_magic[4]={0xb1, 0x6b, 0x00, 0xb5};

bool	rtc_wake_up=false;

nrf_esb_payload_t        tx_payload;
uint8_t									 esb_tx_status = 0xFF;

nrf_esb_payload_t        rx_payload;
uint8_t									 esb_rx_status = 0xFF;

void nrf_esb_rx_event_handler(nrf_esb_evt_t const * p_event)
{		int i,j,k,l;
		

    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:

						//nrf_esb_write_payload(&tx_payload);
											dbg_print("tx_SUCC!\r\n");
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            (void) nrf_esb_flush_tx();
											dbg_print("tx_FAILED!\r\n");
						esb_rx_status = 0xEE;
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:

						if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {								
								if ((rx_payload.length > 0) && (rx_payload.rssi > CONFIG_RSSI_MIN)){
									i=-1;
									j=-1;
									i = memcmp(rx_payload.data, &parent.id,4);
									j = memcmp(&rx_payload.data[rx_payload.length-4], config_ack_magic, 4);
									k = memcmp(&rx_payload.data[rx_payload.length-4], data_magic, 4);
									l = memcmp(&rx_payload.data[rx_payload.length-4], data_end_magic, 4);
									dbg_print("i: %d j: %d k: %d l: %d\r\n", i, j, k, l);
									dbg_print("RX: %X-%X-%X-%X PAR: %X\r\n",rx_payload.data[0], rx_payload.data[1], rx_payload.data[2], rx_payload.data[3], parent.id );
									if(i == 0){
										if(j==0){
											esb_rx_status = 0xAA;		
											//dbg_print("GOT: %d", rx_payload.length);										
											RingBufWrite(&rbo_esb_rx, &rx_payload.data[4], (rx_payload.length-8));
										}
										else if(k == 0){
											esb_rx_status = 0xBB;
											RingBufWrite(&rbo_esb_rx, &rx_payload.data[4], (rx_payload.length-8));
										}
										else if(l == 0){
											esb_rx_status = 0xCC;
											RingBufWrite(&rbo_esb_rx, &rx_payload.data[4], (rx_payload.length-8));
										}
									}
									else	esb_rx_status = 0xEE;
										
								}
								else	esb_rx_status = 0xEE;
              }
							else esb_rx_status = 0xEE;
							break;
			}
}



void nrf_esb_tx_event_handler(nrf_esb_evt_t const * p_event)
{		int i,j;
	

    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
						dbg_print("TX_success!\r\n");
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            (void) nrf_esb_flush_tx();
						dbg_print("TX_fail!\r\n");
						esb_tx_status = 0xEE;
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
						if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
							{		dbg_print("TX_recived len: %d\r\n", rx_payload.length);
								if ((rx_payload.length > 0) && (rx_payload.rssi > CONFIG_RSSI_MIN)){
									i=-1;
									j=-1;
									i = memcmp(&rx_payload.data[rx_payload.length-4], config_ack_magic,4);
									j = memcmp(&rx_payload.data[rx_payload.length-4], data_magic,4);
									if(i == 0){
										dbg_print("Uspjeh conf\r\n");
										esb_tx_status = 0xAA;
									}
									else if(j == 0){
										dbg_print("Uspjeh data\r\n");
										esb_tx_status = 0xBB;
									}									
									else	esb_tx_status = 0xEE;
										
								}
								else				esb_tx_status = 0xEE;
              }
							else esb_tx_status = 0xEE;
							break;
			}
	
}

/*****Handler used when probing*****************************************/
void nrf_esb_probe_event_handler(nrf_esb_evt_t const * p_event){
	int i;
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
						(void) nrf_esb_flush_tx();
						//esb_probe_status = 0xEE;
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            (void) nrf_esb_flush_tx();
						esb_probe_status = 0xEE;
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
						if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {
                if ((rx_payload.length > 0) && (rx_payload.rssi > PROBE_RSSI_MIN)){
									for(i = 0; i<4; i++){
										if(rx_payload.data[i+4] != probe_magic[i]) break;										
									}
									if(i == 4)	esb_probe_status = 0xAA;
									else				esb_probe_status = 0xEE;
                }
								else esb_probe_status = 0xEE;
            }
						else esb_probe_status = 0xEE;

            break;
    }
}

/**************************************************************************/

/*****Handler used when getting probed*****************************************/
void nrf_esb_get_probed_event_handler(nrf_esb_evt_t const * p_event){	
	int i;
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
						(void) nrf_esb_flush_tx();
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            (void) nrf_esb_flush_tx();
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:

						if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {						
                if ((rx_payload.length > 0) && (rx_payload.rssi > PROBE_RSSI_MIN)){
									for(i = 0; i<4; i++){
										if(rx_payload.data[i+4] != probe_magic[i]) break;										
									}
									if(i == 4){
											memcpy(&global_temp_probe.parent_id, rx_payload.data,4);
										dbg_print("Global_parent_ID: %X\r\n", global_temp_probe.parent_id);
										global_temp_probe.rssi = rx_payload.rssi;
										esb_probe_status = 0xAA;
									}
                }
            }


            break;
    }
}
/********************************************************************************/

uint32_t ESB_Init(void * evt_handler, esb_select_config_t config, 
									uint32_t base_addr, uint8_t prefix, uint8_t rf_ch)
{
    uint32_t err_code;
    uint8_t base_addr_0[4];
	nrf_esb_config_t nrf_esb_config;
		
		switch (config)
    {
    	case RX_CONFIG:
				ESB_RX_CONFIG(nrf_esb_config);
    		break;
    	case TX_CONFIG:
				ESB_TX_CONFIG(nrf_esb_config);
    		break;
			case PROBE_CONFIG:
				ESB_PROBE_CONFIG(nrf_esb_config);
    		break;
			case GET_PROBED_CONFIG:
				ESB_GET_PROBED_CONFIG(nrf_esb_config);
    		break;
    	default:
				dbg_print("Invalid ESB config selected! Val: %d", config);
    		break;
    }
		
		
    nrf_esb_config.event_handler =(nrf_esb_event_handler_t) evt_handler;

    err_code = nrf_esb_init(&nrf_esb_config);
		
	nrf_esb_enable_pipes(0x01); //enable only pipe 0
	UINT32_TO_UINT8_ARRAY(base_addr_0, base_addr);
    err_code = nrf_esb_set_base_address_0(base_addr_0);


    nrf_esb_update_prefix(0, prefix);
		
		err_code = nrf_esb_set_rf_channel(rf_ch);

    return err_code;
}



__inline unsigned int get_nrf_id(void){//isprobano
	return NRF_FICR->DEVICEID[0];	
}


bool probe(unsigned int id){//isprobano

	ESB_Init(nrf_esb_probe_event_handler, 
					PROBE_CONFIG, 
					id, 
					PROBE_PREFIX, 
					ESB_PROBE_CH);
	
	UINT32_TO_UINT8_ARRAY(probe_payload.data,get_nrf_id());//add id data to payload
	probe_payload.data[4] = PROBE_M0;
	probe_payload.data[5] = PROBE_M1;
	probe_payload.data[6] = PROBE_M2;
	probe_payload.data[7] = PROBE_M3;
	probe_payload.length	=	8;
	probe_payload.pipe		=	0;	
	
	esb_probe_status = 0xFF;
	nrf_esb_write_payload(&probe_payload);
	
	while(esb_probe_status == 0xFF) __WFI();
	
	if(esb_probe_status == 0xAA) return true;
	else return false;	
}


void get_probed(probe_t *out){//isprobano
	
	ESB_Init(nrf_esb_get_probed_event_handler, GET_PROBED_CONFIG, get_nrf_id(), PROBE_PREFIX, ESB_PROBE_CH);
	
	UINT32_TO_UINT8_ARRAY(probe_payload.data,get_nrf_id());//add id data to payload
	probe_payload.data[4] = PROBE_M0;
	probe_payload.data[5] = PROBE_M1;
	probe_payload.data[6] = PROBE_M2;
	probe_payload.data[7] = PROBE_M3;
	probe_payload.length	=	8;
	probe_payload.pipe		=	0;	
	nrf_esb_write_payload(&probe_payload);
	
	esb_probe_status = 0xFF;
	nrf_esb_start_rx();
	
	while(esb_probe_status == 0xFF) __WFI();
	nrf_esb_stop_rx();	
	
	if(esb_probe_status == 0xAA) {
		uint32_t irq_masked = __disable_irq();
			out->parent_id = global_temp_probe.parent_id;
			out->rssi = global_temp_probe.rssi;
		if(!irq_masked)	__enable_irq();		
	}
	
	while(!nrf_esb_is_idle());
	nrf_esb_disable();

}


void sleep_until_next_interval(void){ //isprobano
	
	rtc_wake_up = false;
	while(!rtc_wake_up) __WFI();	
}

void sleep_for(unsigned int ncount){//treba prmijenit da bude upravljan sa timerom kako bi se definirao custom timeslice
	unsigned int i;

	for(i=0;i<ncount;i++){
		timer_done = false;
		timer_start(MINI_TIMESLICE);
		while(!timer_done) __WFI();	
		NRF_TIMER0->TASKS_SHUTDOWN=0x01;	
		NVIC_DisableIRQ(TIMER0_IRQn);	
	}
}

void set_config_data(node_t *to, config_data_t *config_data){
	int i,j;
	uint32_t id;

	
	ESB_Init(nrf_esb_tx_event_handler, 
					TX_CONFIG, 
					to->id, 
					PROBE_PREFIX, 
					ESB_CONFIG_CH);
	id = get_nrf_id();
	UINT32_TO_UINT8_ARRAY(tx_payload.data, id);
	memcpy(&tx_payload.data[4], &config_data->children_number, 4);
	memcpy(&tx_payload.data[8], &config_data->start_number, 4);
	memcpy(&tx_payload.data[12], &config_data->resync_interval, 4);
	memcpy(&tx_payload.data[16], config_ack_magic, 4);
	tx_payload.length = 20;
	
	esb_tx_status = 0xFF;
	nrf_esb_write_payload(&tx_payload);	
	
	while(esb_tx_status == 0xFF) __WFI();
	dbg_print("Poslano!\r\n");
	
	
	for(i=0;i<config_data->children_number;){
		dbg_print("ch-i: %d\r\n",(config_data->children_number - i));
		if((config_data->children_number - i) < 6){
			UINT32_TO_UINT8_ARRAY(tx_payload.data, id);
			for(j=0; j< config_data->children_number; j++){
				memcpy(&tx_payload.data[4*j+4], &config_data->children[i+j].id, 4);
			}
			memcpy(&tx_payload.data[4*j+4], config_ack_magic, 4);
				tx_payload.length = (4*config_data->children_number+8);
				i=config_data->children_number;
		}
		else{
			for(j=0; j< 6; j++){
				memcpy(&tx_payload.data[4*j], &config_data->children[i+j].id, 4);
			}
			memcpy(tx_payload.data, &config_data->children[i], 24);
			tx_payload.length = 24;
			i=i+6;
		}
SEND_CONF:		timer_delay(10000);
							esb_tx_status = 0xFF;
							nrf_esb_write_payload(&tx_payload);	
							dbg_print("DATA_SEND!\r\n");
							while(esb_tx_status == 0xFF) __WFI(); 
							if(esb_tx_status != 0xAA) goto SEND_CONF;
		
		}
}


void get_config_data(node_t *from, config_data_t *data){
	
		uint8_t temp[4];
		int i;
	
RETRY_CONFIG_RX:	ESB_Init(nrf_esb_rx_event_handler, 
														RX_CONFIG, 
														get_nrf_id(), 
														PROBE_PREFIX, 
														ESB_CONFIG_CH);
										
										UINT32_TO_UINT8_ARRAY(probe_payload.data,get_nrf_id());
										memcpy(&tx_payload.data[4], config_ack_magic, 4);
										tx_payload.length = 8;
										
										nrf_esb_write_payload(&tx_payload);
										RingBufFlush(&rbo_esb_rx);
										esb_rx_status = 0xFF;
										nrf_esb_start_rx();
										
										while(esb_rx_status == 0xFF) __WFI();
										nrf_esb_write_payload(&tx_payload);
										if((esb_rx_status == 0xAA) && RingBufUsed(&rbo_esb_rx)>=12){
												RingBufRead(&rbo_esb_rx, temp, 4);
												memcpy(&data->children_number, temp, 4);
												RingBufRead(&rbo_esb_rx, temp, 4);
												memcpy(&data->start_number, temp, 4);
												RingBufRead(&rbo_esb_rx, temp, 4);
												memcpy(&data->resync_interval, temp, 4);
										}
										else{
											nrf_esb_disable(); //shutdown ESB
											goto RETRY_CONFIG_RX;//reinit ESB and listen for packets again
										}
										
										while(esb_rx_status == 0xFF) __WFI();
										nrf_esb_write_payload(&tx_payload);
										if(esb_rx_status == 0xAA){
											for(i=0; i< data->children_number;){
												while(RingBufEmpty(&rbo_esb_rx)) __WFI();
												RingBufRead(&rbo_esb_rx, temp, 4);
												memcpy(&data->children[i++].id, temp, 4);
												dbg_print("A");
											}
										}
										else{
										}
}



__inline  void clock_monotonic_reset(void){
	 NRF_RTC0->TASKS_STOP = 0x01;
	 NRF_RTC0->TASKS_CLEAR=0x01;
	 NRF_RTC0->TASKS_START=0x01;	 
 }
 
 void get_config_ack(config_ack_t *config_ack){
	 
	 
	 
 }
 
 void set_config_ack(node_t *to, config_ack_t *config_ack){
	 
	 
 }
 
 
__inline int  get_my_packet_data(void){ //read IC die temperature, something more useful in the future...
	
	*(uint32_t *) 0x4000C504 = 0;
	NRF_TEMP->POWER = TEMP_POWER_POWER_Enabled<<TEMP_POWER_POWER_Pos;
	NRF_TEMP->EVENTS_DATARDY = 0x00;
	NRF_TEMP->TASKS_START = 0x01;
	while(NRF_TEMP->EVENTS_DATARDY == 0){}
	
	
	return (int)(((NRF_TEMP->TEMP & MASK_SIGN) != 0) ? (NRF_TEMP->TEMP | MASK_SIGN_EXTENSION) : (NRF_TEMP->TEMP)/4);

}


void get_start_beacon(node_t *from, config_data_t *data){
	
	
	
}


void set_start_beacon(node_t *to, config_data_t *data){
	
	
	
}

void get_node_data(unsigned char *data, int* len){
	int i=0;
	int used=0;
	
	ESB_Init(nrf_esb_rx_event_handler, 
					RX_CONFIG, 
					get_nrf_id(), 
					DATA_PREFIX, 
					ESB_CONFIG_CH);
	
	UINT32_TO_UINT8_ARRAY(tx_payload.data,get_nrf_id());
	memcpy(&tx_payload.data[4], data_magic, 4);
	tx_payload.length = 8;
	
	nrf_esb_write_payload(&tx_payload);
	RingBufFlush(&rbo_esb_rx);
	esb_rx_status = 0xFF;
	nrf_esb_start_rx();
	dbg_print("Slusam!");

	RX_WAIT:	while(esb_rx_status == 0xFF) __WFI();
				nrf_esb_write_payload(&tx_payload);
				if(esb_rx_status == 0xBB){
					used = RingBufUsed(&rbo_esb_rx);
					RingBufRead(&rbo_esb_rx, &data[i], used);
					i = i+used;
					dbg_print("B\t");
					goto RX_WAIT;
				}
				else if(esb_rx_status == 0xCC){//recived data_end_magic
					used = RingBufUsed(&rbo_esb_rx);
					RingBufRead(&rbo_esb_rx, &data[i], used);
					*len = i+used;
					dbg_print("C\t");
				}
				else goto RX_WAIT;
}

void set_node_data(node_t *to, unsigned char *data, int len){
	uint8_t base_addr[4];
	uint32_t temp_addr;
	int32_t cnt=0;
	uint32_t id;
	
		ESB_Init(nrf_esb_tx_event_handler, 
					TX_CONFIG, 
					to->id, 
					DATA_PREFIX, 
					ESB_CONFIG_CH);

	len = len - 4; //skrati za id
	//slazi palode i salji
	id=get_nrf_id();
	memcpy(tx_payload.data, &id, 4);
	for(cnt=0;cnt<len;){
		if((len - cnt) <= 24 ){ //data fits in one packet
			memcpy(&tx_payload.data[4], &data[cnt+4], len);
			memcpy(&tx_payload.data[4+len], data_end_magic, 4);
			cnt = len;
			tx_payload.length =  8 + len;
		}
		else{
			memcpy(&tx_payload.data[4], &data[cnt+4], 24);
			memcpy(&tx_payload.data[28], data_magic, 4);
			cnt = cnt + 24;
			tx_payload.length =  32;
		}
		DATA_SEND_RTRY:		timer_delay(1000000);//1000ms
							esb_tx_status = 0xFF;
							nrf_esb_write_payload(&tx_payload);	
							dbg_print("DATA_SEND!\r\n");
							while(esb_tx_status == 0xFF) __WFI(); 
							if(esb_tx_status != 0xBB) goto DATA_SEND_RTRY;
	}	
}








