#include <stdio.h>
#include <stdlib.h>
#include "escalonador_io.h"

static void escalonador(Processo *processos, int numProcessos, Gantt *gantt) {

    int tempo_atual = 0;

    for (int i = 0; i < numProcessos; i++) {

        if (tempo_atual < processos[i].chegada) {
            registrarFaixaGantt(gantt, ID_IDLE, tempo_atual, processos[i].chegada);
            tempo_atual = processos[i].chegada;
        }

        int inicio = tempo_atual;
        int fim = inicio + processos[i].cpu;

        registrarFaixaGantt(gantt, processos[i].id, inicio, fim);

        processos[i].primeira_execucao = inicio;
        processos[i].termino = fim;
        processos[i].retorno = processos[i].termino - processos[i].chegada;
        processos[i].espera = processos[i].retorno - processos[i].cpu;
        processos[i].resposta = processos[i].primeira_execucao - processos[i].chegada;

        tempo_atual = fim;
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
