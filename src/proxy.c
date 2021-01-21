#include <sys/socket.h>  
#include <sys/types.h>  
#include <resolv.h>  
#include <string.h>  
#include <stdlib.h>  
#include <pthread.h>  
#include<unistd.h>  
#include<netdb.h> //hostent  
#include<arpa/inet.h>  

#define DEFAULT_BUFLEN 64
#define BUF_SIZE 1024
#define DEFAULT_PORT 27015
#define AUTH 0x02	//Trazeni metod autentifikacije

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
    uint8_t ver;
    uint8_t status;
} auth_status;

typedef struct {
    uint8_t ver;
    uint8_t uLength;
    uint8_t passLength;
} upass_auth_len;

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint8_t rsv; 
    uint8_t atyp;
} client_request;

typedef struct {
    uint8_t ver;
    uint8_t rep;
    uint8_t rsv;
    uint8_t atyp;
} proxy_response;

/*
 * Funkcija koja prosleđuje pakete između dve utičnice
 * 
 * fd1 - utičnica 1
 * fd2 - utičnica 2
*/
void tcp_forward(int fd1, int fd2)
{
    uint8_t buf[BUF_SIZE];

    int nfds;
    nfds = fd1 > fd2 ? fd1 : fd2;
    nfds += 1;

    fd_set readfds;
    struct timeval timeout;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fd1, &readfds);
        FD_SET(fd2, &readfds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret;
        ret = select(nfds, &readfds, NULL, NULL, &timeout);
        if (ret == 0) continue;
        if (ret == -1) {
            perror("select greška");
            return;
        }

        bzero(buf, BUF_SIZE);
        if (FD_ISSET(fd1, &readfds)) {
            ret = recv(fd1, buf, BUF_SIZE, 0);
            if (ret == 0) {
                printf("Veza prekinuta od strane udaljenog hosta fd1\n");
                return;
            }
            if (ret == -1) {
                perror("recv greška");
                return;
            }
            send(fd2, buf, ret, 0);
        } else {
            ret = recv(fd2, buf, BUF_SIZE, 0);
            if (ret == 0) {
                printf("Veza prekinuta od strane udaljenog hosta fd2\n");
                return;
            }
            if (ret == -1) {
                perror("recv greška");
                return;
            }
            send(fd1, buf, ret, 0);
        }
    }
}

