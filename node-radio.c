#include "radio-platform.h"
#include "node.h"

#include <memory.h>

#define MAX_BUFFER_SIZE 100

static unsigned char output_buffer[MAX_BUFFER_SIZE];
static unsigned char input_buffer[MAX_BUFFER_SIZE];

void int_to_byte_array(int i, int *start, unsigned char *array) {
    array[*start] = (i >> 24) & 0xFF;
    *start = *start + 1;
    array[*start] = (i >> 16) & 0xFF;
    *start = *start + 1;
    array[*start] = (i >> 8) & 0xFF;
    *start = *start + 1;
    array[*start] = (i >> 0) & 0xFF;
    *start = *start + 1;
}

void byte_array_to_int(unsigned char *array, int *start, int *i) {
    *i = 0;
    *i |= array[(*start)++] << 24;
    *i |= array[(*start)++] << 16;
    *i |= array[(*start)++] << 8;
    *i |= array[(*start)++] << 0;
}

void node_radio_set_data(node_t *to, node_data_t *data) {
    int cnt = 0, i;
    int_to_byte_array(to->id, &cnt, output_buffer);
    int_to_byte_array(data->data_length, &cnt, output_buffer);
    for (i = 0; i < data->data_length; i++) {
        int_to_byte_array(data->packets[i].id, &cnt, output_buffer);
        int_to_byte_array(data->packets[i].data, &cnt, output_buffer);
    }
    // simulating sending of data
    //set_node_data(output_buffer);
    memcpy(input_buffer, output_buffer, MAX_BUFFER_SIZE);
}

void node_radio_get_data(node_t *from, node_data_t *data) {
    int cnt = 0, i = 0, tmp;
    // here is retrieval of data
    //get_node_data(input_buffer);
    byte_array_to_int(input_buffer, &cnt, &i);
    from->id = i;
    byte_array_to_int(input_buffer, &cnt, &i);
    data->data_length = i;
    for (i = 0; i < data->data_length; i++) {
        byte_array_to_int(input_buffer, &cnt, &(data->packets[i].id));
        byte_array_to_int(input_buffer, &cnt, &(data->packets[i].data));
    }

}


int main() {
    node_t to = {
        .id = 1,
        .start_number = 0
    };

    node_data_t data = {
        .data_length = 2,
        .packets = {
            {.id = 0xdeadbeef, .data = 0xAAAAAAAA},
            {.id = 0xdeadbeef, .data = 0xFFFFFFFF}
        }
    };

    node_t from;
    node_data_t recv_data;

    node_radio_set_data(&to, &data);
    node_radio_get_data(&from, &recv_data);


    node_loop();


}
