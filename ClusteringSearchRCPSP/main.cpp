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
#ifdef EXIBIR_MELHOR
	lerSolucao(file, solucao1);
	exibirSolucao(solucao1);
#endif
	lerArquivo(file);
#ifdef MODO_OPERARACAO
	double const alfa = 0.995, tempInicial = 100, tempCongelamento = 0.001, tempoMax = 900;
	double tempoMelhor, tempoTotal;
	int SAMax = SA_MAX;
	srand(time(0));
	contador = 0;
	simAnnealing(alfa, tempInicial, tempCongelamento, SAMax * (numTarefas * numRecursos), solucao, tempoMax, tempoMelhor, tempoTotal);
	std::cout << contador << ", " << INT_MAX << std::endl;
	std::cout << "tempo_melhor: " << tempoMelhor << std::endl;
	std::cout << "tempo_total: " << tempoTotal << std::endl;
#endif
#ifdef MODO_DBGHEU
	heuristicaConGul(solucao);
	calculoFO(solucao);
#endif
	exibirSolucao(solucao);
	escreverEmArquivo(escrita, solucao);
	system("pause");
}

void gerarEstruturasAuxiliares() {
	// INICIALIZA AS MATRIZES COMO 0
	for (int x = 0; x < numTarefas; x++)
	{
		for (int j = 0; j < numTarefas; j++)
		{
			matrizSucessorAntecessor[x][j] = 0;
		}
		matrizRangeAntecessorSucessorTarefa[x][0] = 0;
		matrizRangeAntecessorSucessorTarefa[x][1] = 0;
	}
	for (int indiceTarefa = 0; indiceTarefa < numTarefas; indiceTarefa++)
	{
		int minInicioProx = 0, maxFinalAnt = 0;
		for (int indiceProximo = 0; indiceProximo < tarefas[indiceTarefa].numProximos; indiceProximo++)
		{
			// CONSTROI A MATRIZ AUXILIAR BINARIA DOS SUCESSORES E ANTECESSORES
			int nProximo = tarefas[indiceTarefa].vetProximos[indiceProximo];
			matrizSucessorAntecessor[indiceTarefa][nProximo - 1] = 1;
			// CONSTROI A MATRIZ AUXILIAR COM OS TEMPOS MINIMOS DOS SUCESSORES
			if (minInicioProx < matrizTarefasPosicaoInicialFinal[nProximo - 1][0]) {
				minInicioProx = matrizTarefasPosicaoInicialFinal[nProximo - 1][0];
			}
		}
		for (int indiceAnteriores = 0; indiceAnteriores < tarefas[indiceTarefa].numAnteriores; indiceAnteriores++)
		{
			int nAnterior = tarefas[indiceTarefa].vetAnteriores[indiceAnteriores];
			// CONSTROI A MATRIZ AUXILIAR COM OS TEMPOS MAXIMOS DOS ANTECESSORES
			if (maxFinalAnt < matrizTarefasPosicaoInicialFinal[nAnterior - 1][1]) {
				maxFinalAnt = matrizTarefasPosicaoInicialFinal[nAnterior - 1][1];
			}
		}
		matrizRangeAntecessorSucessorTarefa[indiceTarefa][1] = minInicioProx;
		matrizRangeAntecessorSucessorTarefa[indiceTarefa][0] = maxFinalAnt;
	}
}

