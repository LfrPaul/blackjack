/**
 * @file joueur.h
 * @author Alexis POPIEUL et Paul LEFEVRE
 * @brief Définition des structures de joueurs et de liste de joueurs + prototypes de fonctions concernant les joueurs
 * @version 0.1
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "carte.h"

#define MIN_JOUEURS 2
#define MAX_JOUEURS 12
#define CODE_BAL_JOUEUR 1
#define CODE_BAL_MISE 2

struct joueur { // structure d'un joueur
    pid_t pid;
    char pseudo[100];
    int solde;
    int mise;
    carte_t main[5];
    int nbCartes;
};
typedef struct joueur joueur_t;


struct listeJoueurs { // structure de la liste des joueurs
    int nbJoueurs;
    joueur_t joueurs[12];
};
typedef struct listeJoueurs listeJoueurs_t;



struct newPlayer{
    long mtype;
    joueur_t joueur;
};
typedef struct newPlayer newPlayer_t;


struct croupier{ // structure du croupier
    pid_t pid;
    carte_t main[5];
    int nbCartes;
};
typedef struct croupier croupier_t;


int findPlayerIndex(pid_t pid, listeJoueurs_t* listeJ);
int uneCartePourTousJoueurs(paquetCarte_t* pCartes, listeJoueurs_t* pJoueurs);
void afficherMainJoueur(joueur_t *joueur);
void afficherMainCroupier(croupier_t *croupier);
void uneCartePourUnePersonnne(paquetCarte_t* pCartes, carte_t * main, int* nbCartes);