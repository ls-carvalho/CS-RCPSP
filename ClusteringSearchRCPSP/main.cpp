#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <memory>
#include <time.h>
#include <math.h>
#include "Solucao.h"

//#define SEED 10

//#define EXIBIR_MELHOR
//#define MODO_DBGHEU

// PARAMETROS:
#define SA_MAX 3
#define PENALIDADE_INVIABILIDADE 1.2

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
	double const alfa = 0.995, tempInicial = 100, tempCongelamento = 0.001, tempoMax = 900;
	double tempoMelhor, tempoTotal;
	int SAMax = SA_MAX;
	srand(time(0));
	contador = 0;
	simAnnealing(alfa, tempInicial, tempCongelamento, SAMax * (numTarefas * numRecursos), solucao, tempoMax, tempoMelhor, tempoTotal);
	std::cout << contador << ", " << INT_MAX << std::endl;
	std::cout << "tempo_melhor: " << tempoMelhor << std::endl;
	std::cout << "tempo_total: " << tempoTotal << std::endl;
	exibirSolucao(solucao);
	escreverEmArquivo(escrita, solucao);
	system("pause");

}

void calcularOrdem(Solucao& solucao) {
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		// INICIALIZA O VETOR DE 0 A N-1
		solucao.ordemTarefa[nTarefa] = nTarefa; // TODO => ACHO QUE ISSO VAI DAR ERRAD, POIS É USADO NA LEITURA DA SOLUÇÃO, MAS É GERADO NA HEURISTÍCA TAMBÉM.
		// PREENCHE A MATRIZ DE TEMPO INCIAL x FINAL
		posicaoInicialFinalTarefas[nTarefa][0] = solucao.ordemTempo[nTarefa];
		posicaoInicialFinalTarefas[nTarefa][1] = solucao.ordemTempo[nTarefa] + tarefas[nTarefa].duration;
	}
	// ORDENA O VETOR COM BASE NOS VALORES DO VETOR DA ORDEM DOS TEMPOS, POR TAREFA
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		for (int proxTarefa = nTarefa + 1; proxTarefa < numTarefas; proxTarefa++) {
			if (solucao.ordemTempo[solucao.ordemTarefa[nTarefa]] > solucao.ordemTempo[solucao.ordemTarefa[proxTarefa]]) {
				int temp = solucao.ordemTarefa[nTarefa];
				solucao.ordemTarefa[nTarefa] = solucao.ordemTarefa[proxTarefa];
				solucao.ordemTarefa[proxTarefa] = temp;
			}
		}
	}
}

