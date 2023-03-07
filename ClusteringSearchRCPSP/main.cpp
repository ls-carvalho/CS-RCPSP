#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <memory>
#include <time.h>
#include <math.h>
#include "Solucao.h"

#define SEED 10

//#define EXIBIR_MELHOR
//#define MODO_DBGHEU
#define MODO_DBGLEITURA
//#define MODO_EXECUCAO

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
#ifdef MODO_DBGLEITURA
	std::cout << "Numero de Tarefas: " << numTarefas << std::endl;
	std::cout << "Numero de Recursos: " << numRecursos << std::endl;
	std::cout << "Tempo Horizonte: " << tempoHorizonte << std::endl;
	for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
	{
		std::cout << "Recurso " << nRecurso + 1 << ": " << recursosDisponiveis[nRecurso] << std::endl;
	}
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++)
	{
		std::cout << "Tarefa: " << tarefas[nTarefa].numJob << std::endl;
		std::cout << "Duracao: " << tarefas[nTarefa].duration << std::endl;
		std::cout << "Numero de Sucessores: " << tarefas[nTarefa].numProximos << std::endl;
		for (int nProximo = 0; nProximo < tarefas[nTarefa].numProximos; nProximo++)
		{
			if (nProximo == 0) {
				std::cout << "Sucessores: " << std::endl;
			}
			std::cout << tarefas[nTarefa].vetProximos[nProximo] << std::endl;
		}
		for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
		{
			if (nRecurso == 0) {
				std::cout << "Recursos: " << std::endl;
			}
			std::cout << "R " << nRecurso + 1 << ": " << tarefas[nTarefa].vetRecursos[nRecurso] << std::endl;
		}
	}
#endif
#ifdef MODO_EXECUCAO
	double const alfa = 0.995, tempInicial = 100, tempCongelamento = 0.001, tempoMax = 120;
	double tempoMelhor, tempoTotal;
	int SAMax = 3;
	srand(SEED);
	contador = 0;
	simAnnealing(alfa, tempInicial, tempCongelamento, SAMax * (numTarefas * numRecursos), solucao, tempoMax, tempoMelhor, tempoTotal);
	std::cout << contador << ", " << INT_MAX << std::endl;
	std::cout << "tempo_melhor: " << tempoMelhor << std::endl;
	std::cout << "tempo_total: " << tempoTotal << std::endl;
	exibirSolucao(solucao);
	escreverEmArquivo(escrita,solucao);
#endif
	system("pause");

}

void heuristicaConAle(Solucao& solucao) {
	// LIMPA AS VARIAVEIS
	solucao.makespan = 0;
	solucao.ResultFO = 0;
	memset(solucao.ordem, -1, sizeof(solucao.ordem));

	// INSERE UM TEMPO INCIAL ALEATORIO PARA CADA TAREFA
	for (int nTarefa = 0; nTarefa < NUM_JOBS; nTarefa++) {
		int tempoInicial;
		tempoInicial = rand() % tempoHorizonte;
		solucao.ordem[nTarefa] = tempoInicial;
#ifdef MODO_DBGHEU
		std::cout << "Tarefa: " << nTarefa << std::endl;
		std::cout << "Tempo Inicial: " << tempoInicial << std::endl;
#endif
	}
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
				pch = strtok(linha, " "); // CAPTA O NUMERO DO JOB
				int numeroTarefa = atoi(pch);
				pch = strtok(NULL, " "); // CAPTA O TEMPO INICIAL
				int tempoInicial = atoi(pch);
				solucao.ordem[numeroTarefa - 1] = tempoInicial;
				fgets(linha, 100, fp);
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
	fputs(linha, fp);
	// ESCREVENDO O MAKESPAN
	strcpy(linha, "Makespan: ");
	_itoa(solucao.ResultFO, aux, 10);
	strcat(linha, aux);
	fputs(linha, fp);
	// ESCREVENDO A LINHA SEPARADORA
	strcpy(linha, "------------------");
	fputs(linha, fp);
	// ESCREVENDO O CABECALHO
	strcpy(linha, "Job Start Time");
	fputs(linha, fp);
	// ITERANDO AS TAREFAS E 
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		// ESCREVENDO O NUMERO DA TAREFA
		_itoa(nTarefa + 1, aux, 10);
		strcat(linha, aux);
		strcat(linha, "\t");
		// ESCREVENDO OS TEMPOS INCIAIS
		_itoa(solucao.ordem[nTarefa], aux, 10);
		strcat(linha, aux);
		fputs(linha, fp);
	}
	fclose(fp);
}

