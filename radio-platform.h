#ifndef RADIO_PLATFORM_H
#define RADIO_PLATFORM_H

#include <stdbool.h>

#include "node.h"


#define CONFIG_ACK_MAGIC    0xdeadbeef
#define START_BEACON_MAGIC  0xfeedbabe
#define PROBE_MAGIC         0x900df00d

// Bird's API
void get_config_data(config_data_t *data);
void get_node_data(node_data_t *data);
void get_config_ack();
void get_start_beacon();
int  get_my_packet_data();
void sleep_for(int ncount);


/**
 * Return true if probe is successful.
 * @param  id target node base address
 * @return    [description]
 */
bool probe(int id);

/**
 * Wait until probe flag is received. Should save prober id and rssi in out.
 * @param out saves prober id and rssi
 */
void get_probed(probe_t *out);


void set_config_data(node_t *node, config_data_t *config_data);

#endif /* end of include guard: RADIO_PLATFORM_H */
