#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <cassert>

static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t query(int fd, const char *text) {
    const size_t k_max_msg = 4096;
    size_t len_sz = strlen(text);
    if (len_sz > k_max_msg) {
        fprintf(stderr, "request too long\n");
        return -1;
    }

    uint32_t len = (uint32_t)len_sz;
    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    if (write_all(fd, wbuf, 4 + len)) {
        fprintf(stderr, "write() error\n");
        return -1;
    }

    char rbuf[4 + k_max_msg + 1];
    if (read_full(fd, rbuf, 4)) {
        fprintf(stderr, "read() error\n");
        return -1;
    }

    uint32_t rlen = 0;
    memcpy(&rlen, rbuf, 4);
    if (rlen > k_max_msg) {
        fprintf(stderr, "response too long\n");
        return -1;
    }

    if (read_full(fd, &rbuf[4], rlen)) {
        fprintf(stderr, "read() error\n");
        return -1;
    }
    rbuf[4 + rlen] = '\0';
    printf("server says: %s\n", &rbuf[4]);
    return 0;
}
//Creer un client TCP
/*
    The connect() syscall takes 3 arguments.
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    fd is the file descriptor returned by socket().
    addr is a pointer to a struct sockaddr with the address to connect to.
    sizeof(addr) is the size of the address structure.
*/
int main(int argc, char* argv[]){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0){
        perror("erreur socket");     
        exit(EXIT_FAILURE);   
    }

    // Set socket options
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        perror("connect");
        exit(EXIT_FAILURE);
    }

    if (query(fd, "hello1")) {
        close(fd);
        exit(EXIT_FAILURE);
    }
    if (query(fd, "hello2")) {
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);

    return 1;
}
