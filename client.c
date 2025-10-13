#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


int main(int argc,char** argv){

    int sd;
    struct sockaddr_in sa;		    /* Structure Internet sockaddr_in */
    struct hostent *hptr ; 		    /* Infos sur le serveur */
    char* serveur ;        		    /* Nom du serveur distant */
    int port;
    char buffer[256]= {0};          /* Buffer */
    char calcul[256];

    /* verification du nombre d'arguments de la ligne de commande */
    if (argc != 3) {
        printf("Erreur d'arguments\n");
        printf("Syntaxe : ./client Adresse port Calcul\n");
        exit(1);
    }

    /* Recuperation nom du serveur */
    serveur = argv[1];
    port = atoi(argv[2]);

    /* Recuperation des infos sur le serveur dans /etc/hosts ou par DNS */
    if((hptr = gethostbyname(serveur)) == NULL) {
        printf("Probleme de recuperation d'infos sur le serveur\n");
        exit(1);
    }

    /* Initialisation la structure sockaddr sa avec les infos formattees : */
    bcopy((char *)hptr->h_addr, (char*)&sa.sin_addr, hptr->h_length);

    /* Famille d'adresse : AF_INET = PF_INET */
    sa.sin_family = AF_INET;

    /* Initialisation du numero du port */
    sa.sin_port = htons(port);

    while(1){
        /* Etablissement de la connexion avec le serveur ftp */
        printf("\n");
        printf("Calcul : ");
        fgets(calcul,256,stdin);
        if(calcul[0]=='quit'){
            exit(0);
        }

        /* Creation de la socket TCP */
        if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Probleme lors de la creation de socket\n");
            exit(1);
        }

        /* Etablissement de la connexion avec le serveur ftp */
        if((connect(sd, (struct sockaddr *)&sa, sizeof(sa))) < 0 ) {
            printf("Probleme de connexion avec le serveur\n");
            exit(1);
        }

        /* Envoi de la commande ftp vers serveur ftp */
        write(sd, calcul, strlen(calcul));

        /* Lecture de la reponse du serveur ftp */
        int n = read(sd, buffer, sizeof(char)*256);

        write(1,buffer,n);
        close(sd);
    }

}

