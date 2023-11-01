/*
 * crc.h
 *
 *  Created on: Mar 9, 2022
 *      Author: narek.aleksanyan
 */

#ifndef CRC_H_
#define CRC_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
uint32_t crc32(const void *buf, uint8_t size);
unsigned short crc16(unsigned char* data_p, unsigned short length);
#ifdef __cplusplus
}
#endif
#endif /* CRC_H_ */
