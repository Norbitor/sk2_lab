#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define BUFFER_SIZE 512
#define END_STRING "END."
#define MAX_CLIENTS 50

int g_server_socket;
int* g_connection_fd;
int g_conn_shm_id;
char g_net_buffer[BUFFER_SIZE];

void int_signal_handler()
{
    fprintf(stderr, "Gracefully exiting server...\n");
    // if (g_connection_fd != 0)
    //     close(g_connection_fd);
    close(g_server_socket);
    exit(EXIT_SUCCESS);
}

struct sockaddr_in prepare_address()
{
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(1337);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    return my_addr;
}

void server_init()
{
    g_conn_shm_id = shmget(40498, MAX_CLIENTS*sizeof(int), IPC_CREAT|0600);
    g_connection_fd = (int*)shmat(g_conn_shm_id, NULL, 0);
    g_connection_fd[0] = 1;

    g_server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (g_server_socket == -1)
    {
        perror("An error occured while creating a socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my_addr = prepare_address();

    if (bind(g_server_socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        perror("An error occured while binding a socket");
        close(g_server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(g_server_socket, 10) == -1)
    {
        perror("An error occured while trying to listen");
        close(g_server_socket);
        exit(EXIT_FAILURE);
    }
}

void server_loop()
{
    socklen_t sock_size = sizeof(struct sockaddr_in);
    char client_addr[16];
    char input[BUFFER_SIZE];

    for (;;)
    {
        struct sockaddr_in client_info;
        g_connection_fd = (int*)shmat(g_conn_shm_id, NULL, 0);
        int conn_id = g_connection_fd[0]++;
        g_connection_fd[conn_id] = accept(g_server_socket, NULL, NULL);
        if (g_connection_fd[conn_id] == -1)
        {
            perror("Accept failed");
            close(g_server_socket);
            exit(EXIT_FAILURE);
        }
        getpeername(g_connection_fd[conn_id], (struct sockaddr *)&client_info, &sock_size);
        inet_ntop(AF_INET, &(client_info.sin_addr), client_addr, INET_ADDRSTRLEN);
        printf("# A new client from %s came in!\n", client_addr);

        if(fork() == 0)
        {
            for (;;)
            {
                memset(g_net_buffer, 0, BUFFER_SIZE);
                printf("aaaaaa");
                int read_bytes = read(g_connection_fd[conn_id], g_net_buffer, BUFFER_SIZE);
                if (read_bytes == 0)
                {
                    break;
                }
                printf("CLIENT> %s\n", g_net_buffer);
                if (strcmp(g_net_buffer, END_STRING) == 0)
                {
                    break;
                }

            }
            puts("# This conversation has ended. Waiting for a new adventurer!");

            if (shutdown(g_connection_fd[conn_id], SHUT_RDWR) == -1)
            {
                close(g_connection_fd[conn_id]);
            }
            close(g_connection_fd[conn_id]);
            g_connection_fd[conn_id] = 0;
            exit(EXIT_SUCCESS);
        }
        else
        {
            g_connection_fd[conn_id] = 0;
        }
        waitpid(-1, NULL, WNOHANG);
    }
}

int main()
{
    signal(SIGINT, int_signal_handler);
    signal(SIGTERM, int_signal_handler);

    memset(g_net_buffer, 0, BUFFER_SIZE);

    server_init();
    server_loop();

    close(g_server_socket);
    return EXIT_SUCCESS;
}