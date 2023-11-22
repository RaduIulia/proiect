#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

/* portul folosit */
#define PORT 2024

int numar_utilizatori_activi;

/* codul de eroare returnat de anumite apeluri */
extern int errno;

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    char msg[1024];		//mesajul primit de la client
    char msgrasp[1024]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket
    int num,logat;

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }

    /* servim in mod concurent clientii... */
    while (1)
    {
    	int client;
    	int length = sizeof (from);

    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);

    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);

    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}

    	int pid;
    	if ((pid = fork()) == -1) {
    		close(client);
    		continue;
    	} else if (pid > 0) {
    		// parinte
    		close(client);
    		while(waitpid(-1,NULL,WNOHANG));
    		continue;
    	} else if (pid == 0) {
    		// copil
    		close(sd);
    		int activ=1;
    		int stadiu=0;
    		char anterior[2048];

    		/* s-a realizat conexiunea, se astepta mesajul */
    		bzero (msg, 1024);
    		printf ("[server]Asteptam mesajul...\n");
    		fflush (stdout);
    		numar_utilizatori_activi++;
    		logat=0;
    		
    		while(activ)
    		{

    		/* citirea mesajului */
    		if ((num=read (client, msg, 1024)) <= 0)
    		{
    			perror ("[server]Eroare la read() de la client.\n");
    			close (client);	/* inchidem conexiunea cu clientul */
    			continue;		/* continuam sa ascultam */
    		}
    		else{
                 while(num==0)
                 {sleep(1);}
                 msg[num]='\0';
                 }
                printf("stadiu : %d",stadiu);

    		printf ("[server]Mesajul a fost receptionat...%s\n", msg);
    		if (strcmp(msg,"exit")==0 || strcmp(msg,"iesire")==0)
    		   {
    		   activ=0;
    		   break;
    		   }
    		switch (logat)
    		{
    		case 0:
    		if (strcmp(msg,"autentificare")==0 || strcmp(msg,"Autentificare")==0 || strcmp(msg,"login")==0  || strcmp(msg,"Login")==0)
    		{
    		stadiu=1;
    		bzero(msgrasp,1024);
    		strcat(msgrasp,"Nume de utilizator: ");
    		} else
    		if (strcmp(msg,"inregistrare")==0 || strcmp(msg,"Inregistrare")==0 || strcmp(msg,"signin")==0  || strcmp(msg,"Signin")==0)
    		{
    		stadiu=2;
    		bzero(msgrasp,1024);
    		strcat(msgrasp,"Alege un nume de utilizator: ");
    		}
    		else
    		if(stadiu==1)
    		{
    		bzero(msgrasp,1024);
    		//strcat(msgrasp,"Cautare nume utilizator ");
    		const char *filename="utilizatori.txt";
		FILE *file=fopen(filename, "r");
		char buf[2048];
		int gasit=0;
		if (file==NULL)
		{
		file=fopen(filename,"w");
		if (file==NULL)
		{
		perror("Eroare creare utilizatori.txt");
		}
		}
		else
		{
		while(fgets(buf,sizeof(buf),file))
		{
    		if(strstr(buf,msg)!=NULL)
    		{
    		gasit=1;
    		break;
    		}
		}
		}
		if(gasit==1)
		{
		strcpy(anterior,msg);
		strcat(msgrasp,"Introduce parola: ");
		stadiu=3;
		file=fopen(filename,"a");
		fprintf(file,"%s\n",msg);
		}
		else
		{
		strcat(msgrasp,"Nume de utilizator inexistent! Reincearca: ");
		}

		fclose(file);
    		}
    		else
    		if(stadiu==2)
    		{
    		bzero(msgrasp,1024);
    		//strcat(msgrasp,"Verificare nume utilizator ");
    		const char *filename="utilizatori.txt";
		FILE *file=fopen(filename, "r");
		char buf[2048];
		int gasit=0;
		if (file==NULL)
		{
		file=fopen(filename,"w");
		if (file==NULL)
		{
		perror("Eroare creare utilizatori.txt");
		}
		}
		else
		{
		while(fgets(buf,sizeof(buf),file))
		{
    		if(strstr(buf,msg)!=NULL)
    		{
    		gasit=1;
    		break;
    		}
		}
		}
		if(gasit==0)
		{
		strcpy(anterior,msg);
		strcat(msgrasp,"Nume de utilizator ales!\nAlege o parola: ");
		stadiu=4;
		file=fopen(filename,"a");
		fprintf(file,"%s\n",msg);
		}
		else
		{
		strcat(msgrasp,"Nume de utilizator folosit! Incearca altul: ");
		}

		fclose(file);
    		}
    		else
    		if(stadiu==3)
    		{
    		int ok=0;
		const char *filename="utilizatori.txt";
		FILE *file = fopen(filename, "r");
 	        if (file == NULL) {
        	perror("Error opening file");
    		}

    		char line[1000];

    		while (fgets(line, sizeof(line), file)) {
    		printf("%s %s %s* \n",line,anterior,msg);
        	char *found_position = strstr(line, anterior);

        	if (found_position != NULL) {
        	printf("gasit : %s*\n",found_position);
            	char *next_word = found_position + strlen(anterior);
            	while (*next_word == ' ' || *next_word == '\t') {
                next_word++; 
            	}
            	printf("gasit 2 : %s*\n",next_word);
            	next_word[strcspn(next_word, "\n")] = '\0';

            	//trim_whitespace(next_word);

            	if (strcmp(next_word, msg)==0) {
            	logat=1;
                printf("Found '%s' after '%s' in the file.\n", msg, anterior);
                strcat(msgrasp,"\nAi intrat in cont! \n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-citeste istoric conversatie/read conversations \n-iesire/quit \nAi primit ");
                break;
            	}
            	else
            	{
            	strcat(msgrasp,"Parola incorecta! Reincearca: ");
            	}
        	}
    		}
    		fclose(file);
    		}
    		else
    		if(stadiu==4)
    		{
    		const char *filename="utilizatori.txt";
		FILE *file = fopen(filename, "r");
    		if (file == NULL) {
       		perror("Error opening file");
    		}

    		FILE *temp_file = fopen("temp_file.txt", "w");
    		if (temp_file == NULL) {
        	perror("Error creating temporary file");
        	fclose(file);
    		}

    		char line[1000];

    		while (fgets(line, sizeof(line), file)) {
        	char *found_position = strstr(line, anterior);

        	if (found_position != NULL) {
            	size_t position = found_position - line;
            	fwrite(line, 1, position, temp_file);
            	fprintf(temp_file, "%s %s\n", anterior, msg);
            	found_position += strlen(anterior);
        	} else 
        	{
            	fputs(line, temp_file);
        	}
    		}
    		fclose(file);
    		fclose(temp_file);
    		if (rename("temp_file.txt", filename) == 0) 
    		{
        	printf("Word '%s' appended with '%s' in the file.\n", anterior, msg);
        	logat=1;
                strcat(msgrasp,"Cont creat! \n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-citeste istoric conversatie/read conversations \n-iesire/quit \nAi primit ");
    		} 
    		else 
    		{
        	perror("Error replacing file");
    		}
    		}
    		else
    		{
    		bzero(msgrasp,1024);
    		strcat(msgrasp,"Inregistrare / Autentificare: ");
    		}    		
    		break;
    		default:
    		bzero(msgrasp,1024);
    		if (strcmp(msg,"help")==0 || strcmp(msg,"ajutor")==0)
    		{  
    		strcat(msgrasp,"\n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-citeste mesaje primte/read received messages \n-iesire/quit ");
    		}
    		else
    		strcat(msgrasp,"Comanda inexistenta! Reincercati: ");
    		break;
    		}
    		
    		printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
    		/* returnam mesajul clientului */
    		if (write (client, msgrasp, 1024) <= 0)
    		{
    			perror ("[server]Eroare la write() catre client.\n");
    			continue;		/* continuam sa ascultam */
    		}
    		else
    			printf ("[server]Mesajul a fost trasmis cu succes.\n");
    		}
    		/* am terminat cu acest client, inchidem conexiunea */
    		close (client);
    		numar_utilizatori_activi--;
    		exit(0);
    	}

    }				/* while */
}				/* main */
