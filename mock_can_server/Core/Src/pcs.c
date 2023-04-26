/** Protocol Communication System */

#include "pcs.h"

static waiter_t *check_waiter(pcs_conf_t *c, uint8_t cmd, uint8_t dest_id);
static waiter_t *add_to_waiter(pcs_conf_t *c, uint8_t cmd, uint8_t dest_id);
static void task_recv(void *arg);
static void task_send(void *arg);
static void send_message_phy(pcs_conf_t *c, send_msg_t *msg);
static void add_frame_to_pool(pcs_conf_t *c, recv_frame_t *msg);
static void parse_pool(pcs_conf_t *c, uint8_t box_num);

void pcs_init(pcs_conf_t *cfg)
{
    cfg->q_recv = xQueueCreate(5, sizeof(recv_frame_t));
    cfg->q_send = xQueueCreate(2, sizeof(send_msg_t));
    xTaskCreate(task_recv, "recv", 256, (void *)cfg, 0, &cfg->th_recv);
    xTaskCreate(task_send, "send", 256, (void *)cfg, 0, &cfg->th_send);
    for (int i = 0; i < PCS_MAX_DEV_CNT; ++i)
    {
        cfg->waiter_pool[i].sem = xSemaphoreCreateCounting(1, 0);
    }
}

static void cmd_create(pcs_conf_t *c, syspkg_t *pkg, uint8_t *payload,
                       uint32_t size)
{
    pkg->synqseq = SYNQSEQ_DEF;
    pkg->src_id = c->self_net_id;
    pkg->byte_cnt = sizeof(syspkg_t) + size;

    if (payload == NULL)
    {
        pkg->crc16 =
            utils_crc16((uint8_t *)pkg, (sizeof(syspkg_t) - sizeof(uint16_t)));
    }
    else
    {
        pkg->crc16 =
            utils_crc16((uint8_t *)pkg, (sizeof(syspkg_t) - sizeof(uint16_t)));
        pkg->crc16 += utils_crc16(payload, (pkg->byte_cnt - sizeof(syspkg_t)));
    }
}

void pcs_send_message(pcs_conf_t *c, uint16_t dest, syspkg_t *pkg,
                      uint8_t *payload, uint32_t size)
{
    cmd_create(c, pkg, payload, size);
    send_msg_t send_pkg = {.pkg = *pkg, .dest_id = dest};

    if ((pkg->byte_cnt > sizeof(syspkg_t)) && (payload != NULL))
    {
        memcpy(send_pkg.payload, payload, pkg->byte_cnt - sizeof(syspkg_t));
    }
    xQueueSend(c->q_send, (void *)&send_pkg, 0);
}

pcs_resp_t *pcs_send_with_resp(pcs_conf_t *c, uint16_t dest, syspkg_t *pkg,
                               uint8_t *payload, uint32_t size,
                               uint8_t wait_cmd, uint32_t timeout)
{
    cmd_create(c, pkg, payload, size);
    send_msg_t send_pkg = {.pkg = *pkg, .dest_id = dest};

    if ((pkg->byte_cnt > sizeof(syspkg_t)) && (payload != NULL))
        memcpy(send_pkg.payload, payload, pkg->byte_cnt - sizeof(syspkg_t));

    waiter_t *w = add_to_waiter(c, wait_cmd, dest);

    xQueueSend(c->q_send, (void *)&send_pkg, 0);
    if (xSemaphoreTake(w->sem, timeout))
    {
        return &w->resp;
    }
    return NULL;
}

static waiter_t *add_to_waiter(pcs_conf_t *c, uint8_t cmd, uint8_t dest_id)
{
    static uint8_t dev_cnt = 0;
    uint8_t exist = 0;
    waiter_t *w;

    for (int i = 0; i < dev_cnt; ++i)
    {
        if (c->waiter_pool[i].dest_id == dest_id)
        {
            w = &c->waiter_pool[i];
            c->waiter_pool[i].wait_flag = 1;
            c->waiter_pool[i].cmd = cmd;
            exist = 1;
        }
    }
    if (!exist)
    {
        w = &c->waiter_pool[dev_cnt];
        c->waiter_pool[dev_cnt].dest_id = dest_id;
        c->waiter_pool[dev_cnt].wait_flag = 1;
        c->waiter_pool[dev_cnt].cmd = cmd;
        dev_cnt++;
    }
    return w;
}

