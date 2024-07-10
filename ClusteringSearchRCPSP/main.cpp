#define _CRT_SECURE_NO_WARNINGS

// OBJETIVOS ATUAIS:
//	1 => IMPLEMENTAR/REFATORAR A REORGANIZAÇÃO DE TEMPOS PARA GERACAO DE HEURISTICA CONSTRUTIVA
//	2 => IMPLEMENTAR GERACAO DE VIZINHOS
//	3 => IMPLEMENTAR/REFATORAR A REORGANIZAÇÃO DE TEMPOS PARA GERACAO DE VIZINHOS
// OBJETIVOS ADICIONAIS:
//	1 => TENTAR APLICAR indiceOrdem = inicio EM reorganizarTempos
//	2 => CRIAR METODO PARA ALTERAR TODAS AS ESTRUTURAS SIMULTANEAMENTE
//	3 => APLICAR BOOLEAN

#include <iostream>
#include <stdlib.h>
#include <memory>
#include <time.h>
#include <math.h>
#include "Solucao.h"

// DEFINE BOOLEAN
#define TRUE 1
#define FALSE 0

// OPÇÕES:
//#define EXIBIR_MELHOR
//#define MODO_DBGHEU
#define MODO_OPERARACAO

// PARAMETROS:
#define SA_MAX 3

int main()
{
	Solucao solucao, solucao1;
	char file[25] = "j10";
	char escrita[25] = "j10\0";
	lerArquivo(file);
#ifdef MODO_DBGHEU
	heuristicaConGul(solucao);
	calcularFO(solucao);
#endif
#ifdef MODO_OPERARACAO
	double const alfa = 0.995, tempInicial = 100, tempCongelamento = 0.001, tempoMax = 300;
	double tempoMelhor, tempoTotal;
	int SAMax = SA_MAX;
	srand(time(0));
	contador = 0;
	simAnnealing(alfa, tempInicial, tempCongelamento, SAMax * (numTarefas * numRecursos), solucao, tempoMax, tempoMelhor, tempoTotal);
	std::cout << contador << ", " << INT_MAX << std::endl;
	std::cout << "tempo_melhor: " << tempoMelhor << std::endl;
	std::cout << "tempo_total: " << tempoTotal << std::endl;
#endif
	exibirSolucao(solucao);
	escreverEmArquivo(escrita, solucao);
	int aux = isViavel(solucao);
	std::cout << "Solucao Viavel: " << aux << std::endl;
	system("pause");
}

int isViavel(Solucao& solucao) {
	// VERIFICA SE RESPEITA RECURSOS EM CADA TEMPO
	for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
	{
		for (int tempo = 0; tempo < tempoHorizonte; tempo++)
		{
			if (solucao.matrizTempoRecurso[tempo][nRecurso] > recursosDisponiveis[nRecurso]) {
				return 0;
			}
		}
	}
	// VARRE O VETOR DE TAREFAS
	for (int indiceTarefa = 0; indiceTarefa < numTarefas; indiceTarefa++)
	{
		// TEMPO_FINAL_TAREFA > HORIZONTE
		if (solucao.matrizTempoInicialFinalTarefa[indiceTarefa][1] > tempoHorizonte) { 
			return 0;
		}
		for (int indiceSucessor = 1; indiceSucessor < numTarefas; indiceSucessor++)
		{
			if (matrizIndiceSucessorAntecessor[indiceTarefa][indiceSucessor] == 1) {
				// TEMPO_INICIAL_SUCESSOR <= TEMPO_FINAL_TAREFA ? INVIAVEL : VIAVEL.
				if (solucao.matrizTempoInicialFinalTarefa[indiceSucessor][0] < solucao.matrizTempoInicialFinalTarefa[indiceTarefa][1]) {
					return 0;
				}
			}
		}
	}
	return 1;
}

