#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0

// * -----------------------------------------------------------------------------
// * Variáveis globais utilizadas como parâmetro de execução do programa.
// * -----------------------------------------------------------------------------
int debug;
int debug_caminhos;
int alvo = 0;

// * -----------------------------------------------------------------------------
// * Estrutura de dados básicas para representar em memória o problema tratado.
// * -----------------------------------------------------------------------------
struct problema {
    int tamanho;
    int** elementos;
};

struct nodo {
    int indice;
    int valor;
};

struct informacao_execucao {
    int valor_encontrado;
    double tempo;
    int* solucao;
};

// * -----------------------------------------------------------------------------
// * Bloco de funções básicas para a implementação do método GVNS
// * -----------------------------------------------------------------------------
int calcular_custo(struct problema, int*);
void construir_solucao(struct problema, float, float, int*);
void encontrar_melhor_vizinho(struct problema p, int* solucao_inicial, int vizinhanca, int* solucao);
void vnd(struct problema, int, int*, int*);
void gerar_vizinho_aleatorio(struct problema, int, int*, int*);
void gvns(struct problema, int, int, int*, int*);

// * -----------------------------------------------------------------------------
// * Bloco de funções que implementam os movimentos de exploração de vizinhança.
// * -----------------------------------------------------------------------------
void realizar_random_double_bridge(struct problema, int*, int*);
void realizar_swap(struct problema, int*, int*);
void realizar_swap_2opt(struct problema p, int*, int, int, int*);
void realizar_insercao(struct problema, int*, int*);
void realizar_2opt(struct problema, int*, int*);
void realizar_oropt2(struct problema, int*, int*);
void realizar_oropt3(struct problema, int*, int*);
void realizar_path_relinking(struct problema, int*, int*, int*);
int localizar_elemento(int*, int, int, int);
void realizar_swap_restrito(struct problema, int*, int*, int*);

// * -----------------------------------------------------------------------------
// * Bloco de funções auxiliares.
// * -----------------------------------------------------------------------------
void selection_sort(struct nodo *, int);
void copiar_solucao(int, int*, int*);
int* inicializar_solucao(int, int*);
int rnd(int, int);
void ler_arquivo(struct problema*, char[20]);
void imprimir_solucao(int, int*);
void linha();


/*
 * Function: main
 * -----------------------------------------------------------------------------
 *   Ponto de entrada da execução do programa. O programa aceita os seguintes 
 *   parâmetros (que deverão ser passados via linha de comando).
 *
 *   arquivo: arquivo que serão analisado.
 *   iteracoes: quantidade de iterações que serão realizadas pelo método GVNS.
 *   vizinhancas: quantidade de vizinhanças que serão exploradas pelo método GVNS.
 *   valor limite: 5.
 *   construcao_aleatoria: 1 indica que será utilizada uma construção aleatória e 0
 *   indica uma solução gulosa.
 *   execucoes: quantidade de execucoes do método GVNS.
 *   debug: indica se o programa será executado em modo debug imprimindo
 *   informações relevantes para analise e identificação de defeitos.
 *
 *   Ao término da execução do programa a seguinte saida é exibida na tela (caso
 *   o programa não esteja em modo debug):
 *   <NOME_ARQUIVO>;<MELHOR_VALOR>;<TEMPO_MELHOR_VALOR>;<PIOR_VALOR>;<TEMPO_PIOR_VALOR>;<MEDIA_VALOR>;<TEMPO_MEDIA>
 *
 *   <NOME_ARQUIVO>: no do arquivo que foi analisado.
 *   <MELHOR_VALOR>: melhor solução encontrada para o problema.
 *   <TEMPO_MELHOR_VALOR>: tempo em segundos demorado para execução que encontrou
 *   a melhor valor para o problema.
 *   <PIOR_VALOR>: pior solução encontrada para o problema.
 *   <TEMPO_PIOR_VALOR>: tempo em segundos demorado para execução que encontrou
 *   a pior solução para o problema.
 *   <MEDIA_VALOR>: média das soluções encontradas para o problema.
 *   <TEMPO_MEDIA>: tempo médio em segundos demorado para as execuções dos métodos.
 *
 *   Exemplo:
 *   teste.txt;560;0.03;800;0.05;700;0.06
 */
