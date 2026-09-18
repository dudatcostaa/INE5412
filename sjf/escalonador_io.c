#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "escalonador_io.h"

/* Retorna 1 se a linha deve ser ignorada (vazia ou comentario com '#'). */
static int ignorarLinha(const char *linha) {
    for (const char *c = linha; *c != '\0'; c++) {
        if (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n') {
            continue;
        }
        return *c == '#';
    }
    return 1;
}

/* Le o valor inteiro depois do '=' em linhas no formato "chave=valor". */
static int lerValorDepoisDoIgual(const char *linha) {
    const char *pos = strchr(linha, '=');

    if (pos == NULL) {
        fprintf(stderr, "Erro no formato da linha: %s\n", linha);
        exit(1);
    }

    return atoi(pos + 1);
}

/* Remove '\n'/'\r' do final da linha lida com fgets. */
static void removerQuebraDeLinha(char *linha) {
    size_t len = strlen(linha);
    while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r')) {
        linha[--len] = '\0';
    }
}

/* Formata um id de processo (ou ID_IDLE) no formato usado na saida:
 * "P<numero>" ou "IDLE".
 */
static void formatarId(char *destino, size_t tamanho, int id) {
    if (id == ID_IDLE) {
        snprintf(destino, tamanho, "IDLE");
    } else {
        snprintf(destino, tamanho, "P%d", id);
    }
}

void lerArquivoEntrada(const char *nomeArquivo, Processo **processos, int *numProcessos) {
    FILE *arquivo = fopen(nomeArquivo, "r");

    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo: %s\n", nomeArquivo);
        exit(1);
    }

    char linha[MAX_LINHA_LEN];
    int n = 0;

    /* Primeira passada: descobre `n` para poder alocar o vetor de
     * processos com o tamanho exato informado no arquivo. */
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        removerQuebraDeLinha(linha);

        if (ignorarLinha(linha)) {
            continue;
        }

        if (strncmp(linha, "n=", 2) == 0) {
            n = lerValorDepoisDoIgual(linha);
        }
    }

    if (n <= 0) {
        fprintf(stderr, "Erro: valor de n invalido ou nao informado.\n");
        fclose(arquivo);
        exit(1);
    }

    Processo *vetor = malloc((size_t)n * sizeof(Processo));
    if (vetor == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memoria.\n");
        fclose(arquivo);
        exit(1);
    }

    /* Segunda passada: agora le os processos propriamente ditos. */
    rewind(arquivo);
    int total = 0;

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        removerQuebraDeLinha(linha);

        if (ignorarLinha(linha) || strncmp(linha, "n=", 2) == 0) {
            continue;
        }

        if (total == n) {
            fprintf(stderr, "Erro: numero de processos maior que n.\n");
            fprintf(stderr, "Esperado: %d\n", n);
            free(vetor);
            fclose(arquivo);
            exit(1);
        }

        Processo p;
        memset(&p, 0, sizeof(p));
        p.termino = -1;
        p.primeira_execucao = -1;
        p.retorno = -1;
        p.espera = -1;
        p.resposta = -1;

        /* O id no arquivo vem no formato "P<numero>" (ex.: "P1");
         * "%*[^0-9]" descarta o prefixo nao numerico antes de ler o
         * numero propriamente dito. */
        if (sscanf(linha, "%*[^0-9]%d %d %d", &p.id, &p.chegada, &p.cpu) != 3) {
            fprintf(stderr, "Erro ao ler linha: %s\n", linha);
            free(vetor);
            fclose(arquivo);
            exit(1);
        }

        vetor[total++] = p;
    }

    fclose(arquivo);

    if (total != n) {
        fprintf(stderr, "Erro: numero de processos diferente de n.\n");
        fprintf(stderr, "Esperado: %d | Lido: %d\n", n, total);
        free(vetor);
        exit(1);
    }

    *processos = vetor;
    *numProcessos = total;
}

void escreverSaida(const char *nomeArquivo, const Gantt *gantt,
                    const Processo *processos, int numProcessos,
                    double mediaRetorno, double mediaEspera, double mediaResposta) {
    FILE *out = fopen(nomeArquivo, "w");

    if (out == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida.\n");
        exit(1);
    }

    fprintf(out, "GANTT:\n");

    char idFormatado[16];

    for (int i = 0; i < gantt->quantidade; i++) {
        formatarId(idFormatado, sizeof(idFormatado), gantt->faixas[i].id);
        fprintf(out, "%d-%d %s\n", gantt->faixas[i].inicio, gantt->faixas[i].fim, idFormatado);
    }

    fprintf(out, "\nRESULTADOS:\n");
    fprintf(out, "ID CHEGADA CPU TERMINO RETORNO ESPERA RESPOSTA\n");

    for (int i = 0; i < numProcessos; i++) {
        const Processo *p = &processos[i];
        formatarId(idFormatado, sizeof(idFormatado), p->id);
        fprintf(out, "%s %d %d %d %d %d %d\n",
                idFormatado, p->chegada, p->cpu, p->termino, p->retorno, p->espera, p->resposta);
    }

    fprintf(out, "\nMEDIAS:\n");
    fprintf(out, "RETORNO=%.2f\n", mediaRetorno);
    fprintf(out, "ESPERA=%.2f\n", mediaEspera);
    fprintf(out, "RESPOSTA=%.2f\n", mediaResposta);

    fclose(out);
}

void inicializarGantt(Gantt *gantt) {
    gantt->faixas = NULL;
    gantt->quantidade = 0;
    gantt->capacidade = 0;
}

void registrarFaixaGantt(Gantt *gantt, int id, int inicio, int fim) {
    if (gantt->quantidade == gantt->capacidade) {
        int novaCapacidade = (gantt->capacidade == 0) ? 8 : (gantt->capacidade * 2);
        FaixaGantt *novoVetor = realloc(gantt->faixas, (size_t)novaCapacidade * sizeof(FaixaGantt));
        if (novoVetor == NULL) {
            fprintf(stderr, "Erro: falha ao realocar memoria do gantt.\n");
            free(gantt->faixas);
            exit(1);
        }
        gantt->faixas = novoVetor;
        gantt->capacidade = novaCapacidade;
    }

    FaixaGantt *faixa = &gantt->faixas[gantt->quantidade];
    faixa->id = id;
    faixa->inicio = inicio;
    faixa->fim = fim;

    gantt->quantidade++;
}

void liberarGantt(Gantt *gantt) {
    free(gantt->faixas);
    gantt->faixas = NULL;
    gantt->quantidade = 0;
    gantt->capacidade = 0;
}
