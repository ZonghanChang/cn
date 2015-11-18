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




#define maxVertices 1000
#define maxEdges 1000
int graph[maxVertices][maxVertices];
/* Input graph must be undirected,weighted and connected*/
typedef struct Edge
{
    int from,to,weight;
}Edge;
int compare(const void * x,const void * y)
{
    return (*(Edge *)x).weight - (*(Edge *)y).weight;
}
Edge E[maxEdges];
int parent[maxVertices];
void init(int vertices)
{
    int iter=0;
    for(iter=0;iter<vertices;iter++)
    {
        parent[iter]=-1;
    }

}
int Find(int vertex)
{
    if(parent[vertex]==-1)return vertex;
    return parent[vertex] = Find(parent[vertex]); /* Finding its parent as well as updating the parent 
                             of all vertices along this path */
}
void Union(int parent1,int parent2)
{
    /* This can be implemented in many other ways. This is one of them */
    parent[parent1] = parent2;
}
void Kruskal(int vertices,int edges)
{
    memset(graph,-1,sizeof(graph)); /* -1 represents the absence of edge between them */
    /* Sort the edges according to the weight */
    qsort(E,edges,sizeof(Edge),compare);
    /* Initialize parents of all vertices to be -1.*/
    init(vertices);
    int totalEdges = 0,edgePos=0,from,to,weight;
    Edge now;
    while(totalEdges < vertices -1)
    {
        if(edgePos==edges)
        {
            /* Input Graph is not connected*/
            exit(0);
        }
        now = E[edgePos++];
        from = now.from;
        to = now.to;
        weight=now.weight;
        /* See If vetices from,to are connected. If they are connected do not add this edge. */
        int parent1 = Find(from);
        int parent2 = Find(to);
        if(parent1!=parent2)
        {
            graph[from][to] = weight;
            Union(parent1,parent2);
            totalEdges++;
        }
    }

}






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

    int i,j;
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


    if ((rv = getaddrinfo("nunki.usc.edu", TCPPORT, &hints, &servinfo)) != 0) {
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

    struct sockaddr local_addr;
    socklen_t local_addrlen = sizeof local_addr;
    char l[INET6_ADDRSTRLEN];
    
    getsockname(sockfd,&local_addr,&local_addrlen);
    inet_ntop(local_addr.sa_family,get_in_addr(&local_addr),l, sizeof l);
    printf("The client has TCP port number %u and IP address %s\n",ntohs(((struct sockaddr_in *)&local_addr)->sin_port),l);

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
			continue; 
		}
        inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);


            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1); 
            }
            received[buf[0]] = 1;
            for(i = 0;i < 4;i++){
                matrix[buf[0]][i] = buf[i + 1];
            }

            printf("The Client receives neighbor information from the Server %c with TCP port number %u and IP address %s(The Server %c's TCP port number and IP address).\n",'A' + buf[0],ntohs(((struct sockaddr_in *)&their_addr)->sin_port),s ,'A' + buf[0]);
            close(new_fd);

            printf("The Server%c has the following neighbor information:\n",'A' + buf[0]);
            printf("Neighbor----Cost\n");
            for(i = 1;i <= 4;i++){
                if(buf[i] != 0){
                    printf("server%c     %d\n",'A' + i - 1, buf[i]);
                }
            }
            

            getsockname(new_fd,&local_addr,&local_addrlen);
            inet_ntop(local_addr.sa_family,get_in_addr(&local_addr),l, sizeof l);
            printf("For this connection with Server%c,The Client has TCP port number %u and IP address %s.\n",'A' + buf[0], ntohs(((struct sockaddr_in *)&local_addr)->sin_port),l);
            for(i = 0;i < 4;i++){
                receivedNumber += received[i];
            }
            if(receivedNumber == 4){
                break;
            }
            receivedNumber = 0;
    }
    

    //UDP
    
    char server[INET6_ADDRSTRLEN];
    char *servers[4] = {SERVERUDPPORTA,SERVERUDPPORTB,SERVERUDPPORTC,SERVERUDPPORTD};
    for(i = 0;i < 4;i++){
        int sockfd;
        struct addrinfo hints, *servinfo, *p;
        struct sockaddr local_addr;
        socklen_t local_addrlen = sizeof local_addr;
        char l[INET6_ADDRSTRLEN];
        struct sockaddr server_addr;
        socklen_t server_addrlen = sizeof server_addr;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; 
        if ((rv = getaddrinfo("127.0.0.1", servers[i], &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1; }
        // loop through all the results and make a socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue; 
            }
            break; 
        }
        int m;
        for(m = 0;m < 4;m++){
            for(j = 0;j < 4;j++){
                udpData[m * 4 + j] = matrix[m][j];
            }
        }
        

        //getpeername(sockfd,&server_addr,&server_addrlen);
        //inet_ntop(server_addr.sa_family,get_in_addr(&server_addr),server, sizeof server);
        
        inet_ntop(p->ai_family,get_in_addr(p->ai_addr),server, sizeof server);
        printf("The Client has sent the network topology to the network topology to the Server %c with UDP port number %u and IP address %s (Server %c's UDP port number and IP address) as follows:\n",'A' + i, ntohs(((struct sockaddr_in *)(p->ai_addr))->sin_port),server,'A' + i);

        if ((numbytes = sendto(sockfd, udpData, 16 * sizeof *udpData, 0,p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1); 
        }

        getsockname(sockfd,&local_addr,&local_addrlen);
        inet_ntop(p->ai_addr->sa_family,get_in_addr(p->ai_addr),l, sizeof l);
        printf("For this connection with Server%c The Client has UDP port number %u and IP address %s.\n", 'A' + i, ntohs(((struct sockaddr_in *)&local_addr)->sin_port),l);

        printf("Edge----Cost\n");
        int k;
        for(k = 0;k < 16;k++){
            if(udpData[k] != 0 && k/4 < k%4){
                printf("%c%c      %d\n",'A' + k/4, 'A' + k%4,udpData[k]);
            }
        }
        freeaddrinfo(servinfo);
    }
    


    int vertices = 4,edges = 0;
    int from,to,weight;
    int totalCost = 0;
    
    
    for(i = 0;i < 4;i++){
        for(j = i + 1;j < 4;j++){
            if(matrix[i][j] != 0){
                E[edges].from = i; 
                E[edges].to = j; 
                E[edges].weight = matrix[i][j];
                edges++;
            }
        }
    }
    /* Finding MST */
    Kruskal(vertices,edges);
    /* Printing the MST */
    for(i=0;i<vertices;i++){
        for(j=0;j<vertices;j++){
            if(graph[i][j]!=-1){
                totalCost += graph[i][j];
            }
        }
    }
    printf("The Client has calculated a tree.The tree cost is %d\n",totalCost);
    for(i=0;i<vertices;i++){
        for(j=0;j<vertices;j++){
            if(graph[i][j]!=-1){
                printf("%c%c     %d\n",'A' + i,'A' + j,graph[i][j]);
            }
        }
    }
    return 0;
}

