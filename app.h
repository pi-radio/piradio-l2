/*
 * app.h
 *
 *  Created on: Aug 2, 2021
 *      Author: george
 */

#ifndef APP_H_
#define APP_H_

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

/*The L1 OFDM frame structure includes 10 symbols, the first of which
 * is the synchronization word. Of the next 9, 160 carriers are allocated
 * for pilot tones. This leaves a total of 5760 subcarriers for data per
 * frame. Considering the packet structure of the L2 which includes 8 bytes
 * header and 2 bytes tail (CRC) and the maximum payload is 230 bytes. */

#define HEADER_SIZE                    8
#define TAIL_SIZE                      2
#define HEADER_TAIL_SIZE               (HEADER_SIZE + TAIL_SIZE)
#define MAX_PACKET_SIZE                240
#define MAX_PAYLOAD_PER_PACKET         (MAX_PACKET_SIZE - HEADER_TAIL_SIZE)

#define MAX_TRANSFER_LEN               100000
//#define DDR_MAP_SIZE                   0x40000000
//#define DDR_MAP_MASK                   (DDR_MAP_SIZE - 1)

/* Maximum payload size per transfer coming from upper layers */
#define MAX_PAYLOAD                    0x10000000 // 1GB
#define DDR_BASE_ADDRESS               0x10000000



typedef struct header{
	uint16_t seq_num;
	uint16_t packet_len;
	uint16_t src_addr;
	uint16_t dst_addr;
}header_t;

#endif /* APP_H_ */