int calcularPosicao(Solucao& solucao, int nTarefa) {
	int posicao = -1;
	for (int indiceOrdem = 0; indiceOrdem < numTarefas; indiceOrdem++)
	{
		if (solucao.ordemTarefa[indiceOrdem] == nTarefa) {
			posicao = indiceOrdem + 1;
			break;
		}
	}
	return posicao;
}

void calcularMatrizTempoUltimoAntecessorPrimeiroSucessorTarefa(Solucao& solucao) {
	// INICIALIZA AS MATRIZES
	memset(solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa, 0, sizeof(solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa));
	solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[0][0] = 0;
	solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[0][1] = 0;
	// PULA A PRIMEIRA TAREFA (INICIO 0, DURAÇÃO 0, INTERVALO 0)
	for (int indiceTarefa = 1; indiceTarefa < numTarefas; indiceTarefa++)
	{
		int menorTempoInicioSucessor = tempoHorizonte, maiorTempoFinalAntecessor = 0;
		// CALCULA UPPER BOUND, OU SEJA, MENOR TEMPO DE INICIO DENTRE OS SUCESSORES
		for (int indiceSucessor = 0; indiceSucessor < numTarefas; indiceSucessor++)
		{
			if (matrizIndiceSucessorAntecessor[indiceTarefa][indiceSucessor] == 1 && menorTempoInicioSucessor > solucao.matrizTempoInicialFinalTarefa[indiceSucessor][0]) {
				menorTempoInicioSucessor = solucao.matrizTempoInicialFinalTarefa[indiceSucessor][0];
			}
		}
		// CALCULA LOWER BOUND, OU SEJA, MAIOR TEMPO DE FIM DENTRE OS ANTECESSORES
		for (int indiceAntecessor = 0; indiceAntecessor < numTarefas; indiceAntecessor++)
		{
			if (matrizIndiceSucessorAntecessor[indiceAntecessor][indiceTarefa] == 1 && maiorTempoFinalAntecessor < solucao.matrizTempoInicialFinalTarefa[indiceAntecessor][1]) {
				maiorTempoFinalAntecessor = solucao.matrizTempoInicialFinalTarefa[indiceAntecessor][1];
			}
		}
		solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[indiceTarefa][0] = maiorTempoFinalAntecessor;
		solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[indiceTarefa][1] = menorTempoInicioSucessor;
	}
	// ATUALIZA A POSIÇÃO FINAL DA ULTIMA TAREFA (OVERRIDE EM TEMPO HORIZONTE)
	if (numTarefas - 1 >= 0) {
		solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[numTarefas - 1][1] = solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[numTarefas - 1][0];
	}
	else {
		std::cout << "BUFFER OVERRUN: numTarefas - 1 = " << numTarefas - 1 << std::endl;
	}
}

