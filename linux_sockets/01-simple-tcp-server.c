#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 512

int main()
{
    char buff[BUFFER_SIZE];

    int my_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (my_socket == -1)
    {
        perror("An error occured while creating a socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(1337);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(my_socket, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1)
    {
        perror("An error occured while binding a socket");
        close(my_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(my_socket, 10) == -1)
    {
        perror("An error occured while trying to listen");
        close(my_socket);
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
        int connection_fd = accept(my_socket, NULL, NULL);
        if (connection_fd == -1)
        {
            perror("Accept failed");
            close(my_socket);
            exit(EXIT_FAILURE);
        }
        puts("Hello to new client");
        write(connection_fd, "HELLO!", 7);

        int read_bytes;
        puts("Reading from new client");
        read_bytes = read(connection_fd, buff, BUFFER_SIZE);
        printf("Read from socket (size %d): %s\n", read_bytes, buff);

        puts("Responding");
        write(connection_fd, "THANKS!", 8);

        if (shutdown(connection_fd, SHUT_RDWR) == -1) {
            perror("shutdown failed");
            close(connection_fd);
            close(my_socket);
            exit(EXIT_FAILURE);
        }
        close(connection_fd);
    }

    close(my_socket);
    return EXIT_SUCCESS;
}