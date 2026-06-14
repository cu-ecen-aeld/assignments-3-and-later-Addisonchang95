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
    struct addrinfo* serverinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;
    int rd=getaddrinfo(NULL, SERVER_PORT, &hints, &serverinfo);
    if(rd!=0)
    {
        syslog(LOG_ERR,"GetInfo Error: %s", gai_strerror(rd));
        return -1;
    }
    int fd=socket(serverinfo->ai_family,serverinfo->ai_socktype,serverinfo->ai_protocol);
    if(fd<0)
    {
        syslog(LOG_ERR,"Socket Error: %m");
        return -1;
    }
    rd=bind(fd,serverinfo->ai_addr,serverinfo->ai_addrlen);
    if (rd!=0)
    {
        syslog(LOG_ERR,"Socket Error: %m");
        return -1;
    }

    // if argv has -d: switch to deamon mode
    if (daemon_mode) {
        setup_daemon();
    }

    // [對齊規格 2.c]：開始監聽連線 (listen())
    // ----------------------------------------------------
    // TODO: 這裡接下來要填入 listen 邏輯
    // ----------------------------------------------------

    // [對齊規格 2.h]：進入無窮迴圈，直到被訊號打斷
    while (1) {
        // [對齊規格 2.c]：接收客人的連線 (accept())
        // ----------------------------------------------------
        int client_fd = -1; // TODO: 未來這裡會等於 accept() 的回傳值
        // ----------------------------------------------------

        if (client_fd == -1) {
            // 如果是被訊號打斷導致 accept 失敗，就跳出無窮迴圈
            break;
        }

        // [對齊規格 2.d]：取得客戶端 IP 並且紀錄到 Syslog
        char client_ip[INET6_ADDRSTRLEN] = "0.0.0.0"; // TODO: 未來用 inet_ntop 轉換
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        // [對齊規格 2.e & 2.f]：開始跟這個客人講電話（接收資料、寫檔、回傳）
        handle_client_connection(client_fd, client_ip);
    }

    // [對齊規格 2.i]：優雅收工清理現場
    if (server_fd != -1) {
        close(server_fd);
    }
    unlink(DATA_FILE_PATH); // 刪除 /var/tmp/aesdsocketdata 檔案
    syslog(LOG_INFO, "Caught signal, exiting");
    closelog();

    return 0;
}

void handle_signals(int signal_number) {
    // 規格 2.i 的訊號實作區
}

void setup_daemon(void) {
    // 規格 5 的 Daemon 實作區
}

void handle_client_connection(int client_fd, const char *client_ip) {
    // 規格 2.e, 2.f, 2.g 的資料處理與回傳實作區
}