void reorganizarTempos(Solucao& solucao) {
	// INICIALIZA A MATRIZ TEMPOxRECURSO
	memset(solucao.matrizTempoRecurso, 0, sizeof(solucao.matrizTempoRecurso));
	// AJUSTA OS HORARIOS, SEGUINDO ORDEM DA SOLUCAO (IGNORANDO A PRIMEIRA E A ULTIMA TAREFA => 1 ATÉ N-1)
	for (int indiceOrdem = 1; indiceOrdem < (numTarefas - 1); indiceOrdem++)
	{
		int nTarefa = solucao.ordemTarefa[indiceOrdem];
		int indiceTarefa = nTarefa - 1;
		// DEFINE O MAIOR HORARIO DE FIM DOS ANTECESSORES
		int tempoInicialDisponivel = solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[indiceTarefa][0];
		// TENTA ENCONTRAR UM TEMPO COM RECURSO DISPONIVEL PARA TODA A DURAÇÃO DA TAREFA
		int hasRecursoDisponivelNoTempoAtual = 0;
		while (hasRecursoDisponivelNoTempoAtual == 0) {
			hasRecursoDisponivelNoTempoAtual = 1;
			for (int indiceRecurso = 0; indiceRecurso < numRecursos; indiceRecurso++)
			{
				if (tarefas[indiceTarefa].vetRecursos[indiceRecurso] == 0) {
					continue;
				}
				else {
					int isDentroLimiteRecurso = 1;
					for (int tempoRecurso = tempoInicialDisponivel; tempoRecurso < (tempoInicialDisponivel + tarefas[indiceTarefa].duration); tempoRecurso++)
					{
						if (solucao.matrizTempoRecurso[tempoRecurso][indiceRecurso] + tarefas[indiceTarefa].vetRecursos[indiceRecurso] > recursosDisponiveis[indiceRecurso]) {
							isDentroLimiteRecurso = 0;
							break;
						}
					}
					if (isDentroLimiteRecurso == 0) {
						tempoInicialDisponivel += 1;
						hasRecursoDisponivelNoTempoAtual = 0;
						break;
					}
				}
			}
		}
		// DEFINE OS NOVOS TEMPOS DE INICIO E FIM DA TAREFA ATUAL
		solucao.tempoTarefa[nTarefa - 1] = tempoInicialDisponivel;
		solucao.matrizTempoInicialFinalTarefa[nTarefa - 1][0] = solucao.tempoTarefa[nTarefa - 1];
		solucao.matrizTempoInicialFinalTarefa[nTarefa - 1][1] = solucao.tempoTarefa[nTarefa - 1] + tarefas[nTarefa - 1].duration;
		calcularMatrizTempoUltimoAntecessorPrimeiroSucessorTarefa(solucao);
		// ATUALIZA A MATRIZ DE TEMPOxRECURSO PARA A TAREFA ATUAL
		for (int indiceRecurso = 0; indiceRecurso < numRecursos; indiceRecurso++)
		{
			if (tarefas[nTarefa - 1].vetRecursos[indiceRecurso] != 0) {
				for (int tempo = solucao.matrizTempoInicialFinalTarefa[nTarefa - 1][0]; tempo < solucao.matrizTempoInicialFinalTarefa[nTarefa - 1][1]; tempo++)
				{
					solucao.matrizTempoRecurso[tempo][indiceRecurso] += tarefas[nTarefa - 1].vetRecursos[indiceRecurso];
				}
			}
		}
	}
	// REDEFINE O HORARIO DE INICIO/FIM DA ULTIMA TAREFA
	int maiorTempoFim = 0;
	for (int indiceTarefa = 0; indiceTarefa < (numTarefas - 1); indiceTarefa++)
	{
		if (maiorTempoFim < solucao.matrizTempoInicialFinalTarefa[indiceTarefa][1]) {
			maiorTempoFim = solucao.matrizTempoInicialFinalTarefa[indiceTarefa][1];
		}
	}
	if (numTarefas - 1 >= 0) {
		solucao.matrizTempoInicialFinalTarefa[numTarefas - 1][0] = maiorTempoFim;
		solucao.matrizTempoInicialFinalTarefa[numTarefas - 1][1] = maiorTempoFim;
		solucao.tempoTarefa[numTarefas - 1] = maiorTempoFim;
	}
	else {
		std::cout << "BUFFER OVERRUN: numTarefas - 1 = " << numTarefas - 1 << std::endl;
	}
	calcularOrdem(solucao);
}

void calcularOrdem(Solucao& solucao) {
	// ORDENA O VETOR DA ORDEM DAS TAREFAS COM BASE NOS VALORES DO VETOR DA ORDEM DOS TEMPOS (BUBBLESORT)
	for (int indiceOrdem = 0; indiceOrdem < numTarefas; indiceOrdem++) {
		for (int indiceOrdemProx = indiceOrdem + 1; indiceOrdemProx < numTarefas; indiceOrdemProx++) {
			int nTarefaAtual = solucao.ordemTarefa[indiceOrdem];
			int nTarefaProxima = solucao.ordemTarefa[indiceOrdemProx];
			if (solucao.tempoTarefa[nTarefaAtual - 1] > solucao.tempoTarefa[nTarefaProxima - 1]) {
				solucao.ordemTarefa[indiceOrdem] = nTarefaProxima;
				solucao.ordemTarefa[indiceOrdemProx] = nTarefaAtual;
			}
		}
	}
}

