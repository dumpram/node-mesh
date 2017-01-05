#ifndef RADIO_PLATFORM_H
#define RADIO_PLATFORM_H

#include <stdbool.h>

#include "node.h"


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
void get_config_data(config_data_t *data);
/**
 * Gets node_data from air. It is blocking.
 * @param data data structure to fill
 */
void get_node_data(node_data_t *data);
/**
 * Should amend highest_start_number in given data structure.
 * @param data
 */
void get_start_beacon(config_data_t *data);
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
 * Not sure yet.
 */
void get_config_ack();
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


void set_config_data(node_t *node, config_data_t *config_data);

#endif /* end of include guard: RADIO_PLATFORM_H */
