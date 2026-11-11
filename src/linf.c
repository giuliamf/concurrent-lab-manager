#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_ALUNOS 10
#define NUM_COMPUTADORES 3

int computadores[NUM_COMPUTADORES] = {0};

pthread_mutex_t mutex_computadores;
sem_t vagas;

void *aluno(void *arg)
{
    int *id_ptr = (int *)arg;
    int id = *id_ptr;

    printf("Aluno %d aguardando uma vaga.\n", id);

    sem_wait(&vagas);

    printf("Aluno %d conseguiu uma vaga.\n", id);

    pthread_mutex_lock(&mutex_computadores);

    int computador_escolhido = -1;

    for (int i = 0; i < NUM_COMPUTADORES; i++)
    {
        if (computadores[i] == 0)
        {
            computador_escolhido = i;
            computadores[i] = 1;
            break;
        }
    }

    pthread_mutex_unlock(&mutex_computadores);

    printf(
        "Aluno %d esta usando o computador %d.\n",
        id,
        computador_escolhido
    );

    sleep(1);

    pthread_mutex_lock(&mutex_computadores);

    computadores[computador_escolhido] = 0;

    pthread_mutex_unlock(&mutex_computadores);

    sem_post(&vagas);

    printf(
        "Aluno %d liberou o computador %d.\n",
        id,
        computador_escolhido
    );

    return NULL;
}

int main(void)
{
    pthread_mutex_init(&mutex_computadores, NULL);

    if (sem_init(&vagas, 0, NUM_COMPUTADORES) == -1)
    {
        perror("Erro ao inicializar o semaforo");
        pthread_mutex_destroy(&mutex_computadores);
        return 1;
    }

    printf("Sistema de Gerenciamento do LINF\n");
    printf("Estado do computador 0: %d\n", computadores[0]);

    pthread_t threads[NUM_ALUNOS];
    int ids[NUM_ALUNOS];

    for (int i = 0; i < NUM_ALUNOS; i++)
    {
        ids[i] = i + 1;

        pthread_create(
            &threads[i],
            NULL,
            aluno,
            &ids[i]
        );
    }

    for (int i = 0; i < NUM_ALUNOS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&vagas);
    pthread_mutex_destroy(&mutex_computadores);

    return 0;
}