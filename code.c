#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

int nVertices,    // Número de vértices do grafo
    nArestas,    // Número de arestas do grafo
    **matAdj,    // Matriz de adjacências do grafo
    **arestas;    // Lista de arestas do grafo

// ----------------------------------------------------------------------------
void inicializa(char* nome_arq_entrada){
    FILE *arq_entrada;
    arq_entrada = fopen(nome_arq_entrada, "rt");
    if (arq_entrada == NULL){
        printf("\nArquivo texto de entrada não encontrado\n");
        exit(1);
    }
    fscanf(arq_entrada, "%d", &nVertices);
    matAdj = (int**) malloc(nVertices * sizeof(int*));
    for (int i = 0; i < nVertices; i++)
        matAdj[i] = (int*) malloc(nVertices * sizeof(int));
    for (int i = 0; i < nVertices; i++)
        for (int j = 0; j < nVertices; j++)
            fscanf(arq_entrada, "%d", &(matAdj[i][j]));
    fclose(arq_entrada);
}

// ----------------------------------------------------------------------------
void aloca_arestas(){
    arestas = (int**) malloc(nArestas * sizeof(int*));
    for (int i = 0; i < nArestas; i++)
        arestas[i] = (int*) malloc(2 * sizeof(int));
}

// ----------------------------------------------------------------------------
void conta_arestas(){
    nArestas = 0;
    #pragma omp parallel for reduction(+:nArestas)
    for (int i = 0; i < nVertices; i++)
        for (int j = 0; j < nVertices; j++)
            if (matAdj[i][j] != 0){
                nArestas++;
            }
}

// ----------------------------------------------------------------------------
void converte(){
    int k = 0;
    #pragma omp parallel for
    for (int i = 0; i < nVertices; i++)
        for (int j = 0; j < nVertices; j++)
            if (matAdj[i][j] != 0){
                #pragma omp critical
                {
                    arestas[k][0] = i;
                    arestas[k][1] = j;
                    k++;
                }
            }
}

// ----------------------------------------------------------------------------
void finaliza(char* nome_arq_saida){
    FILE *arq_saida;
    arq_saida = fopen(nome_arq_saida, "wt");
    fprintf(arq_saida, "%d\n", nArestas);
    for (int i = 0; i < nArestas; i++)
        fprintf(arq_saida, "%d %d\n", arestas[i][0], arestas[i][1]);
    fclose(arq_saida);
    for (int i = 0; i < nArestas; i++)
        free(arestas[i]);
    free(arestas);
    for (int i = 0; i < nVertices; i++)
        free(matAdj[i]);
    free(matAdj);
}

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    char nome_arq_entrada[100],
          nome_arq_saida[100] ;

    if(argc != 3)
    {
        printf("O programa foi executado com parâmetros incorretos.\n");
		printf("Uso: ./converte_seq arquivo_entrada arquivo_saída\n");
		exit(1);
	}
	strcpy(nome_arq_entrada, argv[1]) ;
	strcpy(nome_arq_saida, argv[2]) ;
	inicializa(nome_arq_entrada);

	double tini = omp_get_wtime();
	conta_arestas();

	double tfin = omp_get_wtime();
	double tempo1 = tfin - tini;
	aloca_arestas();
	tini = omp_get_wtime(); 
	converte();
	tfin = omp_get_wtime();
	double tempo2 = tfin - tini;
	double tempo = tempo1 +tempo2;
	printf("Tempo: %f + %f = %f s\n", tempo1, tempo2, tempo);
	finaliza(nome_arq_saida);
	return 0 ;
}