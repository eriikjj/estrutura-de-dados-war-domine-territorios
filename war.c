#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

// --- Constantes Globais ---
#define NUM_TERRITORIOS 12
#define TAM_NOME 40
#define MISSÕES 2

// Cores / donos (apenas inteiros para simplicidade)
#define DONO_VAZIO 0
#define DONO_JOGADOR 1
#define DONO_INIMIGO_1 2
#define DONO_INIMIGO_2 3

// Estrutura de um território
typedef struct {
    char nome[TAM_NOME];
    int dono;    // cor/exército que domina
    int tropas;  // número de tropas
} Territorio;

// --- Protótipos das funções ---
Territorio* alocarMapa(int n);
void inicializarTerritorios(Territorio* mapa, int n);
void liberarMemoria(Territorio** mapa);

void exibirMenuPrincipal(void);
void exibirMapa(const Territorio* mapa, int n);
void exibirMissao(int missaoId, int parametro);

void faseDeAtaque(Territorio* mapa, int n);
void simularAtaque(Territorio* mapa, int idxOrigem, int idxDestino);

int sortearMissao(int* parametro);
int verificarVitoria(const Territorio* mapa, int n, int missaoId, int parametro);

void limparBufferEntrada(void);
int lerInteiroSeguro(int minimo, int maximo);

// --- Implementação ---

Territorio* alocarMapa(int n) {
    Territorio* m = (Territorio*) calloc(n, sizeof(Territorio));
    return m; // NULL será checado pelo chamador
}

void inicializarTerritorios(Territorio* mapa, int n) {
    // Exemplo de nomes e distribuição inicial simples
    const char* nomes[NUM_TERRITORIOS] = {
        "Amazônia", "Pará", "Marajó", "Belém",
        "Vigia", "Santarém", "Castanhal", "Cametá",
        "Bragança", "Altamira", "Capanema", "Breves"
    };

    // Distribuição de donos e tropas para iniciar
    for (int i = 0; i < n; ++i) {
        strncpy(mapa[i].nome, nomes[i % NUM_TERRITORIOS], TAM_NOME-1);
        mapa[i].nome[TAM_NOME-1] = '\0';
        // Alterna donos para criar disputa
        if (i % 3 == 0) mapa[i].dono = DONO_JOGADOR;
        else if (i % 3 == 1) mapa[i].dono = DONO_INIMIGO_1;
        else mapa[i].dono = DONO_INIMIGO_2;

        // Número inicial de tropas
        mapa[i].tropas = 1 + rand() % 6; // 1 a 6 tropas
    }
}

void liberarMemoria(Territorio** mapa) {
    if (mapa && *mapa) {
        free(*mapa);
        *mapa = NULL;
    }
}

void exibirMenuPrincipal(void) {
    printf("\n--- MENU PRINCIPAL ---\n");
    printf("1 - Fase de ataque\n");
    printf("2 - Verificar missão / status\n");
    printf("0 - Sair\n");
    printf("Escolha: ");
}

void exibirMapa(const Territorio* mapa, int n) {
    puts("\n--- MAPA ATUAL ---");
    printf("%3s | %-20s | %-8s | %s\n", "#", "TERRITÓRIO", "DONO", "TROPAS");
    printf("----+----------------------+----------+-------\n");
    for (int i = 0; i < n; ++i) {
        const char* donoStr;
        switch (mapa[i].dono) {
            case DONO_JOGADOR: donoStr = "JOGADOR"; break;
            case DONO_INIMIGO_1: donoStr = "INIMIGO A"; break;
            case DONO_INIMIGO_2: donoStr = "INIMIGO B"; break;
            default: donoStr = "NEUTRO"; break;
        }
        printf("%3d | %-20s | %-8s | %4d\n", i, mapa[i].nome, donoStr, mapa[i].tropas);
    }
}

