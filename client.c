#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define TCPPORT "25798" 
#define SERVERUDPPORTA "21798" 
#define SERVERUDPPORTB "22798"
#define SERVERUDPPORTC "23798"
#define SERVERUDPPORTD "24798"
#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 100
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{


    int sockfd, new_fd,numbytes;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    int buf[5];
    int matrix[4][4];
    int received[4];
    int receivedNumber = 0;
    int udpData[16];

/**
	int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                    const char *service,  // e.g. "http" or port number
                    const struct addrinfo *hints,
                    struct addrinfo **res);
	You give this function three input parameters, and it gives you a pointer to a linked-list, res, of results.

*/
    if ((rv = getaddrinfo(NULL, TCPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
	}
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
			continue; 
		}
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
			exit(1); 
		}

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: bind");
			continue; 
		}
		break;
	}
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "client: failed to bind\n");
        exit(1);
	}
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
	}

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
		exit(1); 
	}
    printf("client: waiting for connections...\n");
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
			continue; 
		}
        inet_ntop(their_addr.ss_family,
            	  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("client: got connection from %s\n", s);
      //  if (!fork()) { // this is the child process
      //      close(sockfd); // child doesn't need the listener
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1); 
            }
            received[buf[0]] = 1;
            for(int i = 0;i < 4;i++){
                matrix[buf[0]][i] = buf[i + 1];
            }
            close(new_fd);
            
            for(int i = 0;i < 4;i++){
                receivedNumber += received[i];
            }
            printf("%d\n",receivedNumber );
            if(receivedNumber == 4){
                break;
            }
            receivedNumber = 0;
			//exit(0); 
		//}
       // close(new_fd);  // parent doesn't need this
    }
    /**
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            printf("%d\n", matrix[i][j]);
        }
        printf("\n");
    }
    */

    //UDP
    


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; 
    if ((rv = getaddrinfo("127.0.0.1", SERVERUDPPORTA, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1; }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue; 
        }
        break; 
    }
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            udpData[i * 4 + j] = matrix[i][j];
        }
    }
    if ((numbytes = sendto(sockfd, udpData, 16 * sizeof *udpData, 0,p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1); 
    }
    freeaddrinfo(servinfo);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; 
    if ((rv = getaddrinfo("127.0.0.1", SERVERUDPPORTB, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1; }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue; 
        }
        break; 
    }
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            udpData[i * 4 + j] = matrix[i][j];
        }
    }
    if ((numbytes = sendto(sockfd, udpData, 16 * sizeof *udpData, 0,p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1); 
    }
    freeaddrinfo(servinfo);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; 
    if ((rv = getaddrinfo("127.0.0.1", SERVERUDPPORTC, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1; }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue; 
        }
        break; 
    }
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            udpData[i * 4 + j] = matrix[i][j];
        }
    }
    if ((numbytes = sendto(sockfd, udpData, 16 * sizeof *udpData, 0,p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1); 
    }
    freeaddrinfo(servinfo);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; 
    if ((rv = getaddrinfo("127.0.0.1", SERVERUDPPORTD, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1; }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue; 
        }
        break; 
    }
    for(int i = 0;i < 4;i++){
        for(int j = 0;j < 4;j++){
            udpData[i * 4 + j] = matrix[i][j];
        }
    }
    if ((numbytes = sendto(sockfd, udpData, 16 * sizeof *udpData, 0,p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1); 
    }
    freeaddrinfo(servinfo);


    close(sockfd);
	return 0; 
}