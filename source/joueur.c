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

void afficherMainJoueur(joueur_t *joueur){
    if(joueur->nbCartes < 5){
        for(int i = 0; i < joueur->nbCartes; i++){
            printf("**********\n");
            printf("Carte n°%d -> valeur %d -> couleur %s \n", i, joueur->main[i].valeur, joueur->main[i].couleur);
            printf("**********\n");
        }
    }
    else {
        printf("?\n");
    }
}

void afficherMainCroupier(croupier_t *croupier){
    if(croupier->nbCartes < 5){
        for(int i = 0; i < croupier->nbCartes; i++){
            printf("**********\n");
            printf("Carte n°%d \n valeur %d \n couleur %s \n", i, croupier->main[i].valeur, croupier->main[i].couleur);
            printf("**********\n");
        }
    }
    else {
        printf("?\n");
    }
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