void exibirMissao(int missaoId, int parametro) {
    printf("\n--- MISSÃO SECRETA ---\n");
    if (missaoId == 1) {
        printf("Destruir todo o exército do inimigo %d.\n", parametro);
        printf("(Ou seja: eliminar todos os territórios cujo dono == %d)\n", parametro);
    } else if (missaoId == 2) {
        printf("Conquistar ao menos %d territórios distintos.\n", parametro);
    } else {
        printf("Missão desconhecida.\n");
    }
}

void faseDeAtaque(Territorio* mapa, int n) {
    printf("\n=== FASE DE ATAQUE ===\n");
    printf("Digite o índice do território de ORIGEM (ou -1 para cancelar): ");
    int origem = lerInteiroSeguro(-1, n-1);
    if (origem == -1) {
        printf("Ataque cancelado.\n");
        return;
    }
    printf("Digite o índice do território de DESTINO: ");
    int destino = lerInteiroSeguro(0, n-1);
    if (origem == destino) {
        printf("Origem e destino iguais. Operação abortada.\n");
        return;
    }

    // Validações básicas: o território de origem deve pertencer ao jogador e ter >=2 tropas
    if (mapa[origem].dono != DONO_JOGADOR) {
        printf("Você não controla o território de origem.\n");
        return;
    }
    if (mapa[origem].tropas < 2) {
        printf("Você precisa de ao menos 2 tropas no território de origem para atacar.\n");
        return;
    }

    simularAtaque(mapa, origem, destino);
}

// Função que compara inteiros para qsort (descendente)
static int cmp_desc(const void* a, const void* b) {
    return (*(int*)b) - (*(int*)a);
}

void simularAtaque(Territorio* mapa, int idxOrigem, int idxDestino) {
    Territorio* A = &mapa[idxOrigem];
    Territorio* D = &mapa[idxDestino];

    printf("\n--- SIMULAÇÃO DE ATAQUE: %s -> %s ---\n", A->nome, D->nome);
    printf("Tropas atacante: %d | Tropas defensor: %d\n", A->tropas, D->tropas);

    // Determina quantos dados cada lado rola
    int dadosAtq = (A->tropas - 1 >= 3) ? 3 : (A->tropas - 1);
    if (dadosAtq < 1) {
        printf("Atacante não tem tropas suficientes para atacar.\n");
        return;
    }
    int dadosDef = (D->tropas >= 2) ? 2 : 1;

    // Sorteia os valores
    int rolAtq[3] = {0,0,0};
    int rolDef[2] = {0,0};
    for (int i = 0; i < dadosAtq; ++i) rolAtq[i] = 1 + rand() % 6;
    for (int i = 0; i < dadosDef; ++i) rolDef[i] = 1 + rand() % 6;

    // Ordena desc para comparar
    qsort(rolAtq, dadosAtq, sizeof(int), cmp_desc);
    qsort(rolDef, dadosDef, sizeof(int), cmp_desc);

    printf("Dados atacante: ");
    for (int i = 0; i < dadosAtq; ++i) printf("%d ", rolAtq[i]);
    printf(" | Dados defensor: ");
    for (int i = 0; i < dadosDef; ++i) printf("%d ", rolDef[i]);
    printf("\n");

    // Comparações (pares)
    int pares = (dadosAtq < dadosDef) ? dadosAtq : dadosDef;
    for (int i = 0; i < pares; ++i) {
        if (rolAtq[i] > rolDef[i]) {
            // defensor perde 1 tropa
            D->tropas -= 1;
            printf("Comparacao %d: atacante vence -> defensor perde 1 (agora %d)\n", i+1, D->tropas);
        } else {
            // atacante perde 1 tropa
            A->tropas -= 1;
            printf("Comparacao %d: defensor vence -> atacante perde 1 (agora %d)\n", i+1, A->tropas);
        }
    }

    // Se defensor zerou tropas, território conquistado
    if (D->tropas <= 0) {
        printf("Território %s conquistado!\n", D->nome);
        D->dono = A->dono; // agora pertence ao jogador
        // mover pelo menos 1 tropa do atacante para o conquistado
        int mover = 1;
        if (A->tropas - mover < 1) mover = A->tropas - 1; // garante que origem não fique vazia
        if (mover < 1) mover = 1; // forçar ao menos 1 se possível
        A->tropas -= mover;
        D->tropas = mover;
        printf("Movidas %d tropas de %s para %s.\n", mover, A->nome, D->nome);
    }
}

