#ifndef RADIO_PLATFORM_H
#define RADIO_PLATFORM_H

#include <stdbool.h>

#include "node.h"

extern node_status_t node_status;
extern node_state_t node_state;

#define CONFIG_ACK_MAGIC    0xdeadbeef
#define START_BEACON_MAGIC  0xfeedbabe
#define PROBE_MAGIC         0x900df00d

/**
 * Gets unique ID for nrf51822 truncated to 32 bits.
 * @return 32-bit ID
 */
int get_nrf_id();
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
void get_node_data(unsigned char *data);
/**
 * Sends node data to parent.
 * @param to   parent parameters
 * @param data data
 */
void set_node_data(unsigned char *data);
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
int  get_my_packet_data();
/**
 * Sleep for ncount of time slices. It is blocking.
 * @param ncount
 */
void sleep_for(int ncount);
/**
 * Sleep until monotonic(RTC) clock interrupt doesn't occur.
 */
void sleep_until_next_interval();
/**
 * Reset RTC counter to zero.
 */
void clock_monotonic_reset();
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
bool probe(int id);
/**
 * Wait until probe flag is received. Should save probe id and rssi in out.
 * Change channel to config channel.
 * @param out saves prober id and rssi
 */
void get_probed(probe_t *out);


#endif /* end of include guard: RADIO_PLATFORM_H */
