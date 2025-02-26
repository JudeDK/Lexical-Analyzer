#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    TIP_CONSTANTA, IDENTIFICATOR_UNIC, SEPARARE, OP_CODE,
    ANOTATIE, SECVENTA_TEXT, VALOARE_NUMERICA, EROARE_GENERAL, END_DE_FISIER
} CategorieToken;

typedef struct {
    CategorieToken categorie;
    char* continut;
    int dimensiune, linie_sursa;
} TipTokenNou;

const char* categorie_la_text(CategorieToken categorie) {
    const char* categorii[] = {
        "Keyword", "Identificator", "Separator", "Operator",
        "Comentariu", "String", "Numar", "Eroare", "EOF"
    };
    return (categorie >= 0 && categorie <= END_DE_FISIER) ? categorii[categorie] : "Necunoscut";
}

int este_cuvant_rezervat(const char* secventa) {
    const char* lista_cuvinte[] = {"int", "if", "else", "string", "return", "scanf", "printf", NULL};
    for (int j = 0; lista_cuvinte[j]; j++) {
        if (!strcmp(secventa, lista_cuvinte[j])) return 1;
    }
    return 0;
}

TipTokenNou preia_token(FILE* fisier_intrare, int* numar_linie) {
    static int simbol_urmator = ' ';
    char cont[256];
    int i = 0;

    while (isspace(simbol_urmator)) {
        if (simbol_urmator == '\n') (*numar_linie)++;
        simbol_urmator = fgetc(fisier_intrare);
    }

    if (simbol_urmator == EOF) return (TipTokenNou){END_DE_FISIER, NULL, 0, *numar_linie};

    if (isdigit(simbol_urmator) || (simbol_urmator == '.' && isdigit(fgetc(fisier_intrare)))) {
        int este_zecimal = (simbol_urmator == '.');
        if (este_zecimal) cont[i++] = '.', simbol_urmator = fgetc(fisier_intrare);

        do {
            cont[i++] = simbol_urmator;
            simbol_urmator = fgetc(fisier_intrare);
        } while (isdigit(simbol_urmator));

        if (simbol_urmator == '.') {
            este_zecimal = 1;
            cont[i++] = '.';
            simbol_urmator = fgetc(fisier_intrare);
            while (isdigit(simbol_urmator)) {
                cont[i++] = simbol_urmator;
                simbol_urmator = fgetc(fisier_intrare);
            }
        }

        cont[i] = '\0';
        return (TipTokenNou){VALOARE_NUMERICA, strdup(cont), i, *numar_linie};
    }

    if (isalpha(simbol_urmator) || simbol_urmator == '_') {
        do {
            cont[i++] = simbol_urmator;
            simbol_urmator = fgetc(fisier_intrare);
        } while (isalnum(simbol_urmator) || simbol_urmator == '_');

        cont[i] = '\0';
        return (TipTokenNou){este_cuvant_rezervat(cont) ? TIP_CONSTANTA : IDENTIFICATOR_UNIC, strdup(cont), i, *numar_linie};
    }

    if (simbol_urmator == '"') {
        cont[i++] = simbol_urmator;
        while ((simbol_urmator = fgetc(fisier_intrare)) != '"' && simbol_urmator != EOF) {
            if (simbol_urmator == '\\') cont[i++] = simbol_urmator, simbol_urmator = fgetc(fisier_intrare);
            cont[i++] = simbol_urmator;
        }
        cont[i++] = '"';
        cont[i] = '\0';
        simbol_urmator = fgetc(fisier_intrare);
        return (TipTokenNou){SECVENTA_TEXT, strdup(cont), i, *numar_linie};
    }

    if (simbol_urmator == '/') {
        int verificare = fgetc(fisier_intrare);
        if (verificare == '*') {
            cont[i++] = '/', cont[i++] = '*';
            while (1) {
                simbol_urmator = fgetc(fisier_intrare);
                if (simbol_urmator == EOF) {
                    cont[i] = '\0';
                    return (TipTokenNou){EROARE_GENERAL, strdup(cont), i, *numar_linie};
                }
                if (simbol_urmator == '\n') (*numar_linie)++;
                cont[i++] = simbol_urmator;
                if (simbol_urmator == '*' && (simbol_urmator = fgetc(fisier_intrare)) == '/') {
                    cont[i++] = '/';
                    break;
                }
            }
            cont[i] = '\0';
            simbol_urmator = fgetc(fisier_intrare);
            return (TipTokenNou){ANOTATIE, strdup(cont), i, *numar_linie};
        } else if (verificare == '/') {
            cont[i++] = '/', cont[i++] = '/';
            while ((simbol_urmator = fgetc(fisier_intrare)) != '\n' && simbol_urmator != EOF) {
                cont[i++] = simbol_urmator;
            }
            cont[i] = '\0';
            simbol_urmator = fgetc(fisier_intrare);
            return (TipTokenNou){ANOTATIE, strdup(cont), i, *numar_linie};
        } else {
            simbol_urmator = verificare;
            return (TipTokenNou){OP_CODE, strdup("/"), 1, *numar_linie};
        }
    }

    if (strchr("+-*/%=&|!<>", simbol_urmator)) {
        cont[i++] = simbol_urmator;
        int caracter_extra = fgetc(fisier_intrare);
        if ((cont[0] == '+' && caracter_extra == '+') || (cont[0] == '-' && caracter_extra == '-') ||
            (cont[0] == '=' && caracter_extra == '=') || (cont[0] == '!' && caracter_extra == '=') ||
            (cont[0] == '&' && caracter_extra == '&') || (cont[0] == '|' && caracter_extra == '|')) {
            cont[i++] = caracter_extra;
            simbol_urmator = fgetc(fisier_intrare);
        } else {
            simbol_urmator = caracter_extra;
        }
        cont[i] = '\0';
        return (TipTokenNou){OP_CODE, strdup(cont), i, *numar_linie};
    }

    if (strchr("()[]{};,", simbol_urmator)) {
        cont[i++] = simbol_urmator;
        cont[i] = '\0';
        simbol_urmator = fgetc(fisier_intrare);
        return (TipTokenNou){SEPARARE, strdup(cont), i, *numar_linie};
    }

    cont[i++] = simbol_urmator;
    cont[i] = '\0';
    simbol_urmator = fgetc(fisier_intrare);
    return (TipTokenNou){EROARE_GENERAL, strdup(cont), i, *numar_linie};
}

int main() {
    FILE* fisier_sursa = fopen("cod.txt", "r");
    int numar_linie = 1;
    TipTokenNou token;
    while ((token = preia_token(fisier_sursa, &numar_linie)).categorie != END_DE_FISIER) {
        printf("Token: \"%s\", Tip: %s, Dimensiune: %d, Linie: %d\n", token.continut, categorie_la_text(token.categorie), token.dimensiune, token.linie_sursa);
        free(token.continut);
    }
    fclose(fisier_sursa);
    return 0;
}
