#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char stock_symbol[10];

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address setup
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to Stock Price Server.\n");
    printf("Enter stock symbols (e.g., AAPL, GOOG) or type 'exit' to quit:\n");

    while (1) {
        printf("Stock Symbol: ");
        scanf("%s", stock_symbol);

        if (strcmp(stock_symbol, "exit") == 0) {
            break;
        }

        // Send stock symbol to server
        send(client_socket, stock_symbol, strlen(stock_symbol), 0);

        // Receive response
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Server Response: %s", buffer);
        } else {
            printf("Server disconnected.\n");
            break;
        }
    }

    close(client_socket);
    return 0;
}