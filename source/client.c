/* ========================================================================== */
/* Date : 28 novembre 2022                                                    */
/* Auteur : Paul Lefevre                                                      */
/* Fichier bal3_s                                                              */
/* ========================================================================== */

#include "../include/joueur.h"

key_t cle, keyConnexion, cleJoueur, cleCroupier, cleCarte, cleBalTour;
int balConnexionID;
int memConnexionID; // ID de la mémoire partagée des joueurs
int memCroupier; // ID de la mémoire partagée du croupier
int memCartes; // ID de la mémoire partagée des joueurs
int balTourID; // ID de la file de messages
struct msqid_ds buf;

sem_t *semInit;
sem_t *semMain;
sem_t *semAffiche;
sem_t *semTour;
sem_t *semFinTour;

newPlayer_t my_infos;

pid_t monPid;

// variables des joueurs
listeJoueurs_t *listeJoueurs;
int indexJoueur;

// Le croupier
croupier_t *croupierJeu;

// variables des cartes
paquetCarte_t *paquetCartes;

// structure de reception dans la BAL pour les resultats de tour 
resultatTour_t recoitResultats;

int main(int argc, char* argv[]) {

    keyConnexion = ftok("token/balConnexion", 22); // création de la clé
    cleJoueur = ftok("token/shmJoueur", 22); // création de la clé pour la mémoire partagée des joueurs
    cleCroupier = ftok("token/shmCroupier", 22); // création de la clé pour la mémoire partagée du croupier
    cleCarte = ftok("token/shmCarte", 22); // création de la clé pour la mémoire partagée des cartes
    cleBalTour = ftok("token/balTour", 22); // création de la clé pour la mémoire partagée du croupier

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balConnexionID
    balConnexionID = msgget(keyConnexion, 0666); 

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balTourID
    balTourID = msgget(cleBalTour, IPC_CREAT | 0666); 

    // Récupération de la mémoire partagée de la liste de joueurs (ou ouverture si déjà existante),
    // et récupération de son ID dans memConnexionID
    memConnexionID = shmget(cleJoueur, sizeof(listeJoueurs_t), IPC_EXCL | 0666);

    // Récupération de la mémoire partagée du croupier (ou ouverture si déjà existante),
    // et récupération de son ID dans memConnexionID
    memCroupier = shmget(cleCroupier, sizeof(croupier_t), IPC_EXCL | 0666);

    // création de la mémoire partagée du paquet de carte (ou ouverture si déjà existante),
    // et récupération de son ID dans memCartes
    memCartes = shmget(cleCarte, sizeof(paquetCarte_t), IPC_EXCL | 0666);

    // Ouverture des sémaphores
    semInit = sem_open("/INITPARTIE.SEMAPHORE", O_RDONLY, 0600, 0);
    semMain = sem_open("/MAIN.SEMAPHORE", O_RDONLY, 0600, 0);
    semAffiche = sem_open("/AFFICHE.SEMAPHORE", O_RDONLY, 0600, 0);
    semFinTour = sem_open("/FINTOUR.SEMAPHORE", O_RDWR, 0600, 0);

    if(balConnexionID == -1 || memConnexionID == -1 || memCroupier == -1 || memCartes == -1 || balTourID == -1){
        printf("il y a eu un erreur pour créer la BAL ou la memoire partagée.\n");
        exit(-1);
    }

    // on s'attache aux mémoires
    listeJoueurs = (listeJoueurs_t *) shmat(memConnexionID,NULL,0);
    croupierJeu = (croupier_t *) shmat(memCroupier,NULL,0);
    paquetCartes = (paquetCarte_t *) shmat(memCartes,NULL,0);


    printf("Client\n");

    monPid = getpid();

    my_infos.mtype = 1;
    my_infos.joueur.pid = monPid;
    my_infos.joueur.solde = 1000;
    my_infos.joueur.mise = 1001;
    my_infos.joueur.nbCartes = 0;
    my_infos.joueur.sommeCartes = 0;

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
    for(int i = 0; i<listeJoueurs->nbJoueurs; i++){
        if(listeJoueurs->joueurs[i].pid == monPid){
            printf("Je suis ce joueur dans la liste et PID = %d\n", listeJoueurs->joueurs[i].pid);
            indexJoueur = i;
            afficherMainJoueur(&(listeJoueurs->joueurs[i]));
        }
    }
    printf("Fin de l'affichage de la main\n");



    printf("Affichage de la main du croupier \n");
    afficherMainCroupier(croupierJeu);
    printf("Fin de l'affichage de la main du croupier\n");

    // on previent le serveur que l affichage des cartes est terminé
    sem_post(semAffiche);


    // on attend que ca soit notre tour de jouer
    // la sémaphore du tour 
    char nomSem[25] = "/JOUEUR_";
    char chainePID[7]; // le PID peut être un nombre contenant jusqu a 7 chiffres
    sprintf(chainePID, "%d", getpid()); // convertir le PID en chaine de caractères
    strcat(nomSem, chainePID);
    strcat(nomSem, ".SEMAPHORE");
    printf("Je suis le JOUEUR et je dois ouvrir la sémaphore : %s\n", nomSem);
    semTour = sem_open(nomSem, O_RDONLY, 0600, 0);

    printf("On a reussi a ouvrir la semaphore\n");

    printf("Je suis le joueur %d et mon PID est %d, j'attend mon tour\n", indexJoueur, getpid());
    sem_wait(semTour);

    int sortir = 0;
    int score;
    do{
        score = 0;
        char choix[1];
        for(int i = 0; i<listeJoueurs->joueurs[indexJoueur].nbCartes; i++){
            score = score + listeJoueurs->joueurs[indexJoueur].main[i].valeur;
        }
        printf("Votre score de cartes est de : %d\n", score);

        if(score > 21){
            printf("Votre score est de %d, dépasse 21 donc vous perdez ce tour\n", score);
            sortir = 1;
        }

        if(sortir == 0){
            printf("Voulez vous Tirer une nouvelle carte [T] ou Rester avec vos cartes actuelles [R]\n");
            scanf("%[^\n]s", choix);
            getchar();

            if(strcmp(choix, "T") == 0){
                printf("Vous voulez tirer une nouvelle carte\n");
                uneCartePourUnePersonnne(paquetCartes, 
                                    listeJoueurs->joueurs[indexJoueur].main, 
                                    &(listeJoueurs->joueurs[indexJoueur].nbCartes));
                afficherMainJoueur(&(listeJoueurs->joueurs[indexJoueur]));
            }
            else if(strcmp(choix, "R") == 0){
                printf("Vous restez avec vos cartes actuelles, on attend le tirage du croupier\n");
                sortir = 1;
            }
            else{
                printf("Votre saisie n'est pas correcte\n");
            }
        }
        
    }while(sortir == 0);
    listeJoueurs->joueurs[indexJoueur].sommeCartes = score;
    sem_post(semFinTour);

    printf("\n\n******** RESULTAT ********\n");
    msgrcv(balTourID, &recoitResultats, sizeof(resultatTour_t), getpid(), 0);
    if(recoitResultats.gagne == 1){
        printf("J'ai gagné !!!!!!!!!!! et mon PID = %d\n", getpid());
        printf("J'ai récu un gain de : %d \n", recoitResultats.gain);
        listeJoueurs->joueurs[indexJoueur].solde = listeJoueurs->joueurs[indexJoueur].solde + recoitResultats.gain;
    }
    if(recoitResultats.gagne == 0){
        printf("J'ai perdu et mon PID = %d\n", getpid());
    }
    printf("La main finale du croupier est :\n");
    afficherMainCroupier(croupierJeu);


    return 0;
}