void calcularMatrizBinariaAntecessoresSucessores() {
	memset(matrizIndiceSucessorAntecessor, 0, sizeof(matrizIndiceSucessorAntecessor));
	for (int indiceTarefa = 0; indiceTarefa < numTarefas; indiceTarefa++)
	{
		for (int indiceSucessor = 0; indiceSucessor < tarefas[indiceTarefa].numProximos; indiceSucessor++)
		{
			int indiceTarefaSucessora = tarefas[indiceTarefa].vetProximos[indiceSucessor] - 1;
			matrizIndiceSucessorAntecessor[indiceTarefa][indiceTarefaSucessora] = 1;
		}
	}
}

int calcularNumeroAntecessoresNaoAlocadosIndice(int indiceTarefaAlvo) {
	int resultado = 0;
	// PULA A PRIMEIRA TAREFA, QUE FOI ALOCADA MANUALMENTE
	for (int indiceTarefaAtual = 1; indiceTarefaAtual < numTarefas; indiceTarefaAtual++)
	{
		if (matrizIndiceSucessorAntecessor[indiceTarefaAtual][indiceTarefaAlvo] == 1) resultado++;
	}
	return resultado;
}

void heuristicaConGul(Solucao& solucao) {
	// INICIALIZA AS VARIAVEIS
	solucao.makespan = 0;
	solucao.resultFO = 0;
	memset(solucao.tempoTarefa, -1, sizeof(solucao.tempoTarefa));
	memset(solucao.ordemTarefa, -1, sizeof(solucao.ordemTarefa));
	// ALOCA PRIMEIRA
	solucao.tempoTarefa[0] = 0;
	solucao.ordemTarefa[0] = 1;
	solucao.ordemTarefa[numTarefas - 1] = numTarefas;
	calcularMatrizBinariaAntecessoresSucessores();
	int numeroTarefasAntecessorasNaoAlocadas[NUM_JOBS];
	numeroTarefasAntecessorasNaoAlocadas[0] = -1;
	for (int indiceTarefa = 1; indiceTarefa < numTarefas; indiceTarefa++)
	{
		numeroTarefasAntecessorasNaoAlocadas[indiceTarefa] = calcularNumeroAntecessoresNaoAlocadosIndice(indiceTarefa);
	}
	solucao.matrizTempoInicialFinalTarefa[0][0] = 0;
	solucao.matrizTempoInicialFinalTarefa[0][1] = 0;
	for (int indiceOrdemTarefa = 1; indiceOrdemTarefa < numTarefas - 1; indiceOrdemTarefa++) // -1?
	{
		for (int indiceTarefaAtual = 1; indiceTarefaAtual < numTarefas - 1; indiceTarefaAtual++) // -1?
		{
			// SE A TAREFA JA FOI ALOCADA, PASSA PARA A PROXIMA.
			if (numeroTarefasAntecessorasNaoAlocadas[indiceTarefaAtual] == -1) {
				continue;
			}
			// SE TODOS OS ANTECESSORES FORAM ALOCADOS, ALOCA A TAREFA.
			if (numeroTarefasAntecessorasNaoAlocadas[indiceTarefaAtual] == 0) {
				solucao.ordemTarefa[indiceOrdemTarefa] = tarefas[indiceTarefaAtual].numJob;
				numeroTarefasAntecessorasNaoAlocadas[indiceTarefaAtual] = -1;
				for (int indiceProximo = 0; indiceProximo < tarefas[indiceTarefaAtual].numProximos; indiceProximo++)
				{
					int indiceSucessor = tarefas[indiceTarefaAtual].vetProximos[indiceProximo] - 1;
					numeroTarefasAntecessorasNaoAlocadas[indiceSucessor] -= 1;
				}
				break;
			}
		}
		int indiceTarefa = solucao.ordemTarefa[indiceOrdemTarefa] - 1;
		// TEMPO INICIAL = TEMPO FINAL DA TAREFA ANTERIOR
		solucao.matrizTempoInicialFinalTarefa[indiceTarefa][0] = solucao.matrizTempoInicialFinalTarefa[indiceOrdemTarefa - 1][1];
		// TEMPO FINAL = TEMPO INICIAL + DURAÇÃO
		solucao.matrizTempoInicialFinalTarefa[indiceTarefa][1] = solucao.matrizTempoInicialFinalTarefa[indiceTarefa][0] + tarefas[indiceTarefa].duration;
		solucao.tempoTarefa[indiceTarefa] = solucao.matrizTempoInicialFinalTarefa[indiceTarefa][0];
	}
	// POSIÇÃO INICIAL DA TEREFA FINAL = POSIÇÃO FINAL DA TAREFA ANTERIOR À FINAL
	if (numTarefas - 2 >= 0) {
		solucao.matrizTempoInicialFinalTarefa[numTarefas - 1][0] = solucao.matrizTempoInicialFinalTarefa[numTarefas - 2][1];
	}
	else {
		std::cout << "BUFFER OVERRUN: numTarefas - 2 = " << numTarefas - 2 << std::endl;
	}
	// POSIÇÃO FINAL DA TEREFA FINAL = POSIÇÃO INICIAL DA TEREFA FINAL (SEM DURAÇÃO)
	solucao.matrizTempoInicialFinalTarefa[numTarefas - 1][1] = solucao.matrizTempoInicialFinalTarefa[numTarefas - 1][0];
	solucao.tempoTarefa[numTarefas - 1] = solucao.matrizTempoInicialFinalTarefa[numTarefas - 1][0];
	calcularMatrizTempoUltimoAntecessorPrimeiroSucessorTarefa(solucao);
	reorganizarTempos(solucao);
}