int main(int argc, char *argv[]) {
    int* solucao;
    int iteracoes;
    int vizinhancas;
    int construcao_aleatoria;
    int execucoes;
    clock_t inicio;
    struct problema p;
    struct informacao_execucao* informacoes_execucao;
    
    
   if(argc >= 7) {
       //lendo o arquivo da instância
       ler_arquivo(&p, argv[1]);
        
       iteracoes = atoi(argv[2]);
       vizinhancas = atoi(argv[3]);
       construcao_aleatoria = atoi(argv[4]);
       execucoes = atoi(argv[5]);
       
       debug = atoi(argv[6]);
       debug_caminhos = debug;
       
       if(argc == 8) {
           alvo = atoi(argv[7]);
       }
    } else {
        //-- configurações de teste
        ler_arquivo(&p, "/Users/gleissonassis/Dropbox/Mestrado/Implementações/minima-latencia/grasp-minimalatencia/instancias/40_1_100_1000.txt");
        
        iteracoes = 1000;
        vizinhancas = 5;
        construcao_aleatoria = 1;
        execucoes = 100;
        debug = 0;
        debug_caminhos = 0;
        alvo = 3481;
    }
    
    informacoes_execucao = (struct informacao_execucao*) malloc(execucoes * sizeof(struct informacao_execucao));
    
    for(int i = 0; i < execucoes; i++) {
        srand(i);
        
        inicio = clock();
        
        solucao = inicializar_solucao(p.tamanho, NULL);
        if(construcao_aleatoria) {
            construir_solucao(p, 1, 1, solucao);
        } else {
            construir_solucao(p, 0.0001, 0.0001, solucao);
        }
        gvns(p, iteracoes, vizinhancas, solucao, solucao);
        
        informacoes_execucao[i].tempo = (double)(clock() - inicio) / CLOCKS_PER_SEC;
        informacoes_execucao[i].valor_encontrado = calcular_custo(p, solucao);
        informacoes_execucao[i].solucao = inicializar_solucao(p.tamanho, solucao);
        
        free(solucao);
    }
    
    long total = 0;
    double total_execucao = 0;
    int melhor_valor = INT_MAX;
    int pior_valor = 0;
    double tempo_melhor = 0;
    double tempo_pior = 0;
    
    for(int i = 0; i < execucoes; i++) {
        if(debug) {
            printf("Execucao: %d\n", i);
            printf("Valor: %d\n", informacoes_execucao[i].valor_encontrado);
            printf("Tempo: %.2fs\n", informacoes_execucao[i].tempo);
            imprimir_solucao(p.tamanho, informacoes_execucao[i].solucao);
            linha();
        }
        
        total += informacoes_execucao[i].valor_encontrado;
        total_execucao += informacoes_execucao[i].tempo;
        
        if(melhor_valor >= informacoes_execucao[i].valor_encontrado) {
            melhor_valor = informacoes_execucao[i].valor_encontrado;
            tempo_melhor = informacoes_execucao[i].tempo;
        }
        
        if(informacoes_execucao[i].valor_encontrado >= pior_valor) {
            pior_valor = informacoes_execucao[i].valor_encontrado;
            tempo_pior = informacoes_execucao[i].tempo;
        }
        
        if(alvo > 0) {
            printf("%s;%d;%d;%.2f\n", argv[1], i, informacoes_execucao[i].valor_encontrado, informacoes_execucao[i].tempo);
        }
        
        free(informacoes_execucao[i].solucao);
    }
    
    printf("%s;%d;%.2f;%d;%.2f;%.2f;%.2f\n", argv[1], melhor_valor, tempo_melhor, pior_valor, tempo_pior, (double)(total / execucoes), (double)(total_execucao / execucoes));
    
    free(p.elementos);
    free(informacoes_execucao);
    
    return 0;
}

// * -----------------------------------------------------------------------------
// * Bloco de funções básicas para a implementação do método GVNS
// * -----------------------------------------------------------------------------

/*
 * Function: calcular_custo
 * -----------------------------------------------------------------------------
 *   Calcula o custo de uma solução.
 *
 *   p: estrutura de dados representando o problema.
 *   solucao: solução que terá o custo avaliado.
 */
int calcular_custo(struct problema p, int* solucao) {
    int custo = 0;
    int i;
    if(!solucao) {
        printf("!!!!!!");
    }
    for(i = 0; i < p.tamanho; i++) {
        custo += p.elementos[solucao[i]][solucao[i + 1]] * (p.tamanho - i);
    }
    
    return custo;
}

/*
 * Function: construir_solucao
 * -----------------------------------------------------------------------------
 *   Constroi um solução viável para o problema. A construção pode ser gulosa
 *   ou aleatória, dependendo dos parâmetros informados como percentual_inicial
 *   e percentual_final.
 *
 *   p: estrutura de dados representando o problema.
 *   percentual_inicial: indica o percentual inicial de elementos que serão
 *   analisados para a construção do caminho.
 *   percentual_final: indica o percentual final de elementos que serão
 *   analisados para a construção do caminho.
 *   solucao: solucao gerada.
 */
void construir_solucao(struct problema p, float percentual_inicial, float percentual_final, int* solucao) {
    int iv, indice_selecionado, indice_selecionado2;
    int *inserido;
    struct nodo *vizinhos;
    int numero_candidatos;
    float taxa_crescimento, percentual_atual;
    
    //estabelecendo o numero de candidatos a entrar no solucao
    numero_candidatos = ceil(percentual_inicial * p.tamanho);
    
    taxa_crescimento = (percentual_final - percentual_inicial) / p.tamanho;
    
    percentual_atual = percentual_inicial + taxa_crescimento;
    
    //alocando memoria para o array que ira informar se um elemento
    //ja foi inserido no solucao ou nao e inicializando os valores
    inserido = malloc(p.tamanho * sizeof(int));
    for(int i = 0; i < p.tamanho; i++) {
        inserido[i] = FALSE;
    }
    
    //alocando memoria para o array que irá armazenar informações sobre a vizinhança
    //do elemento atual
    vizinhos = (struct nodo*) malloc((p.tamanho) * sizeof(struct nodo));
    
    solucao[0] = 0;
    inserido[0] = TRUE;
    
    for(int i = 0; i < p.tamanho; i++) {
        //indice do vizinho atual;
        iv = 0;
        
        //construindo a lista de vizinhos e seus respectivos valores
        for(int j = 0; j < p.tamanho; j++) {
            //nao pode ser selecionado um elemento que ja esteja no solucao
            if(!inserido[j]) {
                vizinhos[iv].indice = j;
                vizinhos[iv].valor = p.elementos[i][j];
                
                iv++;
            }
        }
        
        if(iv == 0) {
            solucao[i + 1] = 0;
            //printf("Vertice inserido no solucao: %d\n", solucao[i + 1]);
        } else {
            //ordenando a lista de vizinhos por custo da aresta
            selection_sort(vizinhos, iv);
            
            //selecionando um elemento aleatorio para entrar no solucao
            do {
                if(numero_candidatos > iv) {
                    indice_selecionado = rand() % iv;
                    indice_selecionado2 = rand() % iv;
                } else {
                    indice_selecionado = rand() % numero_candidatos;
                    indice_selecionado2 = rand() % numero_candidatos;
                }
                
                if(!inserido[vizinhos[indice_selecionado2].indice] && vizinhos[indice_selecionado].valor > vizinhos[indice_selecionado2].valor) {
                    indice_selecionado = indice_selecionado2;
                }
            } while(inserido[vizinhos[indice_selecionado].indice] == TRUE);
            solucao[i + 1] = vizinhos[indice_selecionado].indice;
            inserido[vizinhos[indice_selecionado].indice] = TRUE;
            
            //printf("Vertice inserido no solucao: %d\n", solucao[i + 1]);
            
            numero_candidatos = ceil(percentual_atual * p.tamanho);
            percentual_atual += taxa_crescimento;
            
        }
    }
    
    //liberando memoria alocada para os arrays
    free(inserido);
    free(vizinhos);
}

