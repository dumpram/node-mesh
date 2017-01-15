#ifndef RADIO_PLATFORM_H
#define RADIO_PLATFORM_H

#include <stdbool.h>
#include	<string.h>

#include	"node.h"

#include 	"nrf_esb.h"
#include	"nrf51.h"
#include 	"nrf51_bitfields.h"
#include	"my_nrf_error.h"
#include	"ringbuf.h"



typedef enum {
	RX_CONFIG,
	TX_CONFIG,
	PROBE_CONFIG,
	GET_PROBED_CONFIG
} esb_select_config_t;


extern node_status_t node_status;
extern node_state_t node_state;

#define ESB_RX_CONFIG(X)	 do{ 	X.protocol               = NRF_ESB_PROTOCOL_ESB_DPL;      \
																X.mode                   = NRF_ESB_MODE_PRX;                 \
																X.event_handler          = 0;                                \
																X.bitrate                = NRF_ESB_BITRATE_2MBPS;            \
																X.crc                    = NRF_ESB_CRC_16BIT;                \
																X.tx_output_power        = NRF_ESB_TX_POWER_0DBM;            \
																X.retransmit_delay       = 300;                              \
																X.retransmit_count       = 50;                                \
																X.tx_mode                = NRF_ESB_TXMODE_AUTO;              \
																X.radio_irq_priority     = 1;                                \
																X.event_irq_priority     = 2;                                \
																X.payload_length         = NRF_ESB_MAX_PAYLOAD_LENGTH;                               \
																X.selective_auto_ack     = false;                         \
}while(0);

#define ESB_TX_CONFIG(X)	 do{ 	X.protocol               = NRF_ESB_PROTOCOL_ESB_DPL;      \
																X.mode                   = NRF_ESB_MODE_PTX;                 \
																X.event_handler          = 0;                                \
																X.bitrate                = NRF_ESB_BITRATE_2MBPS;            \
																X.crc                    = NRF_ESB_CRC_16BIT;                \
																X.tx_output_power        = NRF_ESB_TX_POWER_0DBM;            \
																X.retransmit_delay       = 300;                              \
																X.retransmit_count       = 50;                                \
																X.tx_mode                = NRF_ESB_TXMODE_AUTO;              \
																X.radio_irq_priority     = 1;                                \
																X.event_irq_priority     = 2;                                \
																X.payload_length         = NRF_ESB_MAX_PAYLOAD_LENGTH;                               \
																X.selective_auto_ack     = false;                         \
}while(0);


#define ESB_PROBE_CONFIG(X)	 do{ 	X.protocol             = NRF_ESB_PROTOCOL_ESB_DPL;      \
																X.mode                   = NRF_ESB_MODE_PTX;                 \
																X.event_handler          = 0;                                \
																X.bitrate                = NRF_ESB_BITRATE_2MBPS;            \
																X.crc                    = NRF_ESB_CRC_16BIT;                \
																X.tx_output_power        = NRF_ESB_TX_POWER_0DBM;            \
																X.retransmit_delay       = 300;                              \
																X.retransmit_count       = 5;                                \
																X.tx_mode                = NRF_ESB_TXMODE_AUTO;              \
																X.radio_irq_priority     = 1;                                \
																X.event_irq_priority     = 2;                                \
																X.payload_length         = NRF_ESB_MAX_PAYLOAD_LENGTH;                               \
																X.selective_auto_ack     = false;                         \
}while(0);

#define ESB_GET_PROBED_CONFIG(X)	 do{ 	X.protocol       = NRF_ESB_PROTOCOL_ESB_DPL;      \
																X.mode                   = NRF_ESB_MODE_PRX;                 \
																X.event_handler          = 0;                                \
																X.bitrate                = NRF_ESB_BITRATE_2MBPS;            \
																X.crc                    = NRF_ESB_CRC_16BIT;                \
																X.tx_output_power        = NRF_ESB_TX_POWER_0DBM;            \
																X.retransmit_delay       = 300;                              \
																X.retransmit_count       = 5;                                \
																X.tx_mode                = NRF_ESB_TXMODE_AUTO;              \
																X.radio_irq_priority     = 1;                                \
																X.event_irq_priority     = 2;                                \
																X.payload_length         = NRF_ESB_MAX_PAYLOAD_LENGTH;                               \
																X.selective_auto_ack     = false;                         \
}while(0);