int sortearMissao(int* parametro) {
    // Missões definidas:
    // 1 -> destruir exército inimigo X (parâmetro = DONO_INIMIGO_1 ou DONO_INIMIGO_2)
    // 2 -> conquistar N territórios (parâmetro = número de territórios necessários)
    int id = 1 + rand() % MISSÕES; // 1..MISSÕES
    if (id == 1) {
        // sorteia qual inimigo
        *parametro = (rand() % 2 == 0) ? DONO_INIMIGO_1 : DONO_INIMIGO_2;
    } else if (id == 2) {
        *parametro = 1 + rand() % (NUM_TERRITORIOS/2); // número razoável
    }
    return id;
}

int verificarVitoria(const Territorio* mapa, int n, int missaoId, int parametro) {
    if (missaoId == 1) {
        // Verifica se não existe nenhum território com dono == parametro
        for (int i = 0; i < n; ++i) {
            if (mapa[i].dono == parametro) return 0; // ainda existe
        }
        return 1; // missão cumprida
    } else if (missaoId == 2) {
        int cont = 0;
        for (int i = 0; i < n; ++i) if (mapa[i].dono == DONO_JOGADOR) ++cont;
        return (cont >= parametro) ? 1 : 0;
    }
    return 0;
}

void limparBufferEntrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int lerInteiroSeguro(int minimo, int maximo) {
    int v;
    while (1) {
        if (scanf("%d", &v) == 1) {
            limparBufferEntrada();
            if (v >= minimo && v <= maximo) return v;
            printf("Valor inválido. Digite um número entre %d e %d: ", minimo, maximo);
        } else {
            limparBufferEntrada();
            printf("Entrada inválida. Digite um número entre %d e %d: ", minimo, maximo);
        }
    }
}

// --- Função principal ---
int main(void) {
    setlocale(LC_ALL, "");
    srand((unsigned) time(NULL));

    int n = NUM_TERRITORIOS;
    Territorio* mapa = alocarMapa(n);
    if (!mapa) {
        fprintf(stderr, "Falha ao alocar memória para o mapa.\n");
        return 1;
    }

    inicializarTerritorios(mapa, n);

    int missaoId;
    int missaoParam;
    missaoId = sortearMissao(&missaoParam);

    int opcao;
    do {
        exibirMapa(mapa, n);
        exibirMissao(missaoId, missaoParam);
        exibirMenuPrincipal();
        opcao = lerInteiroSeguro(0, 2);
        switch (opcao) {
            case 1:
                faseDeAtaque(mapa, n);
                break;
            case 2: {
                int venceu = verificarVitoria(mapa, n, missaoId, missaoParam);
                if (venceu) printf("\nPARABÉNS! Você cumpriu a missão.\n");
                else printf("\nMissão ainda não cumprida. Continue tentando.\n");
                break;
            }
            case 0:
                printf("Saindo do jogo...\n");
                break;
            default:
                printf("Opção desconhecida.\n");
        }
        printf("\nPressione Enter para continuar...");
        getchar();
    } while (opcao != 0 && !verificarVitoria(mapa, n, missaoId, missaoParam));

    if (verificarVitoria(mapa, n, missaoId, missaoParam)) {
        printf("\nVocê venceu a partida! Missão cumprida.\n");
    }

    liberarMemoria(&mapa);

    return 0;
}
// ============================================================================
//         PROJETO WAR ESTRUTURADO - DESAFIO DE CÓDIGO
// ============================================================================
//        
// ============================================================================
//
// OBJETIVOS:
// - Modularizar completamente o código em funções especializadas.
// - Implementar um sistema de missões para um jogador.
// - Criar uma função para verificar se a missão foi cumprida.
// - Utilizar passagem por referência (ponteiros) para modificar dados e
//   passagem por valor/referência constante (const) para apenas ler.
// - Foco em: Design de software, modularização, const correctness, lógica de jogo.
//
// ============================================================================

