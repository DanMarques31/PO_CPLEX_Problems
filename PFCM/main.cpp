#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <limits.h>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

#define CPLEX_TIME_LIM 3600 //3600 segundos

struct aresta {
    char no_origem, no_destino;
    int custo, fluxo_max, fluxo_min;  
};

int qntd_arestas;
vector<aresta> arestas;


void add_aresta(vector<aresta> &arestas, char no_origem, char no_destino, int custo, int fluxo_max, int fluxo_min) {
    aresta e = {no_origem, no_destino, custo, fluxo_max, fluxo_min};
    arestas.push_back(e);
}

bool read_input(const string &file_name, vector<aresta> &arestas) {
    ifstream infile(file_name);

    if (!infile) {
        cerr << "Erro ao abrir o arquivo." << endl;
        return false;
    }
    
    infile >> qntd_arestas;

    char no_origem, no_destino;
    int custo, fluxo_max, fluxo_min;

    for (int i =0; i < qntd_arestas; i++) {
        infile >> no_origem >> no_destino >> custo >> fluxo_max >> fluxo_min;

        add_aresta(arestas, no_origem, no_destino, custo, fluxo_max, 0);
    }

    infile.close();
    return true;
}

void cplex() {
    IloEnv env;

    int num_var = 0, num_res = 0;

    IloArray<IloNumVarArray> x(env);
	for(int i = 0; i < qntd_arestas; i++){
		x.add(IloNumVarArray(env));
		for(int j = 0; j < qntd_arestas; j++ ){
			x[i].add(IloIntVar(env, 0, INT_MAX));
			num_var++;
		}
	}

    IloModel model (env);
    IloExpr sum(env);
    IloExpr sum2(env);
    
    sum.clear();

    // for (int i = 0; i < qntd_arestas; i++) {
    //     sum += (arestas[i].custo * );
    // }
}

int main() {    
    // vector<aresta> arestas;

    if (!read_input("in.txt", arestas))
        cout << "Erro.";
    
    printf("Verificacao da leitura dos dados:\n");
	printf("Quantidade de arestas: %d\n", qntd_arestas);
	printf("Origem - Destino - Custo - Fluxo máximo - Fluxo mínimo\n");
    for(int i = 0; i < qntd_arestas; i++)
        printf("%c %c %d %d %d\n", arestas[i].no_origem, arestas[i].no_destino, arestas[i].custo, arestas[i].fluxo_max, arestas[i].fluxo_min);
	printf("\n");

    return 0;
}