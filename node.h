#ifndef NODE_H
#define NODE_H

#define MAX_CHILDREN_NUMBER 16
#define MAX_PACKET_NUMBER 32

typedef struct node_t {
    int start_number;
    int id;
} node_t;

typedef struct probe_t {
    int id;
    int rssi;
    char accept;
} probe_t;

typedef enum node_state_t {
    NORMAL_STATE,
    SYNC_STATE
} node_state_t;

typedef struct config_data_t {
    int children_number;
    int highest_start_number;
    int resync_interval;
    node_t children[MAX_CHILDREN_NUMBER]; // fix this
} config_data_t;

typedef struct node_packet_t {
    int id;
    int data;
} node_packet_t;

typedef struct node_data_t {
    int data_length;
    node_packet_t packets[MAX_PACKET_NUMBER];
} node_data_t;

#endif /* end of include guard: NODE_H */