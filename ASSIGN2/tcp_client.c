#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "10.0.0.1"
#define PORT 5000
#define BUF_SIZE 4096
#define TOTAL_SIZE (2 * 1024 * 1024) // 2MB

void* send_data(void* arg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    char buffer[BUF_SIZE];
    memset(buffer, (int)(long)arg, BUF_SIZE); // unique data

    int sent = 0;
    while (sent < TOTAL_SIZE) {
        int n = send(sock, buffer, BUF_SIZE, 0);
        sent += n;
    }

    close(sock);
    return NULL;
}

int main() {
    pthread_t threads[8];

    for (long i = 0; i < 8; i++) {
        pthread_create(&threads[i], NULL, send_data, (void*)i);
    }

    for (int i = 0; i < 8; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
