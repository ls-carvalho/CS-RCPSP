// TrabalhoOtimização.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdlib.h>
#include <memory>
#include <time.h>
#include <math.h>
#include "Solucao.h"

#define SEED 10

//#define EXIBIR_MELHOR


int main()
{
	Solucao solucao, solucao1;
	char file[25] = "P-n101-k4";
	//char escrita[25] = "P-n101-k4-3\0";
#ifdef EXIBIR_MELHOR
	lerSolucao(file, solucao1);
	exibirSolucao(solucao1);
#endif // EXIBIR_MELHOR
	lerArquivo(file);
	if (tipoArquivo == 0) {
		calculaCusto();
	}
	double const alfa = 0.995, tempInicial = 100, tempCongelamento = 0.001, tempoMax = 120;
	double tempoMelhor, tempoTotal;
	int SAMax = 3;
	srand(SEED);
	contador = 0;
	simAnnealing(alfa, tempInicial, tempCongelamento, SAMax * (numClientes * (numCarros + 1)), solucao, tempoMax, tempoMelhor, tempoTotal);
	std::cout << contador << ", " << INT_MAX << std::endl;
	std::cout << "tempo_melhor: " << tempoMelhor << std::endl;
	std::cout << "tempo_total: " << tempoTotal << std::endl;
	exibirSolucao(solucao);
	//escreverEmArquivo(escrita,solucao);
	system("pause");

}

void heuristicaConGul(Solucao& solucao) {
	//Limpeza das estruturas da solução
	memset(solucao.rota, -1, sizeof(solucao.rota));
	memset(solucao.carroCliente, 0, sizeof(solucao.carroCliente));
	memset(solucao.carroPeso, 0, sizeof(solucao.carroPeso));
	int clientesNaoAtendidos[NUM_CLIENTES],     //Clientes ainda não atendidos
		clienteAtual[NUM_CARROS],				//Cliente atualmente atendido por determinado carro
		melhorCB,								//Cliente com a distância mais curta
		melhorCBpos,							//Posição correspondente ao cliente no vetor clientesNaoAtendidos
		numClientesNaoAtendidos,				//Número total de clientes ainda não atendidos
		aux;
	bool cabe;
	numClientesNaoAtendidos = numClientes - 1;
	//Zerando o vetor de clientes não atendidos
	memset(clienteAtual, 0, sizeof(clienteAtual));
	//atribuindo os valores dos clientes no vetor de clientes não atendidos
	for (int i = 0; i < numClientes - 1; i++) {
		clientesNaoAtendidos[i] = i + 1;
	}
	//Colocando os clientes mais pesados nos carros (procedimento utilizado para não sobrar
	//cliente muito pesado no final e não caber em nenhum carro entrando assim em loop)
	for (int i = 0; i < numCarros; i++) {
		melhorCB = clientesNaoAtendidos[0];
		melhorCBpos = 0;
		for (int j = 1; j < numClientesNaoAtendidos; j++) {
			if (clientes[clientesNaoAtendidos[j]].peso > clientes[melhorCB].peso) {
				melhorCB = clientesNaoAtendidos[j];
				melhorCBpos = j;
			}
		}
		solucao.rota[i][solucao.carroCliente[i]] = melhorCB;
		solucao.carroCliente[i]++;
		aux = clientesNaoAtendidos[numClientesNaoAtendidos - 1];
		clientesNaoAtendidos[numClientesNaoAtendidos - 1] = clientesNaoAtendidos[melhorCBpos];
		clientesNaoAtendidos[melhorCBpos] = aux;
		numClientesNaoAtendidos--;
		clienteAtual[i] = melhorCB;
		solucao.carroPeso[i] += clientes[melhorCB].peso;
	}
	//Atribuindo os clientes mais próximos aos carros
	while (numClientesNaoAtendidos > 0) {
		for (int i = 0; i < numCarros; i++) {
			cabe = false;
			//Testando se os clientes não acabaram
			if (numClientesNaoAtendidos == 0) {
				break;
			}
			//Atribuindo um valor inicial
			melhorCB = clientesNaoAtendidos[0];
			melhorCBpos = 0;
			//Pegando o clientes mais próximo do ponto em que o carro se encontra
			for (int j = 0; j < numClientesNaoAtendidos; j++) {
				if (custo[clienteAtual[i]][clientesNaoAtendidos[j]] <= custo[clienteAtual[i]][melhorCB]) {
					if ((solucao.carroPeso[i] + clientes[clientesNaoAtendidos[j]].peso) <= capacidade) {
						melhorCB = clientesNaoAtendidos[j];
						melhorCBpos = j;
						cabe = true;
					}
				}
			}
			//teste para saber se o carro comporta o cliente
			if (cabe) {
				//Adiciona o cliente
				solucao.rota[i][solucao.carroCliente[i]] = melhorCB;
				//Aumenta o número de clientes no carro
				solucao.carroCliente[i]++;
				//Efetuando a troca com o último elemento do vetor de clientes não atendidos
				aux = clientesNaoAtendidos[numClientesNaoAtendidos - 1];
				clientesNaoAtendidos[numClientesNaoAtendidos - 1] = clientesNaoAtendidos[melhorCBpos];
				clientesNaoAtendidos[melhorCBpos] = aux;
				//Diminui o número de clientes não atendidos
				numClientesNaoAtendidos--;
				//Altera o cliente atual do carro i
				clienteAtual[i] = melhorCB;
				//Aumenta o peso do carro
				solucao.carroPeso[i] += clientes[melhorCB].peso;
			}
		}
	}
}

