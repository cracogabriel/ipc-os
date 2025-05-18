
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define SERVER_FIFO "/tmp/addition_fifo_server"

int main() {
    char client_fifo[64];
    char buf[4096];
    char input[4000];  // buffer para mensagem do usuário
    int fd_server, fd_client;

    // Cria nome único para FIFO do cliente
    sprintf(client_fifo, "/tmp/client_fifo_%d", getpid());

    // Cria FIFO do cliente
    mkfifo(client_fifo, 0664);

    // Abre FIFO do servidor para escrita
    fd_server = open(SERVER_FIFO, O_WRONLY);
    if (fd_server == -1) {
        perror("Erro ao abrir FIFO do servidor");
        unlink(client_fifo);
        return 1;
    }

    while (1) {
        printf("Digite a mensagem para o servidor (ou 'sair'): ");
        fflush(stdout);
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "sair") == 0) break;

        // Monta mensagem: "<fifo_cliente>,<mensagem>"
        sprintf(buf, "%s,%s\n", client_fifo, input);
        write(fd_server, buf, strlen(buf));

        // Abre FIFO do cliente para leitura
        fd_client = open(client_fifo, O_RDONLY);
        if (fd_client == -1) {
            perror("Erro ao abrir FIFO do cliente");
            break;
        }

        memset(buf, 0, sizeof(buf));
        read(fd_client, buf, sizeof(buf));
        printf("Resposta do servidor: %s\n", buf);
        close(fd_client);
    }

    // Encerra conexão
    close(fd_server);
    unlink(client_fifo);
    printf("Cliente encerrado.\n");
    return 0;
}



