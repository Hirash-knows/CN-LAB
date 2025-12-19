#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 5000
#define BUF_SIZE 4096

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buffer[BUF_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);

    printf("TCP Server listening on port %d\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if (client_fd < 0) continue;

        while (1) {
            ssize_t bytes = recv(client_fd, buffer, BUF_SIZE, 0);
            if (bytes <= 0) break;

            struct timeval tv;
            gettimeofday(&tv, NULL);
            long ts = tv.tv_sec * 1000000 + tv.tv_usec;

            printf("Time(us): %ld | Chunk Size: %ld bytes\n", ts, bytes);
        }
        close(client_fd);
    }
}
