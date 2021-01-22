#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <string.h>
#include <pthread.h>
#include<unistd.h>
#include <stdlib.h>

#define SIZE 1024

void write_file(int sockfd){
     int n;
     FILE *fp;
     char *filename = "recv.txt";
     char buffer[SIZE];

     fp = fopen(filename, "w");
     while (1) {
          n = recv(sockfd, buffer, SIZE, 0);
          if (n <= 0){
               break;
               return;
          }
          fprintf(fp, "%s", buffer);
          bzero(buffer, SIZE);
     }
     puts("Prijem datoteke uspešno izvršen");
     return;
}

int main(int argc,char *argv[])
{  
     //Utičnica servera
     int serverSock;
     serverSock = socket(AF_INET, SOCK_STREAM, 0);
     if(serverSock == -1){
          printf("Greška pri kreiranju utičnice!\n");
          return 1;
     }  
     else printf("Utičnica kreirana\n");

     struct sockaddr_in server;
	server.sin_family = AF_INET;
     server.sin_addr.s_addr = INADDR_ANY; 
     if(argc > 1)
	     server.sin_port = htons(atoi(argv[1]));
	else
		server.sin_port = htons(27000);

     if(bind(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0){
          perror("Greška pri dodavanju adrese utičnici!");
          return 1;
     }

     //Podaci klijenta
	int clientSock, cSize;
	struct sockaddr_in client;

     //Očekivanje nadolazećih konekcija
     listen(serverSock, 1);  
	puts("Čekanje na nadolazeće konekcije...");
  
     //Prihvatanje nadolazećih konekcija
     clientSock = accept(serverSock, (struct sockaddr*)&client, (socklen_t *)&cSize);
     if(clientSock < 0){
		perror("Greška pri prihvatanju konekcije!");
		return 1;
	}
     puts("Konekcija uspostavljena");
     
     write_file(clientSock);

     close(serverSock);
     close(clientSock);

     return 0;  
 }  
