#ifndef __PCS_H
#define __PCS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "utils.h"

#define PCS_SEND_PAY_SIZE 128
#define PCS_MAX_DEV_CNT 1
#define PCS_POOL_SIZE 128

typedef struct {
  uint16_t src_id;
  uint8_t len;
  uint8_t data[8];
} recv_frame_t;

typedef struct {
  uint16_t dest_id;
  syspkg_t pkg;
  uint8_t payload[PCS_SEND_PAY_SIZE];
} send_msg_t;

typedef struct {
  syspkg_t *pkg;
  uint8_t *data;
} pcs_resp_t;

typedef struct {
  uint16_t can_id;
  uint16_t rd_ptr;
  uint16_t wr_cnt;
  uint32_t byte_cnt;
  uint32_t fixtime;
  uint8_t start_packet;
  uint8_t pool[PCS_POOL_SIZE];
  uint8_t ln_data[PCS_POOL_SIZE];
} mailbox_t;

typedef struct {
  uint8_t dest_id;
  uint8_t cmd;
  uint8_t wait_flag;
  SemaphoreHandle_t sem;
  pcs_resp_t resp;
} waiter_t;

typedef struct {
  QueueHandle_t q_recv;
  QueueHandle_t q_send;
  TaskHandle_t th_recv;
  TaskHandle_t th_send;
  mailbox_t mailbox[PCS_MAX_DEV_CNT];
  waiter_t waiter_pool[PCS_MAX_DEV_CNT];
  uint8_t dev_cnt;

  uint8_t proxy_en;
  uint8_t proxy_cnt;
  uint16_t *proxy_list;

  uint8_t self_net_id;
  void (*can_send)(uint16_t dest_id, uint8_t *data, uint32_t size);
  void (*recv_callback)(void *arg);
} pcs_conf_t;

void pcs_init(pcs_conf_t *cfg);
void pcs_send_message(pcs_conf_t *c, uint16_t dest, syspkg_t *pkg,
                      uint8_t *payload, uint32_t size);
pcs_resp_t *pcs_send_with_resp(pcs_conf_t *c, uint16_t dest, syspkg_t *pkg,
                               uint8_t *payload, uint32_t size,
                               uint8_t wait_cmd, uint32_t timeout);
void pcs_recv_message(pcs_conf_t *c, uint16_t id, uint8_t len, uint8_t *data);

#endif //__PCS_H
