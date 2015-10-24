#include <stdio.h>
#include <stdlib.h>
#include "network.h"
#include "http.h"

int sockid;

int main(int argc, char *argv[]) {

    int portnum;
    int s;

    /* check arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port-number>\n", argv[0]);
        return -1;
    }

    /* print splash */
    fprintf(stderr, "HTTP server for Quafios.\n");

    /* decode port number */
    portnum = atoi(argv[1]);

    /* setup network */
    if ((sockid = network_setup(portnum)) == -1)
        return -1;

    /* enter wait-for-connection loop */
    while (1) {
        int s;
        /* accept new connection */
        s = accept_connection(sockid);
        if (s == -1) {
            close_socket(sockid);
            return -1;
        }
        /* initialize buffer */
        init_buffer();
        /* handle HTTP request */
        handle_http(s);
    }

    /* done */
    return 0;

}
