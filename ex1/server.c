/**
 * Resolução do exercício 1 (questão 1.a usando fifo);
 * Server side
 *
 * Authors: Gabriel Craco, Leonardo Jun-Ity, Alan e João Marcos
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#define SERVER_FIFO "/tmp/addition_fifo_server"

int count_vowels(const char *str)
{
    int count = 0;
    while (*str)
    {
        if (strchr("aeiouAEIOU", *str))
        {
            count++;
        }
        str++;
    }
    return count;
}

int count_consonants(const char *str)
{
    int count = 0;
    while (*str)
    {
        if (isalpha(*str) && !strchr("aeiouAEIOU", *str))
        {
            count++;
        }
        str++;
    }
    return count;
}

int count_spaces(const char *str)
{
    int count = 0;
    while (*str)
    {
        if (*str == ' ')
        {
            count++;
        }
        str++;
    }
    return count;
}

int count_length(const char *str)
{
    int length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

int main()
{
    int fd, fd_client;
    char buf[4096];
    char *return_fifo;
    char *client_msg;

    printf("-- SERVIDOR iniciado.\n\n");

    if ((mkfifo(SERVER_FIFO, 0664) == -1) && (errno != EEXIST))
    {
        perror("mkfifo");
        exit(1);
    }

    if ((fd = open(SERVER_FIFO, O_RDONLY)) == -1)
    {
        perror("open");
        exit(1);
    }

    while (1)
    {
        memset(buf, '\0', sizeof(buf));

        int bytes_read = read(fd, buf, sizeof(buf));
        if (bytes_read <= 0)
            continue;

        return_fifo = strtok(buf, ",\n");
        client_msg = strtok(NULL, "\n");

        if (return_fifo == NULL || client_msg == NULL)
        {
            fprintf(stderr, "Formato inválido recebido.\n");
            continue;
        }

        printf("Mensagem de %s: %s\n", return_fifo, client_msg);

        int length = count_length(client_msg);
        int vogais = count_vowels(client_msg);
        int consoantes = count_consonants(client_msg);
        int spaces = count_spaces(client_msg);

        if ((fd_client = open(return_fifo, O_WRONLY)) == -1)
        {
            perror("open: client fifo");
            continue;
        }

        int offset = 0;

        offset += snprintf(buf + offset, sizeof(buf) - offset, "\n\nMensagem: %s\n", client_msg);
        offset += snprintf(buf + offset, sizeof(buf) - offset, "Tamanho da mensagem: %d\n", length);
        offset += snprintf(buf + offset, sizeof(buf) - offset, "Número de vogais: %d\n", vogais);
        offset += snprintf(buf + offset, sizeof(buf) - offset, "Número de consoantes: %d\n", consoantes);
        offset += snprintf(buf + offset, sizeof(buf) - offset, "Número de espaços: %d\n", spaces);
        printf("%s\n", buf);

        write(fd_client, buf, strlen(buf));
        close(fd_client);
    }
}
