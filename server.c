#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 27015

int main() {
    FILE *errorFile = freopen("serverErrorLog.txt", "w", stderr);
    if(!errorFile) {
        return 1;
    }
    int iResult;
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(iResult) {
        fprintf(stderr, "WSAStartup Failed: %d\n", iResult);
        return 1;
    }
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create the socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind() failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    listen(server_fd, 5);
    printf("Server listening on port %d...\n", DEFAULT_PORT);

    while (1)
    {
        SOCKET client_fd = accept(server_fd, NULL, NULL);
        if(client_fd == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed! %d\n", WSAGetLastError());
            continue;
        }
        printf("New Client connected\n");
        char buffer[1024];
        int bytes_recived;
        while((bytes_recived = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
            buffer[bytes_recived] = '\0';
            printf("Received: %s\n", buffer);
        }
        if(bytes_recived == 0) {
            printf("Client disconnected gracefully.\n");
        } else {

        }
    }
    fclose(errorFile);
    closesocket(server_fd);
    return 0;
}