void lerArquivo(char* file) {
	FILE* fp;
	char file_name[25];
	int iterador_nome = 0;
	while (iterador_nome < strlen(file)) {
		file_name[iterador_nome] = file[iterador_nome];
		iterador_nome++;
	}
	file_name[iterador_nome] = '.';
	file_name[iterador_nome + 1] = 's';
	file_name[iterador_nome + 2] = 'm';
	file_name[iterador_nome + 3] = '\0';
	fp = fopen(file_name, "r"); // MODO LEITURA
	if (fp == NULL)
	{
		perror("Erro ao abrir o arquivo.\n");
		exit(EXIT_FAILURE);
	}

	while (!feof(fp)) {
		char linha[100];
		fgets(linha, 100, fp);
		if (strncmp(linha, "jobs (incl. supersource/sink ):  ", strlen("jobs (incl. supersource/sink ):  ")) == 0) {
			// CAPTA O NUMERO DE TAREFAS/JOBS
			char linha_auxiliar[100];
			strcpy(linha_auxiliar, linha);
			int i = strlen("jobs (incl. supersource/sink ):  "), j = 0;
			char variavel_captacao[100];
			while (i < strlen(linha_auxiliar)) {
				variavel_captacao[j] = linha_auxiliar[i];
				i++;
				j++;
			}
			variavel_captacao[j] = '\0';
			numTarefas = atoi(variavel_captacao);
		}
		if (strncmp(linha, "horizon                       :  ", strlen("horizon                       :  ")) == 0) {
			// CAPTA O HORIZONTE
			char linha_auxiliar[100];
			strcpy(linha_auxiliar, linha);
			int i = strlen("horizon                       :  "), j = 0;
			char variavel_captacao[100];
			while (i < strlen(linha_auxiliar)) {
				variavel_captacao[j] = linha_auxiliar[i];
				i++;
				j++;
			}
			variavel_captacao[j] = '\0';
			tempoHorizonte = atoi(variavel_captacao);
		}
		else if (strncmp(linha, "  - renewable                 :  ", strlen("  - renewable                 :  ")) == 0) {
			// CAPTA O NUMERO DE RECURSOS
			char linha_auxiliar[100];
			strcpy(linha_auxiliar, linha);
			int i = strlen("  - renewable                 :  "), j = 0;
			char variavel_captacao[100];
			while (i < strlen(linha_auxiliar)) {
				variavel_captacao[j] = linha_auxiliar[i];
				i++;
				j++;
			}
			variavel_captacao[j] = '\0';
			char* fim_captacao = strchr(variavel_captacao, ' ');
			fim_captacao[0] = '\0';
			numRecursos = atoi(variavel_captacao);
		}
		else if (strncmp(linha, "PRECEDENCE RELATIONS:", strlen("PRECEDENCE RELATIONS:")) == 0) {
			// LER DE "PRECEDENCE RELATIONS:" ATE "************************************************************************", PULANDO UMA LINHA.
			fgets(linha, 100, fp); // PULA A LINHA DO CABECALHO
			fgets(linha, 100, fp);
			while (strncmp(linha, "************************************************************************", strlen("************************************************************************")) != 0) {
				// PARA CADA LINHA
				char* pch;
				pch = strtok(linha, " "); // CAPTA O NUMERO DO JOB
				int numeroTarefa = atoi(pch);
				tarefas[numeroTarefa - 1].numJob = numeroTarefa;
				pch = strtok(NULL, " "); // PULA O MODE
				pch = strtok(NULL, " "); // CAPTA O NUMERO DE SUCESSORES
				int numSucessores = atoi(pch);
				tarefas[numeroTarefa - 1].numProximos = numSucessores;
				int numProximo = 0;
				int numSucessor;
				pch = strtok(NULL, " \n"); // CAPTA O PRIMEIRO SUCESSOR
				while (pch != NULL) {
					numSucessor = atoi(pch);
					tarefas[numeroTarefa - 1].vetProximos[numProximo] = numSucessor;
					numProximo++;
					pch = strtok(NULL, " \n"); // CAPTA O PROXIMO SUCESSOR
				}
				fgets(linha, 100, fp);
			}
		}
		else if (strncmp(linha, "REQUESTS/DURATIONS:", strlen("REQUESTS/DURATIONS:")) == 0) {
			// LER DE "REQUESTS/DURATIONS:" ATE "************************************************************************", PULANDO UMA LINHA.
			fgets(linha, 100, fp); // PULA A LINHA DO CABECALHO
			fgets(linha, 100, fp); // PULA A LINHA SEPARADORA
			fgets(linha, 100, fp);
			while (strncmp(linha, "************************************************************************", strlen("************************************************************************")) != 0) {
				// PARA CADA LINHA
				char* pch;
				pch = strtok(linha, " "); // CAPTA O NUMERO DO JOB
				int numeroTarefa = atoi(pch);
				pch = strtok(NULL, " "); // PULA O MODE
				pch = strtok(NULL, " "); // CAPTA A DURACAO
				int duracao = atoi(pch);
				tarefas[numeroTarefa - 1].duration = duracao;
				int numRecurso = 0;
				int valorRecurso;
				pch = strtok(NULL, " "); // CAPTA O PRIMEIRO RECURSO
				while (pch != NULL) {
					valorRecurso = atoi(pch);
					tarefas[numeroTarefa - 1].vetRecursos[numRecurso] = valorRecurso;
					numRecurso++;
					pch = strtok(NULL, " "); // CAPTA O PROXIMO RECURSO
				}
				fgets(linha, 100, fp);
			}
		}
		else if (strncmp(linha, "RESOURCEAVAILABILITIES:", strlen("RESOURCEAVAILABILITIES:")) == 0) {
			// CAPTA OS RECURSOS DISPONIVEIS
			fgets(linha, 100, fp); // PULA A LINHA DO CABECALHO
			fgets(linha, 100, fp);
			char* pch;
			int numRecurso = 0;
			int valorRecurso;
			pch = strtok(linha, " "); // CAPTA O PRIMEIRO RECURSO
			while (pch != NULL) {
				valorRecurso = atoi(pch);
				recursosDisponiveis[numRecurso] = valorRecurso;
				numRecurso++;
				pch = strtok(NULL, " "); // CAPTA O PROXIMO RECURSO
			}
		}
	}
	fclose(fp);
}

