#include <stdio.h>
#include <string.h>
#include "network.h"

int handle_get(int socket) {

    int err;
    int linesize = -1;

    /* buffers */
    char buf[1024];
    char path[1024];

    /* received get request */
    fprintf(stderr, "Received GET request.\n");

    /* receive space delimiter */
    if (recv_size(socket, buf, 1) < 0)
        return -1;

    /* receive path */
    if (recv_until(socket, path, sizeof(path), ' ') < 0)
        return -1;
    fprintf(stderr, "Path: %s\n", path);

    /* receive space delimiter */
    if (recv_size(socket, buf, 1) < 0)
        return -1;

    /* receive http version */
    if (recv_until(socket, buf, sizeof(buf), '\r') < 0)
        return -1;
    fprintf(stderr, "HTTP version: %s\n", buf);

    /* receive CRLF */
    if (recv_size(socket, buf, 2) < 0)
        return -1;

    /* read HTTP header lines */
    while (linesize) {
        /* receive line */
        linesize = recv_until(socket, buf, sizeof(buf), '\r');
        fprintf(stderr, "h> %s\n", buf);
        /* receive CRLF */
        if (recv_size(socket, buf, 2) < 0)
            return -1;
    }

    /* send HTTP reply */
    send_str(socket, "HTTP/1.1 200 OK\r\n");
    send_str(socket, "\r\n");

    /* send the file */
    send_str(socket, "IT WORKS!\n");
    send_str(socket, "HELLO WORLD FROM QUAFIOS HTTP SERVER!\n");
    send_str(socket, "\r\n");

    /* inform user we have finished processing */
    fprintf(stderr, "GET request has been processed successfully.\n");

    /* done */
    return 0;

}

int handle_post(int socket) {

}

void handle_http(int socket) {

    /* buffer */
    char buf[100] = "";
    int err;

    /* receive HTTP request command */
    recv_until(socket, buf, sizeof(buf), ' ');

    /* handle http request */
    if (!strcmp(buf, "GET")) {
        err = handle_get(socket);
    } else if (!strcmp(buf, "POST")) {
        err = handle_post(socket);
    } else {
        err = 1;
    }

    /* check error */
    if (err) {
        fprintf(stderr, "Error: malformed HTTP request/connection error.\n");
    }

    /* close connection */
    close_socket(socket);

    /* connection closed */
    fprintf(stderr, "Connection closed.\n");

}
