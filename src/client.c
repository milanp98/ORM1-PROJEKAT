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

typedef struct {
    uint8_t ver;
    uint8_t nmethods;
    uint8_t methods;
} client_hello_t;

typedef struct {
    uint8_t ver;
    uint8_t cmd;
    uint8_t rsv; 
    uint8_t atyp;
} client_request;
	
    
 int main(int argc, char* argv[])  
 {  
      //socket variables  
      char IP[200];  
      char port[200];  
      //char buffer[65535];  
      int client_socket;  
      struct sockaddr_in client_sd;  
      printf("\nEnter proxy address:");  
      fgets(IP,sizeof("127.0.01\n")+1,stdin);  
      fputs(IP,stdout);  
      printf("\nEnter a port:");  
      fgets(port,sizeof("5000\n")+1,stdin);  
      fputs(port,stdout);  
  
      // create a socket  
      client_socket = socket(AF_INET, SOCK_STREAM, 0);
	  if (client_socket == -1)
      {
        fprintf(stderr, "Error creating socket %s\n", strerror(errno));

        exit(EXIT_FAILURE);
      }
           
      memset(&client_sd, 0, sizeof(client_sd));  
      // set socket variables  
      client_sd.sin_family = AF_INET;  
      client_sd.sin_port = htons(5000);  
      // assign any IP address to the client's socket  
      client_sd.sin_addr.s_addr = INADDR_ANY;   
      // connect to proxy server at mentioned port number  
      connect(client_socket, (struct sockaddr *)&client_sd, sizeof(client_sd));  
      //send and receive data contunuously  
       while(1)  
            {  		
            
		    char send_buf[1024];
			client_hello_t * client_hello;

			client_hello = (client_hello_t *)send_buf;
			client_hello->ver = 0x05;
			client_hello->nmethods = 0x01; 
			client_hello->methods = 0x02; 
            
            //saljemo strukturu za potvrdu protokola i verifikacije
            if (send(client_socket, client_hello, sizeof(client_hello), 0) < 0)
            {
                perror("send failed. Error");
                return 1;
            } 
            
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
            
           };  
           //close(sd);   
      return 0;  
 }
