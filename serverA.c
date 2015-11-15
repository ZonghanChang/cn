#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define UDPPORT "21798" // the port client will be connecting to
#define CLIENTTCPPORT "25798"
#define MAXDATASIZE 100 // max number of bytes we can get at once

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    FILE *fp;
    char line[50];
    int max = 50;
    int costArray[5];
    int cost = 0;
    costArray[0] = 0;

    if (argc != 2) {
        fprintf(stderr,"usage: server hostname\n");
        exit(1);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    printf("The Server A is up and running.\n");

    // read file
    printf("The Server A has the following neighbor information:\n");
    printf("Neighbor----Cost\n");
    if((fp = fopen("/Users/zonghanchang/Documents/doc/course/EE450/test_trans/someVals", "r")) == NULL){
        printf("error");
        return 1;
    }

    else{
        while(fgets(line, max, fp) != NULL){
            printf("%s",line);
            int pos = strlen(line);
            while(!isalpha(line[pos])){
                pos--;
            }
            int temp = pos;
            while(!isdigit(line[pos])){
                pos++;
            }
            while(isdigit(line[pos])){
                cost = cost * 10 + line[pos] - '0';
                pos++;
            }

            costArray[line[temp] - 'A' + 1] = cost;
            cost = 0;
        }
        printf("\n");   
    }
    // read file end

    if ((rv = getaddrinfo(argv[1], CLIENTTCPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: connect");
            continue; 
        }
        break; 
    }
    
    if (p == NULL) {
        fprintf(stderr, "server: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    //printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    if (send(sockfd, costArray, 30, 0) == -1){
        perror("send");
    }

    close(sockfd);

    printf("The Server A finishes sending its neighbor information to the Client with TCP port number %s and IP address %s\n",CLIENTTCPPORT,s);
    
    return 0; 
}