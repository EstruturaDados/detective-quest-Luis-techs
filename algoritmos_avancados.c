#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===================================================
// 0. DEFINIÇÕES E CONSTANTES GLOBAIS
// ===================================================

#define TAM_HASH 10

// Estrutura auxiliar para a contagem de suspeitos
typedef struct Contador {
    char nome[50];
    int qtd;
    struct Contador *prox; // Para lista ligada de contadores
} Contador;

// ===================================================
// 1. ESTRUTURAS E FUNÇÕES DA MANSÃO (ÁRVORE BINÁRIA)
// ===================================================

typedef struct Sala {
    char nome[50];
    const char *pistaTexto;   // Texto da pista
    const char *pistaSuspeito; // Suspeito associado à pista
    struct Sala *esquerda;
    struct Sala *direita;
    int coletada; // 0 (Não) / 1 (Sim)
} Sala;

Sala* criarSala(const char *nome, const char *pista, const char *suspeito) {
    Sala *novaSala = (Sala *)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        perror("Erro ao alocar memória para a sala");
        exit(EXIT_FAILURE);
    }
    strncpy(novaSala->nome, nome, 49);
    novaSala->nome[49] = '\0';
    novaSala->pistaTexto = pista;
    novaSala->pistaSuspeito = suspeito;
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    novaSala->coletada = (pista != NULL); // Se há pista, marca para ser coletada
    return novaSala;
}

void liberarMansao(Sala *raiz) {
    if (raiz == NULL) return;
    liberarMansao(raiz->esquerda);
    liberarMansao(raiz->direita);
    free(raiz);
}

// ===================================================
// 2. ESTRUTURAS E FUNÇÕES DA BST DE PISTAS
// ===================================================

typedef struct Pista {
    char texto[100];
    struct Pista *esquerda;
    struct Pista *direita;
} Pista;

Pista* criarNoPista(const char *texto) {
    Pista *novaPista = (Pista *)malloc(sizeof(Pista));
    if (novaPista == NULL) {
        perror("Erro ao alocar memória para a pista");
        exit(EXIT_FAILURE);
    }
    strncpy(novaPista->texto, texto, 99);
    novaPista->texto[99] = '\0';
    novaPista->esquerda = NULL;
    novaPista->direita = NULL;
    return novaPista;
}

Pista* inserirPistaBST(Pista *raiz, const char *novaPista) {
    if (raiz == NULL) {
        return criarNoPista(novaPista);
    }
    int comparacao = strcmp(novaPista, raiz->texto);
    if (comparacao < 0) {
        raiz->esquerda = inserirPistaBST(raiz->esquerda, novaPista);
    } else if (comparacao > 0) {
        raiz->direita = inserirPistaBST(raiz->direita, novaPista);
    }
    return raiz;
}

void emOrdem(Pista *raiz) {
    if (raiz != NULL) {
        emOrdem(raiz->esquerda);
        printf("  - %s\n", raiz->texto);
        emOrdem(raiz->direita);
    }
}

void liberarPistasBST(Pista *raiz) {
    if (raiz == NULL) return;
    liberarPistasBST(raiz->esquerda);
    liberarPistasBST(raiz->direita);
    free(raiz);
}

// ===================================================
// 3. ESTRUTURAS E FUNÇÕES DA TABELA HASH
// ===================================================

// Estrutura para um item na tabela hash (relação pista -> suspeito)
typedef struct ItemHash {
    char pista[100];
    char suspeito[50];
    struct ItemHash *prox; // Encadeamento para colisões
} ItemHash;

// Função Hash
int hash(const char *pista) {
    unsigned int soma = 0;
    for (int i = 0; pista[i] != '\0'; i++)
        soma += pista[i];
    return soma % TAM_HASH;
}

/**
 * @brief Insere uma nova relação pista -> suspeito na tabela hash.
 * @param tabela O vetor da tabela hash.
 * @param pista O texto da pista (chave).
 * @param suspeito O nome do suspeito (valor).
 */
void inserirNaHash(ItemHash *tabela[], const char *pista, const char *suspeito) {
    int indice = hash(pista);

    // Cria o novo item
    ItemHash *novoItem = (ItemHash *)malloc(sizeof(ItemHash));
    if (novoItem == NULL) {
        perror("Erro ao alocar memória para o ItemHash");
        exit(EXIT_FAILURE);
    }
    strncpy(novoItem->pista, pista, 99); novoItem->pista[99] = '\0';
    strncpy(novoItem->suspeito, suspeito, 49); novoItem->suspeito[49] = '\0';
    
    // Insere no início da lista encadeada (tratamento de colisão)
    novoItem->prox = tabela[indice];
    tabela[indice] = novoItem;
}

/**
 * @brief Exibe todas as relações pista -> suspeito armazenadas na tabela hash.
 * @param tabela O vetor da tabela hash.
 */
