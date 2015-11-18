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
#define UDPPORT "24798" // the port client will be connecting to
#define CLIENTTCPPORT "25798"
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define MAXBUFLEN 100
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
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    int rv;
    char s[INET6_ADDRSTRLEN];
    int buf[MAXBUFLEN];
    FILE *fp;
    char line[50];
    int max = 50;
    int costArray[5];
    int cost = 0;
    costArray[0] = 3;
    socklen_t addr_len;
    for(int i = 1;i < 5;i++){
        costArray[i] = 0;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    printf("The Server D is up and running.\n");

    // read file
    printf("The Server D has the following neighbor information:\n");
    printf("Neighbor----Cost\n");
    if((fp = fopen("/Users/zonghanchang/Documents/doc/course/EE450/socket/src/serverD.txt", "r")) == NULL){
        printf("error");
        return 1;
    }

    else{
        while(fgets(line, max, fp) != NULL){
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
            printf("server%c     %d\n",line[temp],cost);
            cost = 0;
        }
        printf("\n");   
    }
    // read file end

    if ((rv = getaddrinfo("127.0.0.1", CLIENTTCPPORT, &hints, &servinfo)) != 0) {
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

    //close(sockfd);
    struct sockaddr client_addr;
    socklen_t client_addrlen = sizeof client_addr;
    getpeername(sockfd,&client_addr,&client_addrlen);
    printf("The Server D finishes sending its neighbor information to the Client with TCP port number %u and IP address %s\n",ntohs(((struct sockaddr_in *)&client_addr)->sin_port),s);

    struct sockaddr local_addr;
    socklen_t local_addrlen = sizeof local_addr;
    getsockname(sockfd,&local_addr,&local_addrlen);
    char l[INET6_ADDRSTRLEN];
    inet_ntop(local_addr.sa_family,get_in_addr(&local_addr),l, sizeof l);
    printf("For this connection with Client, the server D has TCP port number %u and IP address %s\n",ntohs(((struct sockaddr_in *)&local_addr)->sin_port),l);

    // UDP
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;


    if ((rv = getaddrinfo("127.0.0.1", UDPPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
        continue; 
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
        continue; 
        }
        break; 
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    //getpeername(sockfd,&client_addr,&client_addrlen);
    //inet_ntop(client_addr.sa_family,get_in_addr(&client_addr),s, sizeof s);
    //printf("The server D has received the network topology from the Client with UDP port number %u and IP address %s (Client's UDP port number and IP address) as follows:\n",(((struct sockaddr_in *)&client_addr)->sin_port),s);
    
    char t[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),t, sizeof t);
    printf("The server D has received the network topology from the Client with UDP port number %u and IP address %s (Client's UDP port number and IP address) as follows:\n",ntohs(((struct sockaddr_in *)&their_addr)->sin_port),t);

    printf("Edge----Cost\n");
    for(int i = 0;i < 16;i++){
        if(buf[i] != 0 && i/4 < i%4){
            printf("%c%c      %d\n",'A' + i/4, 'A' + i%4,buf[i]);
        }
    }


    getsockname(sockfd,&local_addr,&local_addrlen);
    inet_ntop(local_addr.sa_family, get_in_addr(&local_addr),l, sizeof l);
    printf("For this connection with Client,The Server D has UDP port number %u and IP address %s\n",ntohs(((struct sockaddr_in *)&local_addr)->sin_port),l);

    



    return 0; 
}