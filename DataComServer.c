#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Mock stock prices
typedef struct {
    char symbol[10];
    float price;
} Stock;

Stock stock_data[] = {
        {"AAPL", 145.67},
        {"GOOG", 2760.50},
        {"AMZN", 3450.45},
        {"MSFT", 299.10},
        {"TSLA", 759.20},
        {"BTC", 123500},

};

#define NUM_STOCKS (sizeof(stock_data) / sizeof(Stock))

int client_count = 0;  // Shared counter for connected clients
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread-safe access to client_count

void *handle_client(void *client_data);
float get_stock_price(char *symbol);

typedef struct {
    int socket;
    struct sockaddr_in address;
} ClientData;

int main() {
    int server_socket, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t tid;

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind server socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Stock Price Server is running on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Increment client count
        pthread_mutex_lock(&lock);
        client_count++;
        printf("Client connected from %s. Total clients: %d\n",
               inet_ntoa(client_addr.sin_addr), client_count);
        pthread_mutex_unlock(&lock);

        // Prepare client data to pass to thread
        ClientData *client_data = malloc(sizeof(ClientData));
        client_data->socket = new_socket;
        client_data->address = client_addr;

        // Create a thread for each client
        pthread_create(&tid, NULL, handle_client, client_data);
    }

    close(server_socket);
    return 0;
}

void *handle_client(void *client_data) {
    ClientData data = (ClientData)client_data;
    int socket = data->socket;
    struct sockaddr_in client_addr = data->address;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Request from client (%s): %s\n", inet_ntoa(client_addr.sin_addr), buffer);

        // Lookup stock price
        float price = get_stock_price(buffer);
        if (price > 0) {
            snprintf(buffer, BUFFER_SIZE, "Stock Price for %s: $%.2f\n", buffer, price);
        } else {
            snprintf(buffer, BUFFER_SIZE, "Stock symbol not found.\n");
        }

        // Send response
        send(socket, buffer, strlen(buffer), 0);
    }

    // Decrement client count
    pthread_mutex_lock(&lock);
    client_count--;
    printf("Client disconnected (%s). Total clients: %d\n",
           inet_ntoa(client_addr.sin_addr), client_count);
    pthread_mutex_unlock(&lock);

    close(socket);
    free(client_data);
    return NULL;
}

float get_stock_price(char *symbol) {
    for (int i = 0; i < NUM_STOCKS; i++) {
        if (strcmp(stock_data[i].symbol, symbol) == 0) {
            return stock_data[i].price;
        }
    }
    return -1; // Symbol not found
}