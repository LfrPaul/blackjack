/**
 * @file serveur.c
 * @author Alexis POPIEUL et Paul LEFEVRE 
 * @brief La cinématique mis en place par le serveur
 * @version 0.1
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
   
#include "../include/joueur.h"

key_t cleJoueur,cleCarte, keyConnexion, cleCroupier;
int balConnexionID; // ID de la file de messages
int memConnexionID; // ID de la mémoire partagée des joueurs
int memCartes; // ID de la mémoire partagée des joueurs
int memCroupier; // ID de la mémoire partagée du croupier
int descripteur;
struct msqid_ds buf;

sem_t *semInit;
sem_t *semMain;

char input[100];

// variables des joueurs
listeJoueurs_t *listeJoueurs;
int joueurEnJeu = 0;
newPlayer_t new_player;

// variables des cartes
paquetCarte_t *paquetCartes;

// Le croupier
croupier_t *croupierJeu;

int main(int argc, char* argv[]) {

    // Tokens pour la BAL et les mémoire partagées (depuis le répertoire BlackJack)
    keyConnexion = ftok("token/balConnexion", 21); // création de la clé de la BAL
    cleJoueur = ftok("token/shmJoueur", 21); // création de la clé pour la mémoire partagée des joueurs
    cleCarte = ftok("token/shmCarte", 21); // création de la clé pour la mémoire partagée des cartes
    cleCroupier = ftok("token/shmCroupier", 21); // création de la clé pour la mémoire partagée du croupier

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balConnexionID
    balConnexionID = msgget(keyConnexion, IPC_CREAT | 0666); 

    // création de la mémoire partagée de la liste de joueurs (ou ouverture si déjà existante),
    // et récupération de son ID dans memConnexionID
    memConnexionID = shmget(cleJoueur, sizeof(listeJoueurs_t), IPC_EXCL | IPC_CREAT | 0666);

    // création de la mémoire partagée du paquet de carte (ou ouverture si déjà existante),
    // et récupération de son ID dans memCartes
    memCartes = shmget(cleCarte, sizeof(paquetCarte_t), IPC_EXCL | IPC_CREAT | 0666);

    // création de la mémoire partagée du croupier (ou ouverture si déjà existante),
    // et récupération de son ID dans memCartes
    memCroupier = shmget(cleCroupier, sizeof(croupier_t), IPC_EXCL | IPC_CREAT | 0666);

    if(balConnexionID == -1 || memConnexionID == -1 || memCartes == -1 || memCroupier == -1){
        printf("il y a eu un erreur pour créer la BAL ou la memoire partagée.\n");
        exit(-1);
    }

    // Initialisation du sémaphore d'initialisation de la partie
    semInit = sem_open("/INITPARTIE.SEMAPHORE", O_CREAT | O_RDWR, 0600, 0);

    // Initialisation du sémaphore d'affichage des mains de chaque joueur
    semMain = sem_open("/MAIN.SEMAPHORE", O_CREAT | O_RDWR, 0600, 0);


    printf("Je suis le serveur\nCréation de la partie...\n");
    
    // On initialise le paquet de 52 cartes 
    paquetCartes = (paquetCarte_t *) shmat(memCartes,NULL,0);
    initPaquetCartes(paquetCartes);


    // attente de reception d'un message d'un client
    listeJoueurs = (listeJoueurs_t *) shmat(memConnexionID,NULL,0);
    while(listeJoueurs->nbJoueurs != MIN_JOUEURS) {
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
        printf("Fin fils : %d\n", joueurEnJeu);
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

        int index = findPlayerIndex(new_player.joueur.pid, listeJoueurs); // On récupère l'index du joueur dans la liste des joueurs

        if(index != -1) {
            listeJoueurs->joueurs[index].mise = new_player.joueur.mise; // on met à jour sa mise
        }
    }


    // Distribution des cartes 
    printf("on distribue 2 cartes a chaque joueur\n");
    uneCartePourTousJoueurs(paquetCartes, listeJoueurs);
    uneCartePourTousJoueurs(paquetCartes, listeJoueurs);
    
    printf("On affiche la main %d joueurs côté serveur\n", listeJoueurs->nbJoueurs);
    for(int i = 0; i<listeJoueurs->nbJoueurs; i++){
        afficherMainJoueur(&(listeJoueurs->joueurs[i]));
    }
    
    
    // on distribue 1 carte au croupier 
    printf("On distribue 1 carte au croupier\n");
    croupierJeu = (croupier_t *) shmat(memCroupier,NULL,0);
    croupierJeu->pid = getpid();
    croupierJeu->nbCartes = 0;
    uneCartePourUnePersonnne(paquetCartes, croupierJeu->main, &(croupierJeu->nbCartes));

    printf("Carte n°1 du croupier : %d %s \n", croupierJeu->main[0].valeur, croupierJeu->main[0].couleur);
    printf("Le croupier a %d carte(s)\n" , croupierJeu->nbCartes);


    // on fait afficher la propre main de chaque joueur côté joueur
    printf("Affichage des main des joueurs\n");
    for(int i = 0; i < listeJoueurs->nbJoueurs; i++) {
        sem_post(semMain);
        printf("Affichage de la main du joueur %d : %s\n", listeJoueurs->joueurs[i].pid, listeJoueurs->joueurs[i].pseudo);
    }
    

    printf("on supprime la boite au lettre, les mémoires partagées et la semaphore\n");
    shmctl(memConnexionID, IPC_RMID, NULL);
    shmctl(memCartes, IPC_RMID, NULL);
    shmctl(memCroupier, IPC_RMID, NULL);
    msgctl(balConnexionID, IPC_RMID, 0);
    sem_close(semInit);
    sem_unlink("INITPARTIE.SEMAPHORE");
    sem_close(semMain);
    sem_unlink("MAIN.SEMAPHORE");


    return 0;
}