/*
 * Function: encontrar_melhor_vizinho
 * -----------------------------------------------------------------------------
 *   Constroi um solução viável para o problema. A construção pode ser gulosa
 *   ou aleatória, dependendo dos parâmetros informados como percentual_inicial
 *   e percentual_final.
 *
 *   p: estrutura de dados representando o problema.
 *   solucao_inicial: solução inicial que terá a vizinhaça explorada.
 *   vizinhanca: número da vizinhança que será avaliada.
 *   solucao_resultado: melhor vizinho encontrado ao explorar a vizinhança
 *   informada.
 */
void encontrar_melhor_vizinho(struct problema p, int* solucao_inicial, int vizinhanca, int* solucao_resultado) {
    switch (vizinhanca) {
        case 0:
            realizar_swap(p, solucao_inicial, solucao_resultado);
            break;
        case 1:
            realizar_2opt(p, solucao_inicial, solucao_resultado);
            break;
        case 2:
            realizar_insercao(p, solucao_inicial, solucao_resultado);
            break;
        case 3:
            realizar_oropt2(p, solucao_inicial, solucao_resultado);
            break;
        case 4:
            realizar_oropt3(p, solucao_inicial, solucao_resultado);
            break;
        default:
            break;
    }
}

/*
 * Function: vnd
 * -----------------------------------------------------------------------------
 *   O método VND faz uma busca local em todas as vizinhanças informadas.
 *
 *   Princípios básicos:
 *   1) Um ótimo local com relação a uma vizinhança não necessariamente
 *      corresponde a um ótimo com relação a outra vizinhança.
 *   2) Um ótimo global corresponde a um ótimo local para todas as estruturas
 *      de vizinhança
 *   3) Para muitos problemas, ótimos locais com relação a uma vizinhança são
 *      relativamente próximos.
 *
 *   Referência:
 *   http://www.decom.ufop.br/marcone/Disciplinas/InteligenciaComputacional/VNS.ppt
 *
 *   p: estrutura de dados representando o problema.
 *   vizinhancas: número de vizinhanças que serão exploradas.
 *   solucao_inicial: solução inicial que terá a vizinhaça explorada.
 *   solucao_resultado: melhor vizinho encontrado ao explorar as vizinhanças
 *   informadas.
 */
void vnd(struct problema p, int vizinhancas, int* solucao_inicial, int* solucao_resultado) {
    int custo = INT_MAX;
    int custo_tmp = 0;
    
    copiar_solucao(p.tamanho, solucao_inicial, solucao_resultado);
    int* solucao_tmp = inicializar_solucao(p.tamanho, solucao_inicial);
    
    custo = calcular_custo(p, solucao_inicial);
    
    int vizinhanca = 0;
    
    while (vizinhanca < vizinhancas) {
        if(custo <= alvo) {
            return;
        }
        
        if(debug) {
            printf("Iniciando a exploração da vizinhança: %d\n", vizinhanca);
        }
        
        encontrar_melhor_vizinho(p, solucao_resultado, vizinhanca, solucao_tmp);
        
        custo_tmp = calcular_custo(p, solucao_tmp);
        
        if(custo_tmp < custo) {
            copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
            custo = custo_tmp;
            
            if(debug) {
                printf("VND - Custo melhorado (v=%d): %d\n", vizinhanca, custo);
                imprimir_solucao(p.tamanho, solucao_resultado);
            }
            
            vizinhanca = 0;
        } else {
            vizinhanca++;
        }
    }
    
    free(solucao_tmp);
}


/*
 * Function: gerar_vizinho_aleatorio
 * -----------------------------------------------------------------------------
 *   Função que abala uma solução com o objetivo de explorar de forma melhor
 *   o espaço de soluções.
 *
 *   p: estrutura de dados representando o problema.
 *   vizinhanca: vizinhança que será utilizada para gerar uma nova solução.
 *   solucao: solução que terá a vizinhaça explorada.
 *   solucao_resultado: melhor vizinho encontrado ao explorar as vizinhanças
 *   informadas.
 */
