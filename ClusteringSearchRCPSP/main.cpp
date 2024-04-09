#define _CRT_SECURE_NO_WARNINGS

// OBJETIVOS ATUAIS:
//	IMPLEMENTAR GERACAO DE VIZINHOS

// HORARIOS DO ORIENTADOR:
//	TODOS OS DIAS DE MANHA, MENOS SEGUNDA, QUE PODE A PARTIR DAS 10H
//	TODOS OS DIAS DE TARDE, MENOS QUARTA E SEXTA

#include <iostream>
#include <stdlib.h>
#include <memory>
#include <time.h>
#include <math.h>
#include "Solucao.h"

// OPÇÕES:
//#define EXIBIR_MELHOR
#define MODO_DBGHEU
//#define MODO_OPERARACAO

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
			if (matrizTempoRecurso[tempo][nRecurso] > recursosDisponiveis[nRecurso]) {
				return 0;
			}
		}
	}
	// VARRE O VETOR DE TAREFAS
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++)
	{
		if (solucao.tempoTarefa[nTarefa] + tarefas[nTarefa].duration > tempoHorizonte) { // TEMPO_FINAL_TAREFA > HORIZONTE
			return 0;
		}
		for (int nSucessor = 0; nSucessor < tarefas[nTarefa].numProximos; nSucessor++)
		{
			int proximoSucessor = tarefas[nTarefa].vetProximos[nSucessor];
			if (proximoSucessor = numTarefas) continue; // DESCARTA A ULTIMA TAREFA (sink)
			if (solucao.tempoTarefa[proximoSucessor] <= (solucao.tempoTarefa[nTarefa] + tarefas[nTarefa].duration)) { // TEMPO_FINAL_TAREFA >= TEMPO_INICIAL_SUCESSOR ? INVIAVEL : VIAVEL.
				return 0;
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

void calcularEstruturasAuxiliares(Solucao& solucao) {
	// INICIALIZA AS MATRIZES
	memset(matrizPosicaoIntervaloAntecessorSucessorTarefa, 0, sizeof(matrizPosicaoIntervaloAntecessorSucessorTarefa));
	matrizPosicaoIntervaloAntecessorSucessorTarefa[0][0] = 1;
	matrizPosicaoIntervaloAntecessorSucessorTarefa[0][1] = 1;
	// PULA A PRIMEIRA TAREFA (INICIO 0, DURAÇÃO 0, INTERVALO 0)
	for (int indiceTarefa = 1; indiceTarefa < numTarefas; indiceTarefa++)
	{
		int minInicioProx = tempoHorizonte - 1, maxFinalAnt = 0, minPosProx = numTarefas, maxPosAnt = 1;
		// CALCULA UPPER BOUND, OU SEJA, MENOR TEMPO DE INICIO DENTRE OS SUCESSORES
		for (int indiceSucessor = 0; indiceSucessor < numTarefas; indiceSucessor++)
		{
			if (matrizIndiceSucessorAntecessor[indiceTarefa][indiceSucessor] == 1 && minInicioProx > matrizTarefasPosicaoInicialFinal[indiceSucessor][0]) {
				minInicioProx = matrizTarefasPosicaoInicialFinal[indiceSucessor][0];
				minPosProx = calcularPosicao(solucao, indiceSucessor + 1);
			}
		}
		// CALCULA LOWER BOUND, OU SEJA, MAIOR TEMPO DE FIM DENTRE OS ANTECESSORES | TODO => REFATORAR
		for (int indiceAntecessor = 0; indiceAntecessor < numTarefas; indiceAntecessor++)
		{
			if (matrizIndiceSucessorAntecessor[indiceAntecessor][indiceTarefa] == 1 && maxFinalAnt < matrizTarefasPosicaoInicialFinal[indiceAntecessor][1]) {
				maxFinalAnt = matrizTarefasPosicaoInicialFinal[indiceAntecessor][1];
				maxPosAnt = calcularPosicao(solucao, indiceAntecessor + 1);
			}
		}
		matrizPosicaoIntervaloAntecessorSucessorTarefa[indiceTarefa][1] = minPosProx - 1;
		matrizPosicaoIntervaloAntecessorSucessorTarefa[indiceTarefa][0] = maxPosAnt + 1;
	}
	// ATUALIZA A POSIÇÃO FINAL DA ULTIMA TAREFA
	matrizPosicaoIntervaloAntecessorSucessorTarefa[numTarefas - 1][1] = matrizPosicaoIntervaloAntecessorSucessorTarefa[numTarefas - 1][0];
}

void reorganizarTempos(Solucao& solucao, int inicio) {
	// INICIALIZA A MATRIZ TEMPOxRECURSO
	memset(matrizTempoRecurso, 0, sizeof(matrizTempoRecurso));
	// AJUSTA OS HORARIOS, SEGUINDO ORDEM DA SOLUCAO (IGNORANDO A PRIMEIRA E A ULTIMA TAREFA => 1 ATÉ N-1)
	for (int indiceOrdem = 1; indiceOrdem < (numTarefas - 1); indiceOrdem++) // TODO => APLICAR indiceOrdem = inicio
	{
		int nTarefa = solucao.ordemTarefa[indiceOrdem];
		// DEFINE O MAIOR HORARIO DE FIM DOS PREDECESSORES
		int tempoInicial = 0;
		for (int indiceAntecessor = 0; indiceAntecessor < numTarefas; indiceAntecessor++)
		{
			if(matrizIndiceSucessorAntecessor[indiceAntecessor][nTarefa - 1] == 1 && matrizTarefasPosicaoInicialFinal[indiceAntecessor][1] > tempoInicial) {
				tempoInicial = matrizTarefasPosicaoInicialFinal[indiceAntecessor][1];
			}
		}
		// DEFINE O TEMPO QUE POSSUI RECURSO DISPONÍVEL
		int recDisponivel = 0;
		while (recDisponivel == 0) {
			recDisponivel = 1;
			for (int indiceRecurso = 0; indiceRecurso < numRecursos; indiceRecurso++)
			{
				if (tarefas[nTarefa - 1].vetRecursos[indiceRecurso] == 0) {
					continue;
				}
				else {
					int dentroLimiteRecurso = 1;
					for (int tempoRecurso = tempoInicial; tempoRecurso < (tempoInicial + tarefas[nTarefa - 1].duration); tempoRecurso++)
					{
						if (matrizTempoRecurso[tempoRecurso][indiceRecurso] + tarefas[nTarefa - 1].vetRecursos[indiceRecurso] > recursosDisponiveis[indiceRecurso]) {
							dentroLimiteRecurso = 0;
							break;
						}
					}
					if (dentroLimiteRecurso == 0) {
						tempoInicial = tempoInicial + 1;
						recDisponivel = 0;
						break;
					}
				}
			}
		}
		// DEFINE OS NOVOS TEMPOS DE INICIO E FIM DA TAREFA
		solucao.tempoTarefa[nTarefa - 1] = tempoInicial;
		matrizTarefasPosicaoInicialFinal[nTarefa - 1][0] = solucao.tempoTarefa[nTarefa - 1];
		matrizTarefasPosicaoInicialFinal[nTarefa - 1][1] = solucao.tempoTarefa[nTarefa - 1] + tarefas[nTarefa - 1].duration;
		// ATUALIZA A MATRIZ DE TEMPOxRECURSO
		for (int indiceRecurso = 0; indiceRecurso < numRecursos; indiceRecurso++)
		{
			if (tarefas[nTarefa - 1].vetRecursos[indiceRecurso] != 0) {
				for (int tempo = matrizTarefasPosicaoInicialFinal[nTarefa - 1][0]; tempo < matrizTarefasPosicaoInicialFinal[nTarefa - 1][1]; tempo++)
				{
					matrizTempoRecurso[tempo][indiceRecurso] += tarefas[nTarefa - 1].vetRecursos[indiceRecurso];
				}
			}
		}
	}
	// REDEFINE O HORARIO DE INICIO/FIM DA ULTIMA TAREFA
	int maiorTempoFim = 0;
	for (int indiceTarefa = 0; indiceTarefa < (numTarefas - 1); indiceTarefa++)
	{
		if (maiorTempoFim < matrizTarefasPosicaoInicialFinal[indiceTarefa][1]) {
			maiorTempoFim = matrizTarefasPosicaoInicialFinal[indiceTarefa][1];
		}
	}
	if (numTarefas - 1 >= 0) {
		matrizTarefasPosicaoInicialFinal[numTarefas - 1][0] = maiorTempoFim;
		matrizTarefasPosicaoInicialFinal[numTarefas - 1][1] = maiorTempoFim;
		solucao.tempoTarefa[numTarefas - 1] = maiorTempoFim;
	}
	else {
		std::cout << "BUFFER OVERRUN: numTarefas - 1 = " << numTarefas - 1 << std::endl;
	}
	calcularOrdem(solucao);
	calcularEstruturasAuxiliares(solucao);
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

int calcularNumeroAntecessoresIndice(int indiceTarefaAlvo) {
	int resultado = 0;
	for (int indiceTarefaAtual = 0; indiceTarefaAtual < numTarefas; indiceTarefaAtual++)
	{
		if (matrizIndiceSucessorAntecessor[indiceTarefaAtual][indiceTarefaAlvo] == 1) resultado++;
	}
	return resultado;
}

void heuristicaConGul(Solucao& solucao) {
	// INICIALIZA AS VARIAVEIS
	solucao.makespan = 0;
	solucao.ResultFO = 0;
	memset(solucao.tempoTarefa, -1, sizeof(solucao.tempoTarefa)); // TODO => REVISAR NECESSIDADE
	memset(solucao.ordemTarefa, -1, sizeof(solucao.ordemTarefa));
	solucao.tempoTarefa[0] = 0; // TODO => REVISAR NECESSIDADE
	// ALOCA PRIMEIRA E ULTIMA TAREFAS
	solucao.ordemTarefa[0] = 1;
	solucao.ordemTarefa[numTarefas - 1] = numTarefas;
	calcularMatrizBinariaAntecessoresSucessores();
	int numeroTarefasAntecessorasNaoAlocadas[NUM_JOBS];
	numeroTarefasAntecessorasNaoAlocadas[0] = -1;
	numeroTarefasAntecessorasNaoAlocadas[numTarefas - 1] = -1;
	for (int indiceTarefa = 1; indiceTarefa < numTarefas; indiceTarefa++)
	{
		numeroTarefasAntecessorasNaoAlocadas[indiceTarefa] = calcularNumeroAntecessoresIndice(indiceTarefa);
	}
	matrizTarefasPosicaoInicialFinal[0][0] = 0;
	matrizTarefasPosicaoInicialFinal[0][1] = 0;
	for (int indiceOrdemTarefa = 1; indiceOrdemTarefa < numTarefas - 1; indiceOrdemTarefa++)
	{
		for (int indiceTarefaAtual = 1; indiceTarefaAtual < numTarefas - 1; indiceTarefaAtual++)
		{
			// SE A TAREFA JA FOI ALOCADA, PASSA PARA A PROXIMA.
			if (numeroTarefasAntecessorasNaoAlocadas[indiceTarefaAtual] == -1) {
				continue;
			}
			int todosAntecessoresAlocados = 1;
			for (int indiceAntecessor = 0; indiceAntecessor < numTarefas; indiceAntecessor++)
			{
				if (matrizIndiceSucessorAntecessor[indiceAntecessor][indiceTarefaAtual] == 1) {
					if (numeroTarefasAntecessorasNaoAlocadas[indiceAntecessor] != -1) {
						todosAntecessoresAlocados = 0;
						break;
					}
				}
			}
			// SE TODOS OS ANTECESSORES FORAM ALOCADOS, ALOCA A TAREFA.
			if (todosAntecessoresAlocados == 1) {
				solucao.ordemTarefa[indiceOrdemTarefa] = tarefas[indiceTarefaAtual].numJob;
				numeroTarefasAntecessorasNaoAlocadas[indiceTarefaAtual] = -1;
				break;
			}
		}
		int indiceTarefa = solucao.ordemTarefa[indiceOrdemTarefa] - 1;
		// POSIÇÃO INICIAL = POSIÇÃO FINAL DA TAREFA ANTERIOR (EM QUESTÃO DE ORDEM)
		matrizTarefasPosicaoInicialFinal[indiceTarefa][0] = matrizTarefasPosicaoInicialFinal[indiceOrdemTarefa - 1][1];
		// POSIÇÃO FINAL = POSIÇÃO INICIAL + DURAÇÃO
		matrizTarefasPosicaoInicialFinal[indiceTarefa][1] = matrizTarefasPosicaoInicialFinal[indiceTarefa][0] + tarefas[indiceTarefa].duration;
		solucao.tempoTarefa[indiceTarefa] = matrizTarefasPosicaoInicialFinal[indiceTarefa][0]; // TODO => REVISAR NECESSIDADE
	}
	// POSIÇÃO INICIAL DA TEREFA FINAL = POSIÇÃO FINAL DA TAREFA ANTERIOR À FINAL
	if (numTarefas - 2 >= 0) {
		matrizTarefasPosicaoInicialFinal[numTarefas - 1][0] = matrizTarefasPosicaoInicialFinal[numTarefas - 2][1];
	}
	else {
		std::cout << "BUFFER OVERRUN: numTarefas - 2 = " << numTarefas - 2 << std::endl;
	}
	// POSIÇÃO FINAL DA TEREFA FINAL = POSIÇÃO INICIAL DA TEREFA FINAL (SEM DURAÇÃO)
	matrizTarefasPosicaoInicialFinal[numTarefas - 1][1] = matrizTarefasPosicaoInicialFinal[numTarefas - 1][0];
	solucao.tempoTarefa[numTarefas - 1] = matrizTarefasPosicaoInicialFinal[numTarefas - 1][0]; // TODO => REVISAR NECESSIDADE
	reorganizarTempos(solucao, 1);
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
	_itoa(solucao.ResultFO, aux, 10);
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
	solucao.ResultFO = 0;
	// CALCULO DE MAKESPAN E FO
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		tempoFinal = solucao.tempoTarefa[nTarefa] + tarefas[nTarefa].duration;
		if (solucao.makespan < tempoFinal) {
			solucao.makespan = tempoFinal;
		}
	}
	solucao.ResultFO = solucao.makespan;
}

void exibirSolucao(Solucao& solucao) {
	std::cout << "FO: " << solucao.ResultFO << std::endl;
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
	posicaoMaximaInferior = matrizPosicaoIntervaloAntecessorSucessorTarefa[indiceAleatorio][0];
	posicaoMinimaSuperior = matrizPosicaoIntervaloAntecessorSucessorTarefa[indiceAleatorio][1];
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
	reorganizarTempos(solucao, menorIndice);
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
				delta = solucaoVizinha.ResultFO - solucaoAtual.ResultFO;
				if (delta < 0) {
					memcpy(&solucaoAtual, &solucaoVizinha, sizeof(solucaoAtual));
					if (solucaoVizinha.ResultFO < solucao.ResultFO) {
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