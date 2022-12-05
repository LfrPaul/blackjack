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
#include "boiteauxlettres.h"

key_t cle, keyConnexion;
int balConnexionID;
int ret;
int descripteur;
struct msqid_ds buf;

char test[100];

struct newPlayer my_infos;

pid_t monPid;

int main(int argc, char* argv[]) {
    keyConnexion = ftok("balConnexion", 10); // création de la clé

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans balConnexionID
    balConnexionID = msgget(keyConnexion, 0666); 

    printf("Client\n");

    // ret = mkfifo("pipeConnexion", 0666);
    // descripteur = open("tube", O_RDONLY);

    monPid = getpid();

    my_infos.mtype = 1;
    my_infos.pid = monPid;

    printf("Saisissez votre pseudo :");
    scanf("%[^\n]s", my_infos.name);

    printf("Client:  Pid:%d, Nom:%s", my_infos.pid, my_infos.name);

    getchar();
    // On envoie le message au serveur
    msgsnd(balConnexionID, &my_infos, sizeof(my_infos.name) + sizeof(pid_t), 0);

    // création de la file de messages (ou ouverture si déjà existante), et récupération de son ID dans msqid
    // msqid = msgget(cle, IPC_CREAT | 0666); 

    // if(msqid == -1) {
    //     perror("Erreur msgget");
    //     exit(-1);
    // }

    // printf("Je suis le serveur\n");

    // listeJoueurs[0].pid = getpid();
    // strcpy(listeJoueurs[0].pseudo, "Paul");
    // listeJoueurs[0].score = 50;

    // msgp.mtype = 1;
    // msgp.mtext[0].numero = 1;
    // msgp.mtext[0].couleur = 'A';

    // msgp.mtext[0].possesseur = listeJoueurs[0];

    // // envoi de 10 messages dans la file
    // ret = msgsnd(msqid, &msgp, sizeof(msgp.mtext), 0);

    // sleep(4);
    
    return 0;
}

// void printInfoBoites() {
//     // récupération des informations de la file dans la variable buf;
//     ret = msgctl(msqid, IPC_STAT, &buf);

//     if(ret == -1) {
//         printf("Erreur msgctl\n");
//         exit(-1);
//     } else {
//         printf("UID de l'owner : %d\n", (&buf)->msg_perm.uid);
//         printf("GID de l'owner : %d\n", (&buf)->msg_perm.gid);
//         printf("UID du créateur : %d\n", (&buf)->msg_perm.cuid);
//         printf("GID du créateur : %d\n", (&buf)->msg_perm.cgid);
//         printf("Date du dernier msgsnd : %ld\n", (&buf)->msg_stime);
//         printf("Date du dernier msgrcv : %ld\n", (&buf)->msg_rtime);
//         printf("Date de création: %ld\n", (&buf)->msg_ctime);
//         printf("Nombre actuel d'octets dans la file: %ld\n", (&buf)->__msg_cbytes);
//         printf("Nombre de messages dans la file: %ld\n", (&buf)->msg_qnum);
//         printf("Nombre max d'octets autorisés dans la file: %ld\n", (&buf)->msg_qbytes);
//         printf("PID du dernier msgsnd : %d\n", (&buf)->msg_lspid);
//         printf("PID du dernier msgcrv : %d\n", (&buf)->msg_lrpid);
//     }
// }