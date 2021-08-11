/*
 * network.h
 *
 *  Created on: Aug 11, 2021
 *      Author: george
 */

#ifndef SRC_NETWORK_H_
#define SRC_NETWORK_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int
config_tcp_sock(int sockfd, struct sockaddr_in* server, int port);

int
get_tcp_connection(int sockfd, struct sockaddr_in* server, struct sockaddr_in* client);


#endif /* SRC_NETWORK_H_ */
