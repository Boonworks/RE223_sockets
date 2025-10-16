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

// Fonction calculatrice : traite une requête du type "nb1 op nb2"

double calculatrice(char* requete) {

    double nb1;
    double nb2;
    char op;

    // Lecture de la requête envoyée par le client
    sscanf(requete, "%lf %c %lf", &nb1, &op, &nb2);

    // Vérifications des bornes
    if ((nb1 > 10000.) || (nb1 < 0.)) {
        return -1; // erreur : nb1 hors limite
    }
    else if ((nb2 > 10000.) || (nb2 < 0.)) {
        return -1; // erreur : nb2 hors limite
    }
    // Sélection de l’opération
    else if (op == '+') {
        return nb1 + nb2;
    }
    else if (op == '-') {
        return nb1 - nb2;
    }
    else if (op == '*') {
        return nb1 * nb2;
    }
    else if (op == '/') {
        return nb1 / nb2;
    }
    else {
        return -2; // opérateur inconnu
    }
}

int main(int argc, char** argv) {

    struct sockaddr_in addr_serveur;    // Structure d’adresse du serveur
    struct hostent *hptr;               // Informations sur le serveur (non utilisées ici)
    int port;                           // Numéro de port du serveur

    int socket_client;                  // Socket du client une fois connecté
    struct sockaddr_in addr_client;     // Adresse du client
    int longeur_client;                 // Taille de la structure addr_client
    struct hostent *newhptr;            // Infos sur le client (non utilisées ici)

    // Messages d'erreur
    const char* MESSAGE_ERREUR = "[Serveur]: Out of range... \n";
    const char* MESSAGE_CONF = "[Serveur]: La requete doit etre du type: nb1 op nb2 \n";

    // Lecture du port passé en argument

    port = atoi(argv[1]);
    printf("port: %d \n", port);

    // Configuration de l’adresse du serveur

    addr_serveur.sin_family = AF_INET;          // Protocole IPv4
    addr_serveur.sin_port = htons(port);        // Conversion du port en format réseau
    addr_serveur.sin_addr.s_addr = INADDR_ANY;  // Écoute sur toutes les interfaces

    // Création de la socket serveur

    int socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_serveur < 0) {
        perror("[Serveur]: Erreur lors de la création du socket");
        exit(1);
    }

    // Association de la socket à l’adresse (bind)

    int bind_serveur = bind(socket_serveur, (struct sockaddr *) &addr_serveur, sizeof(addr_serveur));
    if (bind_serveur < 0) {
        printf("[Serveur]: Erreur lors du Bind \n");
        exit(1);
    }


    // Mise en écoute du serveur 

    listen(socket_serveur, MAXREQ);
    printf("[Serveur]: Ecoute sur le port %d \n", port);

    int nb_connexion = 0; // Compteur de connexions clients


    // Boucle infinie : attente de nouvelles connexions

    while (1) {

        longeur_client = sizeof(addr_client);

        // Attente d’un client
        socket_client = accept(socket_serveur, (struct sockaddr *) &addr_client, &longeur_client);
        if (socket_client < 0) {
            printf("[Serveur]: Erreur lors de l'acceptation d'une connexion entrante \n");
            exit(1);
        }

        nb_connexion += 1;
        printf("[Serveur]: Connexion No %d depuis le port %d... \n",
                nb_connexion, ntohs(addr_client.sin_port));


        pid_t pid = fork();

        switch (pid) {
            case -1: // Erreur de fork
                perror("fork");
                exit(EXIT_FAILURE);

            case 0: //communication avec le client
            {
                char buffer[BUFFER_SIZE];

                // Boucle de communication avec le client
                while (1) {
                    ssize_t nb = read(socket_client, buffer, BUFFER_SIZE - 1);

                    if (nb <= 0) break; // Fin de connexion
                    buffer[nb] = '\0';  // Terminaison de la chaîne

                    // Si le client tape "quit", on ferme la connexion
                    if (strcmp(buffer, "quit") == 0) {
                        break;
                    }

                    // Calcul du résultat
                    double resultat = calculatrice(buffer);

                    // Gestion des cas d’erreurs
                    if (resultat == -1) {
                        const char* MESSAGE_ERREUR = "[Serveur]: Out of range...\n";
                        write(socket_client, MESSAGE_ERREUR, strlen(MESSAGE_ERREUR));
                    } 
                    else if (resultat == -2) {
                        const char* MESSAGE_CONF = "[Serveur]: La requete doit etre du type: nb1 op nb2\n";
                        write(socket_client, MESSAGE_CONF, strlen(MESSAGE_CONF));
                    } 
                    else {
                        // Envoi du résultat au client
                        char res[BUFFER_SIZE];
                        snprintf(res, sizeof(res), "%lf\n", resultat);
                        write(socket_client, res, strlen(res));
                    }
                }

                // Fermeture de la connexion côté serveur
                close(socket_client);
                exit(EXIT_SUCCESS);
            }

            default:
                break;
        }

    }


    // Fin du programme

    close(socket_serveur);
    printf("[Serveur]: Fin du serveur \n");
    exit(0);
}
