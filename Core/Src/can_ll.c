#include "can_ll.h"
#include "pcs.h"
#include "can.h"

CAN_RxHeaderTypeDef header = {0};
uint8_t data[8] = {0};

uint8_t SELF_NET_ID;
extern CAN_HandleTypeDef hcan;
extern pcs_conf_t pcs_can1;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, data);
    pcs_recv_message(&pcs_can1, header.ExtId, header.DLC, data);
}

void can1_filter_config(uint8_t self_net_id)
{
    SELF_NET_ID = self_net_id;
    CAN_FilterTypeDef filter_cfg = {0};

    filter_cfg.FilterMode = CAN_FILTERMODE_IDMASK;
    filter_cfg.FilterScale = CAN_FILTERSCALE_32BIT;
    filter_cfg.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter_cfg.FilterActivation = ENABLE;
    filter_cfg.SlaveStartFilterBank = 14;

    filter_cfg.FilterBank = 0;
    filter_cfg.FilterIdHigh = (uint16_t)FILTER_HIGH_EXT(SELF_NET_ID << 8);
    filter_cfg.FilterIdLow = (uint16_t)FILTER_LOW_EXT(SELF_NET_ID << 8);
    filter_cfg.FilterMaskIdHigh = (uint16_t)FILTER_HIGH_EXT(0xFF00);
    filter_cfg.FilterMaskIdLow = (uint16_t)FILTER_LOW_EXT(0xFF00);

    HAL_CAN_ConfigFilter(&hcan, &filter_cfg);
}

static void can_send_msg( CAN_HandleTypeDef* can, 
                          uint16_t dest_id, 
                          uint8_t prio,
                          uint8_t *data, 
                          uint8_t size )
{
  CAN_TxHeaderTypeDef TxHeader = {0};
  uint32_t TxMailbox = 0;
  uint8_t net_cast = 0x00;

  TxHeader.StdId = 0;
  net_cast = (dest_id >> 8) & 0xFF;

  TxHeader.ExtId = (prio << 24) | 
                   (net_cast << 16) |
                   ((uint8_t)dest_id << 8) | 
                   SELF_NET_ID;

  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.IDE = CAN_ID_EXT;
  TxHeader.DLC = size;
  TxHeader.TransmitGlobalTime = DISABLE;

  while (HAL_CAN_GetTxMailboxesFreeLevel(can) == 0) {}

  HAL_CAN_AddTxMessage(can, &TxHeader, data, &TxMailbox);
}

void can1_send(uint16_t dest_id, uint8_t *data, uint32_t size)
{
  const uint8_t page = 8;
  uint8_t send_size_chunk = 0;

  for(int iBulk = 0; iBulk < size; iBulk += page)
  {
    send_size_chunk = page;
    if ( size < (iBulk + send_size_chunk) )
    {
      send_size_chunk = size - iBulk;
    }
    can_send_msg( &hcan,
                  dest_id, 
                  CAN_PRIORITY_NORMAL, 
                  (data+iBulk), 
                  send_size_chunk);
  }
}

void can1_start(void)
{
    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
}