void gerar_vizinho_aleatorio(struct problema p, int vizinhanca, int* solucao, int* solucao_resultado) {
    int tmp1;
    int tmp2;
    int tmp3;
    int i;
    int j;
    
    copiar_solucao(p.tamanho, solucao, solucao_resultado);
    
    switch (vizinhanca) {
        case 0:
            i = rnd(1, p.tamanho - 1);
            j = rnd(1, p.tamanho - 1);
            
            if(debug) {
                printf("swap aleatorio entre %d e %d\n", i, j);
            }
            
            tmp1 = solucao_resultado[i];
            solucao_resultado[i] = solucao_resultado[j];
            solucao_resultado[j] = tmp1;
            break;
        case 1:
            i = rnd(1, p.tamanho / 2);
            j = rnd(i + 2, p.tamanho - 1);
            
            if(debug) {
                printf("2opt aleatorio entre %d e %d\n", i, j);
            }
            
            realizar_swap_2opt(p, solucao_resultado, i, j, solucao_resultado);
            break;
        case 2:
            i = rnd(1, p.tamanho - 2);
            do{
                j = rnd(i, p.tamanho - 1);
            }while(i == j);
            
            if(debug) {
                printf("insercao aleatoria entre %d e %d\n", i, j);
            }
            
            tmp1 = solucao_resultado[i];
            
            for(int k = i; k <= j; k++) {
                solucao_resultado[k] = solucao_resultado[k + 1];
            }
            solucao_resultado[j] = tmp1;
            break;
        case 3:
            i = rnd(1, p.tamanho / 2);
            j = rnd(i + 2, p.tamanho - 2);
            
            tmp1 = solucao_resultado[i];
            tmp2 = solucao_resultado[i + 1];
            
            if(debug) {
                printf("or opt2 aleatoria entre %d e %d\n", i, j);
            }
            
            solucao_resultado[i] = solucao_resultado[j];
            solucao_resultado[i + 1] = solucao_resultado[j + 1];
            solucao_resultado[j] = tmp1;
            solucao_resultado[j + 1] = tmp2;
            break;
        case 4:
            i = rnd(1, (p.tamanho-1) / 2);
            j = rnd(i + 3, p.tamanho - 3);
            
            tmp1 = solucao_resultado[i];
            tmp2 = solucao_resultado[i + 1];
            tmp3 = solucao_resultado[i + 2];
            
            if(debug) {
                printf("or opt3 aleatoria entre %d e %d\n", i, j);
            }
            
            solucao_resultado[i] = solucao_resultado[j];
            solucao_resultado[i + 1] = solucao_resultado[j + 1];
            solucao_resultado[i + 2] = solucao_resultado[j + 2];
            solucao_resultado[j] = tmp1;
            solucao_resultado[j + 1] = tmp2;
            solucao_resultado[j + 2] = tmp3;
            break;
        default:
            break;
    }
}

/*
 * Function: vns
 * -----------------------------------------------------------------------------
 *   O método General VNS (GVNS) realiza busca local do VNS feita pelo VND.
 *
 *   1) Proposto por Nenad Mladenovic & Pierre Hansen em 1997
 *   2) Metaheurísticas de busca local que explora o espaço de soluções através
 *   de trocas sistemáticas de estruturas de vizinhança.
 *   3) Explora vizinhanças gradativamente mais “distantes”
 *   4) Focaliza a busca em torno de uma nova solução somente se um movimento
 *   de melhora é realizado.
 *
 *   Referência:
 *   http://www.decom.ufop.br/marcone/Disciplinas/InteligenciaComputacional/VNS.ppt
 *
 *   p: estrutura de dados representando o problema.
 *   iteracoes: número de iterações para o método.
 *   vizinhancas: número de vizinhanças que serão exploradas.
 *   solucao_inicial: solução inicial que terá a vizinhaça explorada.
 *   solucao_resultado: melhor vizinho encontrado ao explorar as vizinhanças
 *   informadas.
 */
void gvns(struct problema p, int iteracoes, int vizinhancas, int* solucao_inicial, int* solucao_resultado) {
    int custo = INT_MAX;
    int custo_tmp = 0;
    int vizinhanca = 0;
    
    int* solucao_tmp = inicializar_solucao(p.tamanho, solucao_inicial);
    copiar_solucao(p.tamanho, solucao_inicial, solucao_resultado);
    
    for(int i = 0; i < iteracoes; i++) {
        if(debug) {
            printf("Iniciando o processo na iteração %d. Melhor custo %d", i, custo);
            linha();
        }
        
        vizinhanca = 0;
        
        while (vizinhanca <= vizinhancas) {
            if(custo <= alvo) {
                return;
            }
            gerar_vizinho_aleatorio(p, vizinhanca, solucao_resultado, solucao_tmp);
            
            vnd(p, vizinhancas, solucao_tmp, solucao_tmp);
            
            custo_tmp = calcular_custo(p, solucao_tmp);
            
            if(custo_tmp < custo) {
                copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
                custo = custo_tmp;
                
                if(debug) {
                    printf("Custo melhorado (GVNS v=%d): %d\n", vizinhanca, custo);
                }
                
                vizinhanca = 0;
            } else {
                if(custo_tmp > custo) {
                    realizar_path_relinking(p, solucao_tmp, solucao_resultado, solucao_tmp);
                    custo_tmp = calcular_custo(p, solucao_tmp);
                
                    if(custo_tmp < custo) {
                        copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
                        custo = custo_tmp;
                    
                        if(debug) {
                            printf("Custo melhorado (PATH): %d\n", custo);
                        }
                    
                        vizinhanca = 0;
                    } else {
                        vizinhanca++;
                    }
                } else {
                    vizinhanca++;
                }
            }
        }
    }
}