// TODO => ADICIONAR PARAMETRO DE MOMENTO DE INICIO DE CALCULO (MENOR INDICE QUE SOFREU ALTERACAO)
void reorganizarTempos(Solucao& solucao, int inicio) {
	// INICIALIZA A MATRIZ TEMPOxRECURSO
	for (int nTempo = 0; nTempo < tempoHorizonte; nTempo++)
	{
		for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
		{
			matrizTempoRecurso[nTempo][nRecurso] = 0;
		}
	}
	// AJUSTA OS HORARIOS, SEGUINDO ORDEM DA SOLUCAO (IGNORANDO A PRIMEIRA E A ULTIMA TAREFA)
	for (int indiceOrdem = 1; indiceOrdem < (numTarefas - 1); indiceOrdem++)
	{
		int nTarefa = solucao.ordemTarefa[indiceOrdem];
		// DEFINE O MAIOR HORARIO DE FIM DOS PREDECESSORES
		int tempoInicial = 0;
		for (int indiceAntecessor = 0; indiceAntecessor < tarefas[nTarefa - 1].numAnteriores; indiceAntecessor++)
		{
			int nTarefaAntecessor = tarefas[nTarefa - 1].vetAnteriores[indiceAntecessor];
			if (matrizTarefasPosicaoInicialFinal[nTarefaAntecessor - 1][1] > tempoInicial) {
				tempoInicial = matrizTarefasPosicaoInicialFinal[nTarefaAntecessor - 1][1];
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
		solucao.ordemTempo[nTarefa - 1] = tempoInicial;
		matrizTarefasPosicaoInicialFinal[nTarefa - 1][0] = solucao.ordemTempo[nTarefa - 1];
		matrizTarefasPosicaoInicialFinal[nTarefa - 1][1] = solucao.ordemTempo[nTarefa - 1] + tarefas[nTarefa - 1].duration;
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
	matrizTarefasPosicaoInicialFinal[numTarefas - 1][0] = maiorTempoFim;
	matrizTarefasPosicaoInicialFinal[numTarefas - 1][1] = maiorTempoFim;
	solucao.ordemTempo[numTarefas - 1] = maiorTempoFim;
	calcularOrdem(solucao, true);
}

void calcularOrdem(Solucao& solucao, bool recalculo) {
	if (recalculo == false) {
		for (int indiceTarefa = 0; indiceTarefa < numTarefas; indiceTarefa++) {
			// INICIALIZA O VETOR DE 0 A N-1
			solucao.ordemTarefa[indiceTarefa] = indiceTarefa;
			// PREENCHE A MATRIZ DE TAREFAxTEMPO[INCIAL||FINAL]
			matrizTarefasPosicaoInicialFinal[indiceTarefa][0] = solucao.ordemTempo[indiceTarefa];
			matrizTarefasPosicaoInicialFinal[indiceTarefa][1] = solucao.ordemTempo[indiceTarefa] + tarefas[indiceTarefa].duration;
		}
	}
	// ORDENA O VETOR DA ORDEM DAS TAREFAS COM BASE NOS VALORES DO VETOR DA ORDEM DOS TEMPOS (BUBBLESORT)
	for (int indiceTarefa = 0; indiceTarefa < numTarefas; indiceTarefa++) {
		for (int proxIndiceTarefa = indiceTarefa + 1; proxIndiceTarefa < numTarefas; proxIndiceTarefa++) {
			int ordemTarefaAtual = solucao.ordemTarefa[indiceTarefa] - 1;
			int ordemTarefaProxima = solucao.ordemTarefa[proxIndiceTarefa] - 1;
			if (solucao.ordemTempo[ordemTarefaAtual] > solucao.ordemTempo[ordemTarefaProxima]) {
				int temp = solucao.ordemTarefa[indiceTarefa];
				solucao.ordemTarefa[indiceTarefa] = solucao.ordemTarefa[proxIndiceTarefa];
				solucao.ordemTarefa[proxIndiceTarefa] = temp;
			}
		}
	}
}

void calcularAntecessores() {
	// INICIALIZA TODOS OS ANTERIORES COMO 0
	for (int indiceTarefa = 0; indiceTarefa < numTarefas; indiceTarefa++)
	{
		tarefas[indiceTarefa].numAnteriores = 0;
	}
	// ATUALIZA O NUMERO DE ANTERIORES DO SUCESSOR E ADICIONA O ANTERIOR DO SUCESSOR NA RESPECTIVA LISTA DO MESMO
	for (int indiceTarefa = 0; indiceTarefa < numTarefas; indiceTarefa++)
	{
		for (int indiceSucessor = 0; indiceSucessor < tarefas[indiceTarefa].numProximos; indiceSucessor++)
		{
			int nTarefaSucessora = tarefas[indiceTarefa].vetProximos[indiceSucessor];
			tarefas[nTarefaSucessora - 1].numAnteriores += 1;
			tarefas[nTarefaSucessora - 1].vetAnteriores[tarefas[nTarefaSucessora - 1].numAnteriores - 1] = tarefas[indiceTarefa].numJob;
		}
	}
}

void heuristicaConGul(Solucao& solucao) {
	// INICIALIZA AS VARIAVEIS
	solucao.makespan = 0;
	solucao.ResultFO = 0;
	memset(solucao.ordemTempo, -1, sizeof(solucao.ordemTempo));
	memset(solucao.ordemTarefa, -1, sizeof(solucao.ordemTarefa));
	solucao.ordemTempo[0] = 0;
	solucao.ordemTarefa[0] = 1;
	solucao.ordemTarefa[numTarefas - 1] = numTarefas;
	calcularAntecessores();
	int antecessoresTarefasNaoAlocadas[NUM_JOBS + NUM_JOBS]; // EVITAR OVERFLOW DE BUFFER
	antecessoresTarefasNaoAlocadas[0] = -1;
	antecessoresTarefasNaoAlocadas[numTarefas] = -1;
	for (int indiceTarefa = 1; indiceTarefa < numTarefas; indiceTarefa++)
	{
		antecessoresTarefasNaoAlocadas[indiceTarefa] = tarefas[indiceTarefa].numAnteriores;
	}
	matrizTarefasPosicaoInicialFinal[0][0] = 0;
	matrizTarefasPosicaoInicialFinal[0][1] = 0;
	for (int ordemTarefa = 1; ordemTarefa < numTarefas - 1; ordemTarefa++)
	{
		for (int indiceTarefaAtual = 1; indiceTarefaAtual < numTarefas - 1; indiceTarefaAtual++)
		{
			// SE A TAREFA JA FOI ALOCADA, PASSA PARA A PROXIMA.
			if (antecessoresTarefasNaoAlocadas[indiceTarefaAtual] == -1){
				continue;
			} 
			int todosAntecessoresAlocados = 1;
			for (int indiceAntecessor = 0; indiceAntecessor < tarefas[indiceTarefaAtual].numAnteriores; indiceAntecessor++)
			{
				if (antecessoresTarefasNaoAlocadas[tarefas[indiceTarefaAtual].vetAnteriores[indiceAntecessor] - 1] != -1) {
					todosAntecessoresAlocados = 0;
					break;
				}
			}
			// SE TODOS OS ANTECESSORES FORAM ALOCADOS, ALOCA A TAREFA.
			if (todosAntecessoresAlocados == 1) { 
				solucao.ordemTarefa[ordemTarefa] = tarefas[indiceTarefaAtual].numJob;
				antecessoresTarefasNaoAlocadas[indiceTarefaAtual] = -1;
				break;
			}
		}
		int indiceTarefa = solucao.ordemTarefa[ordemTarefa] - 1;
		matrizTarefasPosicaoInicialFinal[indiceTarefa][0] = matrizTarefasPosicaoInicialFinal[indiceTarefa - 1][1];
		matrizTarefasPosicaoInicialFinal[indiceTarefa][1] = matrizTarefasPosicaoInicialFinal[indiceTarefa][0] + tarefas[indiceTarefa].duration;
		solucao.ordemTempo[indiceTarefa] = matrizTarefasPosicaoInicialFinal[indiceTarefa][0];
	}
	matrizTarefasPosicaoInicialFinal[numTarefas - 1][0] = matrizTarefasPosicaoInicialFinal[numTarefas - 2][1];
	matrizTarefasPosicaoInicialFinal[numTarefas - 1][1] = matrizTarefasPosicaoInicialFinal[numTarefas - 1][0] + tarefas[numTarefas - 1].duration;
	solucao.ordemTempo[numTarefas - 1] = matrizTarefasPosicaoInicialFinal[numTarefas - 1][0];
	solucao.ordemTarefa[numTarefas - 1] = tarefas[numTarefas - 1].numJob;
	reorganizarTempos(solucao, 0);
	gerarEstruturasAuxiliares();
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

void lerSolucao(char* file, Solucao& solucao) {
	FILE* fp;
	char file_name[25];
	int iterador_nome = 0;
	while (iterador_nome < strlen(file)) {
		file_name[iterador_nome] = file[iterador_nome];
		iterador_nome++;
	}
	file_name[iterador_nome] = '.';
	file_name[iterador_nome + 1] = 's';
	file_name[iterador_nome + 2] = 'o';
	file_name[iterador_nome + 3] = 'l';
	file_name[iterador_nome + 4] = '\0';
	fp = fopen(file_name, "r"); // MODO LEITURA
	if (fp == NULL)
	{
		perror("Erro ao abrir o arquivo.\n");
		exit(EXIT_FAILURE);
	}

	while (!feof(fp)) {
		char linha[100];
		fgets(linha, 100, fp);
		if (strncmp(linha, "FO: ", strlen("FO: ")) == 0) {
			// CAPTA A FO
			char linha_auxiliar[100];
			strcpy(linha_auxiliar, linha);
			int i = strlen("FO: "), j = 0;
			char variavel_captacao[100];
			while (i < strlen(linha_auxiliar)) {
				variavel_captacao[j] = linha_auxiliar[i];
				i++;
				j++;
			}
			variavel_captacao[j] = '\0';
			solucao.ResultFO = atoi(variavel_captacao);
		}
		else if (strncmp(linha, "Makespan: ", strlen("Makespan: ")) == 0) {
			// CAPTA O MAKESPAN
			char linha_auxiliar[100];
			strcpy(linha_auxiliar, linha);
			int i = strlen("Makespan: "), j = 0;
			char variavel_captacao[100];
			while (i < strlen(linha_auxiliar)) {
				variavel_captacao[j] = linha_auxiliar[i];
				i++;
				j++;
			}
			variavel_captacao[j] = '\0';
			solucao.makespan = atoi(variavel_captacao);
		}
		else if (strncmp(linha, "Job Start Time", strlen("Job Start Time")) == 0) {
			// LER DE "Job Start Time" ATE feof(fp).
			fgets(linha, 100, fp);
			while (!feof(fp)) {
				// PARA CADA LINHA
				char* pch;
				pch = strtok(linha, " \t\n"); // CAPTA O NUMERO DO JOB
				int numeroTarefa = atoi(pch);
				numTarefas = numeroTarefa;
				pch = strtok(NULL, " \t\n"); // CAPTA O TEMPO INICIAL
				int tempoInicial = atoi(pch);
				solucao.ordemTempo[numeroTarefa - 1] = tempoInicial;
				fgets(linha, 100, fp);
			}
		}
	}
	fclose(fp);
	calcularOrdem(solucao, false);
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
		_itoa(solucao.ordemTempo[nTarefa], aux, 10);
		strcat(linha, aux);
		strcat(linha, "\n");
		fputs(linha, fp);
	}
	fclose(fp);
}

void calculoFO(Solucao& solucao) {
	int tempoFinal = 0;
	solucao.makespan = 0;
	solucao.ResultFO = 0;
	// CALCULO DE MAKESPAN E FO
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		tempoFinal = solucao.ordemTempo[nTarefa] + tarefas[nTarefa].duration;
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
		std::cout << "Job: " << nTarefa + 1 << " Start Time: " << solucao.ordemTempo[nTarefa] << std::endl;
	}
}

void clonar(Solucao& solucaoC, Solucao& solucaoV) {
	memcpy(&solucaoC, &solucaoV, sizeof(solucaoV));
}

// TODO: REFATORAR DEPOIS DE CRIAR AS ESTR. AUX. E AVERIGUAR A CONSTR. GUL. + AJUSTES
void gerarVizinho(Solucao& solucao) {
	srand(time(0));
	int random_pos, random_time, temp, tarefasUteis, tempoUtil;
	tarefasUteis = numTarefas - 2;
	// TROCA DE TEMPOS INICIAIS (POSICOES) ENTRE DUAS TAREFAS VIZINHAS
	random_pos = rand() % tarefasUteis + 1;
	temp = solucao.ordemTempo[random_pos];
	solucao.ordemTempo[random_pos] = solucao.ordemTempo[random_pos + 1];
	solucao.ordemTempo[random_pos + 1] = temp;
	calculoFO(solucao);
}

void simAnnealing(const double alfa, const double tempInicial, const double tempCongelamento, const int SAMax,
	Solucao& solucao, const double tempoMax, double& tempoMelhor, double& tempoTotal) {

	clock_t clockInicial, clockFinal;
	Solucao solucaoAtual, solucaoVizinha;
	double temperatura, delta, aux;

	clockInicial = clock();
	heuristicaConGul(solucao);
	calculoFO(solucao);
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