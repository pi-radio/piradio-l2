#include "network.h"

int
config_tcp_sock(int sockfd, struct sockaddr_in* server, int port){
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket creation failed");
			return -1;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		    error("setsockopt(SO_REUSEADDR) failed");

		memset(server, 0, sizeof(struct sockaddr_in));


		server->sin_family    = AF_INET; // IPv4
		server->sin_addr.s_addr = INADDR_ANY;
		server->sin_port = htons(port);

		if (bind(sockfd, (const struct sockaddr *)server,
		         sizeof(struct sockaddr_in)) < 0) {
			perror("bind failed");
			return -1;
		}
		return 0;
}

int
get_tcp_connection(int sockfd, struct sockaddr_in* server, struct sockaddr_in* client){
	memset(client, 0, sizeof(struct sockaddr_in));
	if ((listen(sockfd, 5)) != 0) {
	        printf("Listen failed...\n");
	        return -1;
	}

    int connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        return -1;
    }
    return connfd;
}
