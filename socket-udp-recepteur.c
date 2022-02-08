
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

/*  La réception de messages UDP nécessite de préciser l’adresse IP et le numéro de port sur lesquels les messages sont reçus. 
    Ces informations sont définies dans une prise socket représentée par la structure struct sockaddr_in en IPv4. */

#define SERVADDR "0.0.0.0"         // Définition de l'adresse IP d'écoute
#define SERVPORT "0"               // Définition du port d'écoute si 0, port choisi dynamiquement
#define MAXBUFFERLEN 1024
#define MAXHOSTLEN 64
#define MAXPORTLEN 6
#define MAXFILELEN 128


int main(){
	
	int ecode;                      // Code retour des fonctions	
	char serverAddr[MAXHOSTLEN];    // Adresse du récepteur
	char serverPort[MAXPORTLEN];    // Port du récepteur
	int descSock;                   // Descripteur de socket d’écoute
	struct addrinfo hints;          // Contrôle la fonction getaddrinfo
	struct addrinfo *res=0;         // Contient le résultat de la fonction getaddrinfo
	struct sockaddr_storage myinfo; // Informations sur la prise locale
	struct sockaddr_storage from;   // Informations sur l’émetteur
	socklen_t len,src_addr_len;     // Variables utilisées pour stocker les longueurs des structures de socket
	char buffer[MAXBUFFERLEN];      // Tampon de communication entre le client et le serveur
	FILE *f;                        // descripteur de fichier de stockage des données reçues
	char ficRec[MAXFILELEN];
	int fini;
	int nbrec;


	// Mise à zéro de hints
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
	ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
	if (ecode) {
		fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
		exit(1);
	}

	/* Le paramètre hints contient des précisions sur la conversion du nom en adresse */
	/* Le champ address family peut être mis à unspecified permettant de récupérer des adresses IPv4 comme IPv6 */

 
	/* Le champ type est mis à SOCK_DGRAM autorisant UDP et excluant TCP  */
	/* Le protocole est maintenu à unspecified. Ce champ n’a de sens que lorsqu’on a précisé la famille d’adresses */
	/* Si AF_INET ou AF_INET6 avaient été précisés, nous aurions alors mis IPPROTO_UDP ou encore tout simplement 0 (protocole par défaut) */
	/* Le drapeau AI_PASSIVE est positionné car l’adresse est sensée être utilisée par un programme qui peut demander à être joint */
	/* Ce qui a pour effet d’utiliser l’adresse « joker » par défaut et non l’adresse de loopback */

	/* Le drapeau AI_ADDRCONFIG est également positionné afin de retourner les adresses configurées IPv4 comme IPv6 */
	/* Le paramètre res récupère la liste chaînée de structures addrinfo contenant la ou les adresses résolues */
	/* La place mémoire occupée par la liste doit être libérée par appel à freeaddrinfo mais pas avant la création de la prise et de sa publication */

	// Initialisation de la socket de communication IPv4/UDP
	descSock = socket(res->ai_family, res->ai_socktype,res->ai_protocol);
	if (descSock == -1) {
		perror("Erreur création socket d’écoute\n");
		exit(2);
	}

	// Publication de la socket au niveau du système
	// Assignation d'une adresse IP et un numéro de port
	ecode = bind(descSock, res->ai_addr, res->ai_addrlen);
	if (ecode == -1) {
		perror("Erreur liaison de la socket d’écoute");
		exit(3);
	}
	// Nous n'avons plus besoin de cette liste chainée addrinfo
	freeaddrinfo(res);	
	
	// Récupération du nom de la machine et du numéro de port pour affichage à l'écran
	len=sizeof(myinfo);
	ecode=getsockname(descSock, (struct sockaddr *) &myinfo, &len);
	if (ecode == -1) { 
		perror("Récepteur: getsockname"); 
		exit(4);
	}
	
	ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr, MAXHOSTLEN, serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV); 
	if (ecode != 0) {
		fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
		exit(4);
	}
	printf("L'adresse d'ecoute est: %s\n", serverAddr);
	printf("Le port d'ecoute est: %s\n", serverPort);
	/* réception de messages et stockage dans le fichier de nom constitué du nom de la machine et du suffixe « .data » */
	/* le programme se terminant lorsque la chaine « stop » est reçue */

	ecode=gethostname(ficRec, sizeof(ficRec));

	if (ecode == -1) { 
		perror("Récepteur: gethostname"); 
		exit(5);
	}

	strcat(ficRec,".data");
	f = fopen(ficRec,"a");         // ouverture du fichier en mode création et écriture

	if (f==NULL) {
		perror("Récepteur : fopen");
		exit(6);
	}
	
	fini=0;
	while (fini==0) {
		bzero(buffer,sizeof(buffer));
		src_addr_len=sizeof(from);
		nbrec=recvfrom(descSock,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&from,&src_addr_len);
		if (strncmp(buffer+nbrec-5,"stop",4)==0) {
			fini=1;
		}
		else {
			printf("chaine reçue %s\n",buffer);
			fprintf(f,"%s", buffer);
		}
	}
	//Fermeture de la prise : fin de la communication
	close(descSock);
	fclose(f);
}	
	
