#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 512

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "You need to specify server IP!\n");
    }
    char buff[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    int my_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (my_socket == -1)
    {
        perror("An error occured while creating a socket");
        exit(EXIT_SUCCESS);
    }

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(1337);
    my_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if(connect(my_socket, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1)
    {
        perror("An error ocured while connecting to a server");
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
        printf("CLIENT> ");
        fgets(input, BUFFER_SIZE, stdin);
        input[strcspn(input, "\n")] = 0;
        write(my_socket, input, strlen(input) + 1);
        if (strcmp(input, "END.") == 0)
        {
            break;
        }

        int read_bytes = read(my_socket, buff, BUFFER_SIZE);
        if (read_bytes == 0)
        {
            break;
        }
        printf("SERVER> %s\n", buff);
        if (strcmp(buff, "END.") == 0)
        {
            break;
        }
    }
    close(my_socket);
    return EXIT_SUCCESS;
}