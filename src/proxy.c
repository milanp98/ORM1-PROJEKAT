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
    uint8_t cmd;
    uint8_t rsv; 
    uint8_t atyp;
} client_request;

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
        printf("Greska pri kreiranju uticnice!\n");
        return 1;
    }
    else printf("Uticnica kreirana\n");

	struct sockaddr_in proxy;
	proxy.sin_family = AF_INET;
	proxy.sin_addr.s_addr = INADDR_ANY;
	if(argc > 1)
		proxy.sin_port = htons(atoi(argv[1]));
	else
		proxy.sin_port = htons(DEFAULT_PORT);

	if(bind(proxySock, (struct sockaddr *)&proxy, sizeof(proxy)) < 0){
		perror("Greska pri dodavanju adrese uticnici!");
		return 1;
	}

	//Podaci klijenta
	int clientSock, cSize;
	struct sockaddr_in client;

	//Ocekivanje nadolazecih konekcija
	listen(proxySock, 1);
	puts("Cekanje na nadolazece konekcije...");

	clientSock = accept(proxySock, (struct sockaddr *)&client, (socklen_t *)&cSize);
	if(clientSock < 0){
		perror("Greska pri prihvatanju konekcije!");
		return 1;
	}
	puts("Konekcija uspostavljena");

	char bafer1[DEFAULT_BUFLEN], bafer2[DEFAULT_BUFLEN];
	if(recv(clientSock, bafer1, DEFAULT_BUFLEN, 0) < 0){
		perror("Greska pri prijemu podataka od klijenta");
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
			perror("Greska pri slanju potvrde metoda verifikacije!");
			return 1;
		}
	}
	else{
		puts("Pogresna verzija protokola!");
		return 1;
	}

//***************************************************
#pragma region staro

     /*char ip[100];  
     char proxy_port[100];  
     // accept arguments from terminal  
     strcpy(ip,argv[1]); // server ip  
     strcpy(proxy_port,argv[2]); // proxy port    
     printf("proxy port is %s",proxy_port);        
     printf("\n");  
     //socket variables  
     int proxy_fd =0, client_fd=0;  
     struct sockaddr_in proxy_sd;  
     // add this line only if server exits when client exits  
     signal(SIGPIPE,SIG_IGN);  
     // create a socket  
     if((proxy_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
     {  
        printf("\nFailed to create socket");  
     }  
     printf("Proxy created\n");  
     memset(&proxy_sd, 0, sizeof(proxy_sd));  
     // set socket variables  
     proxy_sd.sin_family = AF_INET;  
     proxy_sd.sin_port = htons(atoi(proxy_port));  
     proxy_sd.sin_addr.s_addr = INADDR_ANY;  
     // bind the socket  
     if((bind(proxy_fd, (struct sockaddr*)&proxy_sd,sizeof(proxy_sd))) < 0)  
     {  
          printf("Failed to bind a socket");  
     }  
     // start listening to the port for new connections  
     if((listen(proxy_fd, SOMAXCONN)) < 0)  
     {  
          printf("Failed to listen");  
     }  
      printf("waiting for connection..\n");  
      //accept all client connections continuously  
*/
#pragma endregion

      /*while(1)  
      {  
           client_sock = accept(proxy_fd, (struct sockaddr*)NULL ,NULL);  
           printf("client no. %d connected\n",client_fd); 

          
		   if(client_sock > 0)  
           {            
		     	char buffer[65535];  
				int bytes =0;  
				
				//receive data from client  
		   
			    memset(&buffer, '\0', sizeof(buffer));  
				bytes = read(client_sock, buffer, sizeof(buffer));  
           
				client_hello_t *provera;
				provera = (client_hello_t *)buffer;
				if((provera->ver == 0x05) && (provera->methods == 0x02)) {
					puts("SOCKS5 protokol\n");
					puts("Uspesno izabran username/password metod\n");
				} else {
	  		    puts("Greska\n"); }
			   
 				char username[100];
				char pass[100];
				
				bytes = read(client_fd, username, sizeof(username));                        
				bytes = read(client_fd, pass, sizeof(pass));  
			
				if((strcmp(username, "marko") != 0) || (strcmp(pass, "orm1") != 0)) {					
					puts("Pogresna lozinka ili username\n");	
					return 1;
				} else {
					puts("Prijava uspesna\n");  
			                           
				printf("Client : ");                    
				fputs(username,stdout); 
				printf("\n"); 			
				
				
				bytes = read(client_fd, buffer, sizeof(buffer));
				
				client_request *prov;
				prov = (client_request *)buffer;
				if((prov->cmd == 0x01) && (prov->ver == 0x05) && (prov->rsv == 0x00)) {
					puts("Izabrana opcija TCP CONNECT na server\n");
				} else {
					puts("Greska \n");
					return 1;
	  		    } 
				
				
				char ip_server[100];
				char port_server[100];
      				
				bytes = read(client_fd, ip_server, sizeof(ip_server));    
				bytes = read(client_fd, port_server, sizeof(port_server));
				   
				int server_fd =0;  
				struct sockaddr_in server_sd;  
				// create a socket  
				server_fd = socket(AF_INET, SOCK_STREAM, 0);  
				if(server_fd < 0)  
				{  
					printf("server socket not created\n");  
				}  
				printf("server socket created\n");       
				memset(&server_sd, 0, sizeof(server_sd));  
				// set socket variables  
				server_sd.sin_family = AF_INET;  
				server_sd.sin_port = htons(atoi(port_server));  
				server_sd.sin_addr.s_addr = inet_addr(ip_server);  
				//connect to main server from this proxy server  
				if((connect(server_fd, (struct sockaddr *)&server_sd, sizeof(server_sd)))<0)  
				{  
					printf("server connection not established");  
				}  
				printf("server socket connected\n");  
				
				write(server_fd, username, sizeof(username));
                write(server_fd, pass, sizeof(pass));  
      
                sleep(1);
			}  
           }  
      }  */

	close(proxySock);
	close(clientSock);

	return 0;  
 }  
 
