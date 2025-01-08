#ifndef K_TEST_APP_H_
#define K_TEST_APP_H_

#include <stdio.h>
//Node info to be used for test app
typedef struct
{
    uint8_t macAddr[8];
    uint8_t rssi;
    uint8_t routing_cost;
    uint8_t channel;
    uint8_t padding;
}k_nbors_t;

#endif//K_TEST_APP_H_
