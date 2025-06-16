#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//Creer un client TCP

int main(int argc, char* argv[]){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0){
        perror("erreur socket");     
        exit(EXIT_FAILURE);   
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if(n < 0){
        perror("read");
        exit(EXIT_FAILURE);
    }

    printf("server says: %s\n", rbuf);
    close(fd);

    return 1;
}