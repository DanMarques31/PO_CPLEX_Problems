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
    int custo;  
};

int qntd_origens, qntd_destinos;
vector<aresta> arestas;
vector<no> origens;
vector<no> destinos;
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

void add_no(vector<no> &nos, char id, int qntd, int index) {
    no v = {id, qntd, index};
    nos.push_back(v);
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
    
    infile >> qntd_origens >> qntd_destinos;
    int index_nos = 0;

    // Leitura das ofertas das origens
    for (int i = 0; i < qntd_origens; i++) {
        int oferta;
        infile >> oferta;
        char id = 'A' + i; 
        add_no(origens, id, oferta, index_nos);
        id_to_index[id] = index_nos;
        index_nos++;
    }
    
    // Leitura das demandas dos destinos
    for (int i = 0; i < qntd_destinos; i++) {
        int demanda;
        infile >> demanda;
        char id = '1' + i; 
        add_no(destinos, id, demanda, index_nos);
        id_to_index[id] = index_nos;
        index_nos++;
    }
    
    // Leitura dos custos das arestas da origem 1, 2 e 3.
    for (int i = 0; i < qntd_origens; i++) {
        for (int j = 0; j < qntd_destinos; j++) {
            int custo;
            infile >> custo;
            add_aresta(arestas, i, j, custo);
        }
    }
    
    infile.close();
    return true;
}

void cplex() {
    IloEnv env;

    int num_var = 0, num_res = 0;

    IloArray<IloNumVarArray> x(env);
    for (int i = 0; i < qntd_origens; i++) {
        x.add(IloNumVarArray(env));
        
        for (int j = 0; j < qntd_destinos; j++) {
            x[i].add(IloIntVar(env, 0, INT_MAX));
            num_var++;
        }
    }

    IloModel model(env);
    IloExpr obj(env);
    
    // Função objetivo: Minimizar o custo total
    for (int i = 0; i < qntd_origens; i++) {
        for (int j = 0; j < qntd_destinos; j++) {
            obj += arestas[i * qntd_destinos + j].custo * x[i][j];
        }
    }
    model.add(IloMinimize(env, obj));
    obj.end();

    // Restrições de oferta
    for (int i = 0; i < qntd_origens; i++) {
        IloExpr sum(env);
        for (int j = 0; j < qntd_destinos; j++) {
            sum += x[i][j];
        }
        model.add(sum <= origens[i].qntd);
        sum.end();
        num_res++;
    }

    // Restrições de demanda
    for (int j = 0; j < qntd_destinos; j++) {
        IloExpr sum(env);
        for (int i = 0; i < qntd_origens; i++) {
            sum += x[i][j];
        }
        model.add(sum == destinos[j].qntd);
        sum.end();
        num_res++;
    }

    // Execução do CPLEX
    IloCplex cplex(model);
    cplex.setParam(IloCplex::TiLim, CPLEX_TIME_LIM);

    if (cplex.solve()) {
        cout << "Solution status = " << cplex.getStatus() << endl;
        cout << "Solution value = " << cplex.getObjValue() << endl;
        for (int i = 0; i < qntd_origens; i++) {
            for (int j = 0; j < qntd_destinos; j++) {
                cout << "x[" << i << "][" << j << "] = " << cplex.getValue(x[i][j]) << endl;
            }
        }
    } else {
        cout << "No solution found!" << endl;
    }

    cplex.end();
    env.end();
}

int main() {    
    if (!read_input("in.txt"))
        cout << "Erro ao ler o arquivo de entrada." << endl;
    
    printf("Verificacao da leitura dos dados:\n");
    printf("Quantidade de origens: %d\n", qntd_origens);
    printf("Quantidade de destinos: %d\n", qntd_destinos);
    printf("Ofertas das origens:\n");
    for(int i = 0; i < qntd_origens; i++) {
        printf("%c %d\n", origens[i].id, origens[i].qntd);
    }
    printf("Demandas dos destinos:\n");
    for(int i = 0; i < qntd_destinos; i++) {
        printf("%c %d\n", destinos[i].id, destinos[i].qntd);
    }
    printf("Custos das arestas:\n");
    for(int i = 0; i < qntd_origens; i++) {
        for(int j = 0; j < qntd_destinos; j++) {
            printf("Origem %c -> Destino %c: Custo %d\n", 
                   origens[i].id, destinos[j].id, arestas[i * qntd_destinos + j].custo);
        }
    }

    cplex();

    return 0;
}