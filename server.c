#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 27015

/* TODO
    Multithreading so the server can handle more than one client at a time.
    User accounts(Encryption as well)
*/

unsigned __stdcall ClientHandler(void* client_socket) {
    SOCKET client_fd = *(SOCKET*)client_socket;
    free(client_socket);
    char buffer[1024];
    int bytes_recived;
    while((bytes_recived = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_recived] = '\0';
        printf("Received: %s\n", buffer);
    }
    if(bytes_recived == 0) {
        printf("Client disconnected gracefully.\n");
    } else {
        fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
    }

    closesocket(client_fd);
    return 0;
}

void HandleConsoleCommands(char* command) {
    if(strlwr(command) == "help") {
        printf("debug message");
    }
}

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
    printf("Enter help for a list of commands.\n");

    while (1)
    {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        SOCKET* client_fd = (SOCKET*)malloc(sizeof(SOCKET));
        if (!client_fd) {
            fprintf(stderr, "malloc failed\n");
            continue;
        }
    
        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(*client_fd == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed! %d\n", WSAGetLastError());
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        printf("New Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, &ClientHandler, client_fd, 0, NULL);
        if (thread == NULL) {
            fprintf(stderr, "Error creating thread\n");
            closesocket(*client_fd);
            free(client_fd);
        } else {
            CloseHandle(thread);
        }
        
    }
    fclose(errorFile);
    closesocket(server_fd);
    return 0;
}