void calcularAntecessores() {
	for (int tarefaAtual = 0; tarefaAtual < numTarefas; tarefaAtual++)
	{
		tarefas[tarefaAtual].numAnteriores = 0; // INIALIZA OS ANTERIORES COMO 0
	}

	for (int tarefaAtual = 0; tarefaAtual < numTarefas; tarefaAtual++)
	{
		for (int proxima = 0; proxima < tarefas[tarefaAtual].numProximos; proxima++)
		{
			int tarefaAlvo = tarefas[tarefaAtual].vetProximos[proxima];
			tarefas[tarefaAlvo-1].numAnteriores += 1; // ATUALIZA O NUMERO DE ANTERIORES DO SUCESSOR
			tarefas[tarefaAlvo-1].vetAnteriores[tarefas[tarefaAlvo-1].numAnteriores - 1] = tarefas[tarefaAtual].numJob; // ADICIONA O ANTERIOR DO SUCESSOR NA RESPECTIVA LISTA DO MESMO
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
	int tarefasNaoAlocadas[NUM_JOBS+NUM_JOBS]; // EVITAR OVERFLOW DE BUFFER POR REGISTRAR NUMEROS NEGATIVOS
	tarefasNaoAlocadas[0] = -1;
	tarefasNaoAlocadas[numTarefas] = -1;
	for (int tarefaAtual = 1; tarefaAtual < numTarefas; tarefaAtual++)
	{
		tarefasNaoAlocadas[tarefaAtual] = tarefas[tarefaAtual].numAnteriores;
	}
	posicaoInicialFinalTarefas[0][0] = 0;
	posicaoInicialFinalTarefas[0][1] = 0;
	for (int ordemTarefa = 1; ordemTarefa < numTarefas-1; ordemTarefa++)
	{
		for (int tarefaAtual = 1; tarefaAtual < numTarefas-1; tarefaAtual++)
		{
			if (tarefasNaoAlocadas[tarefaAtual] == -1) continue; // TAREFA JA FOI ALOCADA. PROXIMA.
			if (tarefas[tarefaAtual].numAnteriores == 0) { // TAREFA SEM ANTECESSOR PODE SER ALOCADA. (PODE SER QUE ESSE BLOCO SEJA DESNECESSARIO)
				solucao.ordemTarefa[ordemTarefa] = tarefas[tarefaAtual].numJob;
				tarefasNaoAlocadas[tarefaAtual] = -1;
				break;
			}
			int todosAntecessoresAlocados = 1;
			for (int antecessor = 0; antecessor < tarefas[tarefaAtual].numAnteriores; antecessor++)
			{
				if (tarefasNaoAlocadas[tarefas[tarefaAtual].vetAnteriores[antecessor] - 1] != -1) {
					todosAntecessoresAlocados = 0;
					break;
				}
			}
			if (todosAntecessoresAlocados == 1) { // SE TODOS OS ANTECESSORES FORAM ALOCADOS, ALOCA A TAREFA.
				solucao.ordemTarefa[ordemTarefa] = tarefas[tarefaAtual].numJob;
				tarefasNaoAlocadas[tarefaAtual] = -1;
				break;
			}
		}
		// TODO: PROVAVEL LUGAR ONDE PODE SER INSERIDA LOGICA PARA REALIZAR A ALOCACAO "PARALELA" DE JOBS
		// ALGO COMO "NAO SENDO ANTECESSOR, PODE SER PARALELO"
		int indiceTarefa = solucao.ordemTarefa[ordemTarefa] - 1;
		posicaoInicialFinalTarefas[indiceTarefa][0] = posicaoInicialFinalTarefas[indiceTarefa - 1][1];
		posicaoInicialFinalTarefas[indiceTarefa][1] = posicaoInicialFinalTarefas[indiceTarefa][0] + tarefas[indiceTarefa].duration;
		solucao.ordemTempo[indiceTarefa] = posicaoInicialFinalTarefas[indiceTarefa][0];
	}
	posicaoInicialFinalTarefas[numTarefas - 1][0] = posicaoInicialFinalTarefas[numTarefas - 2][1];
	posicaoInicialFinalTarefas[numTarefas - 1][1] = posicaoInicialFinalTarefas[numTarefas - 1][0] + tarefas[numTarefas - 1].duration;
	solucao.ordemTempo[numTarefas - 1] = posicaoInicialFinalTarefas[numTarefas - 1][0];
	solucao.ordemTarefa[numTarefas - 1] = tarefas[numTarefas - 1].numJob;
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
	calcularOrdem(solucao);
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
	// CALCULO DE MAKESPAN
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		tempoFinal = solucao.ordemTempo[nTarefa] + tarefas[nTarefa].duration;
		if (solucao.makespan < tempoFinal) {
			solucao.makespan = tempoFinal;
		}
	}
	// CALCULO DA FO
	if (isViavel(solucao) == 1) {
		solucao.ResultFO = solucao.makespan;
	}
	else {
		solucao.ResultFO = solucao.makespan * PENALIDADE_INVIABILIDADE;
	}
}

// TODO => REFATORAR
int isViavel(Solucao solucao) {
	for (int nHorizonte = 0; nHorizonte < tempoHorizonte; nHorizonte++)
	{
		for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
		{
			vetorRecursosTempo[nHorizonte][nRecurso] = 0;
		}
	}
	// PREENCHE O VETOR DE RECURSOS X TEMPO
	for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
	{
		for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++)
		{
			int tempoInicialTarefa = solucao.ordemTempo[nTarefa];
			for (int tempoRelativo = tempoInicialTarefa; tempoRelativo < (tempoInicialTarefa + tarefas[nTarefa].duration); tempoRelativo++)
			{
				vetorRecursosTempo[tempoRelativo][nRecurso] = vetorRecursosTempo[tempoRelativo][nRecurso] + tarefas[nTarefa].vetRecursos[nRecurso];
			}
		}
	}
	// VERIFICA SE RESPEITA RECURSOS EM CADA TEMPO
	for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
	{
		for (int tempoReal = 0; tempoReal < tempoHorizonte; tempoReal++)
		{
			if (vetorRecursosTempo[tempoReal][nRecurso] > recursosDisponiveis[nRecurso]) {
				return 0;
			}
		}
	}
	// VARRE O VETOR DE TAREFAS
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++)
	{
		if (solucao.ordemTempo[nTarefa] + tarefas[nTarefa].duration > tempoHorizonte) { // TEMPO_FINAL_TAREFA > HORIZONTE
			return 0;
		}
		for (int nSucessor = 0; nSucessor < tarefas[nTarefa].numProximos; nSucessor++)
		{
			int proximoSucessor = tarefas[nTarefa].vetProximos[nSucessor];
			if (proximoSucessor = numTarefas) continue; // DESCARTA A ULTIMA TAREFA (sink)
			if (solucao.ordemTempo[proximoSucessor] <= (solucao.ordemTempo[nTarefa] + tarefas[nTarefa].duration)) { // TEMPO_FINAL_TAREFA >= TEMPO_INICIAL_SUCESSOR ? INVIAVEL : VIAVEL.
				return 0;
			}
		}
	}
	return 1;
}

void exibirSolucao(Solucao& solucao) {
	std::cout << "Viabilidade: " << isViavel(solucao) << std::endl;
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

// TODO: NECESSARIO UMA FUNCAO QUE SEMPRE GERE UM VIZINHO VIAVEL, MAS APENAS FOCANDO EM PRECEDENCIA. REFATORAR ABAIXO.
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