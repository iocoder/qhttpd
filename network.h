#ifndef __NETWORK_H
#define __NETWORK_H

#define BUFSIZE  4096

void init_buffer();
int send_str(int socket, char *str);
int recv_until(int socket, char *buf, int limit, char del);
int recv_size(int socket, char *buf, int size);
int network_setup(int portnum);
int accept_connection(int sockid);
void close_socket(int socket);

#endif
