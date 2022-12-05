#include <sys/types.h>

struct joueur {
    pid_t pid;
    char pseudo[100];
    int score;
};

struct carte {
    int numero;
    char couleur;
    struct joueur possesseur;
};

struct msgbuf {
    long mtype;
    struct carte mtext[52];
};

struct newPlayer {
    long mtype;
    char name[100];
    pid_t pid;
};

void printInfoBoites();