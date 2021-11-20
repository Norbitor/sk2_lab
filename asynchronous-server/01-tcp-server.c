#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 512
#define END_STRING "END."
#define MAX_CLIENTS 50

int g_server_socket;
char g_net_buffer[BUFFER_SIZE];

int g_connection_fd[MAX_CLIENTS];
fd_set g_connection_fd_set;
int g_max_sd;
int g_activity;

// int get_first_free_slot()
// {
//     // Bad O(n) algorithm
//     for(int i = 0; i < MAX_CLIENTS; i++)
//     {
//         if(g_connection_fd[i] == -1)
//         {
//             return i;
//         }
//     }
//     return -1; // No slot available
// }

void int_signal_handler()
{
    fprintf(stderr, "Gracefully exiting server...\n");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (g_connection_fd[i] > 0)
            close(g_connection_fd[i]);
    }
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
    fprintf(stderr, "Initializing server\n");
    memset(g_net_buffer, 0, BUFFER_SIZE); // Network buffer cleanup
    // Set client descriptors to invalid values to determine unused slots
    memset(g_connection_fd, 0, MAX_CLIENTS * sizeof(int));

    g_server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (g_server_socket == -1)
    {
        perror("An error occured while creating a socket");
        exit(EXIT_FAILURE);
    }
    // if(setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, NULL, NULL) == -1)
    // {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }

    struct sockaddr_in my_addr = prepare_address();

    if (bind(g_server_socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        perror("An error occured while binding a socket");
        close(g_server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(g_server_socket, MAX_CLIENTS) == -1)
    {
        perror("An error occured while trying to listen");
        close(g_server_socket);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Server initialized successfully");
}

void server_loop()
{
    socklen_t sock_size = sizeof(struct sockaddr_in);
    char client_addr[16];
    char input[BUFFER_SIZE];

    fprintf(stderr, "Waiting for connections\n");
    for (;;)
    {
        FD_ZERO(&g_connection_fd_set);
        FD_SET(g_server_socket, &g_connection_fd_set);
        g_max_sd = g_server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = g_connection_fd[i];
            if(sd > 0)
            {
                FD_SET( sd, &g_connection_fd_set);
            }
            if(sd > g_max_sd)
            {
                g_max_sd = sd;
            }
        }


        g_activity = select(g_max_sd + 1, &g_connection_fd_set, NULL, NULL, NULL);

        if (g_activity < 0)
        {
            fprintf(stderr, "Select error\n");
            exit(EXIT_FAILURE);
        }

        // Activity on g_server_socket = new connection, so handle it
        if (FD_ISSET(g_server_socket, &g_connection_fd_set))
        {
            struct sockaddr_in client_info;
            int new_socket = accept(g_server_socket, NULL, NULL);
            if (new_socket == -1)
            {
                perror("Accept failed");
                close(g_server_socket);
                exit(EXIT_FAILURE);
            }
            getpeername(new_socket, (struct sockaddr *)&client_info, &sock_size);
            inet_ntop(AF_INET, &(client_info.sin_addr), client_addr, INET_ADDRSTRLEN);
            printf("# A new client from %s came in! Conversation started.\n", client_addr);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (g_connection_fd[i] == 0)
                {
                    g_connection_fd[i] = new_socket;
                    break;
                }
            }
        }

        for(int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = g_connection_fd[i];

            if( FD_ISSET(sd, &g_connection_fd_set))
            {
                printf("%d set\n", sd);
                int br;
                if ((br = read(sd, g_net_buffer, BUFFER_SIZE)) == 0)
                {
                    struct sockaddr_in client_info;
                    getpeername(sd, (struct sockaddr *)&client_info, &sock_size);
                    inet_ntop(AF_INET, &(client_info.sin_addr), client_addr, INET_ADDRSTRLEN);
                    printf("# A client from %s ended.\n", client_addr);
                    close(sd);
                    g_connection_fd[i] = 0;
                }
                else
                {
                    g_net_buffer[br] = '\0';
                    printf("%s\n", g_net_buffer);
                }
            }
        }
    }

    close(g_server_socket);
}

int main()
{
    // Setup signal handlers for CTRL-C and TERM (15)
    signal(SIGINT, int_signal_handler);
    signal(SIGTERM, int_signal_handler);

    server_init();
    server_loop();
    
    return EXIT_SUCCESS;
}
