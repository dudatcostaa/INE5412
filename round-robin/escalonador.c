#include <stdio.h>
#include <stdlib.h>
#include "escalonador_io.h"

// Defina um valor para o quantum
#define QUANTUM 2

static void escalonador(Processo *processos, int numProcessos, Gantt *gantt) {

    int tempo_atual = 0;
    int concluidos = 0;
    int prox_idx = 0;
    int restante[numProcessos];
    int fila[2000];
    int inicio = 0;
    int fim = 0;

    for (int i = 0; i < numProcessos; i++) {
        restante[i] = processos[i].cpu;
    }

    while (concluidos < numProcessos)
    {
        if(inicio == fim){
            if(tempo_atual < processos[prox_idx].chegada){
                registrarFaixaGantt(gantt, ID_IDLE, tempo_atual, processos[prox_idx].chegada);
                tempo_atual = processos[prox_idx].chegada;
            }
            while (prox_idx < numProcessos && processos[prox_idx].chegada <= tempo_atual)
            {
                fila[fim++] = prox_idx++;
            }
        }

        int p = fila[inicio];
        inicio++;

        if (processos[p].primeira_execucao == -1) {
            processos[p].primeira_execucao = tempo_atual;
        }

        int fatia = (restante[p] > QUANTUM) ? QUANTUM : restante[p];

        registrarFaixaGantt(gantt, processos[p].id, tempo_atual, tempo_atual + fatia);
        tempo_atual += fatia;
        restante[p] -= fatia;

        while (prox_idx < numProcessos && processos[prox_idx].chegada <= tempo_atual)
        {
            fila[fim++] = prox_idx++;
        }

        if (restante[p] > 0) {
            fila[fim++] = p;
        } else {
            processos[p].termino = tempo_atual;
            processos[p].retorno = tempo_atual - processos[p].chegada;
            processos[p].espera = processos[p].retorno - processos[p].cpu;
            processos[p].resposta = processos[p].primeira_execucao - processos[p].chegada;
            concluidos++;
        }
    }
    
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso correto: %s <arquivo de entrada> <arquivo de saida>\n", argv[0]);
        return 1;
    }

    const char *arquivoEntrada = argv[1];
    const char *arquivoSaida = argv[2];

    Processo *processos = NULL;
    int numProcessos = 0;

    lerArquivoEntrada(arquivoEntrada, &processos, &numProcessos);

    Gantt gantt;
    inicializarGantt(&gantt);

    escalonador(processos, numProcessos, &gantt);

    double somaRetorno = 0.0;
    double somaEspera = 0.0;
    double somaResposta = 0.0;

    for (int i = 0; i < numProcessos; i++) {
        somaRetorno += processos[i].retorno;
        somaEspera += processos[i].espera;
        somaResposta += processos[i].resposta;
    }

    double mediaRetorno = numProcessos > 0 ? somaRetorno / numProcessos : 0.0;
    double mediaEspera = numProcessos > 0 ? somaEspera / numProcessos : 0.0;
    double mediaResposta = numProcessos > 0 ? somaResposta / numProcessos : 0.0;

    escreverSaida(arquivoSaida, &gantt, processos, numProcessos,
                  mediaRetorno, mediaEspera, mediaResposta);

    liberarGantt(&gantt);
    free(processos);

    return 0;
}
