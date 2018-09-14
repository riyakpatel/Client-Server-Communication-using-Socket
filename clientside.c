#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

void errorMsg(const char*msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd,portno,n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char message[255];
	if(argc<3)
	{
		fprintf(stderr,"Ip address and port number are required\n");
		fprintf(stderr,"Usage: client <IP Address> <Port Number>\n\n");
		exit(0);
	}

	portno = atoi(argv[2]);
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(sockfd<0)
	{
		errorMsg("Error in socket opening");
	}


	server = gethostbyname(argv[1]);
	
	if(server==NULL)
	{
		fprintf(stderr,"Error no such host\n");
	}

	bzero((char*) &serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
	
	serv_addr.sin_port=htons(portno);

	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
	{
		errorMsg("Connection Failed");
	}

	while(1)
	{
		bzero(message,sizeof(message));	
		fprintf(stderr, "Enter Command: ");
		fgets(message,sizeof(message),stdin);
		fprintf(stderr, "\n");
		message[strcspn(message, "\n")] = 0;
		
		if(strncmp(message,"get",3)== 0 || strncmp(message,"put",3) == 0){
		
			send(sockfd, message, strlen(message), 0);
			
			char serverMsg[5000];
			
			if(recv(sockfd , serverMsg , 5000 , 0) > 0 )
			{			
				char *cmd_argv[2];		
				int j=0;		
				char *cmd = strtok(message," "); 
				while(cmd!= NULL)
				{
					cmd_argv[j++] = cmd;		
					cmd = strtok(NULL," ");
				}
				
				if(strcmp("PUT",serverMsg) == 0)
				{	
					FILE * file;
					size_t read=0;
					file = fopen(cmd_argv[1], "rb"); //open the file for binary input
					
					if(file){
						fseek(file, 0, SEEK_END);
						long fsize = ftell(file);
						fseek(file, 0, SEEK_SET);  //same as rewind(f);
						
						char *string = malloc(fsize + 1);
						
						read = fread(string,fsize,1, file); //issue the read call
						if (read > 0) //if return value is > 0
						{
							string[fsize]='\0';
							//fprintf(stderr, "%s\n",string);
							if(send(sockfd, string, strlen(string), 0) < 0){ //write to client
								fprintf(stderr,"Error in sending file to server..\n");
							}
						}
						fclose(file);
						fprintf(stderr,"File uploaded successfully by client..\n");
						fprintf(stderr,".....................................................................\n\n");
					}
					else{
						send(sockfd, "NOT", strlen("NOT"), 0);
						fprintf(stderr,"File does not exist..\n");
						fprintf(stderr,".....................................................................\n\n");
					}
					bzero(serverMsg,sizeof(serverMsg));	
				}
				else if(strcmp("NOT",serverMsg) == 0)
				{
					fprintf(stderr,"File does not exist in server machine..\n");
					fprintf(stderr,".....................................................................\n\n");
					bzero(serverMsg,sizeof(serverMsg));	
				}
				else{
					fprintf(stderr,"Server file: %s\n\n", serverMsg);
					FILE *file = fopen(cmd_argv[1], "wb+");
					if(file){
						fwrite(serverMsg,strlen(serverMsg),1,file);	
						fclose(file);
						fprintf(stderr,"File downloaded successfully by client..\n");
						fprintf(stderr,".....................................................................\n\n");
					}
					bzero(serverMsg,sizeof(serverMsg));	
				}
			}
		}
		else{
			if(strcmp(message,"quit")== 0){
				send(sockfd, message, strlen(message), 0);
				fprintf(stderr,"connection terminated from client side..\n\n");
				close(sockfd);
				return 0;	
			}
			else{
				fprintf(stderr,"Use get FileName or put FileName commands..\n");
				fprintf(stderr,".....................................................................\n\n");
			}
		}
	}
	close(sockfd);
	return 0;	
}
