/*
 * app.c
 *
 *  Created on: Aug 2, 2021
 *      Author: george
 */
#include "app.h"
#include "utils.h"
#include "crc.h"
#include "string.h"
#include <errno.h>

#define RX_PORT 16682

#define PS_TO_PL_DMA       XPAR_AXIDMA_0_BASEADDR

#define PL_TO_PS_DMA       XPAR_AXIDMA_1_BASEADDR

#define DDR_RX_BASE_ADDR   1
#define SIZE_RX            20000

uint16_t seq_num_global = 0;

int
piradio_l2_tx(int connfd, void* ddr_base, void* ps_to_pl_dma){
    int payload_size = 0;
    int byte_count = 0;
    int n;
    size_t num_packets;
    header_t header;
    header.dst_addr = 0x0001;
    header.src_addr = 0x0002;
    uint16_t crc;
    while(payload_size != -1) {
    	/* Receive size of transfer */
    	n = recv(connfd, &payload_size, sizeof(int),
									 0);
    	/* Receive payload */
    	while(byte_count < payload_size){
    		n = recv(connfd, (uint8_t *)(ddr_base + byte_count), 1500,
    											 0);
    		byte_count+= n;
    	}
    	if(payload_size > 0){
    		num_packets = payload_size / MAX_PAYLOAD_PER_PACKET;
    		if(payload_size % MAX_PAYLOAD_PER_PACKET != 0)
    			num_packets++;
    		for(int i = 0; i < num_packets; i++){
    			header.packet_len = (i * MAX_PAYLOAD_PER_PACKET + MAX_PAYLOAD_PER_PACKET <= payload_size)
    					? (MAX_PAYLOAD_PER_PACKET + HEADER_TAIL_SIZE)
    					: ((payload_size - i * MAX_PAYLOAD_PER_PACKET) + HEADER_TAIL_SIZE );
    			header.seq_num = seq_num_global++;
    			memcpy(ddr_base + MAX_PAYLOAD + i * MAX_PACKET_SIZE, &header, sizeof(header_t));
    			memcpy(ddr_base + MAX_PAYLOAD + i * MAX_PACKET_SIZE + sizeof(header_t)
    					, ddr_base + i * MAX_PAYLOAD_PER_PACKET, header.packet_len - HEADER_TAIL_SIZE);
    			crc = calc_crc(ddr_base + MAX_PAYLOAD + i * MAX_PACKET_SIZE, header.packet_len - TAIL_SIZE);
    			memcpy(ddr_base + MAX_PAYLOAD + i * MAX_PACKET_SIZE + sizeof(header_t) + header.packet_len - HEADER_TAIL_SIZE
    					, &crc, sizeof(uint16_t));
    		}
    		/* Wait until transfer is over */
    		config_tx_dma(ps_to_pl_dma,DDR_BASE_ADDRESS + MAX_PAYLOAD, payload_size + num_packets * HEADER_TAIL_SIZE);
    		while(! (XAXIDMA_IRQ_IOC_MASK && read_from_base(ps_to_pl_dma + XAXIDMA_TX_OFFSET + XAXIDMA_SR_OFFSET))){
    			// Busy wait
    		}
    	}
    	byte_count = 0;
    }
}

int piradio_l2_rx(int connfd, void* ddr_rx_base, void* ddr_rx_payload_base,
		void* bd_base, void* dma_base) {
	uint32_t length;
	size_t total_received = 0, remaining = 0, payload_len= 0, current_used_len = 0;
	setup_sg_desciptors(bd_base, BD_BASE_ADDR, BUFFER_ADDR,
			DESCRIPTOR_CHAIN_LEN);
	rx_state_t state;
	header_t header;
	while (1) {
		setup_sg_dma(dma_base, BD_BASE_ADDR,
				(BD_BASE_ADDR - 1) * XAXIDMA_BD_MINIMUM_ALIGNMENT);
		total_received = 0;
		for (int i = 0; i++; i < DESCRIPTOR_CHAIN_LEN) {
			current_used_len = 0;
			length = is_bd_complete(bd_base, i);
			while (!length) {
				length = is_bd_complete(bd_base, i);
			}
			if (length < BYTES_PER_SYMBOL) {
				state = HEADER_SEARCH;
				continue;
			}
			while (current_used_len < length) {
				switch (state) {
				case HEADER_SEARCH:
					if (current_used_len + HEADER_SIZE < length) {
						header = is_header_valid(
								ddr_rx_base + i * BYTES_PER_SYMBOL
										+ current_used_len);
						if (header) {
							if (header.packet_len < length) {
								payload_len = depacketize(&header,
										ddr_rx_base + i * BYTES_PER_SYMBOL
												+ current_used_len,
										ddr_rx_payload_base + total_received);
								total_received += payload_len;
								current_used_len += header.packet_len;
							} else {
								state = HEADER_FOUND;
								remaining = length - current_used_len;
								current_used_len = length;
							}
						} else{
							current_used_len = length;
							break;
						}
					}
					break;
				case HEADER_FOUND:
					if(header.packet_len > (length + remaining)){
						remaining = remaining + length;
					}
					else{
						payload_len = depacketize(&header,
								ddr_rx_base + i * BYTES_PER_SYMBOL - remaining,
								ddr_rx_payload_base + total_received);
						total_received += payload_len;
						current_used_len = length - (header.packet_len - remaining);
						state = HEADER_SEARCH;
					}
					break;
				}
			}
		}
	}
}

int main(){
	int memfd;
	void * ddr_base;
	void * ps_to_pl_dma;
	void * pl_to_ps_dma;
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		exit(0);
	}
	map_device(&ddr_base, memfd, DDR_BASE_ADDRESS, DDR_MAP_SIZE,
			DDR_MAP_MASK, MAX_TRANSFER_LEN);

	map_device(&ps_to_pl_dma, memfd, PS_TO_PL_DMA, MAP_SIZE,
			MAP_MASK, 0);

	map_device(&pl_to_ps_dma, memfd, PL_TO_PS_DMA, MAP_SIZE,
			MAP_MASK, 0);

	reset_tx_dma_engine(ps_to_pl_dma);
	reset_rx_dma_engine(pl_to_ps_dma);


	int sockfd, connfd;

	int error;

	struct sockaddr_in servaddr, cliaddr;

	struct sockaddr_in servaddr_tx, cliaddr_tx;

	socklen_t len;
	len = sizeof(cliaddr);

	error = config_tcp_sock(sockfd, &servaddr, RX_PORT);

	if(!error)
		exit(0);

	connfd = get_tcp_connection(sockfd, &servaddr, &cliaddr);

	if(!connfd)
		exit(0);

	config_rx_dma(pl_to_ps_dma, DDR_RX_BASE_ADDR, SIZE_RX);

	piradio_l2_tx(connfd, ddr_base, ps_to_pl_dma);


	piradio_l2_rx(connfd, ddr_base, ps_to_pl_dma);


}
