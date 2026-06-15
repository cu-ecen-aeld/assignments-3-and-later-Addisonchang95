#ifndef AESDSOCKET_H
#define AESDSOCKET_H

#define SERVER_PORT "9000"
#define DATAFILE_PATH "/var/tmp/aesdsocketdata"
#define BUF_SIZE 1024

void handle_signals(int signal_number);
void setup_daemon(void);
/**
 * dealing with single client connection and functionality(recv, log and send)
 * @param client_fd client socket fd
 * @param client_ip client socket ip to use for syslog
 */
void handle_client_connection(int client_fd, const char *client_ip);
void close_server(void);
#endif