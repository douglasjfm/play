#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

int create_tcp_socket();
char *get_ip(char *host);
char *build_get_query(char *host, char *page);
void usage();

void* initserv()
{
    int soc = create_tcp_socket(),clisoc,i=0,j=0;
    unsigned peerlen;
    struct sockaddr_in serveraddr, peer;
    char msg[10],peerip[25];
    printf("esperando chamadas...\n");
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(2816);
    if (bind(soc, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
        printf("bind socket falhou\n");
    printf("esperando chamadas...\n");
    if (listen(soc, 5) < 0)
        printf("listen socket falhou\n");
    printf("esperando chamadas...\n");
    peerlen = sizeof(peer);
    clisoc = accept(soc, (struct sockaddr *) &peer,&peerlen < 0);
    printf("chamada recebida ");
    getsockname(clisoc,(struct sockaddr *)&peer,&peerlen);
    sprintf(peerip,"%s",inet_ntoa(peer.sin_addr));
    printf("%s\n",inet_ntoa(peer.sin_addr));
    ouvirf(peerip);
    msg[0] = 'b';
    msg[1] = '\0';
    send(clisoc,msg,strlen(msg),0);
    close(soc);
    return NULL;
}
