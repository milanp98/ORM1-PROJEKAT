#include <sys/socket.h>  
 #include <sys/types.h>  
 #include <resolv.h>  
 #include <string.h>  
 #include <pthread.h>  
 #include<unistd.h>  
 
 // main entry point  
 int main()  
 {  
      int client_fd;   
      int fd = 0 ;  
      struct sockaddr_in server_sd;  
      // create a socket  
      fd = socket(AF_INET, SOCK_STREAM, 0);  
      printf("Server started\n");  
      memset(&server_sd, 0, sizeof(server_sd));  
      // set socket variables  
      server_sd.sin_family = AF_INET;  
      server_sd.sin_port = htons(5010);  
      server_sd.sin_addr.s_addr = INADDR_ANY;  
      // bind socket to the port  
      bind(fd, (struct sockaddr*)&server_sd,sizeof(server_sd));  
      // start listening at the given port for new connection requests  
      listen(fd, SOMAXCONN);  
      // continuously accept connections in while(1) loop  
      while(1)  
      {  
           // accept any incoming connection  
           client_fd = accept(fd, (struct sockaddr*)NULL ,NULL);  
           //printf("accepted client with id: %d",client_fd);  
           // if true then client request is accpted  
           if(client_fd > 0)  
           {  
                   
                printf("proxy connected\n");     
                
				char buffer[65535];  
				int bytes = 0;  
				while(1)  
				{  
					//receive data from client  
					memset(&buffer,'\0',sizeof(buffer));  
					bytes = read(client_fd, buffer, sizeof(buffer));  
					if(bytes <0)  
					{  
						//perror("read");  
					}  
					else if(bytes == 0)  
					{  
					}  
					else  
					{  
						//send the same data back to client  
						// similar to echo server  
						write(client_fd, buffer, sizeof(buffer));  
						//printf("client fd is : %d\n",c_fd);                    
						//printf("From client:\n");                    
						fputs(buffer,stdout);       
					}  
					fflush(stdout);  
				};        
                
           }  
      }  
      close(client_fd);   
      return 0;  
 }  
