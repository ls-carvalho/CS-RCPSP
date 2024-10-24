#pragma once

// CONSTANTES DE LIMITES PARA VETORES
#define NUM_JOBS 50
#define NUM_RESOURCES 10
#define HORIZONTE 200

// DADOS DA SOLUCAO
struct sSolucao {
	int tempoTarefa[NUM_JOBS]; // VETOR COM OS TEMPOS INICIAIS, ORDENADO PELO INDICE DA TAREFA | TODO => VERIFICAR NECESSIDADE
	int ordemTarefa[NUM_JOBS]; // VETOR COM AS TAREFAS, ORDENADO PELA ORDEM DE TEMPOS INCIAIS
	int makespan;
	int resultFO;
	// AUXILIARES
	int matrizTempoRecurso[HORIZONTE][NUM_RESOURCES];
	int matrizTempoInicialFinalTarefa[NUM_JOBS][2]; // [0] = INICIAL | [1] = FINAL
	int matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[NUM_JOBS][2]; // [0] = ULTIMO ANTECESSOR | [1] = PRIMEIRO SUCESSOR
	int matrizTarefaUltimoAntecessorPrimeiroSucessorTarefa[NUM_JOBS][2]; // [0] = ULTIMO ANTECESSOR | [1] = PRIMEIRO SUCESSOR
};
typedef struct sSolucao Solucao;

// DADOS DE ENTRADA
struct sTarefa {
	int numJob;
	int numProximos;
	int vetProximos[NUM_JOBS];
	int vetRecursos[NUM_RESOURCES];
	int duration;
};
typedef struct sTarefa Tarefa;

// VARIAVEIS GLOBAIS DO RCPSP
/// IMUTAVEIS
Tarefa tarefas[NUM_JOBS];
int recursosDisponiveis[NUM_RESOURCES];
int numTarefas;
int numRecursos;
int tempoHorizonte;
int matrizIndiceSucessorAntecessor[NUM_JOBS][NUM_JOBS]; // BINÁRIA: [X][TAREFA] = ANTECESSOR | [TAREFA][Y] = SUCESSOR
/// MUTAVEIS
long int contador;

// PROTÓTIPOS
void calcularMatrizesUltimoAntecessorPrimeiroSucessorTarefa(Solucao& solucao);
void calcularFO(Solucao& solucao);
void calcularMatrizBinariaAntecessoresSucessores();
int calcularNumeroAntecessoresNaoAlocadosIndice(int indiceTarefaAlvo);
void calcularOrdem(Solucao& solucao);
int calcularPosicao(Solucao& solucao, int nTarefa);
void clonar(Solucao& solucaoC, Solucao& solucaoV);
void escreverEmArquivo(char* file_name, Solucao solucao);
void exibirSolucao(Solucao& solucao);
void gerarVizinho(Solucao& solucao);
void exibirEstruturas(Solucao& solucao);
void heuristicaConGul(Solucao& solucao);
int isViavel(Solucao& solucao);
void lerArquivo(char* file_name);
void reorganizarTemposHeuristicaConGul(Solucao& solucao);
void reorganizarTemposGeracaoVizinho(Solucao& solucao);
void simAnnealing(const double alfa, const double tempInicial, const double tempCongelamento, const int SAMax,
	Solucao& solucao, const double tempo_max, double& tempoMelhor, double& tempoTotal);