void exibirHash(ItemHash *tabela[]) {
    printf("\n--- TABELA HASH: RELAÇÕES PISTA -> SUSPEITO ---\n");
    int vazia = 1;
    for (int i = 0; i < TAM_HASH; i++) {
        ItemHash *atual = tabela[i];
        if (atual != NULL) {
            vazia = 0;
            printf("[%d] ", i);
            while (atual != NULL) {
                printf("'%s' -> **%s**", atual->pista, atual->suspeito);
                atual = atual->prox;
                if (atual != NULL) {
                    printf(" | ");
                }
            }
            printf("\n");
        }
    }
    if (vazia) {
        printf("A tabela hash está vazia. Nenhuma pista foi vinculada.\n");
    }
    printf("--------------------------------------------------\n");
}

/**
 * @brief Percorre a tabela hash, conta a frequência de suspeitos e identifica o mais citado.
 * @param tabela O vetor da tabela hash.
 */
void contarFrequencia(ItemHash *tabela[]) {
    Contador *head = NULL;
    Contador *maisCitado = NULL;
    int maxQtd = 0;

    // 1. Contagem de Frequência (usando lista ligada auxiliar)
    for (int i = 0; i < TAM_HASH; i++) {
        ItemHash *item = tabela[i];
        while (item != NULL) {
            // Verifica se o suspeito já está na lista de contadores
            Contador *contadorAtual = head;
            int encontrado = 0;
            while (contadorAtual != NULL) {
                if (strcmp(contadorAtual->nome, item->suspeito) == 0) {
                    contadorAtual->qtd++;
                    encontrado = 1;
                    break;
                }
                contadorAtual = contadorAtual->prox;
            }

            // Se não encontrado, adiciona o novo suspeito à lista
            if (!encontrado) {
                Contador *novoContador = (Contador *)malloc(sizeof(Contador));
                if (novoContador == NULL) {
                    perror("Erro ao alocar Contador");
                    exit(EXIT_FAILURE);
                }
                strncpy(novoContador->nome, item->suspeito, 49); novoContador->nome[49] = '\0';
                novoContador->qtd = 1;
                
                // Insere no início da lista auxiliar
                novoContador->prox = head;
                head = novoContador;
            }
            item = item->prox;
        }
    }

    // 2. Determinação do Suspeito Mais Citado
    Contador *aux = head;
    while (aux != NULL) {
        if (aux->qtd > maxQtd) {
            maxQtd = aux->qtd;
            maisCitado = aux;
        }
        aux = aux->prox;
    }
    
    // 3. Exibição do Resultado e Liberação da Lista Auxiliar
    printf("\n--- ANÁLISE DO CULPADO ---\n");
    if (maisCitado != NULL) {
        printf("🕵️ Suspeito mais citado (CULPADO PROVÁVEL): **%s** (%d pistas)\n", 
               maisCitado->nome, maisCitado->qtd);
    } else {
        printf("Não há dados suficientes para determinar o culpado.\n");
    }
    printf("--------------------------\n");

    // Liberação da lista de contadores
    Contador *liberar;
    aux = head;
    while (aux != NULL) {
        liberar = aux;
        aux = aux->prox;
        free(liberar);
    }
}

/**
 * @brief Libera toda a memória da tabela hash.
 * @param tabela O vetor da tabela hash.
 */
void liberarHash(ItemHash *tabela[]) {
    for (int i = 0; i < TAM_HASH; i++) {
        ItemHash *atual = tabela[i];
        while (atual != NULL) {
            ItemHash *prox = atual->prox;
            free(atual);
            atual = prox;
        }
        tabela[i] = NULL; // Garantir que o ponteiro na tabela é NULL
    }
}

// ===================================================
// 4. FUNÇÃO PRINCIPAL DE EXPLORAÇÃO E INTEGRAÇÃO
// ===================================================

/**
 * @brief Gerencia a navegação interativa, coleta de pistas e interação com as estruturas.
 */