void escreverEmArquivo(char* file_name, Solucao solucao) {
	FILE* fp;
	int iterador_nome = 0;
	char file_name_sol[25], linha[100], aux[100];
	while (iterador_nome < strcspn(file_name, ".")) {
		file_name_sol[iterador_nome] = file_name[iterador_nome];
		iterador_nome++;
	}
	file_name_sol[iterador_nome] = '\0';
	strcat(file_name_sol, ".txt");
	fp = fopen(file_name_sol, "w");
	if (fp == NULL)
	{
		perror("Erro ao abrir o arquivo.\n");
		exit(EXIT_FAILURE);
	}
	// ESCREVENDO A FO
	strcpy(linha, "FO: ");
	_itoa(solucao.resultFO, aux, 10);
	strcat(linha, aux);
	strcat(linha, "\n");
	fputs(linha, fp);
	// ESCREVENDO O MAKESPAN
	strcpy(linha, "Makespan: ");
	_itoa(solucao.makespan, aux, 10);
	strcat(linha, aux);
	strcat(linha, "\n");
	fputs(linha, fp);
	// ESCREVENDO A LINHA SEPARADORA
	strcpy(linha, "------------------");
	strcat(linha, "\n");
	fputs(linha, fp);
	// ESCREVENDO O CABECALHO
	strcpy(linha, "Job Start Time");
	strcat(linha, "\n");
	fputs(linha, fp);
	// ITERANDO AS TAREFAS E 
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		// ESCREVENDO O NUMERO DA TAREFA
		_itoa(nTarefa + 1, aux, 10);
		strcpy(linha, aux);
		strcat(linha, "\t");
		// ESCREVENDO OS TEMPOS INCIAIS
		_itoa(solucao.tempoTarefa[nTarefa], aux, 10);
		strcat(linha, aux);
		strcat(linha, "\n");
		fputs(linha, fp);
	}
	fclose(fp);
}