void calculoFO(Solucao& solucao) {
	solucao.ResultFO = 0;
	int tempoFinal = 0;
	// CALCULO DE MAKESPAN
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++) {
		 tempoFinal = solucao.ordem[nTarefa] + tarefas[nTarefa].duration;
		if (solucao.makespan < tempoFinal) {
			solucao.makespan = tempoFinal;
		}
	}
	// CALCULO DA FO
	if (isViavel(solucao)) {
		solucao.ResultFO = INT_MAX;
	}
}

int isViavel(Solucao solucao) {
	int vetorRecursosTempo[HORIZONTE][NUM_RESOURCES];
	// PREENCHE O VETOR DE RECURSOS X TEMPO
	for (int nRecurso = 0; nRecurso < numRecursos; nRecurso++)
	{
		for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++)
		{
			for (int tempoReal = 0; tempoReal < tempoHorizonte; tempoReal++)
			{
				int tempoInicialTarefa = solucao.ordem[nTarefa];
				for (int tempoRelativo = tempoInicialTarefa; tempoRelativo < (tempoInicialTarefa + tarefas[nTarefa].duration); tempoRelativo++)
				{
					vetorRecursosTempo[tempoRelativo][nRecurso] += tarefas[nTarefa].vetRecursos[nRecurso];
				}
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
		for (int nSucessor = 0; nSucessor < tarefas[nTarefa].numProximos; nSucessor++)
		{
			int proximoSucessor = tarefas[nTarefa].vetProximos[nSucessor];
			if (solucao.ordem[proximoSucessor] <= (solucao.ordem[nTarefa] + tarefas[nTarefa].duration)) { // TEMPO_FINAL_TAREFA >= TEMPO_INICIAL_SUCESSOR ? INVIAVEL : VIAVEL.
				return 0;
			}
		}
	}
	return 1;
}

void exibirSolucao(Solucao& solucao) {
	std::cout << "FO: " << solucao.ResultFO << std::endl;
	std::cout << "Makespan: " << solucao.makespan << std::endl;
	for (int nTarefa = 0; nTarefa < numTarefas; nTarefa++)
	{
		std::cout << "Job: " << nTarefa + 1 << " Start Time: " << solucao.ordem[nTarefa] << std::endl;
	}
}

void clonar(Solucao& solucaoC, Solucao& solucaoV) {
	memcpy(&solucaoC, &solucaoV, sizeof(solucaoV));
}

void gerarVizinho(Solucao& solucao) {
	int random_pos1 = rand() % numTarefas;
	int random_pos2 = rand() % numTarefas;
	int temp = solucao.ordem[random_pos1];
	solucao.ordem[random_pos1] = solucao.ordem[random_pos2];
	solucao.ordem[random_pos2] = temp;
	calculoFO(solucao);
}

void simAnnealing(const double alfa, const double tempInicial, const double tempCongelamento, const int SAMax,
	Solucao& solucao, const double tempoMax, double& tempoMelhor, double& tempoTotal) {

	clock_t clockInicial, clockFinal;
	Solucao solucaoAtual, solucaoVizinha;
	double temperatura, delta, aux;

	clockInicial = clock();
	heuristicaConAle(solucao);
	calculoFO(solucao);
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