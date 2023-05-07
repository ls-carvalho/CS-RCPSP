﻿#pragma once

// TODO => construtiva gulosa => ANALISAR CODIGO PYTHON

// CONSTANTES DE LIMITE PARA VETOR => TODO: AJUSTAR ESSES VALORES PARA OS MAIORES CASOS
#define NUM_JOBS 12
#define NUM_RESOURCES 2
#define HORIZONTE 35

// DADOS DA SOLUCAO
struct sSolucao {
	int ordemTempo[NUM_JOBS]; // VETOR COM OS TEMPOS INICIAIS, ORDENADO PELO INDICE DA TAREFA
	int ordemTarefa[NUM_JOBS]; // VETOR COM AS TAREFAS, ORDENADO PELA ORDEM DE TEMPOS INCIAIS
	int makespan;
	int ResultFO;
};
typedef struct sSolucao Solucao;

// DADOS DE ENTRADA
struct sTarefa {
	int numJob;
	int numProximos;
	int vetProximos[NUM_JOBS];
	int numAnteriores;
	int vetAnteriores[NUM_JOBS];
	int vetRecursos[NUM_RESOURCES];
	int duration;
};
typedef struct sTarefa Tarefa;

// VARIAVEIS GLOBAIS DO RCPSP
Tarefa tarefas[NUM_JOBS];
int recursosDisponiveis[NUM_RESOURCES];
int numTarefas;
int numRecursos;
int tempoHorizonte;
long int contador;
int vetorRecursosTempo[HORIZONTE][NUM_RESOURCES];
int posicaoInicialFinalTarefas[NUM_JOBS][2]; // 0 = INICIAL / 1 = FINAL
int matrizAntecessorSucessor[NUM_JOBS][NUM_JOBS]; // TODO => APLICAR. 0 = NULL / 1 = ANTECESSOR / 2 = SUCESSOR

// PROTOTIPOS
void calcularOrdem(Solucao& solucao);
void calcularAntecessores();
void heuristicaConGul(Solucao& solucao);
void lerArquivo(char* file_name);
void lerSolucao(char* file_name, Solucao& solucao);
void escreverEmArquivo(char* file_name, Solucao solucao);
void calculoFO(Solucao& solucao);
int isViavel(Solucao solucao);
void exibirSolucao(Solucao& solucao);
void clonar(Solucao& solucaoC, Solucao& solucaoV);
void gerarVizinho(Solucao& solucao);
void simAnnealing(const double alfa, const double tempInicial, const double tempCongelamento, const int SAMax,
	Solucao& solucao, const double tempo_max, double& tempoMelhor, double& tempoTotal);