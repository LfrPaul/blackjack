/**
 * @file joueur.c
 * @author Alexis POPIEUL et Paul LEFEVRE 
 * @brief Fonctions des joueurs
 * @version 0.1
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../include/joueur.h"

int findPlayerIndex(pid_t pid, listeJoueurs_t* listeJ) {
    for(int i = 0; i < listeJ->nbJoueurs; i++) {
        if(listeJ->joueurs[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

int uneCartePourTousJoueurs(paquetCarte_t* pCartes, listeJoueurs_t* pJoueurs){
    printf("On distribue une carte a tout les joueurs (%d) parmis les cartes disponibles\n", pJoueurs->nbJoueurs);

    int nbCartesDispo = 0;
    for(int i = 0; i<NB_CARTES; i++){
        if(pCartes->tabCartes[i].dansPioche == 1)
            nbCartesDispo++;
    }


    printf("Il y a %d carte(s) disponible(s)\n", nbCartesDispo);
    srand(time(NULL));
    int indexCarteAlea;
    int iDispo;
    int indexJoueur;

    for(int draw = 0; draw<pJoueurs->nbJoueurs; draw++){
        indexCarteAlea = rand()%(nbCartesDispo);
        iDispo = 0;

        for(int i = 0; i<NB_CARTES ; i++){
            if(pCartes->tabCartes[i].dansPioche == 1){
                if(iDispo == indexCarteAlea){
                    printf("on affecte une carte a un joueur\n");
                    int carteEnMain = pJoueurs->joueurs[draw].nbCartes;
                    memcpy(&(pJoueurs->joueurs[draw].main[carteEnMain]), &(pCartes->tabCartes[i]), sizeof(carte_t));
                    //pJoueurs->joueurs[draw].main[0] = &(segmentListeJoueurs->tabJoueurs[indexJoueur]);
                    pJoueurs->joueurs[draw].nbCartes++;
                    pCartes->tabCartes[i].dansPioche = 0;
                    nbCartesDispo--;
                    printf("affectation reussie\n");
                    iDispo++;
                }
                else{
                    iDispo++;
                }
            }
        }
    }

    return 0;

}

char * affichageCouleur(char couleur[10]) {
    if(strcmp(couleur, "Coeur") == 0 || strcmp(couleur, "Carreau") == 0)
        return RED;
    else if(strcmp(couleur, "Trefle") == 0 || strcmp(couleur, "Pic") == 0)
        return BLCK;
}
char * getSymbole(char couleur[10]) {
    if(strcmp(couleur, "Coeur") == 0) {
        return "♡";
    }
    else if(strcmp(couleur, "Carreau") == 0) {
        return "◇";
    }
    else if(strcmp(couleur, "Trefle") == 0) {
        return "♧";
    } 
    else if(strcmp(couleur, "Pic") == 0) {
        return "♤";
    }
}
char * getCarteName(int valeur) {
    switch(valeur) {
        case 1:
            return "A";
            break;
        case 11:
            return "J";
            break;
        case 12:
            return "D";
            break;
        case 13:
            return "R";
            break;
        default:
            char *buf = malloc(2);
            sprintf(buf, "%d", valeur);
            return buf;
            break;
    }
}

void afficherMainJoueur(joueur_t *joueur){
    printf("%s", BOLD);
    if(joueur->nbCartes < NB_CARTE_EN__MAIN){
        for(int i = 0; i < joueur->nbCartes; i++){
            printf("%s        %s  ", WHT_BG, RESET);
        }
        printf("\n");
        for(int i = 0; i < joueur->nbCartes; i++){
            printf("%s%s", WHT_BG, affichageCouleur(joueur->main[i].couleur));

            printf(" %-*s  %s  ", 2, getCarteName(joueur->main[i].valeur), getSymbole(joueur->main[i].couleur));

            printf("%s  ", RESET);
        }
        printf("\n");
        for(int j = 0; j < 2; j++) {
            for(int i = 0; i < joueur->nbCartes; i++){
                printf("%s        %s  ", WHT_BG, RESET);
            }
            printf("\n");
        }
        for(int i = 0; i < joueur->nbCartes; i++){
            printf("%s%s", WHT_BG, affichageCouleur(joueur->main[i].couleur));

            printf(" %s   %*s ", getSymbole(joueur->main[i].couleur), 2, getCarteName(joueur->main[i].valeur));

            printf("%s  ", RESET);
        }
        printf("\n");
        for(int i = 0; i < joueur->nbCartes; i++){
            printf("%s        %s  ", WHT_BG, RESET);
        }
        printf("\n");
    }
    else {
        printf("?\n");
    }
    printf("%s", RESET_ALL);
}

void afficherMainCroupier(croupier_t *croupier){
    printf("%s", BOLD);
    if(croupier->nbCartes < NB_CARTE_EN__MAIN){
        for(int i = 0; i < croupier->nbCartes; i++){
            printf("%s        %s  ", WHT_BG, RESET);
        }
        printf("\n");
        for(int i = 0; i < croupier->nbCartes; i++){

            if(i == 0 && croupier->debutPartie == 1){
                printf("%s%s", WHT_BG, BLCK);
                printf(" ?    ? ");
            } else {
                printf("%s%s", WHT_BG, affichageCouleur(croupier->main[i].couleur));
                printf(" %-*s  %s  ", 2, getCarteName(croupier->main[i].valeur), getSymbole(croupier->main[i].couleur));
            }

            printf("%s  ", RESET);
        }
        printf("\n");
        for(int j = 0; j < 2; j++) {
            for(int i = 0; i < croupier->nbCartes; i++){
                printf("%s        %s  ", WHT_BG, RESET);
            }
            printf("\n");
        }
        for(int i = 0; i < croupier->nbCartes; i++){
            if(i == 0 && croupier->debutPartie == 1){
                printf("%s%s", WHT_BG, BLCK);
                printf(" ?    ? ");
            } else {
                printf("%s%s", WHT_BG, affichageCouleur(croupier->main[i].couleur));
                printf(" %s   %*s ", getSymbole(croupier->main[i].couleur), 2, getCarteName(croupier->main[i].valeur));
            }

            printf("%s  ", RESET);
        }
        printf("\n");
        for(int i = 0; i < croupier->nbCartes; i++){
            printf("%s        %s  ", WHT_BG, RESET);
        }
        printf("\n");
    }
    else {
        printf("?\n");
    }
    printf("%s", RESET_ALL);
}

void printTable(croupier_t *croupier, listeJoueurs_t* listeJoueurs, int indexJoueur) {
    system("clear");


    for(int i = 0; i < listeJoueurs->nbJoueurs; i++) {
        if(i != indexJoueur) {
            printf("\nMain du joueur %s : \n", listeJoueurs->joueurs[i].pseudo);
            afficherMainJoueur(&(listeJoueurs->joueurs[i]));
            printf("Score : %d\n", getPlayerScore(listeJoueurs->joueurs[i].main, listeJoueurs->joueurs[i].nbCartes));
        }
    }

    printf("--------------------------------------------------------------\n");

    printf("\nMain du croupier : \n");
    afficherMainCroupier(croupier);

    printf("\nVotre main : \n");
    afficherMainJoueur(&(listeJoueurs->joueurs[indexJoueur]));

    printf("Votre score : %d\n", getPlayerScore(listeJoueurs->joueurs[indexJoueur].main, listeJoueurs->joueurs[indexJoueur].nbCartes));

    printf("\nVOTRE MISE : %d\n\n", (listeJoueurs->joueurs[indexJoueur]).mise);
    
}



void uneCartePourUnePersonnne(paquetCarte_t* pCartes, carte_t * main, int* nbCartes){
    printf("On distribue une carte dans la main parmis les cartes disponibles\n");
    //printf("nbCartes = %d\n" , *nbCartes);

    int nbCartesDispo = 0;
    for(int i = 0; i<NB_CARTES; i++){
        if(pCartes->tabCartes[i].dansPioche == 1)
            nbCartesDispo++;
    }

    printf("Il y a %d carte(s) disponible(s)\n", nbCartesDispo);
    srand(time(NULL));
    int indexCarteAlea;
    int iDispo;

    indexCarteAlea = rand()%(nbCartesDispo);
    iDispo = 0;

    for(int i = 0; i<NB_CARTES ; i++){
        if(pCartes->tabCartes[i].dansPioche == 1){
            if(iDispo == indexCarteAlea){
                printf("on affecte une carte à la main\n");
                memcpy((&main[*nbCartes]), &(pCartes->tabCartes[i]), sizeof(carte_t));
                *nbCartes = *nbCartes + 1;
                pCartes->tabCartes[i].dansPioche = 0;
                printf("affectation reussie\n");
                iDispo++;
            }
            else{
                iDispo++;
            }
        }
    }

    //printf("nbCartes = %d\n" , *nbCartes);
}

int getPlayerScore(carte_t main[NB_CARTE_EN__MAIN], int nbCartes) {
    int score = 0, as = 0;
    for(int i = 0; i < nbCartes; i++){
        if(main[i].valeur != 1) {
            score = score + ((main[i].valeur < 10)?main[i].valeur:10);
        } else {
            as++;
        }
    }

    for(int i = 0; i < as; i++) {
        if(score + 11 > 21) {
            score++;
        } else {
            score = score + 11;
        }
    }

    return score;
}