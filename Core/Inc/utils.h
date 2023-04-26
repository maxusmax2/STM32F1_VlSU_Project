#include "main.h"
#ifndef INC_UTILS_H_
#define INC_UTILS_H_
#define SYNQSEQ_DEF 0x4563ffff
typedef struct
{
	uint32_t synqseq;
	uint16_t cmd;
	uint16_t src_id;
	uint16_t dest_id;
	uint8_t misc;
	uint8_t pack_cnt;
	uint16_t byte_cnt;
	uint16_t crc16;
}syspkg_t;
uint16_t utils_crc16(uint8_t *pcBlock, uint32_t len);
#endif /* INC_UTILS_H_ */
