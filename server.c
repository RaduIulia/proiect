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
#include <sys/mman.h>
/* portul folosit */
#define PORT 2024
#define MAX_LENGTH 1024
/* codul de eroare returnat de anumite apeluri */
extern int errno;
//strstr strcmp iuli-iulia
//sa nu iti poti trimite mesaj singur/singura?
//conversatie cu reply la mesaj si istoric
int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    char msg[1024];		//mesajul primit de la client
    char msgrasp[1024]=" ";        //mesaj de raspuns pentru client
    int sd;			//descriptorul de socket
    int num,logat;
    char eu[1024];
    char de_trimis_catre[1024];
    int ok_catre=0;
    int de_deschis=0;
    char istoric[1024];
    char de_eliminat[1024];
    bzero(de_eliminat,1024);
    char de_eliminat2[1024];
    bzero(de_eliminat2,1024);
    bzero(de_trimis_catre,1024);
    char *clienti_online = mmap(NULL, MAX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (clienti_online == MAP_FAILED) {
        perror("Eroare la alocarea memoriei partajate");
        exit(EXIT_FAILURE);
    }
    FILE *fisier;
    char nume_fisier[] = "mail.txt"; 
    int numar_linii = 0;
    char caracter;
    // Deschide fișierul în modul de citire
    fisier = fopen(nume_fisier, "r");

    // Verifică dacă fișierul s-a deschis cu succes
    if (fisier == NULL) {
        printf("Eroare la deschiderea fisierului.\n");
        return 1;
    }
    else
    while ((caracter = fgetc(fisier)) != EOF) {
        if (caracter == '\n') {
            numar_linii++;
        }
    }
    fclose(fisier);
    int *numar_mesaje = mmap(NULL, MAX_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (numar_mesaje == MAP_FAILED) {
        perror("Eroare la alocarea memoriei partajate");
        exit(EXIT_FAILURE);
    }
    else
    *numar_mesaje=numar_linii;
    printf("numar mesaje=%d\n",*numar_mesaje);
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

            	if (strcmp(next_word, msg)==0) {
            	logat=1;
                break;
            	}
        	}
    		}
    		if(logat==1)
    		{
    		strcat(eu,anterior);
            	strcat(clienti_online, anterior);
            	strcat(clienti_online,"\n");
                printf("Found '%s' after '%s' in the file.\n", msg, anterior);
                strcat(msgrasp,"\nAi intrat in cont! \n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-intra in conversatie/open conversation \n-iesire/exit");
                strcat(de_eliminat," -> ");
            	strcat(de_eliminat,eu);       	
		int total;
		char mesaje[2048];
		FILE *tempFile;
		total=0;
		bzero(mesaje,2048);
		char tempFileName[] = "temp_mail.txt";
		tempFile = fopen(tempFileName, "a");
		if (tempFile == NULL) {
      		  printf("Nu am putut crea fisierul temporar.\n");
       		 fclose(tempFile);
       		 }
		const char *filename="mail.txt";
		FILE *file=fopen(filename, "r");
		char buf[2048];
		if (file==NULL)
		{
		file=fopen(filename,"a");
		if (file==NULL)
			{
		perror("Eroare creare mail.txt");
		}
		}
		else
		{
		char line[1000];
		char start='0';
    		while (fgets(line, sizeof(line), file)) {
   		 if(line[0]==start)
  		  {
    			char *inc=" -> ";
    			char *gasit=strstr(line,inc);
    			if (gasit != NULL) {
        		gasit += strlen(inc);
        		char urmcuv[100];
        		int i = 0;

        		while (gasit[i] != ' ' && gasit[i] != '\0') {
            		urmcuv[i] = gasit[i];
            		i++;
        		}
        	urmcuv[i] = '\0';
        	printf("Cuvantul dupa '->' este: %s\n", urmcuv);
        	if(strcmp(urmcuv,eu)==0)
          	{
          	printf(" %s ",line);
          	total = total + 1;
          	char *linie=line+4;
          	strcat(mesaje,linie);
          	char *pos = strchr(line, '0');
          	if (pos != NULL) {
             	*pos = '2';
            	}
            	char *pozitie = strstr(mesaje, de_eliminat);
            	if (pozitie != NULL) {
        // Calculează offset-ul (poziția) în șir
        	size_t offset = pozitie - mesaje;

        // Calculează lungimea substring-ului
        	size_t lungime_substring = strlen(de_eliminat);

        // Utilizează memmove pentru a șterge substring-ul
        	memmove(pozitie, pozitie + lungime_substring, strlen(pozitie + lungime_substring) + 1);
    		}
          	}
          	}
    		}
    		fprintf(tempFile, "%s", line);  
    	}
	}
		fclose(file);
		fclose(tempFile);
		if (rename(tempFileName, filename) != 0) {
        	printf("Nu am putut înlocui fișierul original.\n");
    		}
    		char text[128];
		printf("%s",mesaje);
		sprintf(text, "\nAi primit %d mesaje cat ai fost offline.\n", total);
		strcat(msgrasp,text);
		if (total>0)
		   strcat(msgrasp,mesaje);
		strcat(msgrasp,"\nComanda ta: ");
    		}
    		else
            	{
            	strcat(msgrasp,"Parola incorecta! Reincearca: ");
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
    		strcat(eu,anterior);
    		strcat(clienti_online, anterior);
    		strcat(clienti_online,"\n");
        	printf("Word '%s' appended with '%s' in the file.\n", anterior, msg);
        	logat=1;
                strcat(msgrasp,"Cont creat! \n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-intra in conversatie/open conversation \n-iesire/exit \nAi primit 0 mesaje cat ai fost offline!\nComanda ta: ");
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
    		if (strcmp(msg,"help")==0 || strcmp(msg,"ajutor")==0||strcmp(msg,"x")==0)
    		{
    		de_deschis=0;  
    		strcat(msgrasp,"\nMeniu comenzi:\n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-intra in conversatie/open conversation \n-iesire/exit");
    		strcat(msgrasp, "\nComanda ta: ");
    		}
    		else
    		if (strstr(msg,"utilizatori activi")!=NULL || strstr(msg,"online users")!=NULL)
    		{
    		strcat(msgrasp, "Lista utilizatorilor activi:\n");
       		strcat(msgrasp,clienti_online);
       		strcat(msgrasp, "\nComanda ta: ");
    		}
    		else
 		if (strstr(msg,"toti utilizatorii")!=NULL || strstr(msg,"all users")!=NULL)
   		{
   		const char *filename="utilizatori.txt";
		FILE *file=fopen(filename, "r");
		char buf[2048];
		char linie[128];
		char list[2048];
		bzero(list,2048);
		strcat(list,"Lista tuturor utilizatorilor este:\n");
		while (fgets(linie,sizeof(linie),file))
		{
		char *primul=strtok(linie," ");
		strcat(list,primul);
		strcat(list,"\n");
		}  
       		strcat(msgrasp,list);
       		strcat(msgrasp, "\nComanda ta: ");
    		}
    		else
    		if (strstr(msg,"intra in conversatie")!=NULL || strstr(msg,"open conversation")!=NULL)
    		{
    		strcat(msgrasp,"Cu cine: ");
    		de_deschis=1;
    		}
    		else
    		if (strstr(msg,"trimite mesaj")!=NULL || strstr(msg,"send message")!=NULL)
    		{
    		ok_catre=1;
    		strcat(msgrasp,"Destinatar mesaj: ");
    		}
    		else
    		if(ok_catre==1)
    		{
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
    		printf("%s",buf);
    		gasit=1;
    		break;
    		}
		}
		}
		if(gasit==1)
		{
		ok_catre=2;
		strcpy(de_trimis_catre,msg);
		strcat(msgrasp,"Mesajul de trimis: ");
		}
		else
		 strcat(msgrasp,"Utilizator inexistent. Reintrodu: ");
    		}
    		else
    		if(ok_catre==2)
    		{
    		ok_catre=0;
    		const char *filename="mail.txt";
		FILE *file=fopen(filename, "r");
		char buf[2048];
		if (file==NULL)
		{
		file=fopen(filename,"w");
		if (file==NULL)
		{
		perror("Eroare creare mail.txt");
		}
		}
		int reply=0;
		int lungime_numar=0;
		if (strstr(msg,"<")!=NULL && strstr(msg,">")!=NULL)
		{
		int numar = 0;
               if (msg[0] == '<')
               {
               char *p = &msg[1];  
               char *sfarsit;
               numar = strtol(p, &sfarsit, 10);
               printf("%d",numar);

       
              if (p != sfarsit && *sfarsit == '>') {
            
              printf("Număr găsit: %d\n", numar);
              reply=numar;
               }
               }
                 }
		else
		reply=0;
		printf("\nreply are valoarea %d \n",reply);
		(*numar_mesaje)++;
		if (strstr(clienti_online,de_trimis_catre)!=NULL)
		{
		file=fopen(filename,"a");
		fprintf(file,"1 %d %d %s -> %s : %s \n",reply,*numar_mesaje,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		else
		{
		file=fopen(filename,"a");
		fprintf(file,"0 %d %d %s -> %s : %s \n",reply,*numar_mesaje,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		fclose(file);
		strcat(msgrasp,"Mesaj trimis.");
		strcat(msgrasp, "\nComanda ta: ");
    		}
    		else
    		if(de_deschis==1)
    		{
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
		strcpy(de_trimis_catre,msg);
		strcat(de_eliminat2," -> "); 
            	strcat(de_eliminat2,de_trimis_catre); 
		bzero(istoric,1024);
		strcat(msgrasp,"Conversatia cu ");
		strcat(msgrasp,msg);
		strcat(msgrasp," :\n\n");
		const char *nume_fisier="mail.txt";
		FILE *fisier = fopen(nume_fisier, "r");
                if (fisier == NULL) {
        	perror("Eroare la deschiderea fisierului");
    		}
    		char buf[2048];
    		int numar_linie = 1;
    		while(fgets(buf,sizeof(buf),fisier)) {
        	if (strstr(buf, msg) && strstr(buf, eu) ) {
        	buf[0]='2';
            	char *pozitie = strstr(buf, de_eliminat);
            	printf("\n%s %s",buf,pozitie);
            	if (pozitie != NULL) {
        // Calculează offset-ul (poziția) în șir
        	size_t offset = pozitie - buf;

        // Calculează lungimea substring-ului
        	size_t lungime_substring = strlen(de_eliminat);

        // Utilizează memmove pentru a șterge substring-ul
        	memmove(pozitie, pozitie + lungime_substring, strlen(pozitie + lungime_substring) + 1);
    		}
    		
    		char *pozitie2 = strstr(buf, de_eliminat2);
    		printf("\n%s %s",buf,pozitie2);
            	if (pozitie2 != NULL) {
        // Calculează offset-ul (poziția) în șir
        	size_t offset = pozitie2 - buf;

        // Calculează lungimea substring-ului
        	size_t lungime_substring = strlen(de_eliminat2);

        // Utilizează memmove pentru a șterge substring-ul
        	memmove(pozitie2, pozitie2 + lungime_substring, strlen(pozitie2 + lungime_substring) + 1);
    		}
    		strcat(msgrasp,buf+4);
        	}
    		}	
		de_deschis=2;
    		fclose(fisier);
    		strcat(istoric,msgrasp);
    		strcat(msgrasp,"\n-trimite x sa revii la meniu");
    		strcat(msgrasp,"\n-raspunde");
    		strcat(msgrasp,"\n\nComanda ta: ");	
		}
		else
		{
		strcat(msgrasp,"Nume de utilizator inexistent! Reincearca: ");
		}
		fclose(file);
    		}
    		else
    		if(de_deschis==2&&strcmp(msg,"raspunde")==0)
    		{ 		
    		de_deschis=3;
    		strcat(msgrasp,istoric);
    		strcat(msgrasp,"\nMesajul tau: ");
    		}
    		else
    		if(de_deschis==3)
    		{
    		const char *filename="mail.txt";
		FILE *file=fopen(filename, "r");
		char buf[2048];
		if (file==NULL)
		{
		file=fopen(filename,"w");
		if (file==NULL)
		{
		perror("Eroare creare mail.txt");
		}
		}
		int reply=0;
		int lungime_numar=0;
		if (strstr(msg,"<")!=NULL && strstr(msg,">")!=NULL)
		{
		int numar = 0;
               if (msg[0] == '<')
               {
               char *p = &msg[1];  
               char *sfarsit;
               numar = strtol(p, &sfarsit, 10);
               printf("%d",numar);

       
              if (p != sfarsit && *sfarsit == '>') {
            
              printf("Număr găsit: %d\n", numar);
              reply=numar;
               }
               }
                 }
		else
		reply=0;
		printf("\nreply are valoarea %d \n",reply);
		(*numar_mesaje)++;
		if (strstr(clienti_online,de_trimis_catre)!=NULL)
		{
		file=fopen(filename,"a");
		fprintf(file,"1 %d %d %s -> %s : %s \n",reply,*numar_mesaje,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		else
		{
		file=fopen(filename,"a");
		fprintf(file,"0 %d %d %s -> %s : %s \n",reply,*numar_mesaje,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		fclose(file);
		char numar_str[20];
    		sprintf(numar_str, "%d", *numar_mesaje);
    		strcat(istoric,numar_str);
    		strcat(istoric," ");
		strcat(istoric,eu);
		strcat(istoric," -> ");
		strcat(istoric,de_trimis_catre);
		strcat(istoric," : ");
		strcat(istoric,msg);
		strcat(istoric,"\n");
		char *pozitie = strstr(istoric, de_eliminat);
            	if (pozitie != NULL) {
        // Calculează offset-ul (poziția) în șir
        	size_t offset = pozitie - istoric;

        // Calculează lungimea substring-ului
        	size_t lungime_substring = strlen(de_eliminat);

        // Utilizează memmove pentru a șterge substring-ul
        	memmove(pozitie, pozitie + lungime_substring, strlen(pozitie + lungime_substring) + 1);
    		}
    		strcat(de_eliminat2," -> "); 
            	strcat(de_eliminat2,de_trimis_catre); 
    		char *pozitie2 = strstr(msgrasp, de_eliminat2);
            	if (pozitie2 != NULL) {
        	size_t offset = pozitie2 - msgrasp;
        	size_t lungime_substring = strlen(de_eliminat2);
        	memmove(pozitie2, pozitie2 + lungime_substring, strlen(pozitie2 + lungime_substring) + 1);
    		}
		strcat(msgrasp,istoric);
		strcat(msgrasp,"\nMesaj trimis.");
		strcat(msgrasp,"\n-trimite x sa revii la meniu");
		strcat(msgrasp,"\nMesajul tau: ");		
    		}
    		else
    		 {
    		 strcat(msgrasp,"Comanda inexistenta!");
    		 strcat(msgrasp, "\nComanda ta: ");
    		 }
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
    		char *pozitie = strstr(clienti_online, eu);
    		if (pozitie != NULL) {
        	size_t lungime_sir_extras = strlen(eu);
        size_t offset = pozitie - clienti_online;
        memmove(pozitie, pozitie + lungime_sir_extras, strlen(pozitie + lungime_sir_extras) + 1);
    		exit(0);
    	}
    }				/* while */
    munmap(clienti_online, MAX_LENGTH);
    return 0;
}				/* main */
