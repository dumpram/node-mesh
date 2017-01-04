#ifndef RADIO_PLATFORM_H
#define RADIO_PLATFORM_H

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

#endif /* end of include guard: RADIO_PLATFORM_H */
