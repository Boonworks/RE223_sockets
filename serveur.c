#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAXREQ 10
#define BUFFER_SIZE 100

double calculatrice(char* requete){

    double nb1;
    double nb2;
    char op;

    sscanf(requete, "%lf %c %lf", &nb1, &op, &nb2);

    if ((nb1 > 10000.) || (nb1 < 0.)){
        return -1;
    }
    else if ((nb2 > 10000.) || (nb2 < 0.)){
        return -1;
    }
    else if (op == '+'){
        return nb1 + nb2;
    }
    else if (op == '-'){
        return nb1 - nb2;
    }
    else if (op == '*'){
        return nb1 * nb2;
    }
    else if (op == '/'){
        return nb1 / nb2;
    }
    else{
        return -2;
    }
}

int main(int argc, char** argv)
{

    struct sockaddr_in addr_serveur;    /* Structure Internet sockaddr_in */
    struct hostent *hptr;               /* Infos sur le serveur */
    int port;                           /* Numero de port du serveur */

    int socket_client;                  /* Id de la socket entrante */
    struct sockaddr_in addr_client;     /* sockaddr_in de la connection entrante */
    int longeur_client;
    struct hostent *newhptr;            /* Infos sur le client suivant /etc/hosts */

    const char* MESSAGE_ERREUR = "[Serveur]: Out of range... \n";
    const char* MESSAGE_CONF = "[Serveur]: La requete doit etre du type: nb1 op nb2 \n";

    port = atoi(argv[1]);

    printf("port: %d \n", port);

    /* Initialisation la structure sockaddr sa */
    /* Famille d'adresse : AF_INET = PF_INET */
    addr_serveur.sin_family = AF_INET;

    /* Initialisation du numero du port */
    addr_serveur.sin_port = htons(port);
    addr_serveur.sin_addr.s_addr = INADDR_ANY;

    int socket_serveur = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_serveur < 0){
        perror("[Serveur]: Erreur lors de la création du socket");
        exit(1);
    }

    int bind_serveur = bind(socket_serveur, (struct sockaddr *) &addr_serveur, sizeof(addr_serveur));

    if (bind_serveur < 0){
        printf("[Serveur]: Erreur lors du Bind \n");
        exit(1);
    }

    listen(socket_serveur, MAXREQ);
    printf("[Serveur]: Ecoute sur le port %d \n", port);

    int nb_connexion = 0;

    while(1){

        longeur_client = sizeof(addr_client);


        socket_client = accept(socket_serveur, (struct sockaddr *) &addr_client, &longeur_client);

        if (socket_client < 0){
            printf("[Serveur]: Erreur lors de l'acceptation d'une connexion entrante \n");
            exit(1);
        }

        nb_connexion += 1;

        printf("[Serveur]: Connexion No %d depuis le port %d... \n", nb_connexion, ntohs(addr_client.sin_port));

        pid_t pid;

        pid = fork();

        switch (pid)
        {
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);

            case 0:
                char buffer[BUFFER_SIZE];

            while (1)
            {
                ssize_t nb = read(socket_client, buffer, BUFFER_SIZE - 1);
                if (nb <= 0) break;                 // client fermé / erreur
                buffer[nb] = '\0';

                if (strcmp(buffer, "quit") == 0) {  // fermeture propre
                    break;
                }

                double resultat = calculatrice(buffer);
                if (resultat == -1) {
                    const char* MESSAGE_ERREUR = "[Serveur]: Out of range...\n";
                    write(socket_client, MESSAGE_ERREUR, strlen(MESSAGE_ERREUR));
                } else if (resultat == -2) {
                    const char* MESSAGE_CONF = "[Serveur]: La requete doit etre du type: nb1 op nb2\n";
                    write(socket_client, MESSAGE_CONF, strlen(MESSAGE_CONF));
                } else {
                    char res[BUFFER_SIZE];
                    snprintf(res, sizeof(res), "%lf\n", resultat);
                    write(socket_client, res, strlen(res));
                }
            }

            close(socket_client);
            exit(EXIT_SUCCESS);

            default:
            break;
        }

    }
    close(socket_serveur);
    printf("[Serveur]: Fin du serveur \n");
    exit(0);
}
