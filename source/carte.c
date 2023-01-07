/**
 * @file carte.c
 * @author Alexis POPIEUL et Paul LEFEVRE 
 * @brief fonctions concernant les cartes et le paquet de 52 cartes
 * @version 0.1
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../include/carte.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

int initPaquetCartes(paquetCarte_t* pCartes) {
    char couleur[4][10];

    printf("Initialisation du packet de cartes\n");
    
    strcpy(couleur[0], "Pic");
    strcpy(couleur[1], "Coeur");
    strcpy(couleur[2], "Trefle");
    strcpy(couleur[3], "Carreau");

    for(int i = 0; i<52; i++){
        carte_t uneCarte;
        strcpy(uneCarte.couleur, couleur[(int)ceilf(i/13)]);
        //uneCarte.valeur = (i % 13 < 10) ? (i % 13 + 1): 10;
        uneCarte.valeur = i % 13 + 1;
        uneCarte.dansPioche = 1;
        memcpy(&(pCartes->tabCartes[i]), &uneCarte, sizeof(carte_t));
    }

    pCartes->nbCartes = 52;

    return 1;
}