// Inclusão das bibliotecas padrão necessárias para entrada/saída, alocação de memória, manipulação de strings e tempo.

// --- Constantes Globais ---
// Definem valores fixos para o número de territórios, missões e tamanho máximo de strings, facilitando a manutenção.

// --- Estrutura de Dados ---
// Define a estrutura para um território, contendo seu nome, a cor do exército que o domina e o número de tropas.

// --- Protótipos das Funções ---
// Declarações antecipadas de todas as funções que serão usadas no programa, organizadas por categoria.
// Funções de setup e gerenciamento de memória:
// Funções de interface com o usuário:
// Funções de lógica principal do jogo:
// Função utilitária:

// --- Função Principal (main) ---
// Função principal que orquestra o fluxo do jogo, chamando as outras funções em ordem.
int main() {
    // 1. Configuração Inicial (Setup):
    // - Define o locale para português.
    // - Inicializa a semente para geração de números aleatórios com base no tempo atual.
    // - Aloca a memória para o mapa do mundo e verifica se a alocação foi bem-sucedida.
    // - Preenche os territórios com seus dados iniciais (tropas, donos, etc.).
    // - Define a cor do jogador e sorteia sua missão secreta.

    // 2. Laço Principal do Jogo (Game Loop):
    // - Roda em um loop 'do-while' que continua até o jogador sair (opção 0) ou vencer.
    // - A cada iteração, exibe o mapa, a missão e o menu de ações.
    // - Lê a escolha do jogador e usa um 'switch' para chamar a função apropriada:
    //   - Opção 1: Inicia a fase de ataque.
    //   - Opção 2: Verifica se a condição de vitória foi alcançada e informa o jogador.
    //   - Opção 0: Encerra o jogo.
    // - Pausa a execução para que o jogador possa ler os resultados antes da próxima rodada.

    // 3. Limpeza:
    // - Ao final do jogo, libera a memória alocada para o mapa para evitar vazamentos de memória.

    return 0;
}

// --- Implementação das Funções ---

// alocarMapa():
// Aloca dinamicamente a memória para o vetor de territórios usando calloc.
// Retorna um ponteiro para a memória alocada ou NULL em caso de falha.

// inicializarTerritorios():
// Preenche os dados iniciais de cada território no mapa (nome, cor do exército, número de tropas).
// Esta função modifica o mapa passado por referência (ponteiro).

// liberarMemoria():
// Libera a memória previamente alocada para o mapa usando free.

// exibirMenuPrincipal():
// Imprime na tela o menu de ações disponíveis para o jogador.

// exibirMapa():
// Mostra o estado atual de todos os territórios no mapa, formatado como uma tabela.
// Usa 'const' para garantir que a função apenas leia os dados do mapa, sem modificá-los.

// exibirMissao():
// Exibe a descrição da missão atual do jogador com base no ID da missão sorteada.

// faseDeAtaque():
// Gerencia a interface para a ação de ataque, solicitando ao jogador os territórios de origem e destino.
// Chama a função simularAtaque() para executar a lógica da batalha.

// simularAtaque():
// Executa a lógica de uma batalha entre dois territórios.
// Realiza validações, rola os dados, compara os resultados e atualiza o número de tropas.
// Se um território for conquistado, atualiza seu dono e move uma tropa.

// sortearMissao():
// Sorteia e retorna um ID de missão aleatório para o jogador.

// verificarVitoria():
// Verifica se o jogador cumpriu os requisitos de sua missão atual.
// Implementa a lógica para cada tipo de missão (destruir um exército ou conquistar um número de territórios).
// Retorna 1 (verdadeiro) se a missão foi cumprida, e 0 (falso) caso contrário.

// limparBufferEntrada():
// Função utilitária para limpar o buffer de entrada do teclado (stdin), evitando problemas com leituras consecutivas de scanf e getchar.
