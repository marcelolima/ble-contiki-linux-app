#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>


int main(int argc, char const *argv[]) {

	int sk, n, num_bytes, err;
	uint32_t payload = atoi(argv[3]);
	char *ifname = "bt0";
	struct ifreq ifr;
	struct addrinfo hints, *srvinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;

	err = getaddrinfo(argv[1], argv[2], &hints, &srvinfo);
	if(err < 0) {
		perror("error on getaddrinfo");
		goto error;
	}

	for(p = srvinfo ; p != NULL ; p = p->ai_next) {
		sk = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol);
		if(sk < 0) {
			perror("error on socket create");
			continue;
		}
		break;
	}

	if(p == NULL) {
		perror("failed to bind a socket");
		goto error;
	}

	if (setsockopt(sk, SOL_SOCKET, SO_BINDTODEVICE, ifname, 4) < 0) {
	     fprintf(stderr, "error at setsockopt, ensure it is running as root.\n");
	     exit(1);
	}

	num_bytes = sendto(sk, &payload, sizeof(payload), 0,
		p->ai_addr, p->ai_addrlen);
	if(num_bytes < 0) {
		perror("error on sendto");
		goto error;
	}
	printf("Sent: %d\n", payload);

	num_bytes = recv(sk, &payload, sizeof(payload), 0);
	printf("Received: %d\n", payload);
	
error: 
	freeaddrinfo(srvinfo);
	close(sk);
	return 0;
}

