/*
 * utils.h
 *
 *  Created on: Jul 30, 2021
 *      Author: george
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <bits/stdint-uintn.h>
#include "defines.h"
#include "crc.h"
#include "xaxidma_hw.h"


void
reset_pl(int memfd);

void
write_to_base(void* base, uint32_t value);

uint32_t
read_from_base(void* base);

void
map_device(void** mapped_base, int mem_desc, uint32_t addr, uint32_t map_size,
		uint32_t map_mask, uint32_t size);

void
reset_tx_dma_engine(void* base);

void
reset_rx_dma_engine(void* base);

void
config_tx_dma(void* base, uint32_t src_addr, uint32_t length);

void
config_rx_dma(void* base, uint32_t dst_addr, uint32_t length);

/**
 * Returns number of bytes of payload, or error code
 */
int
depacketize(uint8_t* src, uint8_t* dst);

int
setup_sg_desciptors(void* base, uint32_t buffer_address, int num_desc);

int
setup_sg_dma(void* dma_base, uint32_t bd_head, uint32_t bd_tail);

int
is_bd_complete(void* base, int offset);

int
reset_bd(void* bd_base, int offset);

header_t
is_header_valid(uint8_t* buffer);
#endif /* UTILS_H_ */
