/*
 * utils.c
 *
 *  Created on: Jul 30, 2021
 *      Author: george
 */
#include "utils.h"
#include "xaxidma_hw.h"
#include <sys/mman.h>

header_t
is_header_valid(uint8_t* buffer){
	header_t header;
	memcpy(&header, src, sizeof(header_t));
	if(header.packet_len > MAX_PACKET_SIZE){
		return NULL;
	}
	if(header.dst_addr != LOCAL_MAC_ADDR)
		return NULL;

	return header;
}

int
depacketize(header_t * header, uint8_t* src, uint8_t* dst){
	uint16_t crc_pkt = (src[header->packet_len - 2] << 8) | src[header->packet_len - 1];
	uint16_t crc_calc = calc_crc(src, header->packet_len - 2);

	if(crc_pkt != crc_calc)
		return -2;

	memcpy(dst, &src[sizeof(header_t)], header->packet_len - HEADER_TAIL_SIZE);
	return header->packet_len - HEADER_TAIL_SIZE;
}

void
write_to_base(void* base, uint32_t value){
	*((volatile uint32_t*)base) = value;
}

uint32_t
read_from_base(void* base){
	return *((volatile uint32_t*) base);
}

void
map_device(void** mapped_base, int mem_desc, uint32_t addr, uint32_t map_size,
		uint32_t map_mask, uint32_t size){
	void* tmp = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			mem_desc, addr & ~map_mask);
	if (tmp == (void*) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}

	*mapped_base = tmp + (addr & map_mask);

	memset(*mapped_base, 0, size);

	return;
}

void reset_tx_dma_engine(void* base){
	uint32_t ResetMask = (uint32_t) XAXIDMA_CR_RESET_MASK;
	uint32_t ResetMask_tx;
	uint8_t Timeout = 10;

	write_to_base(base + XAXIDMA_TX_OFFSET +
			XAXIDMA_CR_OFFSET, ResetMask);
	do {

		ResetMask_tx = read_from_base(base + XAXIDMA_TX_OFFSET
				+ XAXIDMA_CR_OFFSET);

		if (!(ResetMask_tx & XAXIDMA_CR_RESET_MASK)) {
			break;
		}
		Timeout -= 1;
	} while (Timeout);

	if(Timeout == 0)
		printf("Engine not reset\n");

	return;
}

void reset_rx_dma_engine(void* base){
	uint32_t ResetMask = (uint32_t) XAXIDMA_CR_RESET_MASK;
	uint32_t ResetMask_rx;
	uint8_t Timeout = 10;

	write_to_base(base + XAXIDMA_RX_OFFSET +
			XAXIDMA_CR_OFFSET, ResetMask);
	do {

		ResetMask_rx = read_from_base(base + XAXIDMA_RX_OFFSET
				+ XAXIDMA_CR_OFFSET);

		if (!(ResetMask_rx & XAXIDMA_CR_RESET_MASK)) {
			break;
		}
		Timeout -= 1;
	} while (Timeout);

	if(Timeout == 0)
		printf("Engine not reset\n");

	return;
}

void config_tx_dma(void* base, uint32_t src_addr, uint32_t length){

	uint32_t Regvalue = read_from_base(base + XAXIDMA_TX_OFFSET +
			XAXIDMA_CR_OFFSET);
	Regvalue = (uint32_t)(Regvalue | XAXIDMA_CR_RUNSTOP_MASK);
	write_to_base(base + XAXIDMA_TX_OFFSET +
	XAXIDMA_CR_OFFSET,Regvalue); // Start engine

	write_to_base(base + XAXIDMA_TX_OFFSET +
	XAXIDMA_DESTADDR_OFFSET, src_addr); // Destination address
	write_to_base(base + XAXIDMA_TX_OFFSET +
		XAXIDMA_BUFFLEN_OFFSET, length);

	return;
}

void config_rx_dma(void* base, uint32_t dst_addr, uint32_t length){
	uint32_t Regvalue = read_from_base(base + XAXIDMA_RX_OFFSET +
			XAXIDMA_CR_OFFSET);
	Regvalue = (uint32_t)(Regvalue | XAXIDMA_CR_RUNSTOP_MASK);
	write_to_base(base + XAXIDMA_RX_OFFSET +
	XAXIDMA_CR_OFFSET,Regvalue); // Start engine

	write_to_base(base + XAXIDMA_RX_OFFSET +
	XAXIDMA_DESTADDR_OFFSET, dst_addr); // Destination address
	write_to_base(base + XAXIDMA_RX_OFFSET +
		XAXIDMA_BUFFLEN_OFFSET, length);
	return;
}

void
reset_pl(int memfd){
	void *reset;

	map_device(&reset, memfd, 0XFF0A0054, DDR_MAP_SIZE,
			DDR_MAP_MASK, 4);

	write_to_base(reset, 0x80000000U);
	usleep(10);
	write_to_base(reset, 0x00000000U);
	usleep(10);
	write_to_base(reset, 0x80000000U);
}

int
setup_sg_desciptors(void* base, uint32_t bd_base_addr, uint32_t buffer_address, int num_desc){
	for(int i = 0; i++; i < num_desc){
		write_to_base(base + i * XAXIDMA_BD_MINIMUM_ALIGNMENT
				+ XAXIDMA_BD_NDESC_OFFSET, bd_base_addr + (i + 1) * XAXIDMA_BD_MINIMUM_ALIGNMENT);
		write_to_base(base + i * XAXIDMA_BD_MINIMUM_ALIGNMENT
				+ XAXIDMA_BD_BUFA_OFFSET, buffer_address + i * BYTES_PER_SYMBOL);
		write_to_base(base + i * XAXIDMA_BD_MINIMUM_ALIGNMENT
				+ XAXIDMA_BD_CTRL_LEN_OFFSET, BYTES_PER_SYMBOL);
		write_to_base(bd_base + i * XAXIDMA_BD_MINIMUM_ALIGNMENT + XAXIDMA_BD_STS_OFFSET, 0);
	}
}

int
setup_sg_dma(void * dma_base, uint32_t bd_head, uint32_t bd_tail){
	write_to_base(dma_base + XAXIDMA_RX_OFFSET + XAXIDMA_CDESC_OFFSET,
			bd_head);
	uint32_t regvalue = read_from_base(dma_base + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET);
	regvalue = (uint32_t) (regvalue | XAXIDMA_CR_RUNSTOP_MASK);
	write_to_base(dma_base + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET, regvalue);
	write_to_base(dma_base + XAXIDMA_RX_OFFSET + XAXIDMA_TDESC_OFFSET, bd_tail);
}

int
is_bd_complete(void* bd_base, int offset){
	if(read_from_base(bd_base + offset * XAXIDMA_BD_MINIMUM_ALIGNMENT +
			XAXIDMA_BD_STS_OFFSET) & XAXIDMA_BD_STS_COMPLETE_MASK)
		return read_from_base(bd_base + offset * XAXIDMA_BD_MINIMUM_ALIGNMENT +
				XAXIDMA_BD_STS_OFFSET) & 0x03FFFFFF;
	else
		return 0;
}

int
reset_bd(void* bd_base, int offset){
	write_to_base(bd_base + offset * XAXIDMA_BD_MINIMUM_ALIGNMENT + XAXIDMA_BD_STS_OFFSET, 0);
}
