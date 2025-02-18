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

struct no {
    char id;
    int index;
};

struct aresta {
    int origem, destino;
    int custo;
};

int qntd_vertices, qntd_arestas;

vector<vector<int>> custos;
vector<no> nos;
vector<aresta> arestas;
std::map<char, int> id_to_index;
vector<no> nos_oferta;
vector<no> nos_demanda;

int get_index(char target_id) {
    auto it = id_to_index.find(target_id);
    
    if (it != id_to_index.end())
        return it->second;
    else
        return -1;
}

bool existe_no(const std::vector<no>& nos, char id_procurado) {
    for (const no& node : nos) {
        if (node.id == id_procurado) {
            return true;
        }
    }
    return false;
}

void printa_matriz() {
    printf("MATRIZ DE CUSTOS.\n");
    for (int i = 0; i < qntd_vertices; i++) {
        for (int j = 0; j < qntd_vertices; j++) {
            printf("%d ", custos[i][j]);
        }
        printf("\n");
    }
}

void add_no(vector<no> &nos_oferta, char id, int index) {
    no v = {id, index};
    nos_oferta.push_back(v);
}

void add_aresta(vector<aresta> &arestas, int origem, int destino, int custo) {
    aresta e = {origem, destino, custo};
    arestas.push_back(e);
}

bool read_input(const string &file_name) {
    ifstream infile(file_name);

    if (!infile) {
        cerr << "Erro ao abrir o arquivo." << endl;
        return false;
    }

    infile >> qntd_vertices >> qntd_arestas;

    custos.resize(qntd_vertices, vector<int>(qntd_vertices, 0)); // Inicializa com zeros

    // Leitura do nó de oferta.
    char no_id;
    int no_index = 0;

    infile >> no_id;
    add_no(nos_oferta, no_id, no_index);
    add_no(nos, no_id, no_index);
    id_to_index[no_id] = no_index; 
    no_index++;

    // Leitura do nó de demanda.
    infile >> no_id;
    add_no(nos_demanda, no_id, no_index);
    add_no(nos, no_id, no_index);
    id_to_index[no_id] = no_index; 
    no_index++;

    // Leitura das arestas e seus custos.
    char no_origem, no_destino;
    int custo;
    int index_origem, index_destino;

    for (int i = 0; i < qntd_arestas; i++) {
        infile >> no_origem >> no_destino >> custo;

        if (!existe_no(nos, no_origem)) {
            add_no(nos, no_origem, no_index);
            id_to_index[no_origem] =  no_index;
            no_index++;
        }
        if (!existe_no(nos, no_destino)) {
            add_no(nos, no_destino, no_index);
            id_to_index[no_destino] = no_index;
            no_index++;
        }

        index_origem = get_index(no_origem);
        index_destino = get_index(no_destino);

        add_aresta(arestas, index_origem, index_destino, custo);
        // add_aresta(arestas, index_destino, index_origem, custo);

        custos[index_origem][index_destino] = custo;
        // custos[index_destino][index_origem] = custo;
    }

    infile.close();
    return true;
}

void cplex() {
    IloEnv env;

    int num_var = 0, num_res = 0;

    IloArray<IloNumVarArray> x(env);
	for(int i = 0; i < qntd_vertices; i++) {
		x.add(IloNumVarArray(env));
		for(int j = 0; j < qntd_vertices; j++) {
			x[i].add(IloIntVar(env, 0, 1));
			num_var++;
		}
	}

    IloModel model (env);
    IloExpr obj(env);
    IloExpr sum(env);
    IloExpr sum2(env);
    
    // FUNCAO OBJETIVO.
    obj.clear();
    for (int i = 0; i < qntd_vertices; i++) {
        for (int j = 0; j < qntd_vertices; j++)
            if (custos[i][j] != 0)
                obj += custos[i][j]*x[i][j];
    }

    model.add(IloMinimize(env, obj));
    obj.end();

    // RESTRICOES.
    for (const no& node : nos) {
        sum.clear();
        for (int j = 0; j < qntd_vertices; j++) {
            if (node.index == j)
                continue;
            if (custos[node.index][j] != 0)
                sum += x[node.index][j];
        }

        sum2.clear();
        for (int k = 0; k < qntd_vertices; k++) {
            if (node.index == k)
                continue;
            if (custos[k][node.index] != 0)
                sum2 += x[k][node.index];
        }

        if (existe_no(nos_oferta, node.id)) {
            model.add((sum - sum2) == 1);
            num_res++;
        } else if (existe_no(nos_demanda, node.id)) {
            model.add((sum - sum2) == -1);
        } else {
            model.add((sum - sum2) == 0);
        }
    }

    // EXECUCAO.
    time_t timer, timer2;
	IloNum value, objValue;
	double runTime;
	string status;

    // Informacoes
    printf("--------Informacoes da Execucao:----------\n\n");
	printf("#Var: %d\n", num_var);
	printf("#Restricoes: %d\n", num_res);
	cout << "Memory usage after variable creation:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	
    IloCplex cplex(model);
	cout << "Memory usage after cplex(Model):  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;

    cplex.setParam(IloCplex::TiLim, CPLEX_TIME_LIM);

    time(&timer);
	cplex.solve();//COMANDO DE EXECUCAO
	time(&timer2);

    bool sol = true;

    switch(cplex.getStatus()) {
		case IloAlgorithm::Optimal: 
			status = "Optimal";
			break;
		case IloAlgorithm::Feasible: 
			status = "Feasible";
			break;
		default: 
			status = "No Solution";
			sol = false;
	}

	cout << endl << endl;
	cout << "Status da FO: " << status << endl;

    if (sol) {
        objValue = cplex.getObjValue();
		runTime = difftime(timer2, timer);

        cout << "Variaveis de decisao: " << endl;
		for(int i = 0; i < qntd_vertices; i++) {
		    for(int j = 0; j < qntd_vertices; j++) {
			    if (custos[i][j] == 0)
                    continue;
                value = IloRound(cplex.getValue(x[i][j]));
			    printf("x[%d][%d]: %.0lf\n", i, j, value);
            }
		}
		printf("\n");
		
		cout << "Funcao Objetivo Valor = " << objValue << endl;
		printf("..(%.6lf seconds).\n\n", runTime);
	} else 
		printf("No Solution!\n");


    cplex.end();
	sum.end();
	sum2.end();

	cout << "Memory usage before end:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	env.end();
}

int main() {
    if (!read_input("in.txt"))
        cout << "Erro." << endl;
    
    printf("--Verificação da leitura dos dados.--\n");
    printf("Quantidade de vertices: %d\n", qntd_vertices);
	printf("Quantidade de arestas: %d\n", qntd_arestas);
	printf("Origem - Destino - Custo\n");
    for(int i = 0; i < qntd_arestas; i = i + 2)
        printf("%d %d %d\n", arestas[i].origem, arestas[i].destino, arestas[i].custo);
    printf("\n");
    printa_matriz();


    cplex();

    return 0;
}