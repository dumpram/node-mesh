#include "radio-platform.h"
#include "node.h"


void node_configuration();
void node_wait_for_start();
void node_wait_data();
void node_add_data();
void node_propagate_data();
void node_sleep_until_next_interval();

static node_state_t current_state = SYNC_STATE;
static node_data_t node_data;
static node_data_t temp_data;
static config_data_t config_data;
static node_t this;

static int resync_counter = 0;


void node_loop() {
    if (resync_counter == config_data.resync_interval) {
        current_state = SYNC_STATE;
        resync_counter = 0;
    }
    if (current_state == SYNC_STATE) {
        node_configuration();
        node_wait_for_start();
    }
    if (current_state == NORMAL_STATE) {
        node_wait_data();
        node_propagate_data();
        resync_counter++;
        node_sleep_until_next_interval();
    }
}

void node_configuration() {
    get_config_data(&config_data);
    node_propagate_config_data();
    get_config_ack();
}

void node_wait_data() {
    int i, sleep_acc = 0, next_sleep, current_start_number;
    // add my data
    node_data.packets[node_data.data_length++].data = get_my_packet_data();
    for (i = 0; i < config_data.children_number; i++) {
        current_start_number = config_data.children[i].start_number;
        next_sleep = config_data.highest_start_number - current_start_number;
        next_sleep -= sleep_acc;
        sleep_for(next_sleep);
        sleep_acc += next_sleep;
        // data retrieval
        get_node_data(&temp_data);
        // data_appending
        node_add_data();
    }
    // sleep until your time has come
    sleep_for(config_data.highest_start_number - this.start_number);
}

void node_propagate_config_data() {
    // probe nodes from list
    // create new config data
    // for every child node send config data
}

void node_propagate_data() {
    
}

void node_add_data() {
    int i;
    if (node_data.data_length == MAX_PACKET_NUMBER) {
        return;
    }
    for (i = 0; i < temp_data.data_length &&
        node_data.data_length < MAX_PACKET_NUMBER; i++) {
        node_data.packets[node_data.data_length++] = temp_data.packets[i];
    }
}

void node_sleep_until_next_interval() {
    // this should set status of node to be ok so when the rtc timer interrupt
    // occurs when next interval is imminent it knows that everything went fine
}
