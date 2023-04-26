#ifndef __CAN_LL_H
#define __CAN_LL_H

#include <stdint.h>

#define CAN_PRIORITY_NORMAL   0x0F

#define CAN_NET_CAST_GROUP    0xFF00
#define CAN_PI_PDU_GROUP      0xFF01
#define CAN_PI_PU_GROUP       0xFF02
#define CAN_PU_PDU_GROUP      0xFF03
#define CAN_VZ_GROUP          0xFF04

#define CAN_IDE_32            0x4
#define FILTER_HIGH_EXT(x)  (x >> 13)
#define FILTER_LOW_EXT(x)   ((x << 3) | CAN_IDE_32)
#define FILTER_HIGH_STD(x)  (x << 5)


void can1_filter_config(uint8_t self_net_id);
void can1_send(uint16_t dest_id, uint8_t *data, uint32_t size);
void can1_start(void);

#endif
