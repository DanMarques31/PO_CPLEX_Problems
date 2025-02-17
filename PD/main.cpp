#include <bits/stdc++.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

#define CPLEX_TIME_LIM 3600 //3600 segundos

int N; // Quantidade de pessoas e de tarefas.
vector<vector<int>> custos; // matriz de custos em que cada pessoa i cobra pela tarefa j

void printa_matriz_custos() {
    printf("MATRIZ DE CUSTOS:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", custos[i][j]);
        }
        printf("\n"); 
    }
};

bool read_input(const string &file_name) {
    ifstream infile(file_name);

    if (!infile) {
        cerr << "Erro ao abrir o arquivo." << endl;
        return false;
    }

    infile >> N;

    custos.resize(N, vector<int>(N));

    // Preenchimento da matriz de custos.
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            infile >> custos[i][j];
        }
    }

    infile.close();
    return true;
}

void cplex() {
    IloEnv env;

    int num_var = 0, num_res = 0;

    IloArray<IloNumVarArray> x(env);
	for(int i = 0; i < N; i++) {
		x.add(IloNumVarArray(env));
		for(int j = 0; j < N; j++) {
			x[i].add(IloIntVar(env, 0, 1));
			num_var++;
		}
	}

    IloModel model (env);
    IloExpr obj(env);
    IloExpr sum(env);

    // FUNCAO OBJETIVO.
    obj.clear();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            obj += custos[i][j]*x[i][j];
    }
    model.add(IloMinimize(env, obj));
    obj.end();

    // RESTRICOES.
    // R1 - Tarefa designada.
    for (int i = 0; i < N; i++) {
        sum.clear();
        for (int j = 0; j < N; j++) {
            sum += x[i][j];
        }
        model.add(sum == 1);
        num_res++;
    }

    // R2 - Pessoa designada
    for (int i = 0; i < N; i++) {
        sum.clear();
        for (int j = 0; j < N; j++) {
            sum += x[j][i];
        }
        model.add(sum == 1);
        num_res++;
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
		for(int i = 0; i < N; i++) {
		    for(int j = 0; j < N; j++) {
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

	cout << "Memory usage before end:  " << env.getMemoryUsage() / (1024. * 1024.) << " MB" << endl;
	env.end();
}

int main() {
    if (!read_input("in.txt"))
        cout << "Erro." << endl;
    
    printf("--Verificação da leitura de dados.--\n");
    printf("Num de pessoas e tarefas: %d.\n", N);
    printa_matriz_custos();

    cplex();

    return 0;
}