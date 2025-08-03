#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>


#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT 27015

/* TODO
    Add the ability to send files like images and compressed files
    Add a graphical user interface
*/

int main() {
    //Init WSA
    int iResult;
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(iResult) {
        fprintf(stderr, "WSAStartup Failed: %d\n", iResult);
        return 1;
    }
    //Create the socket which is used to create a "path" for connections
    SOCKET client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create the socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //Create a structure describing the information of the server so we can locate it and understand what protocol it is using.
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    //Simply connect to the server
    iResult = connect(client_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(iResult == INVALID_SOCKET) {
        fprintf(stderr, "Connect() Failed: %d", WSAGetLastError());
        closesocket(client_fd);
        WSACleanup();
        return 1;
    }

    printf("Connection Success!\n");

    //Loop handles user input and passes data onto server.
    while (1)
    {
        char message[1024];
        printf("Enter message (or 'exit' to quit): ");
        fgets(message, 1024, stdin);
        //Remove the newline so we can display it correct when sent and for when we compare it to "exit"
        message[strcspn(message, "\n")] = '\0';
        if(strcmp(message, "exit") == 0) {
            break;
        }
        //Send the string to the server so it can be displayed and stored.
        iResult = send(client_fd, message, strlen(message), 0);
        if(iResult == SOCKET_ERROR) {
            fprintf(stderr, "Failed to send message%d\n", WSAGetLastError());
            return 1;
        } else {
            printf("Sent successfully\n");
        }
    }
    //Cleanup
    closesocket(client_fd);
    WSACleanup();
    return 0;
}
