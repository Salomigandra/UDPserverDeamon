#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8707

int main() {
    int sockfd;
    struct sockaddr_in servaddr;

    // creating the socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // setting up the server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);

    char buffer[1024];
    int n, len;
    char server_ip[16];
    char prompt[20] = "Client Shell ";

    while (1) {
        // printing the prom to check wheather we are connected to the  prompt or not 
        printf("%s:%d> ", prompt, SERVER_PORT);

        // using fgets to get user input
        fgets(buffer, 1024, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // removeing the newline character
	// help command
	if (strcmp(buffer, "help") == 0) {
            printf("connect [ip address]: connect to the remote server.\n");
            printf("disconnect [ip address]: disconnect from the server.\n");
            printf("quit: quit the client shell.\n");
            continue;
        }

        // handling the connect command
        if (strncmp(buffer, "connect ", 8) == 0) {
            char *ip_str = buffer + 8;
            if (inet_pton(AF_INET, ip_str, &servaddr.sin_addr) <= 0) {
                perror("inet_pton");
                continue;
            }

            // updating prompt with server IP address to know which server we are using or just to know are we in the remote shell
            snprintf(prompt, sizeof(prompt), "%s", ip_str);
            printf("Connected to server at %s:%d\n", ip_str, SERVER_PORT);
            continue;
        }

        // handling the disconnect command
        if (strncmp(buffer, "disconnect", 10) == 0) {
            close(sockfd);

            // create new socket
            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockfd < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }

            printf("Disconnected from server\n");
            snprintf(prompt, sizeof(prompt), "Client Shell");
            continue;
        }

        // handling the  quit command
        if (strcmp(buffer, "quit") == 0) {
            if (sockfd >= 0) {
                close(sockfd);
            }
            printf("Goodbye!\n");
            break;
        }

        // sending user input to the server
        if (sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }

        // to receive response from server and to print it
        n = recvfrom(sockfd, (char *)buffer, 1024, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        printf("%s\n", buffer);
    }

    return 0;
}
