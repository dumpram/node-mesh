#ifndef NODE_H
#define NODE_H

#define MAX_CHILDREN_NUMBER 16
#define MAX_PACKET_NUMBER 32

#define SET_CONFIG_ACK_RETRY_MAX 100

#define NODE_GATEWAY

typedef struct node_t {
    int start_number;
    int id;
} node_t;

typedef struct probe_t {
    int parent_id;
    int rssi;
} probe_t;

typedef enum node_state_t {
    NORMAL_STATE,
    SYNC_STATE
} node_state_t;

typedef enum node_status_t {
    OK_STATUS,
    TIMEOUT_STATUS
} node_status_t;

typedef struct config_data_t {
    int children_number;
    int start_number;
    int highest_start_number;
    int resync_interval;
    node_t children[MAX_CHILDREN_NUMBER]; // fix this
} config_data_t;

typedef struct config_ack_t {
    int from;
    int highest_start_number;
    int successfully_configured;
} config_ack_t;

typedef struct node_packet_t {
    int id;
    int data;
} node_packet_t;

typedef struct node_data_t {
    int data_length;
    node_packet_t packets[MAX_PACKET_NUMBER];
} node_data_t;

static const node_t node_empty;
static const probe_t probe_empty;
static const config_data_t config_data_empty;
static const config_ack_t config_ack_empty;
static const node_data_t node_data_empty;

#ifdef NODE_GATEWAY
static const config_data_t config_data_gateway = {
    .children_number = 5,
    .highest_start_number = 0,
    .resync_interval = 10,
    .start_number = 0,
    .children = {
        {.start_number = 0, .id = 1},
        {.start_number = 0, .id = 1},
        {.start_number = 0, .id = 1},
        {.start_number = 0, .id = 1},
        {.start_number = 0, .id = 1}
    }
};
#endif

#endif /* end of include guard: NODE_H */
