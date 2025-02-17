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
    int qntd;
    int index;
};

struct aresta {
    int origem, destino;
    int custo, fluxo_max, fluxo_min;  
};

struct grafo {
    int origem, destino;
    int custo, fluxo_max;
};

int qntd_vertices, qntd_arestas;
int qntd_oferta, qntd_demanda, qntd_transbordo;
vector<vector<grafo>> custos;
vector<aresta> arestas;
vector<no> nos;
std::map<char, int> id_to_index;
vector<no> nos_oferta;
vector<no> nos_demanda;
vector<no> nos_transbordo;

bool existe_no(const std::vector<no>& nos, char id_procurado) {
    for (const no& node : nos) {
        if (node.id == id_procurado) {
            return true;
        }
    }
    return false;
}

int get_index(char target_id) {
    auto it = id_to_index.find(target_id);
    
    if (it != id_to_index.end())
        return it->second;
    else
        return -1;
}

void add_no(vector<no> &nos_oferta, char id, int qntd, int index) {
    no v = {id, qntd, index};
    nos_oferta.push_back(v);
}

void add_aresta(vector<aresta> &arestas, int origem, int destino, int custo, int fluxo_max, int fluxo_min) {
    aresta e = {origem, destino, custo, fluxo_max, fluxo_min};
    arestas.push_back(e);
}

bool read_input(const string &file_name) {
    ifstream infile(file_name);

    if (!infile) {
        cerr << "Erro ao abrir o arquivo." << endl;
        return false;
    }
    
    infile >> qntd_vertices >> qntd_arestas >> qntd_oferta >> qntd_demanda >> qntd_transbordo;
    int index_nos = 0;

    // Leitura dos nós de oferta.
    for (int i = 0; i < qntd_oferta; i++) {
        char id_no;
        int oferta;
        infile >> id_no >> oferta;
        
        add_no(nos_oferta, id_no, oferta, index_nos);
        add_no(nos, id_no, oferta, index_nos);
        id_to_index[id_no] = index_nos;
        index_nos++;
    }
    
    // Leitura dos nós de demanda.
    for (int i = 0; i < qntd_demanda; i++) {
        char id_no;
        int demanda;
        infile >> id_no >> demanda;
        
        add_no(nos_demanda, id_no, demanda, index_nos);
        add_no(nos, id_no, demanda, index_nos);
        id_to_index[id_no] = index_nos;
        index_nos++;
    }
    
    // Leitura dos nós de transbordo.
    for (int i = 0; i < qntd_transbordo; i++) {
        char id_no;
        int transbordo;
        infile >> id_no >> transbordo;
        
        add_no(nos_transbordo, id_no, transbordo, index_nos);
        add_no(nos, id_no, transbordo, index_nos);
        id_to_index[id_no] = index_nos;
        index_nos++;
    }
    
    char no_origem, no_destino;
    int index_origem, index_destino;
    int custo, fluxo_max, fluxo_min;
    custos.resize(256, vector<grafo>(256));

    // Leitura das arestas
    for (int i = 0; i < qntd_arestas; i++) {
        infile >> no_origem >> no_destino >> custo >> fluxo_max >> fluxo_min;

        index_origem = get_index(no_origem);
        index_destino = get_index(no_destino);

        if (fluxo_max == -1)
            add_aresta(arestas, index_origem, index_destino, custo, INT_MAX, 0);
        else
            add_aresta(arestas, index_origem, index_destino, custo, fluxo_max, 0);

        custos[index_origem][index_destino].custo = arestas[i].custo;
        custos[index_origem][index_destino].fluxo_max = arestas[i].fluxo_max;
    }

    infile.close();
    return true;
}