void explorarSalas(Sala *hall) {
    Sala *atual = hall;
    char escolha;
    
    Pista *raizPistas = NULL;
    ItemHash *tabelaHash[TAM_HASH] = { NULL }; // Inicializa a tabela hash com NULLs
    
    printf("\n--- INÍCIO DA EXPLORAÇÃO ---\n");

    while (atual != NULL) {
        printf("\nVocê está no **%s**.\n", atual->nome);

        // A. Coleta e Integração da Pista
        if (atual->coletada == 1) {
            printf("🔍 **Pista encontrada!** Texto: \"%s\".\n", atual->pistaTexto);
            printf("Essa pista aponta para: **%s**.\n", atual->pistaSuspeito);

            // 1. Adiciona à BST para ordenação alfabética
            raizPistas = inserirPistaBST(raizPistas, atual->pistaTexto);

            // 2. Adiciona à Tabela Hash para associação Suspeito-Pista
            inserirNaHash(tabelaHash, atual->pistaTexto, atual->pistaSuspeito);

            // Marca como coletada para não repetir
            atual->coletada = 0;
        }

        // B. Verifica se é um nó-folha
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("\n--- Fim do Caminho ---\n");
            printf("Esta sala é um beco sem saída. A investigação se encerra aqui.\n");
            goto FIM_DA_INVESTIGACAO;
        }

        // C. Menu e Entrada do Usuário
        printf("Escolha (e/d), 'p' para PISTAS, 'h' para HASH ou 's' para SAIR:> ");
        
        // Limpar buffer antes de ler a nova escolha
        int c;
        while ((c = getchar()) != '\n' && c != EOF); 
        
        if (scanf(" %c", &escolha) != 1) { continue; }

        if (escolha >= 'A' && escolha <= 'Z') { escolha += ('a' - 'A'); }

        Sala *proximaSala = NULL;

        // D. Lógica de Movimentação e Opções
        switch (escolha) {
            case 'e':
                if (atual->esquerda != NULL) {
                    proximaSala = atual->esquerda;
                } else {
                    printf("Não há saída para a esquerda. Tente outra opção.\n"); continue;
                }
                break;
            case 'd':
                if (atual->direita != NULL) {
                    proximaSala = atual->direita;
                } else {
                    printf("Não há saída para a direita. Tente outra opção.\n"); continue;
                }
                break;
            case 'p':
                // Listar Pistas (BST)
                printf("\n--- LISTA DE PISTAS ENCONTRADAS (Em Ordem Alfabética) ---\n");
                if (raizPistas == NULL) {
                    printf("Nenhuma pista foi coletada ainda.\n");
                } else {
                    emOrdem(raizPistas);
                }
                printf("----------------------------------------------------------\n");
                continue;
            case 'h':
                // Ver Tabela Hash
                exibirHash(tabelaHash);
                // Análise do Culpado
                contarFrequencia(tabelaHash);
                continue;
            case 's':
                printf("\nVocê escolheu sair.\n");
                goto FIM_DA_INVESTIGACAO;
            default:
                printf("Opção inválida. Escolha 'e', 'd', 'p', 'h' ou 's'.\n");
                continue;
        }

        // Se o movimento foi válido, avança para a próxima sala
        atual = proximaSala;
    }
    
FIM_DA_INVESTIGACAO:
    printf("\n\n#############################################\n");
    printf("        RELATÓRIO FINAL DE INVESTIGAÇÃO\n");
    printf("#############################################\n");

    // 1. Pistas Coletadas
    printf("\n--- PISTAS COLETADAS ---\n");
    if (raizPistas == NULL) {
        printf("Nenhuma pista foi coletada.\n");
    } else {
        emOrdem(raizPistas);
    }
    
    // 2. Tabela Hash Completa
    exibirHash(tabelaHash);

    // 3. Suspeito Mais Citado
    contarFrequencia(tabelaHash);

    // 4. Liberação de Memória
    printf("\nProcesso de limpeza de memória...\n");
    liberarPistasBST(raizPistas);
    liberarHash(tabelaHash);
    printf("Memória da BST de pistas e Hash liberada.\n");
}


// 5. Função Principal (main)
int main() {
    printf("==========================================\n");
    printf("   🕵️ Mansão Binária | NÍVEL MESTRE (HASH)\n");
    printf("==========================================\n");

    // --- Criação da Mansão (Estrutura da Árvore Binária) ---
    // Definições de Pistas e Suspeitos
    const char *p1 = "Sino de prata com iniciais gravadas.";      const char *s1 = "Mordomo";
    const char *p2 = "A data de 1905 está rabiscada no espelho."; const char *s2 = "Mordomo";
    const char *p3 = "O cheiro de mofo esconde uma passagem.";    const char *s3 = "Jardineiro";
    const char *p4 = "Luvas de jardinagem no lixo.";              const char *s4 = "Jardineiro";
    const char *p5 = "Carta de dívida amassada.";                 const char *s5 = "Motorista";
    
    // Nível 0 (Raiz)
    Sala *hall = criarSala("Hall de Entrada", p1, s1);

    // Nível 1
    Sala *biblioteca = criarSala("Biblioteca", p2, s2);
    Sala *salaJantar = criarSala("Sala de Jantar", p5, s5); // Pista 5
    
    hall->esquerda = biblioteca;
    hall->direita = salaJantar;

    // Nível 2 - Esquerda
    Sala *escritorio = criarSala("Escritório", NULL, NULL); // Sem Pista (folha)
    Sala *jardim = criarSala("Jardim de Inverno", p4, s4); // Pista 4 (folha)
    
    biblioteca->esquerda = escritorio; 
    biblioteca->direita = jardim;      
    
    // Nível 2 - Direita
    Sala *cozinha = criarSala("Cozinha", NULL, NULL); // Sem Pista (folha)
    Sala *porao = criarSala("Porão", p3, s3); // Pista 3 (folha)
    
    salaJantar->esquerda = cozinha; 
    salaJantar->direita = porao;    
    
    // --- Início da Exploração ---
    explorarSalas(hall);

    // --- Liberação da Memória da Mansão ---
    liberarMansao(hall);

    return 0;
}
