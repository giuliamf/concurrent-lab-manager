#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_ALUNOS 10
#define NUM_COMPUTADORES 3

int computadores[NUM_COMPUTADORES] = {0};

sem_t vagas;

void *aluno(void *arg)
{
    int *id_ptr = (int *)arg;
    int id = *id_ptr;

    printf("Aluno %d aguardando uma vaga.\n", id);

    sem_wait(&vagas);

    printf("Aluno %d conseguiu uma vaga.\n", id);

    int computador_escolhido = -1;

    for (int i = 0; i < NUM_COMPUTADORES; i++)
    {
        if (computadores[i] == 0)
        {
            /*
             * Pausa proposital para aumentar a janela de corrida.
             * Outra thread pode verificar o mesmo computador
             * antes que esta thread o marque como ocupado.
             */
            usleep(100000);

            computador_escolhido = i;
            computadores[i] = 1;
            break;
        }
    }

    if (computador_escolhido == -1)
    {
        printf(
            "ERRO: Aluno %d conseguiu uma vaga, mas nao encontrou computador livre.\n",
            id
        );

        sem_post(&vagas);
        return NULL;
    }

    printf(
        "Aluno %d esta usando o computador %d.\n",
        id,
        computador_escolhido
    );

    sleep(1);

    computadores[computador_escolhido] = 0;

    printf(
        "Aluno %d liberou o computador %d.\n",
        id,
        computador_escolhido
    );

    sem_post(&vagas);

    return NULL;
}

int main(void)
{
    if (sem_init(&vagas, 0, NUM_COMPUTADORES) == -1)
    {
        perror("Erro ao inicializar o semaforo");
        return 1;
    }

    printf("=== EXPERIMENTO SEM MUTEX ===\n");

    pthread_t threads[NUM_ALUNOS];
    int ids[NUM_ALUNOS];

    for (int i = 0; i < NUM_ALUNOS; i++)
    {
        ids[i] = i + 1;

        int resultado = pthread_create(
            &threads[i],
            NULL,
            aluno,
            &ids[i]
        );

        if (resultado != 0)
        {
            printf(
                "Erro ao criar a thread do aluno %d.\n",
                ids[i]
            );

            return 1;
        }
    }

    for (int i = 0; i < NUM_ALUNOS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&vagas);

    return 0;
}