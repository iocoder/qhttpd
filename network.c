#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "network.h"

char inbuf[BUFSIZE];

int buf_front = 0;
int buf_back = 0;

void init_buffer() {
    buf_front = buf_back = 0;
}

int send_str(int socket, char *str) {
    return send(socket, str, strlen(str), 0);
}

int recv_until(int socket, char *buf, int limit, char del) {

    int counter = 0;
    char data;

    while (1) {

        /* check if there is data in buffer */
        if (buf_front == buf_back) {
            /* buffer is empty, read a new message */
            buf_back = recv(socket, inbuf, BUFSIZE, buf_front=0);
            if (buf_back <= 0)
                return -1;
        }

        /* reached delimiter? */
        if (inbuf[buf_front] == del) {
            /* done */
            buf[counter] = 0;
            return counter;
        }

        /* now we have got data to read */
        data = buf[counter++] = inbuf[buf_front++];
        if (counter == BUFSIZE) {
            /* user buffer is full */
            return -1;
        }

    }

}

int recv_line(int socket, char *buf, int limit) {

    int expect_linefeed = 0;
    int counter = 0;
    char data;

    while (1) {

        /* check if there is data in buffer */
        if (buf_front == buf_back) {
            /* buffer is empty, read a new message */
            buf_back = recv(socket, inbuf, BUFSIZE, buf_front=0);
            if (buf_back <= 0)
                return -1;
        }

        /* now we have got data to read */
        data = buf[counter++] = inbuf[buf_front++];
        if (counter == BUFSIZE) {
            /* user buffer is full */
            return -1;
        }

        /* reached \r ?? */
        if (data == '\r') {
            expect_linefeed = 1;
        } else if (expect_linefeed) {
            /* reached \n ? */
            if (data == '\n') {
                /* enough data for today */
                buf[counter++] = 0;
                return counter;
            }
        }

    }

}

int recv_size(int socket, char *buf, int size) {

    int counter = 0;

    while (1) {

        /* check if there is data in buffer */
        if (buf_front == buf_back) {
            /* buffer is empty, read a new message */
            buf_back = recv(socket, inbuf, BUFSIZE, buf_front=0);
            if (buf_back <= 0)
                return -1;
        }

        /* now we have got data to read */
        buf[counter++] = inbuf[buf_front++];
        if (counter == size) {
            /* data has been read */
            return size;
        }

    }
}

int network_setup(int portnum) {

    int sockid, status;
    struct sockaddr_in addrport;

    /* 1. create a new communication endpoint:
     * ----------------------------------------
     * format: sockid = socket(family, type, protocol);
     * - family: PF_INET --> IPv4 protocols, internet addresses.
     *           PF_UNIX --> local communication, file addresses.
     * - type: SOCK_STREAM --> reliable, 2-way, connection-based
     *         SOCK_DGRAM  --> unreliable, connectionless
     * - protocol: IPPROTO_TCP --> use TCP
     *             IPPROTO_UDP --> use UDP
     *             0           --> use default
     */
    sockid = socket(PF_INET, SOCK_STREAM, 0);

    if (sockid == -1) {
        fprintf(stderr, "Error: cannot open a new TCP stream socket.\n");
        return -1;
    }

    /* 2. bind the socket to specific dest addr & dest port:
     * ------------------------------------------------------
     * format: status = bind(sockid, &addrport, size);
     * - sockid: socket descriptor
     * - addrport: pointer to struct sockaddr
     *   struct sockaddr {
     *       unsigned short sa_family; // Address family (e.g. AF_INET)
     *       char sa_data[14]; // Family-specific address information
     *   }
     *   struct sockaddr is generic structure for addresses. A special type
     *   is sockaddr_in (socket address for internet):
     *   struct sockaddr_in {
     *       unsigned short sin_family; // Internet protocol (AF_INET)
     *       unsigned short sin_port; // Address port (16 bits)
     *       struct in_addr sin_addr; // Internet address (32 bits)
     *       char sin_zero[8]; // Not used
     *   }
     *   in the above structure, sin_addr is of type "struct in_addr" which
     *   is a 4-byte data structure for storing IPv4 addresses:
     *   struct in_addr {
     *       unsigned long s_addr; // Internet address (32 bits)
     *   }
     *   So we will create a sockaddr_in structure and fill it with family,
     *   port-number, and sin_addr (of type struct in_addr), then
     *   pass its memory location to bind() as a "struct sockaddr *" pointer.
     * - length: length of addrport structure.
     */
    addrport.sin_family = AF_INET;
    addrport.sin_port = htons(portnum); /* converts to network byte order */
    addrport.sin_addr.s_addr = htonl(INADDR_ANY); /* accept any interface */
    status = bind(sockid, (struct sockaddr *) &addrport, sizeof(addrport));
    if (status == -1) {
        fprintf(stderr, "Error: cannot bind the socket to port %d\n", portnum);
        close(sockid);
        return -1;
    }

    /* 3. listening to the port:
     * --------------------------
     * format: status = listen(sockid, queuelen);
     * - sockid: socket descriptor.
     * - queuelen: number of active participants that can wait
     *             for a connection.
     */
    status = listen(sockid, SOMAXCONN);
    if (status == -1) {
        fprintf(stderr, "Error: cannot start listening to the port.\n");
        close(sockid);
        return -1;
    }

    /* done */
    return sockid;

}

int accept_connection(int sockid) {

    int s, addrLen;
    struct sockaddr clientAddr;

    /* 4. wait for a new connection to come, and accept it:
     * -----------------------------------------------------
     * format: s = accept(sockid, &clientAddr, &addrLen);
     * - sockid: socket descriptor
     * - clientAddr: pointer to sockaddr structure that will contain
     *               the address of the client.
     * - addrLen: length of clientAddr structure (must be set properly
     *            before call) which will be adjusted upon return.
     */

    /* set addrLen */
    addrLen = sizeof(clientAddr);

    /* call accept() */
    s = accept(sockid, &clientAddr, &addrLen);
    if (s == -1) {
        fprintf(stderr, "Error: cannot accept new connections.\n");
        return -1;
    }

    /* now we have a new established connection */
    fprintf(stderr, "Accepted a new connection.\n");

    /* return new socket descriptor */
    return s;

}

void close_socket(int socket) {
    /* 5. close socket before exist:
     * ------------------------------
     * format: close(sockid)
     */
    close(socket);
}
