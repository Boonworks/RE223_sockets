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
#define BUFSIZE 256

int main(int argc, char **argv){
	int sock;
	int port;
	struct sockaddr_in serv_address;
	struct hostent *hostp;
	struct servent *serveurp;
	char *myname;
	char buf[BUFSIZE];
	char *host,*user;

	/* Recuperation nom du serveur */
	hostp = gethostbyname(argv[1]);
	/* Recuperation des infos sur le serveur dans /etc/hosts pour par DNS */
	if(hostp == NULL) {
		printf("Probleme de recuperation d'infos sur le serveur\n");
		exit(1);
	}

	/* Copie de l'adresse du serveur et du type d'adresse dans sa */
	bcopy ((char *)hostp->h_addr, (char *)&serv_address.sin_addr, hostp->h_length);
	serv_address.sin_family = AF_INET;

	/* Recuperation numero de port */
	port = atoi(argv[2]);



	/*Creation du Socket et attribution des valeurs au struct*/
	sock=socket(AF_INET, SOCK_STREAM, 0);
	/* Recuperation  des infos sur le serveur et creation du socket*/
	if(sock<0){
		printf("Probleme lors de l'ouverture du socket!\n");
		exit(1);
	}



	/*Recuperation des informations du serveur*/
	serv_address.sin_family=AF_INET;
	serv_address.sin_port=htons(port);
	read(0, buf, BUFSIZE);
	/*Tentative de connexion au serveur*/
	if (connect(sock,(struct sockaddr*) &serv_address, sizeof(serv_address)) < 0 ) {
		perror("Problème de connexion avec le serveur");
		exit(1);
	}

	printf("Connecté au serveur %s:%d\n", argv[1], port);


	/*Boucle de communication*/
	while (1) {
    printf(">> ");
	/*Lecture de l'entrée écrite par le client*/
    if (!fgets(buf, BUFSIZE, stdin))break;
	printf("Vous avez entré : %s", buf);

    buf[strcspn(buf, "\n")] = 0; // Enlève le \n

    if (strcmp(buf, "quit") == 0) break; // Si "quit" → fin

    // Envoi
    if (write(sock, buf, strlen(buf)) <= 0) {
        perror("Erreur lors de l'envoi");
        exit(1);
    }

    // Réception
    int lecture = read(sock, buf, BUFSIZE-1);
    if (lecture <= 0) {
        printf("Serveur déconnecté.\n");
        break;
    }
    buf[lecture] = '\0';

    printf("= %s\n", buf);
}

close(sock);

}
