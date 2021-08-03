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

#define DDR_MAP_SIZE 0x40000000
#define DDR_MAP_MASK (DDR_MAP_SIZE - 1)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

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



#endif /* UTILS_H_ */
