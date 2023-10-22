#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_IP "127.0.0.1"
#define PORT 3001
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char sendbuffer[BUFFER_SIZE];

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(1);
    }

    // Initialize the server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to the server");
        exit(1);
    }

    if (fork() == 0)
     {
        while (1)
        {
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) 
            {
                break;
            }
            buffer[bytes_received] = '\0';  // Null-terminate the received data
            printf("%s\n", buffer);
        }
        close(client_socket);
    } 
    else 
    {
        while (1) 
        {

            fgets(sendbuffer, BUFFER_SIZE, stdin);
            // Send data to the server
            if (send(client_socket, sendbuffer, strlen(sendbuffer), 0) == -1) 
            {
                break;
            }

            // sendbuffer[strlen(sendbuffer)-1] = '\0';
            // printf("%s\n",sendbuffer);
            if (strcmp(sendbuffer,"exit\n\0") == 0) 
            {
                printf("You Disconnected\n");
                return 0;
            }
        }
        close(client_socket);
    }
    // Properly reap child processes to avoid zombie processes
    int status;
    wait(&status);

    return 0;
}
