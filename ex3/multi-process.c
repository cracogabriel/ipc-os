#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define MAX 1024 // tamanho máximo do vetor

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

    // Criação da memória compartilhada para os dois vetores e o vetor resultado
    key_t key = IPC_PRIVATE;
    int shmid = shmget(key, 3 * numElementos * sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0)
    {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }

    // Pai anexa à memória compartilhada
    int *memoria = (int *)shmat(shmid, NULL, 0);
    if (memoria == (int *)-1)
    {
        perror("Erro ao anexar memória compartilhada no pai");
        exit(1);
    }

    // Vetores vetor1 e vetor2 estão na memória compartilhada
    int *vetor1 = memoria;
    int *vetor2 = memoria + numElementos;
    int *resultado = memoria + (2 * numElementos);

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
            
            close(mypipe[0]); // Fecha a leitura no pipe
            shmdt(memoria);   // Desanexa a memória compartilhada do filho
            exit(0);          // Finaliza o processo filho
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
    for (int i = 0; i < numFilhos; i++)
    {
        wait(NULL); // Espera um filho terminar
    }

    // Exibe o resultado final
    printf("Resultado final da soma dos vetores:\n");
    
    printf("[");
    for (int i = 0; i < numElementos; i++)
    {
        printf("%d ", resultado[i]);
    }
    printf("]");

    printf("\n");

    // Liberação de memória compartilhada
    shmdt(memoria);                // pai se desconecta
    shmctl(shmid, IPC_RMID, NULL); // remove a memória compartilhada

    printf("Pai finalizou.\n");
    return 0;
}
