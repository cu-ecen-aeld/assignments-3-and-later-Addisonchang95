#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "aesdsocket.h"

int server_fd = -1;
struct addrinfo* serverInfo;

int main(int argc, char *argv[]) {
    int daemon_mode = 0;

    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = 1;
    }

    openlog("aesdsocket", LOG_PID, LOG_USER);

    //setup signal action for signal capturing
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signals;
    if (sigaction(SIGINT, &sa, NULL) != 0 || sigaction(SIGTERM, &sa, NULL) != 0) {
        syslog(LOG_ERR, "Failed to setup signal handler");
        closelog();
        return -1;
    }

    //get socketinfo and bind socket
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;
    int rd=getaddrinfo(NULL, SERVER_PORT, &hints, &serverInfo);
    if(rd!=0)
    {
        syslog(LOG_ERR,"GetInfo Error: %s", gai_strerror(rd));
        return -1;
    }
    server_fd=socket(serverInfo->ai_family,serverInfo->ai_socktype,serverInfo->ai_protocol);
    if(server_fd<0)
    {
        syslog(LOG_ERR,"Socket Error: %m");
        return -1;
    }
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
        syslog(LOG_ERR, "Setsockopt SO_REUSEADDR Error: %m");
    }
    if ( bind(server_fd,serverInfo->ai_addr,serverInfo->ai_addrlen)!=0)
    {
        syslog(LOG_ERR,"Socket Error: %m");
        close(server_fd);
        return -1;
    }

    // if argv has -d: switch to deamon mode
    if (daemon_mode) {
        setup_daemon();
    }

    //setup listen and accept
    if (listen(server_fd,2)!=0)
    {
        syslog(LOG_ERR, "Listen Error: %m");
    }
    
    //main logic loop
    while (1) 
    {
        int client_fd = -1;
        struct sockaddr_in clientIP;
        socklen_t client_len = sizeof(clientIP);
        client_fd = accept(server_fd, (struct sockaddr *)&clientIP, &client_len);
        if (client_fd == -1) 
        {
            //break if accept interrupt by SIG
            break;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientIP.sin_addr), client_ip, sizeof(client_ip));
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        handle_client_connection(client_fd, client_ip);
    }

    close_server();
    return 0;
}
void close_server(void){
    if (server_fd != -1) 
    {
        close(server_fd);
        server_fd=-1;
    }
    if (serverInfo != NULL) 
    {
            freeaddrinfo(serverInfo);
            serverInfo = NULL;
    }
    unlink(DATAFILE_PATH);
    closelog();
}

void handle_signals(int signal_number) {
    if (signal_number == SIGINT || signal_number == SIGTERM) 
    {
        syslog(LOG_INFO, "Caught signal, exiting");
        close_server();
        exit(EXIT_SUCCESS);
    }
}

void setup_daemon(void) {
    pid_t pid;
    pid=fork();
    if (pid<0)
    {
        syslog(LOG_ERR, "First fork Error: %m");
        close_server();
        exit(EXIT_FAILURE);
    }
    if (pid>0)//parent process
    {
        if (serverInfo != NULL) {
            freeaddrinfo(serverInfo);
            serverInfo = NULL;
        }
        closelog();
        exit(EXIT_SUCCESS);
    }
    else if (pid==0)//child process
    {
        if (setsid() < 0) 
        {
            syslog(LOG_ERR, "Setsid Error: %m");
            close_server();
            exit(EXIT_FAILURE);
        }
        //change dir to root
        if (chdir("/") < 0) 
        {
            syslog(LOG_ERR, "Chdir Error: %m");
            close_server();
            exit(-1);
        }
        //dump std into null
        int dev_null_fd = open("/dev/null", O_RDWR);
        if (dev_null_fd != -1) 
        {
            //dump all 
            dup2(dev_null_fd, STDIN_FILENO);
            dup2(dev_null_fd, STDOUT_FILENO);
            dup2(dev_null_fd, STDERR_FILENO); 
            if (dev_null_fd > 2) 
            {
                close(dev_null_fd); // 功成身退，關閉多開的 fd
            }
        }

    }

}

void handle_client_connection(int client_fd, const char *client_ip) {
    char recv_buffer[BUF_SIZE];
    char *packet_buffer = NULL;
    size_t packet_size = 0;
    ssize_t bytes_received;

    while ((bytes_received = recv(client_fd, recv_buffer, sizeof(recv_buffer), 0)) > 0) {
        //resize packet_size
        char *new_ptr = realloc(packet_buffer, packet_size + bytes_received);
        if (new_ptr == NULL) {
            syslog(LOG_ERR, "Memory allocation failed: %m");
            free(packet_buffer);
            close(client_fd);
            return;
        }
        packet_buffer = new_ptr;
        
        memcpy(packet_buffer + packet_size, recv_buffer, bytes_received);
        packet_size += bytes_received;

        if (packet_buffer[packet_size - 1] == '\n') {
            break;
        }
    }

    if (bytes_received < 0) {
        syslog(LOG_ERR, "Receive failed: %m");
        free(packet_buffer);
        close(client_fd);
        return;
    }

    //write into DataFile
    if (packet_size > 0) 
    {

        int file_fd = open(DATAFILE_PATH, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        if (file_fd == -1) {
            syslog(LOG_ERR, "Failed to open data file for writing: %m");
            free(packet_buffer);
            close(client_fd);
            return;
        }
        
        write(file_fd, packet_buffer, packet_size);
        close(file_fd);
        free(packet_buffer);
    }

    //send back to client
    int file_fd = open(DATAFILE_PATH, O_RDONLY);
    if (file_fd != -1) {
        ssize_t bytes_read;
        char send_buffer[BUF_SIZE];
        
        while ((bytes_read = read(file_fd, send_buffer, sizeof(send_buffer))) > 0) {
            send(client_fd, send_buffer, bytes_read, 0);
        }
        close(file_fd);
    }

    close(client_fd);
    
    syslog(LOG_INFO, "Closed connection from %s", client_ip);
}
