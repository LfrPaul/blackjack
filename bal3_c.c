/* ========================================================================== */
/* Date : 28 novembre 2022                                                    */
/* Auteur : Paul Lefevre                                                      */
/* Fichier bal3_c                                                             */
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
#include "boiteauxlettres.h"

key_t cle;
int msqid;
int ret;
struct msqid_ds buf;

struct msgbuf msgp;

int main(int argc, char* argv[]) {
    cle = ftok("fichierftok", 10); // création de la clé

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans msqid
    msqid = msgget(cle, 0666); 

    if(msqid == -1) {
        perror("Erreur msgget");
        exit(-1);
    }

    printf("Je suis le client (PID:%d)\n", getpid());

    msgrcv(msqid, &msgp, sizeof(msgp.mtext), 1, 0);
        
    printf("Message reçu du serveur\n");
    printf(">Numero: %d\n>Couleur: %c\n", msgp.mtext[0].numero, msgp.mtext[0].couleur);
    printf(">Joueur nom:%s\n>Joueur pid: %d\n", msgp.mtext[0].possesseur.pseudo, msgp.mtext[0].possesseur.pid);

    return 0;
}

void printInfoBoites() {
    // récupération des informations de la file dans la variable buf;
    ret = msgctl(msqid, IPC_STAT, &buf);

    if(ret == -1) {
        printf("Erreur msgctl\n");
        exit(-1);
    } else {
        printf("UID de l'owner : %d\n", (&buf)->msg_perm.uid);
        printf("GID de l'owner : %d\n", (&buf)->msg_perm.gid);
        printf("UID du créateur : %d\n", (&buf)->msg_perm.cuid);
        printf("GID du créateur : %d\n", (&buf)->msg_perm.cgid);
        printf("Date du dernier msgsnd : %ld\n", (&buf)->msg_stime);
        printf("Date du dernier msgrcv : %ld\n", (&buf)->msg_rtime);
        printf("Date de création: %ld\n", (&buf)->msg_ctime);
        printf("Nombre actuel d'octets dans la file: %ld\n", (&buf)->__msg_cbytes);
        printf("Nombre de messages dans la file: %ld\n", (&buf)->msg_qnum);
        printf("Nombre max d'octets autorisés dans la file: %ld\n", (&buf)->msg_qbytes);
        printf("PID du dernier msgsnd : %d\n", (&buf)->msg_lspid);
        printf("PID du dernier msgcrv : %d\n", (&buf)->msg_lrpid);
    }
}