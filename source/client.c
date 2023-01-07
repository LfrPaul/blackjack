/* ========================================================================== */
/* Date : 28 novembre 2022                                                    */
/* Auteur : Paul Lefevre                                                      */
/* Fichier bal3_s                                                              */
/* ========================================================================== */

#include "../include/joueur.h"

key_t cle, keyConnexion, cleJoueur, cleCroupier, cleCarte, cleBalTour, cleBalRejoue;
int balConnexionID;
int memConnexionID; // ID de la mémoire partagée des joueurs
int memCroupier; // ID de la mémoire partagée du croupier
int memCartes; // ID de la mémoire partagée des joueurs
int balTourID; // ID de la file de messages
int balRejoueID; // ID de la file de messages pour faire rejouer les joueurs
struct msqid_ds buf;

sem_t *semInit;
sem_t *semMain;
sem_t *semAffiche;
sem_t *semTour;
sem_t *semFinTour;
sem_t *semFinRejoue;
sem_t *semFinPartie;

// sémaphores utilisés dans le thread d'affichage
sem_t *semAffichageMain;
sem_t *semAffichageMainTerm;

newPlayer_t my_infos;

pid_t monPid;
int monTour = 0;

// variables des joueurs
listeJoueurs_t *listeJoueurs;
int indexJoueur;

// Le croupier
croupier_t *croupierJeu;

// variables des cartes
paquetCarte_t *paquetCartes;

// structure de reception dans la BAL pour les resultats de tour 
resultatTour_t recoitResultats;

char nouvTour[10];
int premierTour;

// thread qui permettra de mettre à jour l'affichage des mains
pthread_t thAffichage;
pthread_mutex_t mutexAffichage = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAffichageFin = PTHREAD_MUTEX_INITIALIZER;

void fonctionAffichage() {
    while(1) {
        sem_wait(semAffichageMain);
        printf("MON TOUR : %d\n", monTour);
        if(monTour != 2) {
            pthread_mutex_lock(&mutexAffichage);
        }

        printTable(croupierJeu, listeJoueurs, indexJoueur);
        
        pthread_mutex_unlock(&mutexAffichageFin);
    }
}