void calcularFO(Solucao& solucao) {
	int tempoFinal = 0;
	solucao.makespan = 0;
	solucao.resultFO = 0;
	// CALCULO DE MAKESPAN E FO
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		tempoFinal = solucao.tempoTarefa[nTarefa] + tarefas[nTarefa].duration;
		if (solucao.makespan < tempoFinal) {
			solucao.makespan = tempoFinal;
		}
	}
	solucao.resultFO = solucao.makespan;
}

void exibirSolucao(Solucao& solucao) {
	std::cout << "FO: " << solucao.resultFO << std::endl;
	std::cout << "Makespan: " << solucao.makespan << std::endl;
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++)
	{
		std::cout << "Job: " << nTarefa + 1 << " Start Time: " << solucao.tempoTarefa[nTarefa] << std::endl;
	}
}

void clonar(Solucao& solucaoC, Solucao& solucaoV) {
	memcpy(&solucaoC, &solucaoV, sizeof(solucaoV));
}

void gerarVizinho(Solucao& solucao) {
inicioGeracaoVizinho:
	srand(time(0));
	int indiceAleatorio, indiceAlvo, ordemAlvo, tarefasUteis, posicaoMinimaSuperior, posicaoMaximaInferior, distanciaTroca, menorIndice, nTarefa;
	// DESCARTA A TAREFA 0 E N
	tarefasUteis = numTarefas - 2;
	// OBTEM RESULTADO ENTRE 1 E N-2
	indiceAleatorio = rand() % tarefasUteis + 1;
	posicaoMaximaInferior = solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[indiceAleatorio][0];
	posicaoMinimaSuperior = solucao.matrizTempoUltimoAntecessorPrimeiroSucessorTarefa[indiceAleatorio][1];
	//if (posicaoMaximaInferior == 1) posicaoMaximaInferior++;
	//if (posicaoMinimaSuperior == numTarefas - 1) posicaoMinimaSuperior--;
	// distanciaTroca = posicaoMinimaSuperior - posicaoMaximaInferior + 1;
	distanciaTroca = posicaoMinimaSuperior - posicaoMaximaInferior;
	if (distanciaTroca <= 0) {
		goto inicioGeracaoVizinho;
	}
	// OBTEM UM RESULTADO ENTRE O MAXIMO INFERIOR ATE O MINIMO SUPERIOR
	ordemAlvo = rand() % distanciaTroca + posicaoMaximaInferior;
	indiceAlvo = ordemAlvo - 1;
	if (indiceAleatorio == indiceAlvo) {
		goto inicioGeracaoVizinho;
	}
	// ALTERA A ORDEM DAS TAREFAS
	nTarefa = solucao.ordemTarefa[indiceAleatorio];
	solucao.ordemTarefa[indiceAleatorio] = solucao.ordemTarefa[indiceAlvo];
	solucao.ordemTarefa[indiceAlvo] = nTarefa;
	// REORGANIZA OS TEMPOS
	if (indiceAleatorio < indiceAlvo) {
		menorIndice = indiceAleatorio;
	}
	else {
		menorIndice = indiceAlvo;
	}
	reorganizarTempos(solucao);
	calcularFO(solucao);
}

