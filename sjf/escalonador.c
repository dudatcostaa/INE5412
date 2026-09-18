#include <stdio.h>
#include <stdlib.h>

#include "escalonador_io.h"

static void escalonador(Processo *processos, int numProcessos, Gantt *gantt) {
    int tempo_atual = 0, concluidos = 0;

    while (concluidos < numProcessos) {
        int melhor = -1, prox_chegada = 1e9;

        // Em uma unica varredura encontra o menor pronto OU a proxima chegada
        for (int i = 0; i < numProcessos; i++) {
            if (processos[i].termino == -1) {
                if (processos[i].chegada <= tempo_atual) {
                    if (melhor == -1 || processos[i].cpu < processos[melhor].cpu) melhor = i;
                } else if (processos[i].chegada < prox_chegada) {
                    prox_chegada = processos[i].chegada;
                }
            }
        }

        // Se ninguem chegou ainda, avanca a CPU com IDLE
        if (melhor == -1) {
            registrarFaixaGantt(gantt, ID_IDLE, tempo_atual, prox_chegada);
            tempo_atual = prox_chegada;
            continue;
        }

        // Executa o job selecionado
        int inicio = tempo_atual;
        tempo_atual += processos[melhor].cpu;

        registrarFaixaGantt(gantt, processos[melhor].id, inicio, tempo_atual);
        processos[melhor].primeira_execucao = inicio;
        processos[melhor].termino = tempo_atual;
        processos[melhor].retorno = tempo_atual - processos[melhor].chegada;
        processos[melhor].espera = processos[melhor].retorno - processos[melhor].cpu;
        processos[melhor].resposta = inicio - processos[melhor].chegada;
        concluidos++;
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
