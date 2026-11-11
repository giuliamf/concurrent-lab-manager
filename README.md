# Concurrent Lab Manager (Gerenciador do LINF)

Projeto desenvolvido para a disciplina de **Programação Concorrente (Trabalho 1: Sincronização entre Processos)** do Departamento de Ciência da Computação da **Universidade de Brasília (UnB)**.

---

## 📌 Visão Geral do Problema

O projeto modela um sistema concorrente de alocação de recursos finitos: o **Laboratório de Informática (LINF)**.

- **Entidades:** $N$ alunos (representados por *threads* POSIX) e $M$ computadores (representados por um vetor compartilhado de estados).
- **Desafio:** Múltiplos alunos desejam acessar o laboratório simultaneamente, mas o número de computadores disponíveis é inferior à quantidade de alunos interessados.
- **Memória Compartilhada:** O estado de ocupação dos computadores (`0 = livre`, `1 = ocupado`) é mantido em um vetor compartilhado (`computadores[]`), sujeito a condições de corrida (*race conditions*) se acessado concorrentemente sem a devida sincronização.

---

## 🛠️ Mecanismos de Sincronização

A solução utiliza a biblioteca POSIX (`pthread` e `semaphore.h`) combinando dois mecanismos fundamentais:

1. **Semáforo Contador (`sem_t vagas`):**
   - Inicializado com a quantidade total de computadores disponíveis ($M$).
   - Regula o acesso global ao laboratório: se todas as vagas estiverem ocupadas, as threads dos alunos excedentes são bloqueadas em `sem_wait(&vagas)` até que algum aluno conclua o uso e execute `sem_post(&vagas)`.

2. **Mutex (`pthread_mutex_t mutex_computadores`):**
   - Garante **exclusão mútua** sobre a região crítica (acesso e modificação do vetor `computadores[]`).
   - Evita que duas threads leiam o mesmo computador como "livre" simultaneamente e o aloquem ao mesmo tempo, impedindo colisões e estados inconsistentes.

```
       [ Aluno (Thread) ]
               │
               ▼
        sem_wait(&vagas)           <- Bloqueia se o laboratório estiver lotado
               │
               ▼
   pthread_mutex_lock(&mutex)      <- Entra na Região Crítica
      Busca e ocupa computador[i]
   pthread_mutex_unlock(&mutex)    <- Sai da Região Crítica
               │
               ▼
           sleep(1)                <- Simula o uso do computador
               │
               ▼
   pthread_mutex_lock(&mutex)      <- Entra na Região Crítica
      Libera computador[i] = 0
   pthread_mutex_unlock(&mutex)    <- Sai da Região Crítica
               │
               ▼
        sem_post(&vagas)           <- Libera vaga para o próximo aluno
```

---

## 📂 Estrutura do Projeto

```text
concurrent-lab-manager/
├── src/
│   ├── linf.c               # Implementação correta e segura (Semáforo + Mutex)
│   └── linf_sem_mutex.c     # Versão de teste/demonstração de condição de corrida (Sem Mutex)
├── .gitignore
└── README.md
```

### Arquivos de Código-Fonte

- **[`src/linf.c`](src/linf.c:1)**:
  Solução completa e sincronizada. Utiliza o semáforo para gerenciar a capacidade e o mutex para proteger o vetor compartilhado, garantindo que nenhum computador seja atribuído a mais de um aluno e prevenindo inconsistências.

- **[`src/linf_sem_mutex.c`](src/linf_sem_mutex.c:1)**:
  Versão criada para fins experimentais e comparativos. Ao remover o mutex e introduzir uma breve pausa (`usleep`), expõe a janela de corrida onde múltiplos alunos disputam e alocam indevidamente o mesmo computador.

---

## 🚀 Como Compilar e Executar

### Pré-requisitos
- Compilador C (`gcc` ou `clang`) com suporte a **POSIX Threads**.
- Ambiente Linux, macOS ou WSL (Windows Subsystem for Linux).

### 1. Compilando a Solução Sincronizada (Correta)

```bash
gcc -Wall -Wextra -pthread src/linf.c -o linf
```

Execução:
```bash
./linf
```

### 2. Compilando a Versão sem Mutex (Demonstração de Condição de Corrida)

```bash
gcc -Wall -Wextra -pthread src/linf_sem_mutex.c -o linf_sem_mutex
```

Execução:
```bash
./linf_sem_mutex
```

---

## 📊 Análise de Resultados

| Cenário | Semáforo | Mutex | Resultado Observado |
| :--- | :---: | :---: | :--- |
| **`linf.c`** | ✅ | ✅ | **Correto:** Capacidade respeitada, alocação estritamente única por computador e liberação segura sem conflitos de memória. |
| **`linf_sem_mutex.c`** | ✅ | ❌ | **Inconsistente:** Múltiplas threads identificam a mesma posição livre simultaneamente, resultando em sobreposição de uso do mesmo computador ou falha na alocação. |

---

## 📄 Especificações do Ambiente e Parâmetros

No código-fonte, os seguintes parâmetros podem ser ajustados via `#define`:
- `NUM_ALUNOS` (padrão: `10`): Quantidade total de threads concorrentes.
- `NUM_COMPUTADORES` (padrão: `3`): Capacidade máxima do laboratório e tamanho do vetor compartilhado.