void pcs_recv_message(pcs_conf_t *c, uint16_t id, uint8_t len, uint8_t *data)
{
    BaseType_t tsk_woken = pdFALSE;
    recv_frame_t frame = {.src_id = id, .len = len};
    memcpy(frame.data, data, len);

    xQueueSendFromISR(c->q_recv, (void *)&frame, &tsk_woken);
}

static void task_recv(void *arg)
{
    pcs_conf_t *cfg = (pcs_conf_t *)arg;
    recv_frame_t frame = {0};

    for (;;)
    {
        if (xQueueReceive(cfg->q_recv, &frame, 1))
            add_frame_to_pool(cfg, &frame);
        for (uint8_t i = 0; i < cfg->dev_cnt; ++i)
            parse_pool(cfg, i);
    }
}

static void task_send(void *arg)
{
    pcs_conf_t *cfg = (pcs_conf_t *)arg;
    send_msg_t msg;

    for (;;)
    {
        if (xQueueReceive(cfg->q_send, &msg, portMAX_DELAY))
            send_message_phy(cfg, &msg);
    }
}

static void send_message_phy(pcs_conf_t *c, send_msg_t *msg)
{
    c->can_send(msg->dest_id, (uint8_t *)&msg->pkg, sizeof(syspkg_t));
    if (msg->pkg.byte_cnt > sizeof(syspkg_t))
        c->can_send(msg->dest_id, msg->payload,
                    msg->pkg.byte_cnt - sizeof(syspkg_t));
}

static void add_frame_to_pool(pcs_conf_t *c, recv_frame_t *msg)
{
    mailbox_t *m = c->mailbox;
    uint32_t size = msg->len;
    uint8_t src_id = msg->src_id;
    uint8_t is_dev_exist = 0;

    for (uint8_t i = 0; i < c->dev_cnt; i++)
    {
        if (src_id == m[i].can_id)
        {
            uint32_t div = PCS_POOL_SIZE - m[i].wr_cnt;
            if (div < size)
            {
                memcpy(m[i].pool + m[i].wr_cnt, msg->data, div);
                m[i].wr_cnt = 0;
                memcpy(m[i].pool, (msg->data + div), (size - div));
                m[i].wr_cnt = (size - div);
            }
            else
            {
                memcpy(m[i].pool + m[i].wr_cnt, msg->data, size);
                m[i].wr_cnt += size;
            }
            is_dev_exist = 1;
            break;
        }
    }
    if (is_dev_exist != 1)
    {
        m[c->dev_cnt].can_id = src_id;
        m[c->dev_cnt].byte_cnt = sizeof(syspkg_t);
        m[c->dev_cnt].rd_ptr = 0;
        m[c->dev_cnt].wr_cnt = size;
        memcpy(m[c->dev_cnt].pool, msg->data, size);
        c->dev_cnt++;
    }
}

