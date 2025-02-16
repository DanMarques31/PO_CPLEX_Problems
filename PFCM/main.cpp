#include <bits/stdc++.h>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN //MACRO - "using namespace" for ILOCPEX

#define CPLEX_TIME_LIM 3600 //3600 segundos

struct aresta {
    int orige, destino;   
};

struct grafo {
    vector<int> vertices_origem;
    vector<int> vertices_destino;
    vector<int> vertices_intermed;
    

};