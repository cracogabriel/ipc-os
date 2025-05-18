/**
 * Resolução do exercício 3 (questão 3);
 *
 * Authors: Gabriel Craco, Leonardo Jun-Ity, Alan e João Marcos
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX 1024              // tamanho máximo do vetor
#define CHILD_FINISHED_CODE 1 // Código de sinalização que o filho acabou

int main()
{
    int numFilhos;
    printf("Informe a quantidade de filhos: ");
    scanf("%d", &numFilhos);

    int numElementos;
    printf("Informe a quantidade de elementos: ");
    scanf("%d", &numElementos);

    if (numFilhos <= 0 || numFilhos > MAX)
    {
        fprintf(stderr, "Número de filhos inválido!\n");
        return 1;
    }

    if (numElementos <= 0)
    {
        fprintf(stderr, "Número de elementos inválido!\n");
        return 1;
    }

    // Criação da memória compartilhada de dados para os dois vetores e o vetor resultado
    key_t key_dados = IPC_PRIVATE;
    int shmid_dados = shmget(key_dados, 3 * numElementos * sizeof(int), IPC_CREAT | 0666);
    if (shmid_dados < 0)
    {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }

    // Pai anexa à memória de dados compartilhada
    int *memoria_dados = (int *)shmat(shmid_dados, NULL, 0);
    if (memoria_dados == (int *)-1)
    {
        perror("Erro ao anexar memória de dados compartilhada no pai");
        exit(1);
    }

    // Criação da memória compartilhada de sincronização para os dois vetores e o vetor resultado
    key_t key_sinc = IPC_PRIVATE;
    int shmid_sinc = shmget(key_sinc, numFilhos * sizeof(int), IPC_CREAT | 0666);
    if (shmid_sinc < 0)
    {
        perror("Erro ao criar memória de sincronização compartilhada");
        exit(1);
    }

    // Pai anexa à memória de sincronização compartilhada
    int *memoria_sinc = (int *)shmat(shmid_sinc, NULL, 0);
    if (memoria_sinc == (int *)-1)
    {
        perror("Erro ao anexar memória de sincronização compartilhada no pai");
        exit(1);
    }

    // Vetores vetor1 e vetor2 estão na memória compartilhada
    int *vetor1 = memoria_dados;
    int *vetor2 = memoria_dados + numElementos;
    int *resultado = memoria_dados + (2 * numElementos);

    // Inicializando os vetores vetor1 e vetor2 com valores
    for (int i = 0; i < numElementos; i++)
    {
        vetor1[i] = i + 1; // Preenche vetor1 com valores de 1 a numElementos
        vetor2[i] = i + 1; // Preenche vetor2 com valores de 1 a numElementos
    }

    // Criação do pipe para comunicação entre pai e filhos
    int mypipe[2];
    if (pipe(mypipe) == -1)
    {
        perror("Falha ao criar o Pipe.");
        return EXIT_FAILURE;
    }

    // Criação dos filhos
    for (int i = 0; i < numFilhos; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("Erro no fork");
            exit(1);
        }
        else if (pid == 0)
        {
            // Processo filho
            close(mypipe[1]); // Fecha a escrita no pipe, pois o filho só vai ler

            int buffer[2];
            read(mypipe[0], buffer, sizeof(buffer)); // Lê os intervalos de índice

            int pos_inicial = buffer[0];
            int pos_final = buffer[1];

            // Realiza a soma dos vetores na memória compartilhada
            for (int j = pos_inicial; j <= pos_final; j++)
            {
                resultado[j] = vetor1[j] + vetor2[j];
                printf("Filho %d (PID %d) somou: vetor1[%d] + vetor2[%d] = %d\n", i, getpid(), j, j, resultado[j]);
            }

            memoria_sinc[i] = CHILD_FINISHED_CODE; // Sinaliza na memória que acabou

            close(mypipe[0]);     // Fecha a leitura no pipe
            shmdt(memoria_dados); // Desanexa a memória compartilhada de dados do filho
            shmdt(memoria_sinc);  // Desanexa a memória compartilhada de sincronização do filho
            exit(0);              // Finaliza o processo filho
        }
    }

    // Processo pai
    for (int i = 0; i < numFilhos; i++)
    {
        int pos_inicial = (numElementos / numFilhos) * i;
        int pos_final = ((numElementos / numFilhos) * (i + 1)) - 1;

        int buffer[2] = {pos_inicial, pos_final};

        close(mypipe[0]);                         // Fecha a leitura no pipe, pois o pai só vai escrever
        write(mypipe[1], buffer, sizeof(buffer)); // Envia o intervalo para o filho

        printf("Pai enviando intervalo: [%d, %d] para o filho %d\n", pos_inicial, pos_final, i);
    }

    // Espera os filhos terminarem
    while (1)
    {
        int filhosFinalizados = 0;
        for (int i = 0; i < numFilhos; i++)
        {

            if (memoria_sinc[i] == CHILD_FINISHED_CODE)
                filhosFinalizados++;
        }
        if (filhosFinalizados == numFilhos)
        {
            printf("Todos os filhos acabaram o calculo. Finalizando espera.\n\n");
            break; // Se todos os filhos acabaram, finaliza a espera
        }

        filhosFinalizados = 0;
    }

    // Exibe o resultado final
    printf("Resultado final da soma dos vetores:\n");

    printf("[");
    for (int i = 0; i < numElementos; i++)
    {
        printf("%d", resultado[i]);
        if (numElementos - 1 == i)
            printf("]");
        else
            printf(", ");
    }

    printf("\n");

    // Liberação de memória compartilhada
    shmdt(memoria_dados);                // pai se desconecta
    shmdt(memoria_sinc);                 // pai se desconecta
    shmctl(shmid_dados, IPC_RMID, NULL); // remove a memória de dados compartilhada
    shmctl(shmid_sinc, IPC_RMID, NULL);  // remove a memória de sincronização compartilhada

    printf("Pai finalizou.\n");
    return 0;
}