// * -----------------------------------------------------------------------------
// * Bloco de funções que implementam os movimentos de exploração de vizinhança.
// * -----------------------------------------------------------------------------

/*
 * Function: realizar_random_double_bridge
 * -----------------------------------------------------------------------------
 *   O movimento double-bridge consiste em remover 4 arcos e reconectar o
 *   caminho gerando uma nova solução viável para o problema. Ao invés de
 *   selecionar arcos fixos (geralmente quebrando a solução em 4 partes iguais),
 *   arcos aleatórios são selecionados, visto que essa função é utilizada como
 *   shake do método GVNS.
 *
 *   p: estrutura de dados representando o problema
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 */
void realizar_random_double_bridge(struct problema p, int* solucao, int* solucao_resultado) {
    int i, j;
    int tmp1, tmp2;
    
    i = rnd(2, p.tamanho / 2);
    j = rnd(i + 2, p.tamanho - 1);
    
    copiar_solucao(p.tamanho, solucao, solucao_resultado);
    
    tmp1 = solucao_resultado[i];
    tmp2 = solucao_resultado[i - 1];
    
    solucao_resultado[i] = solucao_resultado[j];
    solucao_resultado[i - 1] = solucao_resultado[j - 1];
    solucao_resultado[j] = tmp1;
    solucao_resultado[j - 1] = tmp2;
}

/*
 * Function: realizar_swap
 * -----------------------------------------------------------------------------
 *   O movimento swap consiste em selecionar 2 elementos da solução e realizar
 *   a troca de posição.
 *
 *   p: estrutura de dados representando o problema
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 */
void realizar_swap(struct problema p, int* solucao, int* solucao_resultado) {
    int i,j;
    int custo, custo_tmp, custo_inicial;
    int *solucao_tmp;
    int *solucao_inicial;
    int tmp;
    
    custo_inicial = custo = calcular_custo(p, solucao);
    
    solucao_tmp = inicializar_solucao(p.tamanho, NULL);
    solucao_inicial = inicializar_solucao(p.tamanho, solucao);
    
    
    if(debug && debug_caminhos) {
        printf("\nTentando localizar melhor vizinho na vizinhanca swap\n");
        imprimir_solucao(p.tamanho, solucao);
        linha();
    }
    
    for(i = 1; i < p.tamanho; i++) {
        for(j = i + 1; j < p.tamanho; j++) {
            if(custo <= alvo) {
                return;
            }
            
            copiar_solucao(p.tamanho, solucao_inicial, solucao_tmp);
            tmp = solucao_tmp[i];
            solucao_tmp[i] = solucao_tmp[j];
            solucao_tmp[j] = tmp;
            
            custo_tmp = calcular_custo(p, solucao_tmp);
            
            if(debug && debug_caminhos) {
                imprimir_solucao(p.tamanho, solucao_tmp);
                printf("de %d para %d\n", custo_inicial, custo_tmp);
            }
            
            
            if(custo_tmp < custo) {
                custo = custo_tmp;
                copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
            }
        }
    }
    
    free(solucao_tmp);
    free(solucao_inicial);
}

/*
 * Function: realizar_insercao
 * -----------------------------------------------------------------------------
 *   Implementação da estratégia inserção que consiste basicamente em inserir
 *   um nodo entre outros 2 nodos explorando a vizinhaça de uma solução.
 *
 *   p: estrutura de dados representando o problema.
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 */
void realizar_insercao(struct problema p, int* solucao, int* solucao_resultado) {
    int custo, custo_tmp, custo_inicial;
    int* solucao_tmp;
    int* solucao_inicial;
    int tmp;
    
    custo_inicial = custo = calcular_custo(p, solucao);
    
    solucao_inicial = inicializar_solucao(p.tamanho, solucao);
    
    if(debug && debug_caminhos) {
        printf("\nTentando localizar melhor vizinho na vizinhanca inserção\n");
        imprimir_solucao(p.tamanho, solucao);
        linha();
    }
    
    solucao_resultado = inicializar_solucao(p.tamanho, solucao);
    solucao_tmp = inicializar_solucao(p.tamanho, solucao);
    
    for(int i = 1; i < p.tamanho; i++) {
        copiar_solucao(p.tamanho, solucao_inicial, solucao_tmp);
        
        for(int j = i; j < p.tamanho - 1; j++) {
            if(custo <= alvo) {
                return;
            }
            
            tmp = solucao_tmp[j];
            solucao_tmp[j] = solucao_tmp[j + 1];
            solucao_tmp[j + 1] = tmp;
            custo_tmp = calcular_custo(p, solucao_tmp);
            
            if(debug && debug_caminhos) {
                imprimir_solucao(p.tamanho, solucao_tmp);
                printf("de %d para %d\n", custo_inicial, custo_tmp);
            }
            
            if(custo_tmp < custo) {
                custo = custo_tmp;
                copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
            }
        }
    }
    
    free(solucao_tmp);
    free(solucao_inicial);
}

