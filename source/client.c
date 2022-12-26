/* ========================================================================== */
/* Date : 28 novembre 2022                                                    */
/* Auteur : Paul Lefevre                                                      */
/* Fichier bal3_s                                                              */
/* ========================================================================== */

#include "../include/joueur.h"

key_t cle, keyConnexion, cleJoueur, cleCroupier;
int balConnexionID;
int memConnexionID; // ID de la mémoire partagée des joueurs
int memCroupier; // ID de la mémoire partagée du croupier
struct msqid_ds buf;

sem_t *semInit;
sem_t *semMain;

newPlayer_t my_infos;

pid_t monPid;

// variables des joueurs
listeJoueurs_t *listeJoueurs;

// Le croupier
croupier_t *croupierJeu;

int main(int argc, char* argv[]) {

    keyConnexion = ftok("token/balConnexion", 21); // création de la clé
    cleJoueur = ftok("token/shmJoueur", 21); // création de la clé pour la mémoire partagée des joueurs
    cleCroupier = ftok("token/shmCroupier", 21); // création de la clé pour la mémoire partagée du croupier

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balConnexionID
    balConnexionID = msgget(keyConnexion, 0666); 

    // Récupération de la mémoire partagée de la liste de joueurs (ou ouverture si déjà existante),
    // et récupération de son ID dans memConnexionID
    memConnexionID = shmget(cleJoueur, sizeof(listeJoueurs_t), IPC_EXCL | 0666);

    // Récupération de la mémoire partagée du croupier (ou ouverture si déjà existante),
    // et récupération de son ID dans memConnexionID
    memCroupier = shmget(cleCroupier, sizeof(croupier_t), IPC_EXCL | 0666);

    // Ouverture des sémaphores
    semInit = sem_open("/INITPARTIE.SEMAPHORE", O_RDONLY, 0600, 0);
    semMain = sem_open("/MAIN.SEMAPHORE", O_RDONLY, 0600, 0);

    if(balConnexionID == -1 || memConnexionID == -1 || memCroupier == -1){
        printf("il y a eu un erreur pour créer la BAL ou la memoire partagée.\n");
        exit(-1);
    }

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

    // on soustrait le solde du joueur avec la mise qu il vient de faire
    my_infos.joueur.solde = my_infos.joueur.solde - my_infos.joueur.mise;
    
    msgsnd(balConnexionID, &my_infos, sizeof(my_infos.joueur), 0);
    
    printf("Joueur %d : %s, mise : %d, solde : %d\n", my_infos.joueur.pid, my_infos.joueur.pseudo, my_infos.joueur.mise, my_infos.joueur.solde);
    

    printf("Affichage de ma main\n");
    sem_wait(semMain);
    listeJoueurs = (listeJoueurs_t *) shmat(memConnexionID,NULL,0);
    for(int i = 0; i<listeJoueurs->nbJoueurs; i++){
        if(listeJoueurs->joueurs[i].pid == monPid){
            printf("Je suis ce joueur dans la liste et PID = %d\n", listeJoueurs->joueurs[i].pid);
            afficherMainJoueur(&(listeJoueurs->joueurs[i]));
        }
    }
    printf("Fin de l'affichage de la main\n");



    printf("Affichage de la main du croupier \n");
    croupierJeu = (croupier_t *) shmat(memCroupier,NULL,0);
    for(int i = 0; i<croupierJeu->nbCartes; i++){
        afficherMainCroupier(croupierJeu);
    }
    printf("Fin de l'affichage de la main du croupier\n");

    return 0;
}