int main(int argc, char* argv[]) {

    premierTour = 1;
    keyConnexion = ftok("token/balConnexion", 28); // création de la clé
    cleJoueur = ftok("token/shmJoueur", 28); // création de la clé pour la mémoire partagée des joueurs
    cleCroupier = ftok("token/shmCroupier", 28); // création de la clé pour la mémoire partagée du croupier
    cleCarte = ftok("token/shmCarte", 28); // création de la clé pour la mémoire partagée des cartes
    cleBalTour = ftok("token/balTour", 28); // création de la clé pour la mémoire partagée du croupier
    cleBalRejoue = ftok("token/balRejoue", 28); // création de la clé pour la mémoire partagée du croupier

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balConnexionID
    balConnexionID = msgget(keyConnexion, 0666); 

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balTourID
    balRejoueID = msgget(cleBalRejoue, 0666); 

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balTourID
    balTourID = msgget(cleBalTour, IPC_EXCL | 0666); 

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
    semFinRejoue = sem_open("/FINREJOUETOUR.SEMAPHORE", O_RDWR, 0600, 0);
    semAffichageMain = sem_open("/AFFICHAGEMAIN.SEMAPHORE", O_RDWR, 0600, 0);
    semAffichageMainTerm = sem_open("/FINAFFICHAGEMAIN.SEMAPHORE", O_RDWR, 0600, 0);
    semFinPartie = sem_open("/FINPARTIE.SEMAPHORE", O_RDWR, 0600, 0);

    if(balConnexionID == -1 || memConnexionID == -1 || memCroupier == -1 || memCartes == -1 || balTourID == -1 || balRejoueID == -1){
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


    pthread_create(&thAffichage, NULL, (void *)fonctionAffichage, NULL);

    do{
        if(premierTour == 0)
            my_infos.joueur.solde = listeJoueurs->joueurs[indexJoueur].solde;

        printf("Solde: %d \n", my_infos.joueur.solde);

        printf("Mise\n");
        //my_infos.joueur.mise = 1001;

        do{
            printf("Saisissez votre mise :");
            scanf("%d", &my_infos.joueur.mise);
            getchar();
            printf("Mise: %d\n", my_infos.joueur.mise);

            my_infos.mtype = 2;
        }while(my_infos.joueur.mise > my_infos.joueur.solde);

        // on soustrait le solde du joueur avec la mise qu il vient de faire
        /*
        if(premierTour == 1){
            //my_infos.joueur.solde = my_infos.joueur.solde - my_infos.joueur.mise;
            printf("Joueur nouveau %d : %s, mise : %d, solde : %d\n", my_infos.joueur.pid, my_infos.joueur.pseudo, my_infos.joueur.mise, my_infos.joueur.solde);
        }       
        else{
            //listeJoueurs->joueurs[indexJoueur].solde = listeJoueurs->joueurs[indexJoueur].solde - my_infos.joueur.mise;
            printf("Joueur %d : %s, mise : %d, solde : %d\n", listeJoueurs->joueurs[indexJoueur].pid, listeJoueurs->joueurs[indexJoueur].pseudo, listeJoueurs->joueurs[indexJoueur].mise, listeJoueurs->joueurs[indexJoueur].solde);
        }
        */

        msgsnd(balConnexionID, &my_infos, sizeof(my_infos.joueur), 0);

        sem_wait(semMain);

        indexJoueur = findPlayerIndex(monPid, listeJoueurs);

        // on previent le serveur que l affichage des cartes est terminé
        sem_post(semAffichageMain);
        pthread_mutex_unlock(&mutexAffichage);

        pthread_mutex_lock(&mutexAffichageFin);
        sem_post(semAffiche);

        // création du thread


        // on attend que ca soit notre tour de jouer
        // la sémaphore du tour 
        char nomSem[25] = "/JOUEUR_";
        char chainePID[7]; // le PID peut être un nombre contenant jusqu a 7 chiffres
        sprintf(chainePID, "%d", getpid()); // convertir le PID en chaine de caractères
        strcat(nomSem, chainePID);
        strcat(nomSem, ".SEMAPHORE");
        semTour = sem_open(nomSem, O_CREAT | O_RDONLY, 0600, 0);

        sem_wait(semTour);
        monTour = 1;

        int sortir = 0;
        int score;
        do{
            pthread_mutex_lock(&mutexAffichageFin);
            score = 0;
            char choix[1];
            score = getPlayerScore(listeJoueurs->joueurs[indexJoueur].main, listeJoueurs->joueurs[indexJoueur].nbCartes);
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
                    uneCartePourUnePersonnne(paquetCartes, 
                                        listeJoueurs->joueurs[indexJoueur].main, 
                                        &(listeJoueurs->joueurs[indexJoueur].nbCartes));
                    pthread_mutex_unlock(&mutexAffichage);
                    for(int i = 0; i < listeJoueurs->nbJoueurs; i++){
                        sem_post(semAffichageMain);
                    }
                    // afficherMainJoueur(&(listeJoueurs->joueurs[indexJoueur]));
                }
                else if(strcmp(choix, "R") == 0){
                    sortir = 1;
                }
                else{
                    printf("Votre saisie n'est pas correcte\n");
                    pthread_mutex_unlock(&mutexAffichageFin);
                }
            }
            
        }while(sortir == 0);
        monTour = 2;
        pthread_mutex_unlock(&mutexAffichageFin);
        listeJoueurs->joueurs[indexJoueur].sommeCartes = score;
        sem_post(semFinTour);

        sem_wait(semFinPartie);

        printf("\n\n******** RESULTAT ********\n");
        // printf PERDU in ascii art


        msgrcv(balTourID, &recoitResultats, sizeof(resultatTour_t), getpid(), 0);
        if(recoitResultats.gagne == 1){
            printf("%s ██████╗  █████╗  ██████╗ ███╗   ██╗███████╗\n", GRN);
            printf("██╔════╝ ██╔══██╗██╔════╝ ████╗  ██║██╔════╝\n");
            printf("██║  ███╗███████║██║  ███╗██╔██╗ ██║█████╗  \n");
            printf("██║   ██║██╔══██║██║   ██║██║╚██╗██║██╔══╝  \n");
            printf("╚██████╔╝██║  ██║╚██████╔╝██║ ╚████║███████╗\n");
            printf(" ╚═════╝ ╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝\n");
            printf("Vous avez gagné\n");
            printf("Vous recevez : %d jetons\n", recoitResultats.gain);
            printf("Votre ancien solde est de %d\n", listeJoueurs->joueurs[indexJoueur].solde);
            listeJoueurs->joueurs[indexJoueur].solde = listeJoueurs->joueurs[indexJoueur].solde + recoitResultats.gain;
            printf("Votre nouveau solde est de %d\n", listeJoueurs->joueurs[indexJoueur].solde);
        }
        if(recoitResultats.gagne == 2){
            printf("%s███████╗ ██████╗  █████╗ ██╗     ██╗████████╗███████╗\n", YEL);
            printf("██╔════╝██╔════╝ ██╔══██╗██║     ██║╚══██╔══╝██╔════╝\n");
            printf("█████╗  ██║  ███╗███████║██║     ██║   ██║   █████╗  \n");
            printf("██╔══╝  ██║   ██║██╔══██║██║     ██║   ██║   ██╔══╝  \n");
            printf("███████╗╚██████╔╝██║  ██║███████╗██║   ██║   ███████╗\n");
            printf("╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝   ╚═╝   ╚══════╝\n");
            printf("Vous avez fait égalité\n");
            printf("Vous recevez : %d jetons\n", recoitResultats.gain);
            printf("Votre ancien solde est de %d\n", listeJoueurs->joueurs[indexJoueur].solde);
            listeJoueurs->joueurs[indexJoueur].solde = listeJoueurs->joueurs[indexJoueur].solde + recoitResultats.gain;
            printf("Votre nouveau solde est de %d\n", listeJoueurs->joueurs[indexJoueur].solde);
        }
        if(recoitResultats.gagne == 0){
            printf("%s██████╗ ███████╗██████╗ ██████╗ ██╗   ██╗\n", RED);
            printf("██╔══██╗██╔════╝██╔══██╗██╔══██╗██║   ██║\n");
            printf("██████╔╝█████╗  ██████╔╝██║  ██║██║   ██║\n");
            printf("██╔═══╝ ██╔══╝  ██╔══██╗██║  ██║██║   ██║\n");
            printf("██║     ███████╗██║  ██║██████╔╝╚██████╔╝\n");
            printf("╚═╝     ╚══════╝╚═╝  ╚═╝╚═════╝  ╚═════╝ \n");
            printf("Vous avez perdu\n");
            printf("Votre solde est de %d\n", listeJoueurs->joueurs[indexJoueur].solde);
        }
        printf("%s", RESET);
        printf("La main finale du croupier est :\n");
        afficherMainCroupier(croupierJeu);
        printf("Le total de la main du croupier est : %d\n", croupierJeu->sommeCartesCroupier);

        if(listeJoueurs->joueurs[indexJoueur].solde <= 0){
            printf("Vous n'avez plus d'argent, vous quittez la partie \n");
            strcpy(nouvTour, "N");
        }
        else{
            do{
                // on demande au joueur s il veut rejouer un tour 
                printf("Voulez-vous rejouer un tour ? Oui [O] ou Non [N]\n");
                scanf("%[^\n]s", nouvTour);
                getchar();
            }while((strcmp(nouvTour, "N") != 0) && (strcmp(nouvTour, "O") != 0));
        }


        printf("Je vide ma main \n");
        listeJoueurs->joueurs[indexJoueur].nbCartes = 0;

    
        printf("On prévient le serveur qu'on rejoue ou non\n");
        rejoueTour_t envoieRejoueTour;
        envoieRejoueTour.mtype = getpid();
        envoieRejoueTour.pidJoueur = getpid();
        if(strcmp(nouvTour, "N") == 0)
            envoieRejoueTour.rejoue = 0;
        else 
            envoieRejoueTour.rejoue = 1;
        msgsnd(balRejoueID, &envoieRejoueTour, sizeof(rejoueTour_t), 0);

        premierTour = 0;

        printf("On attend que de savoir si les autres joueurs restent dans la partie ou non\n");
        sem_wait(semFinRejoue);

    }while(strcmp(nouvTour, "O") == 0);

    printf("Client : %s quitte la partie\n", listeJoueurs->joueurs[indexJoueur].pseudo);


    return 0;
}