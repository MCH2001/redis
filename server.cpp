#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cassert>

/*
    Explications du SOCKET API
    int accept(int sockfd, struct sockaddr *addr, socklen_t len);
    int connect(int sockfd, const struct sockaddr *addr, socklen_t len);
    int bind(int sockfd, const struct sockaddr *addr, socklen_t len);

    We never used struct sockaddr, instead we forcibly cast either struct 
    sockaddr_in or struct sockaddr_in6 to this pointer type. 
    
    Here is how these structs are defined.
    // IPv4:port
    struct sockaddr_in {
        sa_family_t     sin_family;     // AF_INET
        uint16_t        sin_port;       // port number, big-endian
        struct in_addr  sin_addr;       // IPv4 address
    };
    // IPv6:port
    struct sockaddr_in6 {
        sa_family_t     sin6_family;    // AF_INET6
        uint16_t        sin6_port;      // port number, big-endian
        uint32_t        sin6_flowinfo;
        struct in6_addr sin6_addr;      // IPv6 address
        uint32_t        sin6_scope_id;
    };



    The socket API is weird in that it defines many pointless types.

    struct sockaddr has no use at all; struct sockaddr * is practically just void *.
    struct sockaddr_storage is supposed to hold both address types, which can be trivially replaced by union { struct sockaddr_in v4; struct sockaddr_in6 v6 }.
    struct sockaddr_in & struct sockaddr_in6 are the only useful and concrete structs.
    sin_addr & sin6_addr are pointlessly nested structs with just a single field.
    *_family is practically a 16-bit integer, yet it has its own type.
*/


/*Pseudo Code appel socket
    fd = socket()
    bind(fd, address)
    listen(fd)
    while True:
        conn_fd = accept(fd)
        do_something_with(conn_fd)
        close(conn_fd)

    The socket() syscall takes 3 integer arguments.

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    AF_INET is for IPv4. Use AF_INET6 for IPv6 or dual-stack sockets.
    SOCK_STREAM is for TCP. Use SOCK_DGRAM for UDP.
    The 3rd argument is 0 and useless for our purposes.
*/


/*
    Functions to read and write all bytes
    These functions are used to read and write all bytes from/to a file descriptor.
    They are used to ensure that the entire buffer is read or written, even if the read or write system calls return less than the requested number of bytes.
    @param fd: file descriptor to read from or write to
    @param buf: buffer to read from or write to
    @param n: number of bytes to read or write
    @return: 0 on success, -1 on error
*/
static int32_t read_full(int fd, char *buf, size_t n){
    while (n > 0) {
        ssize_t rv = read(fd,buf, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t)rv <= n);
        n -= size_t(rv);
        buf += rv;
    }
    return 0;
}
/*
    The write_all function writes all bytes from the buffer to the file descriptor.
    It is used to ensure that the entire buffer is written, even if the write system call returns less than the requested number of bytes.
    @param fd: file descriptor to write to
    @param buf: buffer to write from
    @param n: number of bytes to write
    @return: 0 on success, -1 on error
*/
static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

/*
    The one_request function reads a request from the client and writes a response back to the client.
    It is used to handle a single request from a client.
    @param connfd: file descriptor for the connection with the client
    @return: 1 on success, -1 on error
*/
static int one_request(int connfd){
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if(n<0){
        perror("read() error");
        return;
    }
    printf("Client says %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
    return 1;
}

/*
    The socketCall function creates a TCP server that listens for incoming connections on port 1234.
    It accepts connections and handles requests from clients using the one_request function.
    @return: 1 on success, -1 on error
*/
int socketCall(){
    //Obtain a socket handle
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    //Set socket options
    int val =1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    //Bind to an adress
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234); // port
    addr.sin_addr.s_addr = htonl(0);
    int rv = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(rv){ perror("bind()"); exit(1); }

    //listen
    rv = listen(fd, SOMAXCONN);
    if(rv){perror("listen()");  exit(1); }
    
    //Accept connections
    while(true){
        //accept the connection
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd<0){ continue; }

        //One client at the time
        while (true){
            /*
            The one_request function will read 1 request and write 1 response. 
            The problem is, how does it know how many bytes to read? 
            This is the primary function of an application protocol. 
            Usually a protocol has 2 levels of structures:
            */
            int32_t err = one_request(connfd);
            if (err){
                break;
            }

        }
        close(connfd);

    }
    return 1;
}

//TODO 3.3 Create a TCP Client
//https://build-your-own.org/redis/03_hello_cs

int main(int argc, char* argv[]){
    socketCall();
    //do_something();

    return 1;
}