void cplex() {
    IloEnv env;

    int num_var = 0, num_res = 0;

    IloArray<IloNumVarArray> x(env);
	for (int i = 0; i < qntd_vertices; i++) {
		x.add(IloNumVarArray(env));
		
        for (int j = 0; j < qntd_vertices; j++ ) {
			x[i].add(IloIntVar(env, 0, INT_MAX));
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
            obj += arestas[i].custo * x[i][j];
    }

    model.add(IloMinimize(env, obj));
    obj.end();

    // RESTRICOES
    // Restricao de oferta.
    for (const no& node : nos_oferta) {
        sum.clear();
        for (int j = 0; j < qntd_vertices; j++) {
            if (custos[node.index][j].custo != 0)
                sum += x[node.index][j];
        }
        // for (const aresta& e : custos[node.id][node.id].edges) {
        //     sum += x[e.origem][e.destino];
        // }
        
        sum2.clear();
        for (int k = 0; k < qntd_vertices; k++) {
            if (custos[k][node.index].custo != 0)
                sum2 += x[k][node.index];
        }
        
        model.add((sum - sum2) <= node.qntd);
        num_res++;


        // for (int i = 0; i < qntd_vertices; i++) {
        //    sum.clear();
    
        //    for (int j = 0; j < qntd_vertices; j++) {
        //         if (existe_no(nos_oferta, custos[i][j].vertex.id)) {
        //             if (custos[i][j].edge.custo != 0) 
        //                 sum += x[i][j];
        //         }
        //     }
    
        //     sum2.clear();
        //     for (int k = 0; k < qntd_vertices; k++) {
        //         if (existe_no(nos_oferta, custos[k][i].vertex.id)) {
        //             if (custos[k][i].edge.custo != 0) 
        //                 sum2 += x[k][i];
        //         }
        //     }
        //     model.add(sum - sum2)
    
        // }
    }

    // Restricao de demanda.
    for (const no& node : nos_demanda) {
        sum.clear();
        for (int j = 0; j < qntd_vertices; j++) {
            if (custos[node.index][j].custo != 0)
                sum += x[node.index][j];
        } 

        sum2.clear();
        for (int k = 0; k < qntd_vertices; k++) {
            if (custos[k][node.index].custo != 0)
                sum2 += x[k][node.index];
        }

        model.add((sum - sum2) <= -node.qntd);
        num_res++;
    }

    // // Restricao de conservação de fluxo.
    for (const no& node : nos_transbordo) {
        sum.clear();
        for (int j = 0; j < qntd_vertices; j++) {
            if (custos[node.index][j].custo != 0)
                sum += x[node.index][j];
        }   

        sum2.clear();
        for (int k = 0; k < qntd_vertices; k++) {
            if (custos[k][node.index].custo != 0)
                sum2 += x[k][node.index];
        } 
        
        model.add((sum - sum2) == 0);
        num_res++;
    }

    // // Restricao de capacidades
    for (int i = 0; i < qntd_arestas; i++) {
        for (int j = 0; j < qntd_arestas; j++) {
            if (custos[i][j].fluxo_max != 0) {
                model.add(x[i][j] <= custos[i][j].fluxo_max);
                num_res++;
            }
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
                // printf("\n\nCUSTO: %d\n\n", custos[i][j].edges[i].custo);
			    if (custos[i][j].custo == 0)
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
        cout << "Erro.";
    
    printf("Verificacao da leitura dos dados:\n");
    printf("Quantidade de vertices: %d\n", qntd_vertices);
	printf("Quantidade de arestas: %d\n", qntd_arestas);
	printf("Quantidade de nós de oferta: %d\n", qntd_oferta);
	printf("Quantidade de nós de demanda: %d\n", qntd_demanda);
	printf("Quantidade de nós de transbordo: %d\n", qntd_transbordo);
	printf("Origem - Destino - Custo - Fluxo máximo - Fluxo mínimo\n");
    for(int i = 0; i < qntd_arestas; i++)
        printf("%d %d %d %d %d\n", arestas[i].origem, arestas[i].destino, arestas[i].custo, arestas[i].fluxo_max, arestas[i].fluxo_min);
    printf("\n");
    for(int i = 0; i < qntd_oferta; i++) {
        printf("%c %d\n", nos_oferta[i].id, nos_oferta[i].qntd);
    }
    for(int i = 0; i < qntd_demanda; i++) {
        printf("%c %d\n", nos_demanda[i].id, nos_demanda[i].qntd);
    }
    for(int i = 0; i < qntd_transbordo; i++) {
        printf("%c %d\n", nos_transbordo[i].id, nos_transbordo[i].qntd);
    }

    cplex();

    return 0;
}