/*
 * Funkcija koja proverava da li postoji traženi metod
 * 
 * *verif - objekat u kom se traži metod
 * metod - traženi metod
 * 
 * Vraća 1 ako je metod pronađen, 0 ako nije
 *
*/
int metodPostoji(cl_metod_verifikacije *verif, uint8_t metod){
	int i;
	for(i=0; i < verif->nmethods; i++){
		if(verif->methods[i] == metod)
			return 1;
	}
	return 0;
}

 
 int main(int argc,char *argv[])  
 {  
	 //Uticnica proksija
	 int proxySock;
	 proxySock = socket(AF_INET,SOCK_STREAM, 0);
	 if(proxySock == -1){
        printf("Greška pri kreiranju utičnice!\n");
        return 1;
    }
    else printf("Utičnica kreirana\n");

	struct sockaddr_in proxy;
	proxy.sin_family = AF_INET;
	proxy.sin_addr.s_addr = INADDR_ANY;
	if(argc > 1)
		proxy.sin_port = htons(atoi(argv[1]));
	else
		proxy.sin_port = htons(DEFAULT_PORT);

	if(bind(proxySock, (struct sockaddr *)&proxy, sizeof(proxy)) < 0){
		perror("Greška pri dodavanju adrese utičnici!");
		return 1;
	}

	//Podaci klijenta
	int clientSock, cSize;
	struct sockaddr_in client;

	//Ocekivanje nadolazecih konekcija
	listen(proxySock, 1);
	puts("Čekanje na nadolazeće konekcije...");

	clientSock = accept(proxySock, (struct sockaddr *)&client, (socklen_t *)&cSize);
	if(clientSock < 0){
		perror("Greška pri prihvatanju konekcije!");
		return 1;
	}
	puts("Konekcija uspostavljena");

	char bafer1[DEFAULT_BUFLEN], bafer2[DEFAULT_BUFLEN];
	if(recv(clientSock, bafer1, DEFAULT_BUFLEN, 0) < 0){
		perror("Greška pri prijemu podataka od klijenta!");
		return 1;
	}

	cl_metod_verifikacije *provera;
	provera = (cl_metod_verifikacije *)bafer1;
	if(provera->ver == 0x05){
		sr_metod_verifikacije *potvrda;
		potvrda = (sr_metod_verifikacije *)bafer2;
		potvrda->ver = 0x05;

		if(metodPostoji(provera, AUTH))
			potvrda->method = AUTH;
		else
			potvrda->method = 0xff;

		if(send(clientSock, potvrda, sizeof(potvrda), 0) < 0){
			perror("Greška pri slanju potvrde metoda verifikacije!");
			return 1;
		}
	}
	else{
		puts("Pogrešna verzija protokola!");
		return 1;
	}

	//Prijem korisničkog imena i lozinke
	char baferIme[DEFAULT_BUFLEN], baferLozinka[DEFAULT_BUFLEN];
	if(recv(clientSock, baferIme, DEFAULT_BUFLEN, 0) < 0){
		perror("Greška pri prijemu podataka od klijenta!");
		return 1;
	}
	if(recv(clientSock, baferLozinka, DEFAULT_BUFLEN, 0) < 0){
		perror("Greška pri prijemu podataka od klijenta!");
		return 1;
	}
	for(int i = 0; i<strlen(baferIme); i++){	
		if(baferIme[i] == '\n'){
			baferIme[i] = 0;
			break;
		}
	}
	for(int i = 0; i<strlen(baferLozinka); i++){	
		if(baferLozinka[i] == '\n'){
			baferLozinka[i] = 0;
			break;
		}
	}

	//Status autorizacije
	auth_status *authStatus;
	uint8_t authBaf[2];
	authStatus = (auth_status *)authBaf;
	authStatus->ver = 0x01;
	if(strcmp(baferIme, "admin") == 0 && strcmp(baferLozinka, "bafer") == 0){
		puts("Prijava uspešna");
		authStatus->status = 0x00;
	}
	else{
		puts("Prijava neuspešna");
		authStatus->status = 0x01;
	}
	if(send(clientSock, authStatus, sizeof(authStatus), 0) < 0){
		perror("Greška pri slanju potvrde metoda verifikacije!");
		return 1;
	}
	if(authStatus->status != 0x00) return 0;

	//Zahtev i usluge
	char bafZahtev[DEFAULT_BUFLEN], serverIP[DEFAULT_BUFLEN];
	uint16_t serverPort;
    if(recv(clientSock, bafZahtev, DEFAULT_BUFLEN, 0) < 0){
        perror("Greška pri prijemu podataka od klijenta!");
        return 1;
    }
    client_request *zahtev;
    zahtev = (client_request *)bafZahtev;
	if(zahtev->ver == 0x05){
		switch(zahtev->cmd){
			case 0x01:	//TCP
				//Prijem adrese servera
				if(recv(clientSock, serverIP, DEFAULT_BUFLEN, 0) < 0){
					perror("Greška pri prijemu adrese od klijenta!");
					return 1;
				}
				for(int i = 0; i<strlen(serverIP); i++){	
					if(serverIP[i] == '\n'){
						serverIP[i] = 0;
						break;
					}
				}
				//Prijem porta servera
				if(recv(clientSock, &serverPort, sizeof(uint16_t), 0) < 0){
					perror("Greška pri prijemu podataka od klijenta!");
					return 1;
				}
				break;

			case 0x02:	//Bind (za FTP) - ne radimo
				close(proxySock);
				close(clientSock);
				return 0;
			case 0x03:	//UDP - ne radimo
				close(proxySock);
				close(clientSock);
				return 0;
			default:
				close(proxySock);
				close(clientSock);
				return 0;
		}
	}
	else{
		puts("Pogrešna verzija protokola!");
		return 1;
	}

	char baferOdgovor[DEFAULT_BUFLEN];
	proxy_response *prOdg;
	prOdg = (proxy_response *)baferOdgovor;
	prOdg->ver = 0x05;   //verzija protokola
	prOdg->rep = 0x00;   //uspešno
	prOdg->rsv = 0x00;   //beskorisno
	prOdg->atyp = 0x01;  //tip adrese
	//Slanje odgovora za traženu uslugu
	if(send(clientSock, prOdg, sizeof(prOdg), 0) < 0){
		perror("Greška pri slanju odgovora klijentu!");
		return 1;
	}

	//Utičnica za server
	int serverSock;
	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSock < 0)  
	{  
		printf("Greška pri kreiranju utičnice za server\n");  
	}  
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(serverPort);
	server.sin_addr.s_addr = inet_addr(serverIP);

	//Povezivanje na glavni server 
	if(connect(serverSock, (struct sockaddr *)&server, sizeof(server))<0)  
	{  
		perror("Greška pri uspostavljanju konekcije sa serverom");
		return 1;
	}  
	puts("Uspostavljena konekcija sa serverom");

	tcp_forward(clientSock, serverSock);

	close(proxySock);
	close(clientSock);

	return 0;  
 }  
 
