#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

int serviceClient(int clientfd);

void errorMsg(const char*msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	if(argc<2)
	{
		fprintf(stderr,"Port number required\n");
		fprintf(stderr,"Usage: server <Port Number>\n\n");
		exit(1);
	}

	int sockfd, clientfd, portno, n;
	char buffer[255];

	struct sockaddr_in serv_addr, cli_addr;

	socklen_t clilen;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0)
	{
		errorMsg("Error in socket opening");
	}

	bzero((char *)&serv_addr,sizeof(serv_addr));

	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0)
	{
		errorMsg("Error in socket binding");
	}

	listen(sockfd,5);
	clilen=	sizeof(cli_addr);
	


	while(1){
		clientfd = accept(sockfd,(struct sockaddr*) &cli_addr, &clilen);
		if(clientfd<0)
		{
			errorMsg("Error in socket accepting");
		}
		printf("Connection Established with client..\n");
		if(!fork()){
			fprintf(stderr, "Now serviceClient method will handle the Client..\n");
			serviceClient(clientfd);
		}
	}
	close(sockfd);
	return 0;
	
}

int serviceClient(int clientfd){
	char clientMsg[255];
	while(1){
		fprintf(stderr,"Server waiting for client command..\n");
		
		bzero(clientMsg,sizeof(clientMsg));	
        recv(clientfd, clientMsg, sizeof(clientMsg),0);
		
		clientMsg[strcspn(clientMsg, "\n")] = 0;
		fprintf(stderr,"\nClient Command: %s\n\n",clientMsg);
				
		int i=strncmp("quit",clientMsg,3);
		if(i==0)
		{
			fprintf(stderr,"Client requested for closing connection\n");
			close(clientfd);
			fprintf(stderr,"Connection terminated by server\n");
			fprintf(stderr,".....................................................................\n\n");
			break;
		}
		
		char *cmd_argv[2];		
		int j=0;		
		char *cmd = strtok(clientMsg," "); //separate command arguments using space
		while(cmd!= NULL)
		{
			cmd_argv[j++] = cmd;	//Store command and its arguments in array			
			cmd = strtok(NULL," ");
		}
		
		
		if(strncmp(cmd_argv[0],"get",3)==0)
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
					if(send(clientfd, string, strlen(string), 0) < 0){ //write to client
						fprintf(stderr,"Error in sending file to client\n");
					}
				}
				fclose(file);
				fprintf(stderr,"File uploaded successfully by server..\n");
				fprintf(stderr,".....................................................................\n\n");
			}
			else{
				send(clientfd, "NOT", strlen("NOT"), 0);
				fprintf(stderr,"File does not exist..\n");
				fprintf(stderr,".....................................................................\n\n");
			}
			
		}
		else if(strncmp(cmd_argv[0],"put",3)==0)
		{			
			send(clientfd, "PUT", strlen("PUT"), 0);
			
			char clientFile[5000];
		
			if(recv(clientfd , clientFile , 5000 , 0) > 0 )
			{				
				if(strcmp("NOT",clientFile) == 0){
					fprintf(stderr,"File does not exist in client machine..\n");
					fprintf(stderr,".....................................................................\n\n");
				}
				else{
					fprintf(stderr,"Client file: %s\n\n", clientFile);
					FILE *file = fopen(cmd_argv[1], "wb+");
					if(file){
						fwrite(clientFile,strlen(clientFile),1,file);	
						fclose(file);
						fprintf(stderr,"File downloaded successfully by server..\n");
						fprintf(stderr,".....................................................................\n\n");
					}
				}
			}			
			bzero(clientFile,sizeof(clientFile));			
		}
		else
		{
			fprintf(stderr,"Use get FileName or put FileName commands..\n");
			fprintf(stderr,".....................................................................\n\n");
		}
	}
	return 0;
}
