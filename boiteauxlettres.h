#include <sys/types.h>

#define MIN_JOUEURS 2
#define MAX_JOUEURS 12

struct joueur { // structure d'un joueur
    pid_t pid;
    char pseudo[100];
    int solde;
    int mise;
};

struct listeJoueurs { // structure de la liste des joueurs
    int nbJoueurs;
    struct joueur joueurs[12];
};

struct newPlayer {
    long mtype;
    struct joueur joueur;
};

void printInfoBoites();
int findPlayerIndex(pid_t pid);