/*
 * Function: realizar_2opt
 * -----------------------------------------------------------------------------
 *   O método consiste em remover 2 arcos não adjacentes e outros 2 são
 *   inseridos, visando gerar vizinhos de uma dada solução.
 *
 *   p: estrutura de dados representando o problema
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 */
void realizar_2opt(struct problema p, int* solucao, int* solucao_resultado) {
    int i,j;
    int custo, custo_tmp, custo_inicial;
    int* solucao_tmp;
    int* solucao_inicial;
    
    custo_inicial = custo = calcular_custo(p, solucao);
    
    solucao_tmp = inicializar_solucao(p.tamanho, NULL);
    solucao_inicial = inicializar_solucao(p.tamanho, solucao);
    
    
    if(debug && debug_caminhos) {
        printf("\nTentando localizar melhor vizinho na vizinhanca 2-opt\n");
        imprimir_solucao(p.tamanho, solucao);
        linha();
    }
    
    for(i = 1; i < p.tamanho - 1; i++) {
        for(j = i + 1; j < p.tamanho; j++) {
            if(custo <= alvo) {
                return;
            }
            
            realizar_swap_2opt(p, solucao_inicial, i, j, solucao_tmp);
            
            custo_tmp = calcular_custo(p, solucao_tmp);
            
            if(debug && debug_caminhos) {
                imprimir_solucao(p.tamanho, solucao_tmp);
                printf("de %d para %d\n", custo_inicial, custo_tmp);
            }
            
            if(custo_tmp < custo) {
                custo = custo_tmp;
                copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
            }
        }
    }
    
    free(solucao_tmp);
    free(solucao_inicial);
}

/*
 * Function: realizar_swap_2opt
 * -----------------------------------------------------------------------------
 *   Método auxiliar para a execução da exploração da vizinhança do movimento
 *   2-opt.
 *
 *   p: estrutura de dados representando o problema.
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 */
void realizar_swap_2opt(struct problema p, int* solucao, int i, int k, int* solucao_resultado) {
    
    int* solucao_tmp = inicializar_solucao(p.tamanho, solucao);
    
    for(int c = 0; c <= i - 1; c++) {
        solucao_tmp[c] = solucao[c];
    }
    
    int dec = 0;
    for(int c = i; c <= k; c++) {
        solucao_tmp[c] = solucao[k - dec];
        dec++;
    }
    
    for(int c = k + 1; c < p.tamanho - 1; c++) {
        solucao_tmp[c] = solucao[c];
    }
    
    copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
    free(solucao_tmp);
}

/*
 * Function: realizar_oropt2
 * -----------------------------------------------------------------------------
 *   O movimento or opt2 consiste em selecionar 2 pares e trocar suas posições.
 *
 *   p: estrutura de dados representando o problema
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 */
void realizar_oropt2(struct problema p, int* solucao, int* solucao_resultado) {
    int i,j;
    int custo, custo_tmp, custo_inicial;
    int* solucao_tmp;
    int* solucao_inicial;
    int tmp1, tmp2;
    
    custo_inicial = custo = calcular_custo(p, solucao);
    
    solucao_tmp = inicializar_solucao(p.tamanho, NULL);
    solucao_inicial = inicializar_solucao(p.tamanho, solucao);
    
    if(debug && debug_caminhos) {
        printf("\nTentando localizar melhor vizinho na vizinhanca or2opt\n");
        imprimir_solucao(p.tamanho, solucao);
        linha();
    }
    
    for(i = 1; i < p.tamanho - 1; i++) {
        copiar_solucao(p.tamanho, solucao_inicial, solucao_tmp);
        
        tmp1 = solucao_tmp[i];
        tmp2 = solucao_tmp[i + 1];
        
        for(j = i + 2; j < p.tamanho - 1; j++) {
            if(custo <= alvo) {
                return;
            }
            
            solucao_tmp[j - 2] = solucao_tmp[j];
            solucao_tmp[j] = tmp2;
            solucao_tmp[j - 1] = tmp1;
            
            custo_tmp = calcular_custo(p, solucao_tmp);
            
            if(debug && debug_caminhos) {
                imprimir_solucao(p.tamanho, solucao_tmp);
                printf("de %d para %d\n", custo_inicial, custo_tmp);
            }
            
            if(custo_tmp < custo) {
                custo = custo_tmp;
                copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
            }
        }
    }
    
    free(solucao_tmp);
    free(solucao_inicial);
}

/*
 * Function: realizar_oropt3
 * -----------------------------------------------------------------------------
 *   O movimento or opt3 consiste em selecionar 2 trios e trocar suas posições.
 *
 *   p: estrutura de dados representando o problema
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 */
void realizar_oropt3(struct problema p, int* solucao, int* solucao_resultado) {
    int i,j;
    int custo, custo_tmp, custo_inicial;
    int *solucao_tmp;
    int *solucao_inicial;
    int tmp1, tmp2, tmp3;
    
    custo_inicial = custo = calcular_custo(p, solucao);
    
    solucao_tmp = inicializar_solucao(p.tamanho, NULL);
    solucao_inicial = inicializar_solucao(p.tamanho, solucao);
    
    if(debug && debug_caminhos) {
        printf("\nTentando localizar melhor vizinho na vizinhanca or3opt\n");
        imprimir_solucao(p.tamanho, solucao);
        linha();
    }
    
    for(i = 1; i < p.tamanho - 1; i++) {
        copiar_solucao(p.tamanho, solucao_inicial, solucao_tmp);
        
        tmp1 = solucao_tmp[i];
        tmp2 = solucao_tmp[i + 1];
        tmp3 = solucao_tmp[i + 2];
        
        for(j = i + 3; j < p.tamanho - 2; j++) {
            if(custo <= alvo) {
                return;
            }
            
            solucao_tmp[j - 3] = solucao_tmp[j];
            solucao_tmp[j] = tmp3;
            solucao_tmp[j - 1] = tmp2;
            solucao_tmp[j - 2] = tmp1;
        
            custo_tmp = calcular_custo(p, solucao_tmp);
            
            if(debug && debug_caminhos) {
                imprimir_solucao(p.tamanho, solucao_tmp);
                printf("de %d para %d\n", custo_inicial, custo_tmp);
            }
            
            if(custo_tmp < custo) {
                custo = custo_tmp;
                copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
            }
        }
    }
    
    free(solucao_tmp);
    free(solucao_inicial);
}

