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

uint16_t seq_num_global = 0;

int main(){
	int memfd;
	void* ddr_base;
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		exit(0);
	}
	map_device(&ddr_base, memfd, DDR_BASE_ADDRESS, DDR_MAP_SIZE,
			DDR_MAP_MASK, MAX_TRANSFER_LEN);

	int sockfd, connfd;

	struct sockaddr_in servaddr, cliaddr;

	struct sockaddr_in servaddr_tx, cliaddr_tx;

	socklen_t len;
	len = sizeof(cliaddr);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	    error("setsockopt(SO_REUSEADDR) failed");

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family    = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(RX_PORT);


	if (bind(sockfd, (const struct sockaddr *)&servaddr,
	         sizeof(servaddr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if ((listen(sockfd, 5)) != 0) {
	        printf("Listen failed...\n");
	        exit(0);
	}

    connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    }
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
    	}
    	byte_count = 0;
    }
}
