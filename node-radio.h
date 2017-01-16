#ifndef NODE_RADIO_H
#define NODE_RADIO_H

void node_radio_set_data(node_t *to, node_data_t *data);
void node_radio_get_data(node_t *from, node_data_t *data);
void node_radio_set_config_ack(node_t *to, config_ack_t *ack);
void node_radio_get_config_ack(config_ack_t *ack);
void node_radio_set_start_beacon(node_t *to, config_data_t *data);
void node_radio_get_start_beacon(node_t *from, config_data_t *data);


#endif /* end of include guard: NODE_RADIO_H */
