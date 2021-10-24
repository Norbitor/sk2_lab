#include <stdio.h>
#include <stdlib.h>
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
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(1337);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(my_socket, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1)
    {
        perror("An error ocured while connecting to a server");
        exit(EXIT_FAILURE);
    }

    int read_bytes = read(my_socket, buff, BUFFER_SIZE);
    if (read_bytes > 0)
    {
        printf("Message from server: %s\n", buff);
    }

    int writed_bytes = write(my_socket, "Message from client", 20);

    read_bytes = read(my_socket, buff, BUFFER_SIZE);
    if (read_bytes > 0)
    {
        printf("Server responded: %s\n", buff);
    }

    close(my_socket);
    return EXIT_SUCCESS;
}