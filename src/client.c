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
#define SIZE 1024
#define FILENAME "fajl.txt"
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

typedef struct {
    uint8_t ver;
    uint8_t rep;
    uint8_t rsv;
    uint8_t atyp;
} proxy_response;

/*
 * Funkcija za slanje fajla
 * 
 * *fp - datoteka koja se šalje
 * sockfd - utičnica na koju se šalje
*/
void send_file(FILE *fp, int sockfd){
  char data[SIZE] = {0};

  while(fgets(data, SIZE, fp) != NULL) {
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("Greška pri slanju fajla");
      exit(1);
    }
    bzero(data, SIZE);
  }
}
    
 int main(int argc, char* argv[])  
 {  
    //Uticnica klijenta
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        printf("Greška pri kreiranju utičnice!\n");
        return 1;
    }
    else printf("Utičnica kreirana\n");

    //Adresa proksija
    char proxyIP[17];
    printf("Adresa proksija: ");
    fgets(proxyIP,17,stdin);

    //Port proksija
    uint16_t proxyPort;
    printf("Port proksija: ");
    scanf("%hd", &proxyPort);

    //Uticnica proksija
    struct sockaddr_in proxy;
    proxy.sin_addr.s_addr = inet_addr(proxyIP);
    proxy.sin_family = AF_INET;
    proxy.sin_port = htons(proxyPort);

    //Uspostavljanje konekcije sa proksijem
    if(connect(sock, (struct sockaddr *)&proxy, sizeof(proxy)) < 0){
        perror("Greška pri uspostavljanju konekcije sa proksijem");
        return 1;
    }
    printf("Uspešna konekcija sa proksijem!\n");

    char bafer1[DEFAULT_BUFLEN];
    cl_metod_verifikacije *podrzane_metode;
    podrzane_metode = (cl_metod_verifikacije *)bafer1;
    podrzane_metode->ver = 0x05;
    podrzane_metode->nmethods = 0x02; 
    podrzane_metode->methods[0] = 0x00; 
    podrzane_metode->methods[1] = 0x02;

    //Slanje strukture za potvrdu metoda verifikacije
    if(send(sock, podrzane_metode, sizeof(podrzane_metode), 0) < 0){
        perror("Greška pri slanju metoda verifikacije proksiju!");
        return 1;
    }


    //Autentifikacija
    char bafer2[DEFAULT_BUFLEN];
    if(recv(sock, bafer2, DEFAULT_BUFLEN, 0) < 0){
        perror("Greška pri prijemu podataka od proksija!");
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
        case 0x02:  //Realizovano
            puts("Korisničko ime/lozinka autentifikacija");

            char korisnickoIme[DEFAULT_BUFLEN], lozinka[DEFAULT_BUFLEN];

            printf("Korisničko ime: ");
            getchar();  //praznjenje ulaznog toka
            fgets(korisnickoIme,DEFAULT_BUFLEN,stdin);
            if(send(sock, korisnickoIme, strlen(korisnickoIme), 0) < 0){
                perror("Greška pri slanju podataka proksiju!");
                close(sock);
                return 1;
            }

            printf("Lozinka: ");
            fgets(lozinka,DEFAULT_BUFLEN,stdin);
            if(send(sock, lozinka, strlen(lozinka), 0) < 0){
                perror("Greška pri slanju podataka proksiju!");
                close(sock);
                return 1;
            }
            break;
        case 0xff:
            puts("Nepodržan metod autentifikacije");
            close(sock);
            return 0;
        default:
            close(sock);
            return 0;
    }

    //Provera validnosti autentifikacije
    uint8_t baf[2];
    if(recv(sock, baf, sizeof(uint8_t)*2, 0) < 0){
        perror("Greška pri prijemu podataka od proksija!");
        return 1;
    }
    auth_status *autentifikacija;
    autentifikacija = (auth_status *)baf;
    if(autentifikacija->status == 0x00){
        char bafer4[DEFAULT_BUFLEN];
        client_request *clZahtev;
        clZahtev = (client_request *)bafer4;
        clZahtev->ver = 0x05;   //verzija protokola
        clZahtev->cmd = 0x01;   //komanda za TCP konekciju
        clZahtev->rsv = 0x00;   //beskorisno
        clZahtev->atyp = 0x01;  //tip adrese
        //Slanje strukture za potvrdu metoda verifikacije
        if(send(sock, clZahtev, sizeof(clZahtev), 0) < 0){
            perror("Greška pri slanju metoda verifikacije proksiju!");
            return 1;
        }

        //Adresa servera
        char serverIP[DEFAULT_BUFLEN];
        printf("Adresa servera: ");
        fgets(serverIP,DEFAULT_BUFLEN,stdin);

        //Slanje adrese servera proksiju
        if(send(sock, serverIP, DEFAULT_BUFLEN, 0) < 0){
            perror("Greška pri slanju metoda verifikacije proksiju!");
            return 1;
        }
        
        //Port servera
        uint16_t serverPort;
        printf("Port servera: ");
        scanf("%hd", &serverPort);

        //Slanje porta servera proksiju
        if(send(sock, &serverPort, sizeof(uint16_t), 0) < 0){
            perror("Greška pri slanju metoda verifikacije proksiju!");
            return 1;
        }
    }
    else{
        puts("Pogrešno korisničko ime/lozinka!");
        close(sock);
        return 0;
    }

    //Prijem odgovora - otprilike beskorisno
    char bafer5[DEFAULT_BUFLEN];
    if(recv(sock, bafer5, DEFAULT_BUFLEN, 0) < 0){
        perror("Greška pri prijemu odgovora od proksija!");
        return 1;
    }
    proxy_response *prOdg;
    prOdg = (proxy_response *)bafer5;
    if(prOdg->rep == 0x00) puts("Uspostava konekcije sa serverom");

    FILE *fp;

    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        perror("Greška pri čitanju fajla");
        close(sock);
        exit(1);
    }

    send_file(fp, sock);

    close(sock);

    return 0;  
 }
