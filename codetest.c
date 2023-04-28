#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

int nVertices,	
	 nArestas,	
	 **matAdj,	
	 **arestas;	

void inicializa(char* nome_arq_entrada){
	FILE *arq_entrada;	// Arquivo texto de entrada
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
    return;
}
int** aloca_arestas(int nArestas){
	arestas = (int**) calloc(nArestas, sizeof(int*));
    for (int i = 0; i < nArestas; i++)
        arestas[i] = (int*) calloc(2, sizeof(int));
    return arestas;
}
void conta_arestas(){
	nArestas = 0;
	#pragma omp parallel for reduction(+:nArestas)
	for (int i = 0; i < nVertices; i++)
		for (int j = 0; j < nVertices; j++)
			if (matAdj[i][j] != 0){
				nArestas++;
			}
    return;
}

/* int** converte(int** matAdj, int nVertices, int nArestas) {
    arestas = (int**) calloc(nArestas, sizeof(int*));
    int local_k = 0;
    int* local_buffer = (int*) calloc(2 * nVertices, sizeof(int));

    #pragma omp parallel shared(arestas)
        // int* local_buffer = (int*) calloc(2 * nVertices, sizeof(int));
        if (!local_buffer) {
            #pragma omp atomic write
            local_k = -1;
        }

        #pragma omp for reduction(+: local_k)
        for (int i = 0; i < nVertices; i++) {
            for (int j = 0; j < nVertices; j++) {
                if (matAdj[i][j] != 0) {
                    if (local_k >= 0) {
                        local_buffer[local_k * 2] = i;
                        local_buffer[local_k * 2 + 1] = j;
                        local_k++;
                    }
                }
            }
        }

        if (local_k >= 0) {
            #pragma omp critical
            {
                memcpy(&arestas[local_k - 1], local_buffer, local_k * 2 * sizeof(int));
                local_k += 2;
            }
        }

        free(local_buffer);

    return arestas;
} */

int** converte(int** matAdj, int nVertices, int nArestas) {
    int** arestas_local = (int**) calloc(nArestas, sizeof(int*));
    int local_k = 0;
    int* local_buffer = (int*) calloc(2 * nVertices, sizeof(int));

    if (!local_buffer) 
        local_k = -1;
        
    else {
        #pragma omp parallel shared(arestas_local)
            #pragma omp for reduction(+: local_k)
            for (int i = 0; i < nVertices; i++) {
                for (int j = 0; j < nVertices; j++) {
                    if (matAdj[i][j] != 0) {
                        local_buffer[local_k * 2] = i;
                        local_buffer[local_k * 2 + 1] = j;
                        local_k++;
                    }
                }
            }

            #pragma omp critical
                memcpy(&arestas_local[local_k - 1], local_buffer, local_k * 2 * sizeof(int));
    }

    free(local_buffer);
    return arestas_local;
}


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
    return;
}
int main(int argc, char** argv){
	char nome_arq_entrada[100],
		  nome_arq_saida[100] ;

	if(argc != 3){
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
	arestas = aloca_arestas(nArestas);
	tini = omp_get_wtime();
	arestas = converte(matAdj, nVertices, nArestas);
	tfin = omp_get_wtime();
	double tempo2 = tfin - tini;
	double tempo = tempo1 + tempo2;
	printf("Tempo: %f + %f = %f s\n", tempo1, tempo2, tempo);
	finaliza(nome_arq_saida);
	return 0 ;
}