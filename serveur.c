/* ========================================================================== */
/* Date : 28 novembre 2022                                                    */
/* Auteur : Paul Lefevre                                                      */
/* Fichier bal3_s                                                              */
/* ========================================================================== */

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
#include "boiteauxlettres.h"

key_t cle, keyConnexion;
int balConnexionID; // ID de la file de messages
int memConnexionID; // ID de la mémoire partagée
int descripteur;
struct msqid_ds buf;

sem_t *semInit;

char input[100];

struct listeJoueurs *listeJoueurs;
int nbJoueurs = 0;

struct newPlayer new_player;

int main(int argc, char* argv[]) {
    keyConnexion = ftok("balConnexion", 10); // création de la clé

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balConnexionID
    balConnexionID = msgget(keyConnexion, IPC_CREAT | 0666); 
    // création de la mémoire partagée (ou ouverture si déjà existante), et récupération de son ID dans memConnexionID
    memConnexionID = shmget(cle, 12*sizeof(struct joueur), IPC_EXCL | IPC_CREAT | 0666);
    // Initialisation du sémaphore
    semInit = sem_open("/INITPARTIE.SEMAPHORE", O_CREAT | O_RDWR, 0600, 0);

    printf("Je suis le serveur\nCréation de la partie...\n");
    
    listeJoueurs = (struct listeJoueurs *) shmat(memConnexionID,NULL,0);

    while(listeJoueurs->nbJoueurs != MIN_JOUEURS) {
        // attente de reception d'un message d'un client
        msgrcv(balConnexionID, &new_player, sizeof(new_player.joueur), 1, 0);

        listeJoueurs->joueurs[listeJoueurs->nbJoueurs] = new_player.joueur;
        
        printf("Message reçu de %d : >%s<\n", new_player.joueur.pid, new_player.joueur.pseudo);
        listeJoueurs->nbJoueurs++;
    }

    int fils = fork(); // création d'un processus fils

    if(fils == 0) {// fils
        while(listeJoueurs->nbJoueurs != MAX_JOUEURS) {
            msgrcv(balConnexionID, &new_player, sizeof(new_player.joueur), 1, 0);

            listeJoueurs->joueurs[listeJoueurs->nbJoueurs] = new_player.joueur;

            printf("Message reçu de %d : >%s<\n", new_player.joueur.pid, new_player.joueur.pseudo);
            listeJoueurs->nbJoueurs++;
        }
        printf("Fin fils : %d\n", nbJoueurs);
        _exit(0);
    } else {// père
        while(strcmp(input, "GO") != 0) { // On attend que le serveur tape GO pour lancer la partie
            printf("Saisissez GO pour lancer la partie :");
            scanf("%[^\n]s", input);
            getchar();
        }
        kill(fils, SIGKILL); // on tue le fils

        printf("nbJouer: %d\n", listeJoueurs->nbJoueurs);

        for(int i = 0; i < listeJoueurs->nbJoueurs; i++) {
            // On libère un jeton de sémaphore pour débloquer chaque joueur (indiquer que la partie commence)
            sem_post(semInit);
            printf("Joueur %d : %s\n", listeJoueurs->joueurs[i].pid, listeJoueurs->joueurs[i].pseudo);
        }
    }

    printf("Attente mise\n");
    for(int i = 0; i < listeJoueurs->nbJoueurs; i++) {
        // Attente de la mise de chaque joueur
        msgrcv(balConnexionID, &new_player, sizeof(new_player.joueur), 2, 0);

        printf("Mise reçue de %d : %d\n", new_player.joueur.pid, new_player.joueur.mise);

        int index = findPlayerIndex(new_player.joueur.pid); // On récupère l'index du joueur dans la liste des joueurs

        if(index != -1) {
            listeJoueurs->joueurs[index].mise = new_player.joueur.mise; // on met à jour sa mise
        }
    }


    return 0;
}

int findPlayerIndex(pid_t pid) {
    for(int i = 0; i < listeJoueurs->nbJoueurs; i++) {
        if(listeJoueurs->joueurs[i].pid == pid) {
            return i;
        }
    }
    return -1;
}