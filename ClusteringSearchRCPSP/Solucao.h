#pragma once

#define NUM_CLIENTES 101
#define NUM_CARROS 14


//Dados da solucao
struct sSolucao {
	int carroCliente[NUM_CARROS];
	int carroPeso[NUM_CARROS];
	int rota[NUM_CARROS][NUM_CLIENTES];
	float ResultFO;
};
typedef struct sSolucao Solucao;

//Dados de entrada
struct sPonto {
	int x;
	int y;
	int num;
	int peso;
};
typedef struct sPonto Cliente;

Cliente clientes[NUM_CLIENTES];
float custo[NUM_CLIENTES][NUM_CLIENTES];
int capacidade;
int numCarros;
int numClientes;
int tipoArquivo;
long int contador;


//Prototipos
void lerArquivo(char* file_name);
void lerSolucao(char* file_name, Solucao& solucao);
void escreverEmArquivo(char* file_name, Solucao solucao);
void calculaCusto();
float distancia(Cliente x, Cliente y);
void heuristicaConGul(Solucao& solucao);
void calculoFO(Solucao& solucao);
void exibirSolucao(Solucao& solucao);
void clonar(Solucao& solucaoC, Solucao& solucaoV);
void gerarVizinho(Solucao& solucao);
void simAnnealing(const double alfa, const double tempInicial, const double tempCongelamento, const int SAMax,
	Solucao& solucao, const double tempo_max, double& tempoMelhor, double& tempoTotal);