/* ========================================================================== */
/* Date : 28 novembre 2022                                                    */
/* Auteur : Paul Lefevre                                                      */
/* Fichier bal3_s                                                              */
/* ========================================================================== */

#include "../include/joueur.h"

key_t cle, keyConnexion;
int balConnexionID;
struct msqid_ds buf;

sem_t *semInit;

newPlayer_t my_infos;

pid_t monPid;

int main(int argc, char* argv[]) {
    keyConnexion = ftok("token/balConnexion", 19); // création de la clé

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balConnexionID
    balConnexionID = msgget(keyConnexion, 0666); 
    // Ouverture du sémaphore
    semInit = sem_open("/INITPARTIE.SEMAPHORE", O_RDONLY, 0600, 0);

    printf("Client\n");

    monPid = getpid();

    my_infos.mtype = 1;
    my_infos.joueur.pid = monPid;
    my_infos.joueur.solde = 1000;
    my_infos.joueur.mise = 1001;
    my_infos.joueur.nbCartes = 0;

    printf("Saisissez votre pseudo :");
    scanf("%[^\n]s", my_infos.joueur.pseudo);

    getchar();

    printf("Client:  Pid:%d, Nom:%s\n", my_infos.joueur.pid, my_infos.joueur.pseudo);
    // On envoie le message au serveur
    msgsnd(balConnexionID, &my_infos, sizeof(my_infos.joueur), 0);

    // On attend que le serveur nous envoie un message pour dire que la partie est prête
    sem_wait(semInit);

    printf("Mise\n");

    while(my_infos.joueur.mise > my_infos.joueur.solde) {
        printf("Saisissez votre mise :");
        scanf("%d", &my_infos.joueur.mise);
        getchar();
        printf("Mise: %d\n", my_infos.joueur.mise);

        my_infos.mtype = 2;
    }
    
    msgsnd(balConnexionID, &my_infos, sizeof(my_infos.joueur), 0);
    
    printf("Joueur %d : %s, mise : %d, solde : %d\n", my_infos.joueur.pid, my_infos.joueur.pseudo, my_infos.joueur.mise, my_infos.joueur.solde);
    
    return 0;
}