/*
 * Function: realizar_path_relinking
 * -----------------------------------------------------------------------------
 *   Implementação da estratégia Path Relinking que consiste basicamente em
 *   gerar soluções intermediárias entre 2 caminhos, após cada solução
 *   intermediária gerada contendo componentes da solução final uma busca local
 *   é realizada, explorando a vizinhanção baseada em trocas simples entre 2
 *   elementos, tentando melhorar a solução.
 *
 *   p: estrutura de dados representando o problema.
 *   origem: caminho inicial.
 *   destino: caminho de destino.
 *   solucao_resultado: a melhor solução encontrada após a execução do path
 *   relinking.
 */
void realizar_path_relinking(struct problema p, int* origem, int* destino, int *solucao_resultado) {
    int custo;
    int custo_tmp;
    int pos;
    int tmp;
    int* origem_tmp;
    int* solucao_swap_tmp;
    int* lista_restrita;
    
    
    solucao_swap_tmp = inicializar_solucao(p.tamanho, NULL);
    origem_tmp = inicializar_solucao(p.tamanho, origem);
    copiar_solucao(p.tamanho, destino, solucao_resultado);
    lista_restrita = inicializar_solucao(p.tamanho, NULL);
    
    custo = calcular_custo(p, destino);
    
    for(int i = 0; i <= p.tamanho; i++) {
        lista_restrita[i] = 0;
    }
    
    custo = calcular_custo(p, origem);
    
    for(int i = 1; i < p.tamanho; i++) {
        if(destino[i] == origem_tmp[i]) {
            lista_restrita[i] = 1;
            continue;
        }
        
        pos = localizar_elemento(origem_tmp, p.tamanho, destino[i], i + 1);
        
        tmp = origem_tmp[i];
        origem_tmp[i] = origem_tmp[pos];
        origem_tmp[pos] = tmp;
        
        lista_restrita[i] = 1;
        
        do {
            if(custo <= alvo) {
                return;
            }
            
            realizar_swap_restrito(p, origem_tmp, solucao_swap_tmp, lista_restrita);
            custo_tmp = calcular_custo(p, solucao_swap_tmp);
            
            if(custo_tmp < custo) {
                custo = custo_tmp;
                copiar_solucao(p.tamanho, solucao_swap_tmp, solucao_resultado);
            }
        } while(custo_tmp < custo);
    }
    
    free(origem_tmp);
    free(solucao_swap_tmp);
}

/*
 * Function: localizar_elemento
 * -----------------------------------------------------------------------------
 *   Implementação da estratégia Path Relinking que consiste basicamente em
 *   gerar soluções intermediárias entre 2 caminhos, após cada solução
 *   intermediária gerada contendo componentes da solução final uma busca local
 *   é realizada, explorando a vizinhanção baseada em trocas simples entre 2
 *   elementos, tentando melhorar a solução.
 *
 *   solucao: solução que será analisada em busca do elemento.
 *   tamanho: tamanho da solução.
 *   valor: valor que será pesquisado na solução.
 *   posicao_inicial: posição que a busca será iniciada.
 *
 *   returns: posição do valor informado e caso não seja encontrado o valor -1
 *   é retornado.
 */
int localizar_elemento(int* solucao, int tamanho, int valor, int posicao_inicial) {
    int i = posicao_inicial;
    
    for(; i < tamanho; i++) {
        if(solucao[i] == valor) {
            return i;
        }
    }
    
    return -1;
}

/*
 * Function: realizar_swap_restrito
 * -----------------------------------------------------------------------------
 *   Método auxiliar para a execuçao do método Path Relinking. O movimento de
 *   troca deve ser realizado apenas nos elementos que não são referentes a
 *   solução destino.
 *
 *   p: estrutura de dados representando o problema.
 *   solucao: solucao que terá sua vizinhança explorada.
 *   solucao_resultado: a melhor solução encontrada após a execução do método.
 *   lista_restrita: lista de elementos que não podem ter suas posições alteradas.
 *   consiste em um vetor de inteiros onde cada posição é marcada como 0 (não é
 *   restrita, ou seja pode ser alterada) ou 1 (é restrita, ou seja, não pode
 *   ser alterada).
 */