void lerArquivo(char* file) {
	FILE* fp;
	tipoArquivo = -1;
	char file_name[25];
	int i = 0;
	while (i < strlen(file)) {
		file_name[i] = file[i];
		i++;
	}
	file_name[i] = '.';
	file_name[i + 1] = 'v';
	file_name[i + 2] = 'r';
	file_name[i + 3] = 'p';
	file_name[i + 4] = '\0';
	fp = fopen(file_name, "r"); //modo leitura
	if (fp == NULL)
	{
		perror("Erro ao abrir o arquivo.\n");
		exit(EXIT_FAILURE);
	}

	while (!feof(fp)) {
		char mystring[100];
		fgets(mystring, 100, fp);
		if (strncmp(mystring, "NAME : ", strlen("NAME : ")) == 0) {
			//CAPTA O NUMERO DE VEICULOS
			char aux[100];
			strcpy(aux, mystring);
			int i = strlen("NAME : "), j = 0;
			char aux2[100];
			while (i < strlen(aux)) {
				aux2[j] = aux[i];
				i++;
				j++;
			}
			aux2[j] = '\0';
			char* p = strchr(aux2, 'k');
			p[0] = '0';
			numCarros = atoi(p);
		}
		else if (strncmp(mystring, "DIMENSION : ", strlen("DIMENSION : ")) == 0) {
			//CAPTA O NUMERO DE CLIENTES
			char aux[100];
			strcpy(aux, mystring);
			int i = strlen("DIMENSION : "), j = 0;
			char aux2[100];
			while (i < strlen(aux)) {
				aux2[j] = aux[i];
				i++;
				j++;
			}
			aux2[j] = '\0';
			numClientes = atoi(aux2);
		}
		else if (strncmp(mystring, "CAPACITY : ", strlen("CAPACITY : ")) == 0) {
			//CAPTA A CAPACIDADE DOS VEICULOS
			char aux[100];
			strcpy(aux, mystring);
			int i = strlen("CAPACITY : "), j = 0;
			char aux2[100];
			while (i < strlen(aux)) {
				aux2[j] = aux[i];
				i++;
				j++;
			}
			aux2[j] = '\0';
			capacidade = atoi(aux2);
		}
		else if (strncmp(mystring, "NODE_COORD_SECTION", strlen("NODE_COORD_SECTION")) == 0) {
			//ler até "DEMAND_SECTION"
			fgets(mystring, 100, fp);
			while (strncmp(mystring, "DEMAND_SECTION", strlen("DEMAND_SECTION")) != 0) {
				//cada linha
				char* pch;
				pch = strtok(mystring, " ");
				int i = atoi(pch);
				clientes[i - 1].num = i;
				pch = strtok(NULL, " ");
				int j = atoi(pch);
				clientes[i - 1].x = j;
				pch = strtok(NULL, " ");
				j = atoi(pch);
				clientes[i - 1].y = j;
				fgets(mystring, 100, fp);
			}
			//ler até "DEPOT_SECTION"
			fgets(mystring, 100, fp);
			while (strncmp(mystring, "DEPOT_SECTION", strlen("DEPOT_SECTION")) != 0) {
				//cada linha
				char* pch;
				pch = strtok(mystring, " ");
				int i = atoi(pch);
				pch = strtok(NULL, " ");
				int j = atoi(pch);
				clientes[i - 1].peso = j;
				fgets(mystring, 100, fp);
			}
			tipoArquivo = 0;
		}
		else if (strncmp(mystring, "EDGE_WEIGHT_SECTION", strlen("EDGE_WEIGHT_SECTION")) == 0) {
			//ler até "DEMAND_SECTION"
			while (strncmp(mystring, "DEMAND_SECTION", strlen("DEMAND_SECTION")) != 0) {
				//cada linha
				char* pch = NULL;
				for (int x = 0; x < numClientes - 1; x++) {
					for (int y = x + 1; y < numClientes; y++) {
						if (pch == NULL) {
							fgets(mystring, 100, fp);
							pch = strtok(mystring, " ");
						}
						else {
							pch = strtok(NULL, " ");
						}
						if (pch == NULL) {
							y--;
						}
						else {
							custo[x][y] = atoi(pch);
							custo[y][x] = atoi(pch);
						}
					}
				}
				fgets(mystring, 100, fp);
			}
			tipoArquivo = 1;
			//ler até "DEPOT_SECTION"
			fgets(mystring, 100, fp);
			while (strncmp(mystring, "DEPOT_SECTION", strlen("DEPOT_SECTION")) != 0) {
				//cada linha
				char* pch;
				pch = strtok(mystring, " ");
				int i = atoi(pch);
				pch = strtok(NULL, " ");
				int j = atoi(pch);
				clientes[i - 1].peso = j;
				fgets(mystring, 100, fp);
			}
		}
	}
	fclose(fp);
}

