#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_ALUNOS 10
#define NUM_COMPUTADORES 3

/*
 * Vetor compartilhado entre todas as threads.
 * Cada posição representa um computador:
 * 0 = livre
 * 1 = ocupado
 */
int computadores[NUM_COMPUTADORES] = {0};

/*
 * Mutex utilizado para proteger o acesso ao vetor computadores.
 * Garante que apenas uma thread por vez possa verificar ou alterar
 * o estado dos computadores.
 */
pthread_mutex_t mutex_computadores;

/*
 * Semáforo contador utilizado para representar as vagas disponíveis.
 * Seu valor inicial será igual ao número de computadores.
 */
sem_t vagas;

/*
 * Função executada por cada thread.
 * Cada thread representa um aluno tentando utilizar um computador.
 */
void *aluno(void *arg)
{
    int *id_ptr = (int *)arg;
    int id = *id_ptr;

    printf("Aluno %d aguardando uma vaga.\n", id);

    /*
     * Tenta adquirir uma vaga no laboratório.
     * Quando todas as vagas estão ocupadas, a thread fica bloqueada
     * até que outro aluno libere uma vaga.
     */
    if (sem_wait(&vagas) == -1)
    {
        perror("Erro ao aguardar uma vaga");
        return NULL;
    }

    printf("Aluno %d conseguiu uma vaga.\n", id);

    /*
     * Início da região crítica.
     * O mutex impede que duas threads consultem e alterem
     * simultaneamente o vetor compartilhado.
     */
    pthread_mutex_lock(&mutex_computadores);

    int computador_escolhido = -1;

    /*
     * Procura o primeiro computador disponível e o marca como ocupado.
     */
    for (int i = 0; i < NUM_COMPUTADORES; i++)
    {
        if (computadores[i] == 0)
        {
            computador_escolhido = i;
            computadores[i] = 1;
            break;
        }
    }

    /*
     * Fim da região crítica.
     * Outras threads podem voltar a acessar o vetor.
     */
    pthread_mutex_unlock(&mutex_computadores);

    /*
     * Como o semáforo controla o mesmo número de vagas existentes,
     * normalmente sempre deve existir um computador disponível após
     * sem_wait(). Esta verificação evita um acesso inválido ao vetor
     * caso ocorra alguma inconsistência.
     */
    if (computador_escolhido == -1)
    {
        fprintf(
            stderr,
            "Erro: aluno %d conseguiu uma vaga, mas nenhum computador estava livre.\n",
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

    /*
     * Simula o período durante o qual o aluno utiliza o computador.
     */
    sleep(1);

    /*
     * A liberação do computador também altera memória compartilhada.
     * Por isso, essa operação deve ser protegida pelo mutex.
     */
    pthread_mutex_lock(&mutex_computadores);

    computadores[computador_escolhido] = 0;

    pthread_mutex_unlock(&mutex_computadores);

    printf(
        "Aluno %d liberou o computador %d.\n",
        id,
        computador_escolhido
    );

    /*
     * Devolve uma vaga ao semáforo.
     * Uma thread que esteja esperando poderá prosseguir.
     */
    sem_post(&vagas);

    return NULL;
}

int main(void)
{
    /*
     * Inicializa o mutex que será utilizado para proteger
     * o vetor compartilhado de computadores.
     */
    if (pthread_mutex_init(&mutex_computadores, NULL) != 0)
    {
        fprintf(stderr, "Erro ao inicializar o mutex.\n");
        return 1;
    }

    /*
     * Inicializa o semáforo com NUM_COMPUTADORES vagas.
     * O segundo argumento igual a 0 indica que o semáforo
     * é compartilhado entre threads do mesmo processo.
     */
    if (sem_init(&vagas, 0, NUM_COMPUTADORES) == -1)
    {
        perror("Erro ao inicializar o semaforo");
        pthread_mutex_destroy(&mutex_computadores);
        return 1;
    }

    printf("Sistema de Gerenciamento do LINF\n");

    /*
     * Vetor que armazena as threads dos alunos e vetor com
     * um identificador individual para cada aluno.
     */
    pthread_t threads[NUM_ALUNOS];
    int ids[NUM_ALUNOS];

    /*
     * Guarda quantas threads foram efetivamente criadas.
     * Isso permite fazer pthread_join somente nas threads válidas
     * caso alguma criação falhe.
     */
    int threads_criadas = 0;
    int erro_criacao = 0;

    /*
     * Cria uma thread para cada aluno.
     */
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
            fprintf(
                stderr,
                "Erro ao criar a thread do aluno %d.\n",
                ids[i]
            );

            erro_criacao = 1;
            break;
        }

        threads_criadas++;
    }

    /*
     * Aguarda a finalização de todas as threads que foram
     * criadas com sucesso.
     */
    for (int i = 0; i < threads_criadas; i++)
    {
        pthread_join(threads[i], NULL);
    }

    /*
     * Após todas as threads terminarem, os mecanismos de
     * sincronização utilizados pelo programa são destruídos.
     */
    sem_destroy(&vagas);
    pthread_mutex_destroy(&mutex_computadores);

    return erro_criacao;
}