void realizar_swap_restrito(struct problema p, int* solucao, int* solucao_resultado, int* lista_restrita) {
    int i,j;
    int custo, custo_tmp, custo_inicial;
    int *solucao_tmp;
    int *solucao_inicial;
    int tmp;
    
    custo_inicial = custo = calcular_custo(p, solucao);
    
    solucao_tmp = inicializar_solucao(p.tamanho, NULL);
    solucao_inicial = inicializar_solucao(p.tamanho, solucao);
    
    copiar_solucao(p.tamanho, solucao, solucao_resultado);
    
    if(debug && debug_caminhos) {
        printf("\nTentando localizar melhor vizinho na vizinhanca swap\n");
        imprimir_solucao(p.tamanho, solucao);
        linha();
    }
    
    for(i = 1; i < p.tamanho; i++) {
        if(lista_restrita[i]) {
            continue;
        }
        
        for(j = i + 1; j < p.tamanho; j++) {
            if(lista_restrita[j]) {
                continue;
            }
            
            copiar_solucao(p.tamanho, solucao_inicial, solucao_tmp);
            tmp = solucao_tmp[i];
            solucao_tmp[i] = solucao_tmp[j];
            solucao_tmp[j] = tmp;
            
            custo_tmp = calcular_custo(p, solucao_tmp);
            
            if(debug && debug_caminhos) {
                imprimir_solucao(p.tamanho, solucao_tmp);
                printf("de %d para %d\n", custo_inicial, custo_tmp);
            }
            
            
            if(custo_tmp < custo) {
                custo = custo_tmp;
                copiar_solucao(p.tamanho, solucao_tmp, solucao_resultado);
            }
        }
    }
    
    free(solucao_tmp);
    free(solucao_inicial);
}

// * -----------------------------------------------------------------------------
// * Bloco de funções auxiliares.
// * -----------------------------------------------------------------------------

/*
 * Function: selection_sort
 * -----------------------------------------------------------------------------
 *   Função auxiliar que implementa do método de ordenação selecion sort.
 *
 *   array: vetor que será ordenado.
 *   n: tamanho do vetor.
 */
void selection_sort(struct nodo* array, int n) {
    int i, j;
    int min;
    struct nodo temp;
    
    for(i = 0; i < n - 1; i++) {
        min=i;
        for(j = i + 1; j < n; j++) {
            if(array[j].valor < array[min].valor) {
                min = j;
            }
        }
        
        temp = array[i];
        array[i] = array[min];
        array[min] = temp;
    }
}

/*
 * Function: copiar_solucao
 * -----------------------------------------------------------------------------
 *   Função auxiliar que copia uma soluçao de origem para uma solução de destino.
 *   Os ponteiros precisam ter sido alocados previamente.
 *
 *   n: tamanho da solução.
 *   origem: solução de origem.
 *   destino: tamanho dos vetores.
 */
void copiar_solucao(int n, int* origem, int* destino) {
    int i;
    
    for(i = 0; i < n + 1; i++) {
        destino[i] = origem[i];
    }
}

/*
 * Function: inicializar_solucao
 * -----------------------------------------------------------------------------
 *   Função auxiliar que inicializa um ponteiro com uma nova solução.
 *
 *   n: tamanho da solução.
 *   solucao_base: solução que será baseada para a nova solução.
 *
 *   returns: um ponteiro alocado contendo uma soluçao base.
 */
int* inicializar_solucao(int n, int* solucao_base) {
    int* nova_solucao;
    
    nova_solucao = malloc((n + 1) * sizeof(int));
    
    if(solucao_base) {
        copiar_solucao(n, solucao_base, nova_solucao);
    }
    
    return nova_solucao;
    
}

/*
 * Function: rnd
 * -----------------------------------------------------------------------------
 *   Gera um número aleatório entre 2 inteiros.
 *
 *   min: limite inferior.
 *   max: limite superior.
 *
 *   returns um número aleatório.
 */
int rnd(int min, int max) {
    min = ceil(min);
    max = floor(max);
    
    return floor(rand() % (max + 1 - min)) + min;
}

/*
 * Function: ler_arquivo
 * -----------------------------------------------------------------------------
 *   Lê um arquivo com o formato definido e mapeia a matriz de adjacência para
 *   memória.
 *
 *   p: estrutura de dados representando o problema.
 *   arquivo: caminho físico para o arquivo que será lido.
 */
void ler_arquivo(struct problema* p, char arquivo[2000]) {
    FILE* fp;
    int t;
    
    fp = fopen(arquivo, "r");
    fscanf(fp, "%d %d\n\n", &p->tamanho, &t);
    
    //alocando espaço para a matriz de adjacencia
    p->elementos = malloc(p->tamanho * p->tamanho * sizeof(int));
    
    
    //pulando as linhas de 1s
    for(int i = 0; i < p->tamanho; i++) {
        fscanf(fp, "%d ", &t);
    }
    
    //percorrendo os elementos da matriz de ajdacencia que estao no arquivo
    for(int i = 0; i < p->tamanho; i++) {
        p->elementos[i] = malloc(p->tamanho * sizeof(int));
        
        for(int j = 0; j < p->tamanho; j++) {
            p->elementos[i][j] = 0;
            fscanf(fp, "%d ", &p->elementos[i][j]);
        }
    }
}

/*
 * Function: imprimir_solucao
 * -----------------------------------------------------------------------------
 *   Função auxiliar que imprime uma solução na tela.
 *
 *   n: tamanho da solução.
 *   origem: solução que será impressa na tela.
 */
void imprimir_solucao(int n, int* solucao) {
    int i;
    
    for(i = 0; i <= n; i++) {
        printf("%d ", solucao[i]);
    }
    printf("\n");
}

/*
 * Function: linha
 * -----------------------------------------------------------------------------
 *   Função auxiliar que imprime uma linha na tela.
 */
void linha() {
    int i;
    printf("\n");
    for(i = 0; i < 80; i++) printf("_");
    printf("\n");
}

