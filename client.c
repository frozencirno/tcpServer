#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 27015

int main() {
    int iResult;
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(iResult) {
        fprintf(stderr, "WSAStartup Failed: %d\n", iResult);
        return 1;
    }
    SOCKET client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create the socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    
    iResult = connect(client_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(iResult == INVALID_SOCKET) {
        fprintf(stderr, "Connect() Failed: %d", WSAGetLastError());
        closesocket(client_fd);
        WSACleanup();
        return 1;
    }

    printf("Connection Success!\n");

    const char* message = "Hello world!";
    iResult = send(client_fd, message, strlen(message), 0);
    if(iResult == SOCKET_ERROR) {
        fprintf(stderr, "Failed to send message%d\n", WSAGetLastError());
        return 1;
    } else {
        printf("Sent successfully\n");
    }

    printf("Press enter to quit.");
    getchar();

    closesocket(client_fd);
    WSACleanup();
    return 0;
}
