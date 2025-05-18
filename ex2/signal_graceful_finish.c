/**
 * Resolução do exercício 2 (questão 2.b com sinais);
 *
 * Authors: Gabriel Craco, Leonardo Jun-Ity, Alan e João Marcos
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* Variáveis globais para armazenar o arquivo e informações pendentes */
FILE *file;
char buffer[256]; // Buffer para armazenar dados

/* Função tratadora de sinais */
void sig_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM)
    {
        // Antes de sair, escreve qualquer informação pendente no arquivo
        if (file != NULL)
        {
            // Prova que finalizou de forma limpa, gravando a frase ao final.
            fprintf(file, "Finalização limpa do programa.\n");
            fclose(file);
            printf("Arquivo fechado e dados salvos. Saindo...\n");
        }
        exit(0); // Finaliza o programa
    }
}

int main(void)
{
    // Associa a função tratadora de sinais
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        printf("\nNão foi possível capturar o SIGINT\n");
        exit(1);
    }
    if (signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        printf("\nNão foi possível capturar o SIGTERM\n");
        exit(1);
    }

    // Exibe o PID
    printf("Meu PID é %d.\n", getpid());

    // Abre ou cria o arquivo onde os dados serão armazenados
    file = fopen("dados.txt", "a");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }

    // Escreve dados no arquivo periodicamente
    for (int i = 0; i < 5; i++)
    {
        snprintf(buffer, sizeof(buffer), "Informação %d para o arquivo\n", i + 1);
        fputs(buffer, file);
        printf("Dados gravados no arquivo: %s", buffer);
        sleep(30); // Simula trabalho por 30 segundos
    }

    // Simula o programa trabalhando infinitamente
    while (1)
    {
        sleep(1);
    }

    return 0;
}