void lerSolucao(char* file, Solucao& solucao) {
	FILE* fp;
	char file_name[25];
	int i = 0;
	while (i < strlen(file)) {
		file_name[i] = file[i];
		i++;
	}
	file_name[i] = '.';
	file_name[i + 1] = 's';
	file_name[i + 2] = 'o';
	file_name[i + 3] = 'l';
	file_name[i + 4] = '\0';
	int num_cliente = 0;
	numCarros = 0;
	numClientes = 1;
	fp = fopen(file_name, "r"); //modo leitura
	if (fp == NULL)
	{
		perror("Erro ao abrir o arquivo.\n");
		exit(EXIT_FAILURE);
	}

	while (!feof(fp)) {
		char mystring[100];
		fgets(mystring, 100, fp);
		if (strncmp(mystring, "Route #", strlen("Route #")) == 0) {
			//CAPTA A ROTA
			char aux[100], aux2[100], aux3[100];
			strcpy(aux, mystring);
			int i = strlen("Route #"), j = 0, veiculo = 1, k = 0;
			while (i < strcspn(aux, ":")) {
				aux2[j] = aux[i];
				i++;
				j++;
			}
			veiculo = atoi(aux2);
			numCarros++;
			i = strcspn(aux, ":") + 2, j = 0;
			while (i < strlen(aux)) {
				aux3[j] = aux[i];
				i++;
				j++;
			}
			aux3[j] = '\0';
			char* pch = NULL;
			pch = strtok(aux3, " ");
			while (pch != NULL && strcmp(pch, "\n")) {
				solucao.rota[veiculo - 1][k] = atoi(pch);
				pch = strtok(NULL, " ");
				k++;
				num_cliente++;
			}
			solucao.carroCliente[veiculo - 1] = num_cliente;
			numClientes += num_cliente;
			num_cliente = 0;
		}
		else if (strncmp(mystring, "Cost ", strlen("Cost ")) == 0) {
			//CAPTA O CUSTO
			char aux[100], aux2[100];
			strcpy(aux, mystring);
			int i = strlen("Cost "), j = 0;
			while (i < strlen(aux)) {
				aux2[j] = aux[i];
				i++;
				j++;
			}
			solucao.ResultFO = atoi(aux2);
		}
		for (int i = 0; i < numCarros; i++) {
			solucao.carroPeso[i] = NULL;
		}
	}
	fclose(fp);
}

void escreverEmArquivo(char* file_name, Solucao solucao) {
	FILE* fp;
	int i = 0;
	char file_name_sol[25], mystring[100], aux[100];
	while (i < strcspn(file_name, ".")) {
		file_name_sol[i] = file_name[i];
		i++;
	}
	file_name_sol[i] = '\0';
	strcat(file_name_sol, ".txt");
	fp = fopen(file_name_sol, "w");
	if (fp == NULL)
	{
		perror("Erro ao abrir o arquivo.\n");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < numCarros; i++) {
		strcpy(mystring, "Route #");
		_itoa(i + 1, aux, 10);
		strcat(mystring, aux);
		strcat(mystring, ": ");
		for (int j = 0; j < solucao.carroCliente[i]; j++) {
			_itoa(solucao.rota[i][j] + 1, aux, 10);
			strcat(mystring, aux);
			strcat(mystring, " ");
		}
		strcat(mystring, "\n");
		fputs(mystring, fp);
	}
	strcpy(mystring, "Cost ");
	_itoa(solucao.ResultFO, aux, 10);
	strcat(mystring, aux);
	fputs(mystring, fp);

	fclose(fp);
}

void calculaCusto() {
	memset(&custo, -1, sizeof(custo));
	for (int i = 0; i < numClientes; i++) {
		for (int j = 0; j < numClientes; j++) {
			custo[i][j] = distancia(clientes[i], clientes[j]);
		}
	}
}

