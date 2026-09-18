#ifndef SCHEDULER_IO_H
#define SCHEDULER_IO_H

#define MAX_LINHA_LEN 512

/* Identificador especial usado em FaixaGantt para indicar que a CPU
 * ficou ociosa durante aquele intervalo (em vez de um numero de
 * processo valido).
 */
#define ID_IDLE (-1)

/* Os processos do arquivo de entrada tem IDs no formato "P<numero>"
 * (ex.: "P1", "P2", ...). Por isso guardamos apenas o numero (`id`)
 * e reconstituimos o texto "P<numero>" na hora de imprimir, evitando
 * o uso de strings para os IDs.
 */
typedef struct {
    int id;                  /* numero do processo (ex.: 1 para "P1") */
    int chegada;             /* instante em que o processo chega na fila de prontos */
    int cpu;                 /* tempo total de CPU que o processo precisa */
    int termino;             /* instante em que o processo termina */
    int primeira_execucao;   /* primeiro instante em que ganhou a CPU (-1 = nunca) */
    int retorno;             /* tempo de retorno */
    int espera;              /* tempo de espera */
    int resposta;            /* tempo de resposta */
} Processo;

/* Uma fatia de tempo exibida na linha do tempo (Gantt).
 * Ex.: inicio=0, fim=3, id=1   ->  "0-3 P1"
 * Quando a CPU fica ociosa, use id=ID_IDLE -> "0-3 IDLE".
 */
typedef struct {
    int id;      /* numero do processo, ou ID_IDLE se a CPU ficou ociosa */
    int inicio;  /* instante em que a faixa comecou */
    int fim;     /* instante em que a faixa terminou */
} FaixaGantt;

/* Representa a linha do tempo (Gantt) completa da simulacao.
 *
 * O escalonador NAO deve manipular os campos desta struct diretamente
 * nem se preocupar com alocacao/realocacao de memoria: basta chamar
 * `registrarFaixaGantt` (ver abaixo) para adicionar cada faixa. Os
 * campos ficam expostos apenas para permitir a leitura das faixas
 * (ex.: em `escreverSaida`).
 */
typedef struct {
    FaixaGantt *faixas;  /* vetor alocado dinamicamente com as faixas registradas */
    int quantidade;      /* quantidade de faixas atualmente armazenadas em `faixas` */
    int capacidade;      /* capacidade atual (em elementos) do vetor `faixas` */
} Gantt;

/* Le o arquivo de entrada no formato:
 *   n=<int>
 *   # comentarios sao ignorados
 *   <id> <chegada> <cpu>
 *   ...
 *
 * Aloca dinamicamente `*processos` (o chamador e responsavel por dar
 * free) e preenche `*numProcessos` com a quantidade de processos
 * lidos. Encerra o programa com exit(1) em caso de erro de formato ou
 * leitura.
 */
void lerArquivoEntrada(const char *nomeArquivo, Processo **processos, int *numProcessos);

/* Escreve o arquivo de saida no formato exigido pelo enunciado:
 *   GANTT:
 *   RESULTADOS:
 *   MEDIAS:
 */
void escreverSaida(const char *nomeArquivo, const Gantt *gantt,
                    const Processo *processos, int numProcessos,
                    double mediaRetorno, double mediaEspera, double mediaResposta);

/* Inicializa um Gantt vazio. Deve ser chamada antes do primeiro uso
 * (por exemplo, no main, antes de chamar o escalonador).
 */
void inicializarGantt(Gantt *gantt);

/* Registra uma nova faixa (id, inicio, fim) no final da linha do
 * tempo do Gantt, cuidando de toda a alocacao/realocacao de memoria
 * necessaria internamente. O escalonador so precisa chamar esta
 * funcao uma vez para cada fatia de execucao ou ociosidade da CPU,
 * na ordem em que ocorreram; nao ha necessidade de lidar com
 * malloc/realloc/capacidade em nenhum momento.
 *
 * Parametros:
 *   gantt       - o Gantt a ser atualizado (deve ter sido inicializado
 *                 com `inicializarGantt`).
 *   id          - numero do processo (ex.: 1 para "P1"), ou ID_IDLE
 *                 se a CPU ficou ociosa nesse intervalo.
 *   inicio, fim - instantes de inicio e fim da faixa.
 */
void registrarFaixaGantt(Gantt *gantt, int id, int inicio, int fim);

/* Libera a memoria alocada internamente por um Gantt. */
void liberarGantt(Gantt *gantt);

#endif /* SCHEDULER_IO_H */
