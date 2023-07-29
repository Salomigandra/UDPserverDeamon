#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 4
#define BUF_SIZE 1024

typedef struct {
    int sock;
    struct sockaddr_in addr;
    pthread_t thread;
    int active;
} client_t;

client_t clients[MAX_CLIENTS];

void* handle_client(void* arg) {
    client_t* client = (client_t*) arg;
    int sock = client->sock;
    struct sockaddr_in addr = client->addr;
    char buf[BUF_SIZE];

    // Sending handle to the client
    char handle[BUF_SIZE];
    snprintf(handle, BUF_SIZE, "%d:%s", ntohs(addr.sin_port), inet_ntoa(addr.sin_addr));
    sendto(sock, handle, strlen(handle), 0, (struct sockaddr*) &addr, sizeof(addr));

    char buffer[1024];
    int len; // inizializing
 
    while (1) {
        int n = recvfrom(sock, (char *)buffer, 1024, 0, NULL, NULL);
        buffer[n] = '\0';
        //printf("Received command from client: %s\n", buffer);
        if (strcmp(buffer, "quit") == 0) {
            printf("Server exiting\n");
            break;
        }
        int pipefd[2];
        pid_t pid;
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // child process
            close(pipefd[0]); // closing unused read end
            // redirecting stdout to pipe
            if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }

            // executing command
            char *args[32];
            char *token = strtok(buffer, " ");
            int i = 0;
            while (token != NULL && i < 32) {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            execvp(args[0], args);

            // returning execvp: as error if there is an error
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            // parent process
            close(pipefd[1]); // closeing unused write end
            // reading output from pipe
            n = read(pipefd[0], buffer, 1024);
            buffer[n] = '\0';
            // sending output back to the client
            sendto(sock, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&addr, sizeof(addr));
            //  printf("Command output sent to client\n");
        }
    }
    // Closeing connection and terminating the thread
    close(sock);
    client->active = 0;
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port> \n", argv[0]);
        // printf("Server is listening for incoming clients...\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
// Creating the UDP socket
int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
if (server_sock < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
}
// Binding the socket to port
struct sockaddr_in server_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(port),
    .sin_addr.s_addr = INADDR_ANY
};
if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
    close(server_sock);
    exit(EXIT_FAILURE);
}
// Listening for incoming client connections
while (1) {
    // Accepting incoming client connections 
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buf[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
ssize_t n = recvfrom(server_sock, buf, BUF_SIZE, 0, (struct sockaddr*) &client_addr, &addrlen);
    if (n < 0) {
        perror("recvfrom");
        continue;
    }
    // Rejecting if more than 4 clients are connected
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            break;
        }
    }
    if (i == MAX_CLIENTS) {
        char msg[] = "Error: Maximum number of clients reached";
        sendto(server_sock, msg, sizeof(msg), 0, (struct sockaddr*) &client_addr, addrlen);
        continue;
    }
// Spawn pthread for handling the clients
    client_t* client = &clients[i];
    client->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->sock < 0) {
        perror("socket");
        continue;
    }
    client->addr = client_addr;
    client->active = 1;

    if (pthread_create(&client->thread, NULL, handle_client, client) != 0) {
        perror("pthread_create");
        client->active = 0;
        close(client->sock);
        continue;
    }

    // Detaching the pthread
    pthread_detach(client->thread);
}

// Closing the server socket
close(server_sock);
return 0;
}