void calculoFO(Solucao& solucao) {
	solucao.ResultFO = 0;
	int peso = 0;
	bool inviavel = false;
	for (int i = 0; i < numCarros; i++) {
		if (solucao.carroCliente[i] > 0) {
			solucao.ResultFO += custo[0][solucao.rota[i][0]];
			peso += clientes[solucao.rota[i][0]].peso;
			for (int j = 0; j < solucao.carroCliente[i] - 1; j++) {
				solucao.ResultFO += custo[solucao.rota[i][j]][solucao.rota[i][j + 1]];
				peso += clientes[solucao.rota[i][j + 1]].peso;
			}
			solucao.ResultFO += (custo[solucao.rota[i][solucao.carroCliente[i] - 1]][0]);
			if (peso > capacidade) {
				inviavel = true;
				break;
			}
		}
		peso = 0;
	}
	if (inviavel) {
		solucao.ResultFO = INT_MAX;
	}
}

float distancia(Cliente pontoX, Cliente pontoY) {
	float quadradoDosCatetos = ((pontoX.x - pontoY.x) * (pontoX.x - pontoY.x)) + ((pontoX.y - pontoY.y) * (pontoX.y - pontoY.y));
	return sqrt(quadradoDosCatetos);
}

void exibirSolucao(Solucao& solucao) {
	std::cout << "Custo: " << solucao.ResultFO << std::endl;
	for (int i = 0; i < numCarros; i++) {
		std::cout << "Peso carro " << i << ": " << solucao.carroPeso[i] << std::endl;
	}
	std::cout << "Rota:" << std::endl;
	for (int i = 0; i < numCarros; i++) {
		std::cout << "Carro " << i << ": " << "0;";
		for (int j = 0; j < solucao.carroCliente[i]; j++) {
			std::cout << solucao.rota[i][j] << ";";
		}
		std::cout << "0" << std::endl;
	}
}

void clonar(Solucao& solucaoC, Solucao& solucaoV) {
	memcpy(&solucaoC, &solucaoV, sizeof(solucaoV));
}

void gerarVizinho(Solucao& solucao) {
	int carroA = 0, carroB = 1, clienteA = 0, clienteB = 1, aux;
	switch (rand() % 3) {
	case 0:
		do {
			carroA = rand() % (numCarros);
		} while (solucao.carroCliente[carroA] <= 1);
		clienteA = rand() % (solucao.carroCliente[carroA]);
		do {
			clienteB = rand() % (solucao.carroCliente[carroA]);

		} while (clienteA == clienteB);
		aux = solucao.rota[carroA][clienteA];
		solucao.rota[carroA][clienteA] = solucao.rota[carroA][clienteB];
		solucao.rota[carroA][clienteB] = aux;
		break;
	case 1:
		do {
			carroA = rand() % (numCarros);
			do {
				carroB = rand() % (numCarros);
			} while (carroA == carroB);
		} while (solucao.carroCliente[carroA] == 0 || solucao.carroCliente[carroB] == 0);
		clienteA = rand() % (solucao.carroCliente[carroA]);
		clienteB = rand() % (solucao.carroCliente[carroB]);
		aux = solucao.rota[carroA][clienteA];
		solucao.rota[carroA][clienteA] = solucao.rota[carroB][clienteB];
		solucao.rota[carroB][clienteB] = aux;
		solucao.carroPeso[carroA] += (clientes[solucao.rota[carroA][clienteA]].peso - clientes[solucao.rota[carroB][clienteB]].peso);
		solucao.carroPeso[carroB] += (clientes[solucao.rota[carroB][clienteB]].peso - clientes[solucao.rota[carroA][clienteA]].peso);
		break;
	case 2:
		do {
			carroA = rand() % numCarros;
			do {
				carroB = rand() % numCarros;
			} while (carroA == carroB);
		} while (solucao.carroCliente[carroA] == 0);
		clienteA = rand() % (solucao.carroCliente[carroA]);
		solucao.rota[carroB][solucao.carroCliente[carroB]] = solucao.rota[carroA][clienteA];
		for (int i = clienteA; i < solucao.carroCliente[carroA]; i++) {
			solucao.rota[carroA][i] = solucao.rota[carroA][i + 1];
		}
		solucao.carroCliente[carroA]--;
		solucao.carroCliente[carroB]++;
		solucao.carroPeso[carroA] -= clientes[solucao.rota[carroB][solucao.carroCliente[carroB] - 1]].peso;
		solucao.carroPeso[carroB] += clientes[solucao.rota[carroB][solucao.carroCliente[carroB] - 1]].peso;

		break;
	}
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

// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln