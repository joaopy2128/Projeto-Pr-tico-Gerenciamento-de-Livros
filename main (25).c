/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

/*
===================================================================================
                PROJETO PRÁTICO — GERENCIADOR DE LIVROS
-----------------------------------------------------------------------------------
TEMA: BIBLIOTECA PESSOAL (Com todos os requisitos da atividade)
====================================================================================
*/

#include <stdio.h>      // I/O, FILE, remove, rename
#include <stdlib.h>     // system, exit
#include <string.h>     // strcspn
#include <ctype.h>      // toupper
#include <locale.h>     // setlocale

/* -------------------------------------------------------------------------------
    ESTRUTURA DE DADOS
    Define o registro 'livro' com mais de 3 tipos de campos.
-------------------------------------------------------------------------------- */
typedef struct {
    char titulo[100];    // char[]
    char autor[50];      // char[]
    int isbn;            // int
    float preco;         // float
} livro;

/* Protótipos das funções (Modularização) */
void configurar_locale(void);
void limpaBuffer(void);
void lerStringSegura(char *s, int tam);
int tamanho(FILE *arq);
void cadastrar(FILE *arq);
void consultar(FILE *arq);
FILE* excluir(FILE *arq); // Retorna o novo ponteiro do arquivo reaberto
void gerar_relatorio(FILE *arq);


/* ====================================================================================
    limpaBuffer (Requisito: Padronização de Nomenclatura)
    ==================================================================================== */
void limpaBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ====================================================================================
    lerStringSegura (Requisito Opcional: Leitura de String Segura)
    ==================================================================================== */
void lerStringSegura(char *s, int tam) {
    // Usa fgets para leitura segura e strcspn para remover o '\n'
    fgets(s, tam, stdin);
    s[strcspn(s, "\n")] = '\0';
}

/* ====================================================================================
    tamanho (Requisito: Calcula o número de registros)
    ==================================================================================== */
int tamanho(FILE *arq) {
    if (!arq) return 0; // Prevenção, caso o arquivo não esteja aberto
    long pos = ftell(arq);     // Salva a posição
    fseek(arq, 0, SEEK_END);   // Move para o final
    long fim = ftell(arq);     // Obtém o tamanho total em bytes
    fseek(arq, pos, SEEK_SET); // Restaura a posição
    
    // Retorna o número de registros (tamanho_total / sizeof(livro))
    return (int)(fim / sizeof(livro));
}

/* ====================================================================================
    cadastrar (Requisito: Salva a struct no final do arquivo)
    ==================================================================================== */
void cadastrar(FILE *arq) {
    if (!arq) {
        printf("[ERRO] Arquivo não está aberto para cadastro.\n");
        return;
    }
    livro novo_livro;
    char confirma;

    printf("\n=== CADASTRAR LIVRO ===\n");
    printf("Registro número: %d\n", tamanho(arq) + 1);

    printf("Título: ");
    lerStringSegura(novo_livro.titulo, sizeof(novo_livro.titulo)); 
    
    printf("Autor: ");
    lerStringSegura(novo_livro.autor, sizeof(novo_livro.autor));
    
    printf("ISBN (número inteiro): ");
    if (scanf("%d", &novo_livro.isbn) != 1) {
        printf("Entrada inválida. ISBN deve ser um número inteiro.\n");
        limpaBuffer(); return;
    }
    
    printf("Preço (R$ 0.00): ");
    if (scanf("%f", &novo_livro.preco) != 1) {
        printf("Entrada inválida. Preço deve ser um número decimal.\n");
        limpaBuffer(); return;
    }

    limpaBuffer(); // Limpar o '\n' após o último scanf (preço)
    printf("Confirmar cadastro (s/n)? ");
    if (scanf("%c", &confirma) != 1) { 
        printf("Entrada inválida. Cancelando cadastro.\n");
        limpaBuffer(); return;
    }
    limpaBuffer(); // Limpar o '\n' após a confirmação

    if (toupper(confirma) == 'S') {
        fseek(arq, 0, SEEK_END);                // Posiciona no fim
        fwrite(&novo_livro, sizeof(livro), 1, arq);   // Salva a struct
        fflush(arq);                            
        printf("Livro cadastrado com sucesso!\n");
    } else {
        printf("Cadastro cancelado.\n");
    }
}

/* ====================================================================================
    consultar (Requisito: Exibe o registro pelo código/índice)
    ==================================================================================== */
void consultar(FILE *arq) {
    if (!arq) {
        printf("[ERRO] Arquivo não está aberto para consulta.\n");
        return;
    }
    int indice;
    livro livro_lido;
    int total = tamanho(arq);

    if (total == 0) {
        printf("\nA biblioteca está vazia. Cadastre livros primeiro.\n");
        return;
    }

    printf("\n=== CONSULTAR LIVRO ===\n");
    printf("Informe o ÍNDICE do livro (1 a %d): ", total);
    if (scanf("%d", &indice) != 1) {
        printf("Entrada inválida!\n");
        limpaBuffer();
        return;
    }
    limpaBuffer(); 

    if (indice <= 0 || indice > total) {
        printf("Índice inválido! A biblioteca possui %d livros.\n", total);
        return;
    }

    // Calcula a posição em bytes: (índice - 1) * tamanho_do_registro
    long pos_byte = (long)(indice - 1) * sizeof(livro);

    // Move o ponteiro (fseek(SEEK_SET))
    if (fseek(arq, pos_byte, SEEK_SET) != 0) {
        printf("[ERRO] Falha ao posicionar ponteiro no arquivo.\n");
        return;
    }

    // Lê o registro (fread)
    size_t lido = fread(&livro_lido, sizeof(livro), 1, arq);

    if (lido != 1) {
        printf("[ERRO] Falha ao ler registro!\n");
        return;
    }

    printf("\n--- LIVRO (ÍNDICE %d) ---\n", indice);
    printf("Título: %s\n", livro_lido.titulo);
    printf("Autor: %s\n", livro_lido.autor);
    printf("ISBN: %d\n", livro_lido.isbn);
    printf("Preço: R$ %.2f\n", livro_lido.preco);
}

/* ====================================================================================
    excluir (Requisito: Excluir item - Abordagem com arquivo temporário)
    ==================================================================================== */
FILE* excluir(FILE *arq) {
    if (!arq) {
        printf("[ERRO] Arquivo não está aberto para exclusão.\n");
        return NULL;
    }

    int indice_excluir, i;
    livro temp_livro;
    FILE *arq_temp;
    int total = tamanho(arq);

    if (total == 0) {
        printf("\nA biblioteca está vazia. Não há nada para excluir.\n");
        return arq; // Retorna o ponteiro original sem alterações
    }

    printf("\n=== EXCLUIR LIVRO ===\n");
    printf("Informe o ÍNDICE do livro a excluir (1 a %d): ", total);
    
    if (scanf("%d", &indice_excluir) != 1) {
        printf("Entrada inválida!\n");
        limpaBuffer(); return arq;
    }
    limpaBuffer();

    if (indice_excluir <= 0 || indice_excluir > total) {
        printf("Índice inválido! A biblioteca possui %d livros.\n", total);
        return arq;
    }

    // 1. Tenta abrir o arquivo temporário em modo de escrita binária (wb)
    arq_temp = fopen("temp_biblioteca.dat", "wb");
    if (!arq_temp) {
        printf("[ERRO] Não foi possível criar arquivo temporário.\n");
        return arq;
    }

    // 2. Volta para o início do arquivo original
    fseek(arq, 0, SEEK_SET);

    // 3. Lê do original e escreve no temporário, pulando o registro a excluir
    for (i = 1; i <= total; i++) {
        fread(&temp_livro, sizeof(livro), 1, arq);
        if (i != indice_excluir) {
            fwrite(&temp_livro, sizeof(livro), 1, arq_temp);
        }
    }

    // 4. Fecha os arquivos
    fclose(arq);
    fclose(arq_temp);

    // 5. Exclui o arquivo original
    if (remove("biblioteca.dat") != 0) {
        printf("[ERRO] Falha ao remover arquivo original.\n");
        exit(1);
    }

    // 6. Renomeia o arquivo temporário para o nome original
    if (rename("temp_biblioteca.dat", "biblioteca.dat") != 0) {
        printf("[ERRO] Falha ao renomear arquivo temporário.\n");
        exit(1);
    }

    printf("Livro no índice %d excluído com sucesso!\n", indice_excluir);
    
    // 7. Reabre o arquivo principal para que as operações continuem
    FILE *arq_novo = fopen("biblioteca.dat", "r+b"); 
    if (!arq_novo) {
        printf("Erro crítico ao reabrir arquivo após exclusão.\n");
        exit(1);
    }
    return arq_novo; // Retorna o novo ponteiro
}

/* ====================================================================================
    gerar_relatorio (Requisito: Gerar relatório em arquivo .txt)
    ==================================================================================== */
void gerar_relatorio(FILE *arq) {
    if (!arq) {
        printf("[ERRO] Arquivo não está aberto para gerar relatório.\n");
        return;
    }
    FILE *arq_txt;
    livro livro_lido;
    int total = tamanho(arq);
    int i = 1;

    if (total == 0) {
        printf("\nA biblioteca está vazia. Relatório não gerado.\n");
        return;
    }

    printf("\n=== GERAR RELATÓRIO (relatorio.txt) ===\n");

    // Abre o arquivo de texto em modo de escrita ('w' para sobrescrever)
    arq_txt = fopen("relatorio.txt", "w");
    if (!arq_txt) {
        printf("[ERRO] Não foi possível criar o arquivo de relatório (relatorio.txt).\n");
        return;
    }
    
    // Escreve o cabeçalho no arquivo de texto
    fprintf(arq_txt, "RELATÓRIO DA BIBLIOTECA PESSOAL\n");
    fprintf(arq_txt, "Total de livros: %d\n", total);
    fprintf(arq_txt, "========================================\n");
    
    // Volta para o início do arquivo binário
    fseek(arq, 0, SEEK_SET);

    // Itera e lê todos os registros
    while (fread(&livro_lido, sizeof(livro), 1, arq) == 1) {
        // Escreve os detalhes do livro no arquivo de texto
        fprintf(arq_txt, "ÍNDICE: %d\n", i++);
        fprintf(arq_txt, "Título: %s\n", livro_lido.titulo);
        fprintf(arq_txt, "Autor: %s\n", livro_lido.autor);
        fprintf(arq_txt, "ISBN: %d\n", livro_lido.isbn);
        fprintf(arq_txt, "Preço: R$ %.2f\n", livro_lido.preco);
        fprintf(arq_txt, "----------------------------------------\n");
    }

    // Fecha o arquivo de texto
    fclose(arq_txt);
    printf("Relatório gerado com sucesso em 'relatorio.txt'!\n");
}

/* ====================================================================================
    configurar_locale
    ==================================================================================== */
void configurar_locale(void) {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    const char *locais[] = {
        "pt_BR.UTF-8",
        "pt_BR.utf8",
        "Portuguese"
    };
    int i; 
    for (i = 0; i < 3; i++) {
        if (setlocale(LC_ALL, locais[i]) != NULL) {
            return;
        }
    }
}

/* ====================================================================================
    main — FUNÇÃO PRINCIPAL (Requisito: Módulo de Arquivos)
    ==================================================================================== */
int main(void) {
    configurar_locale();
    
    // Tenta abrir/criar o arquivo binário (r+b ou w+b)
    FILE *arq = fopen("biblioteca.dat", "r+b"); 
    
    if (!arq) {
        arq = fopen("biblioteca.dat", "w+b");
        if (!arq) {
            printf("Erro crítico ao abrir/criar arquivo de dados.\n");
            return 1;
        }
    }

    int op;
    do {
        printf("\n====== BIBLIOTECA PESSOAL (C PURO) ======\n");
        printf("1. Cadastrar Livro\n");
        printf("2. Consultar Livro (por Índice)\n");
        printf("3. Excluir Livro (por Índice)\n");
        printf("4. Gerar Relatório (.txt)\n");
        printf("5. Sair\n");
        printf("------------------------------------------\n");
        printf("Livros cadastrados: %d\n", tamanho(arq));
        printf("Opção: ");
        
        if (scanf("%d", &op) != 1) {
            printf("Digite um número válido.\n");
            limpaBuffer(); 
            continue;
        }
        limpaBuffer(); 

        switch (op) {
            case 1: 
                cadastrar(arq); 
                break;
            case 2: 
                consultar(arq); 
                break;
            case 3: 
                // A função excluir fecha e reabre o arquivo, retornando o novo ponteiro
                // É necessário atualizar o ponteiro 'arq' no main.
                arq = excluir(arq); 
                break;
            case 4: 
                gerar_relatorio(arq); 
                break;
            case 5: 
                printf("Fechando arquivo e salvando dados...\n"); 
                break;
            default: 
                printf("Opção inválida!\n");
        }
    } while (op != 5);

    if (arq) {
        fclose(arq); 
    }
    return 0;
}