/**
 * @file carte.h
 * @author Alexis POPIEUL et Paul LEFEVRE 
 * @brief Définition des structures de cartes et de paquet de cartes + prototypes de fonctions concernant les cartes
 * @version 0.1
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h> 

#define CODE_BAL_CARTE 10
#define NB_CARTES 52

struct carte {
    char couleur[10];
    int valeur;
    int dansPioche; // 1 si la carte est dans la pioche ou 0 si la carte est dans la main d'un joueur
};
typedef struct carte carte_t;


struct paquetCarte {
    int nbCartes;
    carte_t tabCartes[52];
};
typedef struct paquetCarte paquetCarte_t;

// Prototype
int initPaquetCartes();