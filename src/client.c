#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#define DEFAULT_BUFLEN 64

typedef struct {
    uint8_t ver;
    uint8_t nmethods;
    uint8_t methods[2];
} cl_metod_verifikacije;

typedef struct {
    uint8_t ver;
    uint8_t method;
} sr_metod_verifikacije;

typedef struct {
	char username[DEFAULT_BUFLEN];
	char password[DEFAULT_BUFLEN];
    uint8_t ver;
    uint8_t uLength;
    uint8_t passLength;
} upass_auth;

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint8_t rsv; 
    uint8_t atyp;
} client_request;
	
    
 int main(int argc, char* argv[])  
 {  
    //Uticnica klijenta
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        printf("Greska pri kreiranju uticnice!\n");
        return 1;
    }
    else printf("Uticnica kreirana\n");

    //Adresa proksija
    char proxyIP[17];
    printf("Adresa proksija: ");
    fgets(proxyIP,17,stdin);

    //Port proksija
    uint16_t proxyPort;
    printf("Port proksija: ");
    scanf("%hd", &proxyPort);
    //sin_addr.s_addr

    //Uticnica proksija
    struct sockaddr_in proxy;
    proxy.sin_addr.s_addr = inet_addr(proxyIP);
    proxy.sin_family = AF_INET;
    proxy.sin_port = htons(proxyPort);

    //Uspostavljanje konekcije sa proksijem
    if(connect(sock, (struct sockaddr *)&proxy, sizeof(proxy)) < 0){
        perror("Greska pri uspostavljanju konekcije sa proksijem");
        return 1;
    }
    printf("Uspesna konekcija sa proksijem!\n");

    char bafer1[DEFAULT_BUFLEN];
    cl_metod_verifikacije *podrzane_metode;
    podrzane_metode = (cl_metod_verifikacije *)bafer1;
    podrzane_metode->ver = 0x05;
    podrzane_metode->nmethods = 0x02; 
    podrzane_metode->methods[0] = 0x00; 
    podrzane_metode->methods[1] = 0x02;

    //Slanje strukture za potvrdu metoda verifikacije
    if(send(sock, podrzane_metode, sizeof(podrzane_metode), 0) < 0){
        perror("Greska pri slanju metoda verifikacije proksiju!");
        return 1;
    }

    char bafer2[DEFAULT_BUFLEN];
    if(recv(sock, bafer2, DEFAULT_BUFLEN, 0) < 0){
        perror("Greska pri prijemu podataka od proksija!");
        return 1;
    }
    sr_metod_verifikacije *prijemPotvrde;
    prijemPotvrde = (sr_metod_verifikacije *)bafer2;
    switch(prijemPotvrde->method){
        case 0x00:  //Nije realizovano jer nije tema zadatka
            puts("Metod bez autentifikacije");
            close(sock);
            return 0;
        case 0x01:  //Isto nije tema zadatka
            puts("GSSAPI");
            close(sock);
            return 0;
        case 0x02:  
            puts("Korisnicko ime/lozinka autentifikacija");

            char bafer3[DEFAULT_BUFLEN*3], korisnickoIme[DEFAULT_BUFLEN], lozinka[DEFAULT_BUFLEN];
            memset(bafer3, 1, sizeof(bafer3));
            upass_auth *autorizacija;
            autorizacija = (upass_auth *)bafer3;
            int i;
            printf("Korisničko ime: ");
            getchar();  //praznjenje ulaznog toka
            fgets(korisnickoIme,DEFAULT_BUFLEN,stdin);
            strcpy(autorizacija->username, korisnickoIme);
            autorizacija->uLength = strlen(korisnickoIme);

            printf("Lozinka: ");
            fgets(lozinka,DEFAULT_BUFLEN,stdin);
            strcpy(autorizacija->password, lozinka);
            autorizacija->passLength = strlen(lozinka);

            fputs(autorizacija->username, stdout);
            fputs(autorizacija->password, stdout);

            if(send(sock, autorizacija, sizeof(autorizacija), 0) < 0){
                perror("Greska pri slanju podataka proksiju!");
                close(sock);
                return 1;
            }

            break;
        case 0xff:
            puts("Nepodrzan metod autentifikacije");
            close(sock);
            return 0;
        default:
            close(sock);
            return 0;
    }



//**********************************************************************************
#pragma region staro

      //socket variables  
      /*char IP[200];  
      char port[200];  
      //char buffer[65535];  
      int client_socket;  
      struct sockaddr_in client_sd;  //proxy
      printf("\nEnter proxy address:");  
      fgets(IP,sizeof("127.0.01\n")+1,stdin);  
      fputs(IP,stdout);  
      printf("\nEnter a port:");  
      fgets(port,sizeof("5000\n")+1,stdin);  
      fputs(port,stdout);  */
  
      // create a socket  
      /*client_socket = socket(AF_INET, SOCK_STREAM, 0);
	  if (client_socket == -1)
      {
        fprintf(stderr, "Error creating socket %s\n", strerror(errno));

        exit(EXIT_FAILURE);
      }*/
           
      //memset(&client_sd, 0, sizeof(client_sd));  
      // set socket variables  
      /*client_sd.sin_family = AF_INET;  
      client_sd.sin_port = htons(5000);  
      // assign any IP address to the client's socket  
      client_sd.sin_addr.s_addr = INADDR_ANY;   
      // connect to proxy server at mentioned port number  
      connect(client_socket, (struct sockaddr *)&client_sd, sizeof(client_sd));  */
      //odradjeno do ovog dela
      //send and receive data continuously
#pragma endregion  
       
 /*      while(1)  
            {  		

            puts("Unesite vase korisnicko ime");
            char user[100];
            scanf("%s", user);
            
            puts("Unesite sifru");
            char password[100];
            scanf("%s", password);
            
            
            if (send(client_socket, &user, sizeof(user), 0) < 0)
            {
                perror("send failed. Error");
                return 1;
            } 
            
            if (send(client_socket, &password, sizeof(password), 0) < 0)
            {
                perror("send failed. Error");
                return 1;
            } 
            
            char req_buf[1024];
			client_request * request;
			request = (client_request *)req_buf;
			request->ver = 0x05;
			request->cmd= 0x01;
			request->rsv= 0x00;
			request->atyp = 0x01;
			
			if (send(client_socket, request, sizeof(request), 0) < 0)
            {
                perror("send failed. Error");
                return 1;
            } 
            
               
			puts("Unesite ip adresu servera");
            char server_ip[100];
            scanf("%s", server_ip);
            
            puts("Unesite port servera");
            char server_port[100];
            scanf("%s", server_port);
            
            
            if (send(client_socket, &server_ip, sizeof(server_ip), 0) < 0)
            {
                perror("send failed. Error");
                return 1;
            } 
            
            if (send(client_socket, &server_port, sizeof(server_port), 0) < 0)
            {
                perror("send failed. Error");
                return 1;
            } 
            
           };  */
           //close(sd);

    close(sock);

    return 0;  
 }
