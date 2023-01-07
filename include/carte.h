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

#define BLCK   "\x1B[30m"
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[39m\x1B[49m"
#define RESET_ALL "\x1B[0m"
#define BOLD "\x1B[1m"
#define WHT_BG "\x1B[107m"

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