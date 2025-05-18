/**
 * Resolução do exercício 4 (questão 5.a);
 * Server side
 *
 * Authors: Gabriel Craco, Leonardo Jun-Ity, Alan e João Marcos
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

char *socket_path = "\0socket-translator";

#define MAX_LINES 100       // Número máximo de linhas no arquivo
#define MAX_LINE_LENGTH 256 // Máximo de caracteres por linha

// Função para enviar mensagem ao cliente
void send_message(int client_socket, const char *msg)
{
    // Tenta enviar a mensagem para o cliente
    int sent_bytes = write(client_socket, msg, strlen(msg));
    if (sent_bytes == -1)
    {
        perror("Erro ao enviar mensagem para o cliente");
    }
    else
    {
        printf("Mensagem enviada para o cliente: %s\n", msg);
    }
}

// Função para ler o arquivo e armazenar as linhas em uma lista de strings
void read_file(char ***list, const char *path, int *lineCount)
{
    FILE *file;
    char line[MAX_LINE_LENGTH];

    // Tentar abrir o arquivo para leitura
    file = fopen(path, "r");
    if (file == NULL)
    {
        perror("Não foi possível abrir o arquivo");
        return;
    }

    // Tentar mover o ponteiro do arquivo para o início
    fseek(file, 0, SEEK_SET);

    // Alocar memória para o ponteiro da lista
    *list = malloc(MAX_LINES * sizeof(char *));
    if (*list == NULL)
    {
        perror("Erro de alocação de memória");
        fclose(file);
        return;
    }

    // Ler cada linha do arquivo e armazenar no array de strings
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Remover o caractere de nova linha, se presente
        line[strcspn(line, "\n")] = 0;

        // Alocar memória para armazenar a linha
        (*list)[*lineCount] = malloc(strlen(line) + 1); // +1 para o terminador '\0'
        if ((*list)[*lineCount] == NULL)
        {
            perror("Erro de alocação de memória");
            fclose(file);
            return;
        }

        // Copiar a linha para o array
        strcpy((*list)[*lineCount], line);
        (*lineCount)++;

        // Se atingir o número máximo de linhas, pare
        if (*lineCount >= MAX_LINES)
        {
            break;
        }
    }

    // Fechar o arquivo após a leitura
    fclose(file);
}

// Função para liberar a memória alocada para as listas de strings
void free_list(char **list, int lineCount)
{
    for (int i = 0; i < lineCount; i++)
    {
        free(list[i]); // Liberar cada linha
    }
    free(list); // Liberar a lista de ponteiros
}

// Função para procurar a palavra na lista e retornar o índice ou -1 se não encontrar
int search_word(const char *word, char *list[], int word_count)
{
    for (int i = 0; i < word_count; i++)
    {
        if (strcmp(word, list[i]) == 0)
        {
            return i; // Palavra encontrada, retorna o índice
        }
    }
    return -1; // Palavra não encontrada
}

int main(int argc, char *argv[])
{
    int server_socket,       // descritor do socket
        client_socket,       // socket da conexao do cliente
        received_bytes;      // bytes recebidos
    struct sockaddr_un addr; // endereço socket
    char buf[100];           // buffer de comunicação

    char **en_list, **pt_list, **es_list;
    int en_lineCount = 0, pt_lineCount = 0, es_lineCount = 0;

    read_file(&en_list, "./languages/en.txt", &en_lineCount);
    read_file(&pt_list, "./languages/pt.txt", &pt_lineCount);
    read_file(&es_list, "./languages/es.txt", &es_lineCount);

    // Exibir os conteúdos (apenas para verificação)
    printf("Conteúdo de en.txt:\n");
    for (int i = 0; i < en_lineCount; i++)
    {
        printf("%s\n", en_list[i]);
    }

    printf("\nConteúdo de pt.txt:\n");
    for (int i = 0; i < pt_lineCount; i++)
    {
        printf("%s\n", pt_list[i]);
    }

    printf("\nConteúdo de es.txt:\n");
    for (int i = 0; i < es_lineCount; i++)
    {
        printf("%s\n", es_list[i]);
    }

    if (argc > 1)
        socket_path = argv[1];

    /* cria um socket AF_UNIX do tipo SOCK_STREAM */
    if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(-1);
    }

    /* configura endereço do socket */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    *addr.sun_path = '\0';
    strncpy(addr.sun_path + 1, socket_path + 1, sizeof(addr.sun_path) - 2);

    /* mapeia o socket para o socket_path */
    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* configura para aguardar conexões */
    if (listen(server_socket, 5) == -1)
    {
        perror("listen error");
        exit(-1);
    }

    printf("\n\nServidor iniciando...\n\n");

    while (1)
    {
        /* aguarda conexões dos clientes */
        if ((client_socket = accept(server_socket, NULL, NULL)) == -1)
        {
            perror("accept error");
            continue;
        }

        /* lê dados envidos pelos clientes */
        while ((received_bytes = read(client_socket, buf, sizeof(buf))) > 0)
        {
            printf("read %u bytes: %.*s\n", received_bytes, received_bytes, buf);
            char sourceLanguage[3], targetLanguage[3], word[100];

            if (sscanf(buf, "%2s-%2s:%99s", sourceLanguage, targetLanguage, word) == 3)
            {
                int wordIndex = -1;
                printf("Source Language: %s\n", sourceLanguage);
                printf("Target Language: %s\n", targetLanguage);
                printf("Word: %s\n", word);

                // Encontra o index da palavra a ser traduzida
                if (strcmp(sourceLanguage, "pt") == 0)
                {
                    wordIndex = search_word(word, pt_list, pt_lineCount);
                }
                else if (strcmp(sourceLanguage, "en") == 0)
                {
                    wordIndex = search_word(word, en_list, en_lineCount);
                }
                else if (strcmp(sourceLanguage, "es") == 0)
                {
                    wordIndex = search_word(word, es_list, es_lineCount);
                }

                if (wordIndex == -1)
                {
                    // Manda pro cliente que deu erro ERROR:UNKNOWN
                    send_message(client_socket, "ERROR:UNKNOWN\n");
                }
                else
                {
                    printf("Não é erro, %s\n", targetLanguage);
                    char *msg = malloc(256 * sizeof(char)); // Aloca memória para a mensagem

                    if (msg == NULL)
                    {
                        perror("Erro de alocação de memória para mensagem");
                        exit(1);
                    }

                    if (strcmp(targetLanguage, "pt") == 0)
                    {
                        // Manda pro cliente pt_list[wordIndex]
                        printf("palavra traduzida %s\n", pt_list[wordIndex]);
                        snprintf(msg, 256, "%s\n", pt_list[wordIndex]); // Limita o tamanho da string
                        send_message(client_socket, msg);               // Envia para o cliente
                    }
                    else if (strcmp(targetLanguage, "en") == 0)
                    {
                        // Manda pro cliente en_list[wordIndex]
                        printf("palavra traduzida %s\n", en_list[wordIndex]);
                        snprintf(msg, 256, "%s\n", en_list[wordIndex]);
                        send_message(client_socket, msg);
                    }
                    else if (strcmp(targetLanguage, "es") == 0)
                    {
                        // Manda pro cliente es_list[wordIndex]
                        printf("palavra traduzida %s\n", es_list[wordIndex]);
                        snprintf(msg, 256, "%s\n", es_list[wordIndex]);
                        send_message(client_socket, msg);
                    }
                    else
                    {
                        // Target>message nao existe
                        printf("Erro ao processar a string.\n");
                        send_message(client_socket, "ERROR:UNKNOWN\n");
                    }

                    // Libera a memória depois de enviar a mensagem
                    free(msg);
                }
            }
            else
            {
                printf("Erro ao processar a string.\n");
                send_message(client_socket, "ERROR:UNKNOWN\n");
                // retornar ERROR:UNKNOWN para o cliente
            }
        }

        /* trata erros */
        if (received_bytes == -1)
        {
            perror("read");
            exit(-1);
        }
        else if (received_bytes == 0)
        {
            printf("EOF\n");
            close(client_socket);
        }
    }

    free_list(en_list, en_lineCount);
    free_list(pt_list, pt_lineCount);
    free_list(es_list, es_lineCount);

    return 0;
}