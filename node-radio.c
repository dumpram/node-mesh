#include "radio-platform.h"
#include "node.h"

#define MAX_BUFFER_SIZE 100

static unsigned char output_buffer[MAX_BUFFER_SIZE];
static unsigned char input_buffer[MAX_BUFFER_SIZE];

void int_to_byte_array(int i, int *start, unsigned char *array) {
    array[*start] = (i >> 0) & 0xFF;
    *start = *start + 1;
    array[*start] = (i >> 8) & 0xFF;
    *start = *start + 1;
    array[*start] = (i >> 16) & 0xFF;
    *start = *start + 1;
    array[*start] = (i >> 24) & 0xFF;
    *start = *start + 1;
}

void byte_array_to_int(unsigned char *array, int *start, int *i) {
    *i = 0;
    *i |= array[(*start)++] << 0;
    *i |= array[(*start)++] << 8;
    *i |= array[(*start)++] << 16;
    *i |= array[(*start)++] << 24;
}

void node_radio_set_data(node_t *to, node_t *from, node_data_t *data) {
    int cnt = 0, i;
    //int_to_byte_array(from->id, &cnt, output_buffer);
    int_to_byte_array(data->data_length, &cnt, output_buffer);
    for (i = 0; i < data->data_length; i++) {
        int_to_byte_array(data->packets[i].id, &cnt, output_buffer);
        int_to_byte_array(data->packets[i].data, &cnt, output_buffer);
    }
    // send data
    set_node_data(to, output_buffer, cnt);
}

void node_radio_get_data(node_t *from, node_data_t *data) {
    int cnt = 0, i = 0, len;
    // here is retrieval of data
    get_node_data(input_buffer, &len);
    byte_array_to_int(input_buffer, &cnt, &i);
    from->id = i;
    byte_array_to_int(input_buffer, &cnt, &i);
    data->data_length = i;
    for (i = 0; i < data->data_length; i++) {
        byte_array_to_int(input_buffer, &cnt, &(data->packets[i].id));
        byte_array_to_int(input_buffer, &cnt, &(data->packets[i].data));
    }
}

void node_radio_set_config_ack(node_t *to, config_ack_t *ack) {
    int cnt = 0;
    //int_to_byte_array(ack->from, &cnt, output_buffer);
    int_to_byte_array(ack->highest_start_number, &cnt, output_buffer);
    int_to_byte_array(ack->successfully_configured, &cnt, output_buffer);
    set_node_data(to, output_buffer, cnt);
}

void node_radio_get_config_ack(config_ack_t *ack) {
    int cnt = 0, len;
    get_node_data(input_buffer, &len);
    byte_array_to_int(input_buffer, &cnt, &ack->from);
    byte_array_to_int(input_buffer, &cnt, &ack->highest_start_number);
    byte_array_to_int(input_buffer, &cnt, &ack->successfully_configured);
}

void node_radio_set_start_beacon(node_t *from, node_t *to,
                                            config_data_t *data) {
    int cnt = 0;
    //int_to_byte_array(from->id, &cnt, output_buffer);
    int_to_byte_array(data->highest_start_number, &cnt, output_buffer);
    set_node_data(to, output_buffer, cnt);
}

void node_radio_get_start_beacon(node_t *from, config_data_t *data) {
    int cnt = 0, len;
    int recv_id;
    get_node_data(input_buffer, &len);
    byte_array_to_int(input_buffer, &cnt, &recv_id);
    byte_array_to_int(input_buffer, &cnt, &data->highest_start_number);
}