void simAnnealing(const double alfa, const double tempInicial, const double tempCongelamento, const int SAMax,
	Solucao& solucao, const double tempoMax, double& tempoMelhor, double& tempoTotal) {

	clock_t clockInicial, clockFinal;
	Solucao solucaoAtual, solucaoVizinha;
	double temperatura, delta, aux;

	clockInicial = clock();
	heuristicaConGul(solucao);
	calcularFO(solucao);
	exibirSolucao(solucao);
	clockFinal = clock();
	tempoMelhor = ((double)(clockFinal - clockInicial)) / CLOCKS_PER_SEC;
	tempoTotal = tempoMelhor;

	memcpy(&solucaoAtual, &solucao, sizeof(solucaoAtual));
	while (tempoTotal < tempoMax) {
		temperatura = tempInicial;
		while (temperatura > tempCongelamento) {
			for (int i = 0; i < SAMax; i++) {
				memcpy(&solucaoVizinha, &solucaoAtual, sizeof(solucaoVizinha));
				gerarVizinho(solucaoVizinha);
				contador++;
				delta = solucaoVizinha.resultFO - solucaoAtual.resultFO;
				if (delta < 0) {
					memcpy(&solucaoAtual, &solucaoVizinha, sizeof(solucaoAtual));
					if (solucaoVizinha.resultFO < solucao.resultFO) {
						memcpy(&solucao, &solucaoVizinha, sizeof(solucao));
						clockFinal = clock();
						tempoMelhor = ((double)(clockFinal - clockInicial)) / CLOCKS_PER_SEC;
					}
				}
				else {
					aux = rand() % 1001;
					aux /= 1000;
					if (aux < exp(-delta / temperatura)) {
						memcpy(&solucaoAtual, &solucaoVizinha, sizeof(solucaoAtual));
					}
				}
			}
			temperatura *= alfa;
			clockFinal = clock();
			tempoTotal = ((double)(clockFinal - clockInicial)) / CLOCKS_PER_SEC;
			if (tempoTotal >= tempoMax) {
				break;
			}
		}
	}
}