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

#define MAXBUFLEN 100

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char const *argv[])
{
	int sk, num_bytes, err, val;
	char buf[MAXBUFLEN], s[INET6_ADDRSTRLEN];
	struct addrinfo hints, *srvinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t addr_len = sizeof(their_addr);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(NULL, argv[1], &hints, &srvinfo);
	if(err < 0) {
		goto error;
	}

	for(p = srvinfo ; p != NULL ; p = p->ai_next) {
		sk = socket(p->ai_family, p->ai_socktype, 
			p->ai_protocol);
		if(sk < 0) {
			perror("error on socket create");
			continue;
		}

		bind(sk, p->ai_addr, p->ai_addrlen);
		if(sk < 0) {
			perror("error on socket bind");
			continue;
		}
		break;
	}

	if(p == NULL) {
		perror("failed to bind a socket\n");
		goto error;
	}

	freeaddrinfo(srvinfo);

	while(1) {

		printf("\nListening on port %s...\n", argv[1]);
		num_bytes = recvfrom(sk, buf, MAXBUFLEN-1, 0, 
			(struct sockaddr *) &their_addr, &addr_len);
		if(num_bytes < 0) {
			perror("error on recvfrom");
			goto error;
		}

		buf[num_bytes] = '\0';
		printf("Listener: got packet from %s, Payload: %s\n",
		inet_ntop(their_addr.ss_family,
		    get_in_addr((struct sockaddr *)&their_addr),
		            s, sizeof s), buf);

		val = atoi(buf);
		val = val*2;
		sprintf(buf, "%d", val);

		printf("Sending back: %s \n", buf);
		num_bytes = sendto(sk, buf, strlen(buf), 0, 
			(struct sockaddr *) &their_addr, INET6_ADDRSTRLEN);
		if(num_bytes < 0) {
			perror("error on sendto");
			goto error;
		} else
			printf("Sent\n");
	}

error:
	close(sk);
	return 0;
}