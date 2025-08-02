#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <stdatomic.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 27015

/* TODO
    Multithreading so the server can handle more than one client at a time.
    User accounts(Encryption as well)
*/

atomic_int shutdown_request = 0;

unsigned __stdcall ClientHandler(void* client_socket) {
    SOCKET client_fd = *(SOCKET*)client_socket;
    free(client_socket);
    char buffer[1024];
    int bytes_recived;
    while((bytes_recived = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_recived] = '\0';
        printf("[SERVER]Received: %s\n", buffer);
    }
    if(bytes_recived == 0) {
        printf("[SERVER]Client disconnected gracefully.\n");
    } else {
        fprintf(stderr, "[SERVER]recv failed: %d\n", WSAGetLastError());
    }
    closesocket(client_fd);
    return 0;
}

unsigned __stdcall HandleConsoleCommands() {
    FILE* helpList = fopen("commandList.txt", "rb");
    fseek(helpList, 0, SEEK_END);
    size_t size = ftell(helpList);
    char* content = (char*)malloc(size + 2);
    rewind(helpList);
    size_t bytesRead = fread(content, 1, size, helpList);
    content[size] = '\n';
    content[size+1] = '\0';
    fclose(helpList);
    while(1) {
        char command[1024];
        fgets(command, 1024, stdin);
        command[strcspn(command, "\n")] = '\0';
        if(strcmp(strlwr(command), "help") == 0) {
            printf("%s", content);
        } else if (strcmp(strlwr(command), "shutdown") == 0 || strcmp(strlwr(command), "quit") == 0)
        {
            printf("[SERVER]Shuting down...\n");
            atomic_store(&shutdown_request, 1);
            break;
        } else {
            printf("[SERVER]Command not found.\n");
        }
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
        fprintf(stderr, "[SERVER]WSAStartup Failed: %d\n", iResult);
        return 1;
    }
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == INVALID_SOCKET) {
        fprintf(stderr, "[SERVER]Failed to create the socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "[SERVER]bind() failed: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    listen(server_fd, 5);
    printf("[SERVER]Server listening on port %d...\n", DEFAULT_PORT);
    printf("[SERVER]Enter help for a list of commands.\n");

    while (!atomic_load(&shutdown_request))
    {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        SOCKET* client_fd = (SOCKET*)malloc(sizeof(SOCKET));
        if (!client_fd) {
            fprintf(stderr, "[SERVER]malloc failed\n");
            continue;
        }

        HANDLE consoleThread = (HANDLE)_beginthreadex(NULL, 0, &HandleConsoleCommands, NULL, 0, NULL);
        if (consoleThread == NULL) {
            fprintf(stderr, "[SERVER]Error creating thread\n");
            closesocket(*client_fd);
            free(client_fd);
        } else {
            CloseHandle(consoleThread);
        }

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(server_fd, &readSet);
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int selectResult = select(0, &readSet, NULL, NULL, &timeout);
        if (selectResult == SOCKET_ERROR) {
            fprintf(stderr, "[SERVER]select failed: %d\n", WSAGetLastError());
            continue;
        }

        if (selectResult == 0) {
            // Timeout occurred, check shutdown flag and continue
            free(client_fd);
            continue;
        }
    
        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(*client_fd == INVALID_SOCKET) {
            fprintf(stderr, "[SERVER]Accept failed! %d\n", WSAGetLastError());
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        printf("[SERVER]New Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, &ClientHandler, client_fd, 0, NULL);
        if (thread == NULL) {
            fprintf(stderr, "[SERVER]Error creating thread\n");
            closesocket(*client_fd);
            free(client_fd);
        } else {
            CloseHandle(thread);
        }
    }
    printf("[SERVER]Cleaning up and shutting down...\n");
    fclose(errorFile);
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
