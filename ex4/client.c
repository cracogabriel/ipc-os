/**
 * Resolução do exercício 4 (questão 5.a);
 * Client side
 *
 * Authors: Gabriel Craco, Leonardo Jun-Ity, Alan e João Marcos
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUF_SIZE 256

char *socket_path = "\0socket-translator";

int main(int argc, char *argv[])
{
    int client_socket,       // descritor para o socket (file descriptor)
        sent_bytes;          // número de bytes enviados
    struct sockaddr_un addr; // estrutura de endereço socket unix
    char buf[MAX_BUF_SIZE];  // buffer para troca de mensagens

    // Usando socket_path do argumento, se fornecido
    if (argc > 1)
        socket_path = argv[1];

    // Cria socket UNIX do tipo SOCK_STREAM
    if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("Erro ao criar socket");
        exit(-1);
    }

    // Preenche a estrutura de endereço
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    *addr.sun_path = '\0'; // Usando socket abstrato
    strncpy(addr.sun_path + 1, socket_path + 1, sizeof(addr.sun_path) - 2);

    // Conecta ao servidor
    if (connect(client_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("Erro ao conectar");
        exit(-1);
    }

    printf("Conectado ao servidor de tradução. Digite a palavra para tradução no formato: <origem>-<destino>:<palavra>\n");

    // Loop para o cliente enviar várias palavras
    while (1)
    {
        // Solicita ao usuário para digitar uma palavra
        printf("Digite a solicitação (ou 'NO-NO' para terminar): ");
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            perror("Erro na leitura da entrada");
            break;
        }

        // Remove o caractere de nova linha no final da entrada
        buf[strcspn(buf, "\n")] = 0;

        // Se o usuário digitar "NO-NO", encerra a conexão
        if (strcmp(buf, "NO-NO") == 0)
        {
            printf("Saindo...\n");
            break;
        }

        // Verifica se a entrada está vazia (só foi pressionado enter)
        if (strlen(buf) == 0)
        {
            printf("Entrada vazia detectada. Por favor, digite algo válido ou 'NO-NO' para sair.\n");
            continue; // Solicita ao usuário novamente sem enviar a mensagem
        }

        // Envia a solicitação para o servidor
        sent_bytes = write(client_socket, buf, strlen(buf));
        if (sent_bytes != strlen(buf))
        {
            perror("Erro ao enviar dados");
            break;
        }

        // Aguarda resposta do servidor
        int received_bytes = read(client_socket, buf, sizeof(buf) - 1);
        if (received_bytes > 0)
        {
            buf[received_bytes] = '\0'; // Certifica-se de que a string esteja terminada corretamente
            printf("Resposta do servidor: %s\n", buf);
        }
        else if (received_bytes == 0)
        {
            printf("Servidor encerrou a conexão\n");
            break;
        }
        else
        {
            perror("Erro ao receber resposta");
            break;
        }
    }

    // Fecha o socket após o término
    close(client_socket);
    return 0;
}