static void parse_pool(pcs_conf_t *c, uint8_t box_num)
{
    mailbox_t *mb = &c->mailbox[box_num];
    uint32_t endpoint = 0;
    uint32_t tmpsize = 0;
    uint16_t crc = 0;
    syspkg_t *pkg;

    if (mb->wr_cnt == mb->rd_ptr)
    {
        mb->start_packet = 0;
        return;
    }
    else
    {
        if (mb->start_packet == 0)
        {
            mb->fixtime = xTaskGetTickCount();
            mb->start_packet = 1;
        }
        else
        {
            uint32_t time_div = (xTaskGetTickCount() - mb->fixtime);
            if (time_div >= 200)
            {
                mb->byte_cnt = sizeof(syspkg_t);
                mb->rd_ptr = mb->wr_cnt;
                mb->start_packet = 0;
                return;
            }
        }
        if (mb->wr_cnt < mb->rd_ptr)
            tmpsize = PCS_POOL_SIZE - mb->rd_ptr + mb->wr_cnt;
        else
            tmpsize = mb->wr_cnt - mb->rd_ptr;

        if (tmpsize < mb->byte_cnt)
            return;
    }

    endpoint = (mb->rd_ptr + mb->byte_cnt);
    if (endpoint > PCS_POOL_SIZE)
    {
        endpoint = endpoint - PCS_POOL_SIZE;
    }
    if (endpoint > mb->rd_ptr)
    {
        memcpy(mb->ln_data, mb->pool + mb->rd_ptr, mb->byte_cnt);
    }
    else
    {
        memcpy(mb->ln_data, mb->pool + mb->rd_ptr,
               (PCS_POOL_SIZE - mb->rd_ptr));
        memcpy(mb->ln_data + (PCS_POOL_SIZE - mb->rd_ptr), mb->pool, endpoint);
    }

    pkg = (syspkg_t *)mb->ln_data;

    if (pkg->synqseq != SYNQSEQ_DEF)
    {
        int offset = 0;
        while (pkg->synqseq != SYNQSEQ_DEF)
        {
            if (mb->rd_ptr == mb->wr_cnt)
            {
                mb->byte_cnt = sizeof(syspkg_t);
                mb->start_packet = 0;
                return;
            }
            offset++;
            pkg = (syspkg_t *)(mb->ln_data + offset);
            mb->rd_ptr++;
            if (mb->rd_ptr > PCS_POOL_SIZE)
                mb->rd_ptr = 0;
        }
    }

    if (pkg->synqseq == SYNQSEQ_DEF)
    {
        if (pkg->byte_cnt != mb->byte_cnt)
        {
            if (pkg->byte_cnt < sizeof(syspkg_t) ||
                pkg->byte_cnt >= PCS_POOL_SIZE)
            {
                mb->byte_cnt = sizeof(syspkg_t);
                mb->rd_ptr = mb->wr_cnt;
            }
            else
            {
                mb->byte_cnt = pkg->byte_cnt;
            }
            mb->start_packet = 0;
            mb->ln_data[0] = 0;
            return;
        }

        if (c->proxy_en)
        {
            for (int i = 0; i < c->proxy_cnt; ++i)
            {
                if (pkg->dest_id == c->proxy_list[i])
                    goto allow;
            }
        }

        if ((pkg->dest_id == 0xFF) || (pkg->dest_id == c->self_net_id))
        {
        allow:
            if (pkg->byte_cnt == sizeof(syspkg_t))
            {
                crc = utils_crc16((uint8_t *)pkg,
                                  sizeof(syspkg_t) - sizeof(uint16_t));
            }
            else
            {
                crc = utils_crc16((uint8_t *)pkg,
                                  sizeof(syspkg_t) - sizeof(uint16_t));
                crc += utils_crc16((uint8_t *)(mb->ln_data + sizeof(syspkg_t)),
                                   (pkg->byte_cnt - sizeof(syspkg_t)));
            }
            if (crc == pkg->crc16)
            {
                waiter_t *w = check_waiter(c, pkg->cmd, pkg->src_id);
                if (w)
                {
                    w->resp.pkg = pkg;
                    w->resp.data = ((uint8_t *)pkg) + sizeof(syspkg_t);
                    xSemaphoreGive(w->sem);
                }
                else
                {
                    c->recv_callback((void *)pkg);
                }
            }
            else
            {
                // ERROR!
            }
        }
    }
    mb->start_packet = 0;
    mb->byte_cnt = sizeof(syspkg_t);

    uint32_t bc_temp = mb->rd_ptr + pkg->byte_cnt;
    mb->rd_ptr =
        (bc_temp > PCS_POOL_SIZE) ? (bc_temp - PCS_POOL_SIZE) : bc_temp;
    mb->ln_data[0] = 0;
}

static waiter_t *check_waiter(pcs_conf_t *c, uint8_t cmd, uint8_t dest_id)
{
    for (int i = 0; i < PCS_MAX_DEV_CNT; ++i)
    {
        if (c->waiter_pool[i].dest_id == dest_id)
        {
            if ((c->waiter_pool[i].wait_flag == 1) &&
                (c->waiter_pool[i].cmd == cmd))
                return &c->waiter_pool[i];
        }
    }
    return NULL;
}