#define ESB_PROBE_CH				15	
#define	ESB_CONFIG_CH				75

#define CONFIG_ACK_MAGIC    0xdeadbeef
#define DATA_ACK_MAGIC		0xboob5000
#define DATA_END_MAGIC		0xb16b00b5
#define START_BEACON_MAGIC  0xfeedbabe
#define PROBE_MAGIC         0x900df00d

#define	PROBE_M0						0x90
#define	PROBE_M1						0x0d
#define	PROBE_M2						0xf0
#define	PROBE_M3						0x0d

#define	PROBE_RSSI_MIN			-80
#define PROBE_PREFIX				0x12

#define CONFIG_RSSI_MIN		-100	//accept packets with any RSSI
#define CONFIG_PREFIX		0x87

#define DATA_RSSI_MIN		-100	//accept packets with any RSSI
#define DATA_PREFIX			0x45



#define	MINI_TIMESLICE	(1000*1000) //1 sekunda




#define	UINT32_TO_UINT8_ARRAY(addr,id) do{	addr[0] =(uint8_t)( id & 0x000000FF);\
										addr[1] =(uint8_t)(( id & 0x0000FF00)>>8);\
										addr[2] =(uint8_t)(( id & 0x00FF0000)>>16);\
										addr[3] =(uint8_t)(( id & 0xFF000000)>>24);}while(0);

						
																							


/**
 * Gets unique ID for nrf51822 truncated to 32 bits.
 * @return 32-bit ID
 */
unsigned int get_nrf_id(void);
/**
 * Gets config data from air. It should get start_number and list of
 * potential children nodes.
 *
 * @param data pointer to config data structure to fill.
 */
void get_config_data(node_t *from, config_data_t *data);
/**
 * Sets config data to children node. Doesn't have to send highest_start_number.
 * @param to          children node
 * @param config_data config data to send
 */
void set_config_data(node_t *to, config_data_t *config_data);
/**
 * Gets node_data from child. It is blocking.
 * @param data data structure to fill
 */
void get_node_data(unsigned char *data, int* len);
/**
 * Sends node data to parent.
 * @param to   parent parameters
 * @param data data
 */
void set_node_data(node_t *to, unsigned char *data, int len);
/**
 * Should ammend highest_start_number in given data structure.
 * @param data
 */
void get_start_beacon(node_t *from, config_data_t *data);
/**
 * Sets start to node given.
 * @param to   target node
 * @param data send only highest start number from structure
 */
void set_start_beacon(node_t *to, config_data_t *data);
/**
 * Sensor reading or custom data for one node.
 * @return value of sensor
 */
int  get_my_packet_data(void);
/**
 * Sleep for ncount of time slices. It is blocking.
 * @param ncount
 */
void sleep_for(unsigned int ncount);
/**
 * Sleep until monotonic(RTC) clock interrupt doesn't occur.
 */
void sleep_until_next_interval(void);
/**
 * Reset RTC counter to zero.
 */
void clock_monotonic_reset(void);
/**
 * Wait for config acknowledge and fill all elements of structure.
 * @param from [description]
 */
void get_config_ack(config_ack_t *config_ack);
/**
 * Sets config acknowledge of parent node.
 * @param config_ack
 * @param to id of parent node
 */
void set_config_ack(node_t *to, config_ack_t *config_ack);
/**
 * Return true if probe is successful.
 * @param  id target node base address
 * @return    [description]
 */
bool probe(unsigned int id);
/**
 * Wait until probe flag is received. Should save probe id and rssi in out.
 * Change channel to config channel.
 * @param out saves prober id and rssi
 */
void get_probed(probe_t *out);


#endif /* end of include guard: RADIO_PLATFORM_H */
