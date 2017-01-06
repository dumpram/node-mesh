#include "radio-platform.h"
#include "node.h"


void node_configuration();
void node_wait_for_start();
void node_wait_data();
void node_add_data();
void node_propagate_data();
void node_propagate_config_data();
void node_sleep_until_next_interval();
void node_create_new_config_data();
void node_ammend_this();
void node_ammend_parent();
void node_get_config_ack();
void node_set_config_ack();


static node_state_t current_state = SYNC_STATE;
static node_data_t node_data;
static node_data_t temp_data;
static config_data_t temp_config_data;
static config_data_t config_data;
static config_ack_t config_ack;


static node_t this;
static probe_t out;
static node_t parent;

static bool probe_table[MAX_CHILDREN_NUMBER];

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
    get_probed(&out);
    node_ammend_parent();
    get_config_data(&parent, &config_data);
    node_ammend_this();
    node_propagate_config_data();
    node_get_config_ack();
    node_set_config_ack();
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
        get_node_data(&config_data.children[i], &temp_data);
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
    int i, start_number_cnt = 0;
    for (i = 0; i < config_data.children_number; i++) {
        probe_table[i] = probe(config_data.children[i].id);
    }
    node_create_new_config_data();

    for (i = 0; i < config_data.children_number; i++) {
        if (probe_table[i]) {
            temp_config_data.start_number
                = this.start_number + (++start_number_cnt);
            set_config_data(&config_data.children[i], &temp_config_data);
        }
    }
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

void node_create_new_config_data() {
    int i;
    for (i = 0; i < config_data.children_number; i++) {
        if (!probe_table[i]) {
            temp_config_data.children[temp_config_data.children_number++]
                = config_data.children[i];
            temp_config_data.resync_interval = config_data.resync_interval;
        }
    }
}

void node_get_config_ack() {
    int max_start_number = this.start_number, successfully_configured = 0, i;
    int real_children_number =
        config_data.children_number - temp_config_data.children_number;
    int config_ack_cnt = 0;
    while (config_ack_cnt < real_children_number) {
        get_config_ack(&config_ack);
        for (i = 0; i < config_data.children_number; i++) {
            if (probe_table[i]) {
                if (config_data.children[i].id == config_ack.from) {
                    if (config_ack.highest_start_number > max_start_number) {
                        max_start_number = config_ack.highest_start_number;
                    }
                    successfully_configured +=
                        config_ack.successfully_configured;
                    config_ack_cnt++;
                    break;
                }
            }
        }
    }
    config_ack.from = this.id;
    config_ack.successfully_configured = successfully_configured;
    config_ack.highest_start_number = max_start_number;
}

void node_set_config_ack() {
    set_config_ack(&parent, &config_ack);
}

void node_ammend_this() {
    this.start_number = config_data.start_number;
    this.id = get_nrf_id();
}

void node_ammend_parent() {
    parent.id = out.parent_id;
}

void node_wait_for_start() {
    get_start_beacon(&parent, &config_data);
}
