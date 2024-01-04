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
#include <time.h>
/* portul folosit */
#define PORT 2024
#define MAX_LENGTH 1024
/* codul de eroare returnat de anumite apeluri */
extern int errno;

char* extrage_mesaj(char *searchID) {
    char nume_fisier[] = "mail.txt";
    FILE *file = fopen(nume_fisier, "r");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului");
        return NULL;    }
    char linie[256];  
    while (fgets(linie, sizeof(linie), file) != NULL) {
        if (strstr(linie, searchID) != NULL) {
            fclose(file);
            char *pozitie_doua_puncte = strstr(linie, ": ");
            if (pozitie_doua_puncte != NULL) {
                size_t lungime_linie = strlen(linie);
                if (lungime_linie > 0 && linie[lungime_linie - 1] == '\n') {
                    linie[lungime_linie - 2] = '\0';
                }
                char *text_dupa_doua_puncte = strdup(pozitie_doua_puncte + 2);
                return text_dupa_doua_puncte; }
    }
  }
}

void adauga_text(char *sir_initial, const char *text_adaugat) {
    char *inceput = strchr(sir_initial, '<');
    if (inceput != NULL) {
        char *sfarsit = strchr(inceput, '>');
        if (sfarsit != NULL) {
            sfarsit++;
            size_t dimensiune_sir_initial = inceput - sir_initial;
            size_t dimensiune_text_final = dimensiune_sir_initial + strlen(text_adaugat) + strlen(sfarsit) + 1;
            char *text_final = (char *)malloc(dimensiune_text_final);
            if (text_final == NULL) {
                perror("Eroare la alocarea de memorie");
                return;
            }
            strncpy(text_final, sir_initial, dimensiune_sir_initial);
            text_final[dimensiune_sir_initial] = '\0';
            strncat(text_final, "\"", dimensiune_text_final);
            strncat(text_final, text_adaugat, dimensiune_text_final);
            strncat(text_final, "\"-->", dimensiune_text_final);
            strncat(text_final, sfarsit, dimensiune_text_final);
            strcpy(sir_initial, text_final);
            free(text_final);}
    }
}

void generareRandomLitere(char *letters) {
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int index1, index2, index3;
    srand((unsigned int)time(NULL));
    index1 = rand() % 26;
    index2 = (index1 + rand() % 25 + 1) % 26;
    index3 = (index1 + rand() % 25 + 1) % 26;
    letters[0] = alphabet[index1];
    letters[1] = alphabet[index2];
    letters[2] = alphabet[index3];
    letters[3] = '\0'; 
}

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
    int comanda;
    int refresh=0;
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
    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {	perror ("[server]Eroare la socket().\n");
    	return errno;   }
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
    {	perror ("[server]Eroare la bind().\n");
    	return errno;   }
    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {  	perror ("[server]Eroare la listen().\n");
    	return errno;  }
    /* servim in mod concurent clientii... */
    while (1)
    {	int client;
    	int length = sizeof (from);
    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);
    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);
    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{	perror ("[server]Eroare la accept().\n");
    		continue; 	}
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
    		   {   activ=0;
    		   bzero(msgrasp,1024);
    		   strcat(msgrasp,"out");
    		   }   else
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
    		{	bzero(msgrasp,1024);
    		const char *filename="utilizatori.txt";
		FILE *file=fopen(filename, "r");
		char buf[2048];
		int gasit=0;
		if (file==NULL)
		{
		file=fopen(filename,"w");
		if (file==NULL)
		{ perror("Eroare creare utilizatori.txt"); }
		}	else
		{
		while(fgets(buf,sizeof(buf),file))
		{ char nume_utilizator[20];
		if (sscanf(buf, "%s", nume_utilizator) == 1) {
                 if (strcmp(nume_utilizator, msg) == 0) 
    		{ gasit=1; break; }
		}
		}
		if(gasit==1)
		{
		strcpy(anterior,msg);
		strcat(msgrasp,"Introduce parola: ");
		stadiu=3;
		}
		else
		{	strcat(msgrasp,"Nume de utilizator inexistent! Reincearca: ");}
		fclose(file);
    		}
    		}  else
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
		{ perror("Eroare creare utilizatori.txt");	}
		}
		else
		{
		while(fgets(buf,sizeof(buf),file))
		{	char nume_utilizator[20];
		if (sscanf(buf, "%s", nume_utilizator) == 1) {
                 if (strcmp(nume_utilizator, msg) == 0) 
    		{ gasit=1; break; }
		}
		}
		if(gasit==0)
		{
		strcpy(anterior,msg);
		strcat(msgrasp,"Nume de utilizator ales!\nAlege o parola: ");
		stadiu=4;
		file=fopen(filename,"a");
		fprintf(file,"%s\n",msg);
		}	else
		{
		strcat(msgrasp,"Nume de utilizator folosit! Incearca altul: ");
		}
		fclose(file);
    		}
    		}	else
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
                break; 	}
        	}
    		}
    		if(logat==1)
    		{
    		strcat(eu,anterior);
            	strcat(clienti_online, anterior);
            	strcat(clienti_online,"\n");
                printf("Found '%s' after '%s' in the file.\n", msg, anterior);
                strcat(msgrasp,"\nAi intrat in cont! \n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-intra in conversatie/open conversation \n-reimprospateaza/refresh \n-iesire/exit");
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
		{	perror("Eroare creare mail.txt");	}
		}	else
		{
		char line[1000];
		char start='0';
    		while (fgets(line, sizeof(line), file)) {
   		 if(line[0]==start)
  		  {	char *inc=" -> ";
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
          	char *pos = strchr(line, '0');
          	if (pos != NULL) {
             	*pos = '2';
            	}
            	
    		if (strstr(line,"<")!=NULL && strstr(line,">")!=NULL)
		{ const char *inceput = strchr(line, '<');  // Găsește prima apariție a caracterului '<'
    		const char *sfarsit = strchr(line, '>'); 
    		printf("inceput este %s\nsfrasit este%s",inceput,sfarsit);
		if (inceput != NULL && sfarsit != NULL) {
		char IDgasit[3];
		size_t length = 3; 
        strncpy(IDgasit, inceput + 1, length);
        IDgasit[length] = '\0'; // Adăugăm terminatorul de șir
        printf("\nIDgasit este %s\n",IDgasit);
        printf("\n %s \n",extrage_mesaj(IDgasit));
        //strcat(mesaje,extrage_mesaj(IDgasit));
        char *start1 = strstr(line, "<");
    if (start1 != NULL) {
        size_t startIndex = start1 - line;
        size_t endIndex = startIndex + 5;
        char *modifiedLine = malloc(strlen(line) + strlen(extrage_mesaj(IDgasit)));
        if (modifiedLine == NULL) {
            perror("Eroare la alocarea memoriei");
            exit(EXIT_FAILURE);
        }
        snprintf(modifiedLine, startIndex + 1, "%s", line);
        strcat(modifiedLine, "\"");
        strcat(modifiedLine, extrage_mesaj(IDgasit));
        strcat(modifiedLine, "\"->");
        strcat(modifiedLine, line + endIndex);
        strcpy(line, modifiedLine);
        free(modifiedLine);
        
    }
        
         }
                char *linie=line+2;
                strcat(mesaje,linie);
            	char *pozitie = strstr(mesaje, de_eliminat);
            	if (pozitie != NULL) {
        	size_t offset = pozitie - mesaje;
        	size_t lungime_substring = strlen(de_eliminat);
        	memmove(pozitie, pozitie + lungime_substring, strlen(pozitie + lungime_substring) + 1);
    		}     	
                 }	
          	}
          	}
    		}
    		fprintf(tempFile, "%s", line);  	}
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
                strcat(msgrasp,"Cont creat! \n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-intra in conversatie/open conversation \n-reimprospateaza/refresh \n-iesire/exit \nAi primit 0 mesaje cat ai fost offline!\nComanda ta: ");
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
    		comanda=10;
    		bzero(msgrasp,1024);
    		if (strcmp(msg,"help")==0 || strcmp(msg,"ajutor")==0||strcmp(msg,"x")==0)
    		{
    		de_deschis=0;  
    		strcat(msgrasp,"\nMeniu comenzi:\n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-intra in conversatie/open conversation \n-reimprospateaza/refresh \n-iesire/exit");
    		comanda=1;
    		}
    		else
    		if (strstr(msg,"utilizatori activi")!=NULL || strstr(msg,"online users")!=NULL)
    		{
    		strcat(msgrasp, "Lista utilizatorilor activi:\n");
       		strcat(msgrasp,clienti_online);
       		comanda=2;
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
       		comanda=3;
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
    		bzero(de_trimis_catre,1024);
    		strcat(msgrasp,"Destinatar mesaj: ");
    		}
    		else
    		if(strstr(msg,"reimprospatare")!=NULL || strstr(msg,"refresh")!=NULL)
    		{
    		comanda=4;
    		strcat(msgrasp,"\nMeniu comenzi:\n-ajutor/help\n-vezi utilizatori activi/see online users\n-vezi toti utilizatorii/see all users\n-trimite mesaj/send message\n-intra in conversatie/open conversation \n-reimprospateaza/refresh \n-iesire/exit");	
    		strcat(msgrasp,"\nS-a reimprospatat pagina.");	
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
    		{	printf("%s",buf);
    		gasit=1;
    		break;	}
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
		char randomID[4]; 
                generareRandomLitere(randomID);
		if (strstr(clienti_online,de_trimis_catre)!=NULL)
		{
		file=fopen(filename,"a");
		fprintf(file,"1 %s %s -> %s : %s \n",randomID,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		else
		{
		file=fopen(filename,"a");
		fprintf(file,"0 %s %s -> %s : %s \n",randomID,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		fclose(file);
		strcat(msgrasp,"Mesaj trimis.");
		comanda=5;
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
		{ perror("Eroare creare utilizatori.txt");	}
		}
		else
		{
		while(fgets(buf,sizeof(buf),file))
		{ char nume_utilizator[20];
		if (sscanf(buf, "%s", nume_utilizator) == 1) {
                 if (strcmp(nume_utilizator, msg) == 0) 
    		{ gasit=1; break; }
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
        	size_t offset = pozitie - buf;
        	size_t lungime_substring = strlen(de_eliminat);
        	memmove(pozitie, pozitie + lungime_substring, strlen(pozitie + lungime_substring) + 1);
    		}   		
    		char *pozitie2 = strstr(buf, de_eliminat2);
    		printf("\n%s %s",buf,pozitie2);
            	if (pozitie2 != NULL) {
        	size_t offset = pozitie2 - buf;
        	size_t lungime_substring = strlen(de_eliminat2);
        	memmove(pozitie2, pozitie2 + lungime_substring, strlen(pozitie2 + lungime_substring) + 1);
    		}
    		if (strstr(buf,"<")!=NULL && strstr(buf,">")!=NULL)
		{ const char *inceput = strchr(buf, '<');  // Găsește prima apariție a caracterului '<'
    		const char *sfarsit = strchr(buf, '>'); 
    		printf("inceput este %s\nsfrasit este%s",inceput,sfarsit);
		if (inceput != NULL && sfarsit != NULL) {
        char numar_str[sfarsit - inceput];
        printf("\n este=%s",numar_str); }
                 }	
    		strcat(msgrasp,buf+2);
        	}
    		}	
		de_deschis=2;
    		fclose(fisier);
    		strcat(istoric,msgrasp);
    		strcat(msgrasp,"\n-trimite r sa se reincarce pagina");
    		strcat(msgrasp,"\n-trimite x sa revii la meniu");
    		strcat(msgrasp,"\nComanda ta/ mesajul tau: ");	
    		comanda=6;
		}
		else
		{
		strcat(msgrasp,"Nume de utilizator inexistent! Reincearca: ");
		}
		fclose(file);
    		}
    		else
    		if(de_deschis>1&&strcmp(msg,"r")==0)
    		{			
		bzero(istoric,1024);
		strcat(msgrasp,"Conversatia cu ");
		strcat(msgrasp,de_trimis_catre);
		strcat(msgrasp," :\n\n");
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
    		int numar_linie = 1;
    		while(fgets(buf,sizeof(buf),file)) {
        	if (strstr(buf, de_trimis_catre) && strstr(buf, eu) ) {
        	buf[0]='2';
            	char *pozitie = strstr(buf, de_eliminat);
            	printf("\n%s %s",buf,pozitie);
            	if (pozitie != NULL) {
        	size_t offset = pozitie - buf;
        	size_t lungime_substring = strlen(de_eliminat);
        	memmove(pozitie, pozitie + lungime_substring, strlen(pozitie + lungime_substring) + 1);
    		}   		
    		char *pozitie2 = strstr(buf, de_eliminat2);
    		printf("\n%s %s",buf,pozitie2);
            	if (pozitie2 != NULL) {
        	size_t offset = pozitie2 - buf;
        	size_t lungime_substring = strlen(de_eliminat2);
        	memmove(pozitie2, pozitie2 + lungime_substring, strlen(pozitie2 + lungime_substring) + 1);
    		}
    		if (strstr(buf,"<")!=NULL && strstr(buf,">")!=NULL)
		{ const char *inceput = strchr(buf, '<');  // Găsește prima apariție a caracterului '<'
    		const char *sfarsit = strchr(buf, '>'); 
    		printf("inceput este %s\nsfrasit este%s",inceput,sfarsit);
		if (inceput != NULL && sfarsit != NULL) {
        char numar_str[sfarsit - inceput];
        printf("editat este=%s",numar_str); }
                 }	
    		strcat(msgrasp,buf+2);
    		printf("\n\n buf este %s ", buf+2);
        	}
    		}	
    		fclose(file);
    		strcat(msgrasp,istoric);
    		printf("\n\n\n istoric este %s \n\n\n", istoric);
    		strcat(msgrasp,"\n-trimite r sa se reincarce pagina");
    		strcat(msgrasp,"\n-trimite x sa revii la meniu");
    		strcat(msgrasp,"\nComanda ta/ mesajul tau: ");
    		comanda=7;
    		}
    		else
    		if(de_deschis>1)
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
		char randomID[4]; 
                generareRandomLitere(randomID);
		if (strstr(clienti_online,de_trimis_catre)!=NULL)
		{
		file=fopen(filename,"a");
		fprintf(file,"1 %s %s -> %s : %s \n",randomID,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		else
		{
		file=fopen(filename,"a");
		fprintf(file,"0 %s %s -> %s : %s \n",randomID,eu,de_trimis_catre,msg);
		printf(" %s -> %s : %s \n",eu,de_trimis_catre,msg);
		}
		fclose(file);
		strcat(msgrasp,"Mesaj trimis.");
		strcat(msgrasp,"\n-trimite x sa revii la meniu");
		strcat(msgrasp,"\n-trimite r sa se reincarce pagina");
		strcat(msgrasp,"\nComanda ta/ mesajul tau: ");		
		comanda=8;
    		}
    		else
    		 {
    		 comanda=2;
    		 strcat(msgrasp,"Comanda inexistenta!");
    		 }
    		if(comanda<=10)
    		{
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
		{perror("Eroare creare mail.txt");	}
		}
		else
		{
		char line[1000];
		char start='1';
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
          	char *linie=line+2;
          	strcat(mesaje,linie);
          	char *pos = strchr(line, '1');
          	if (pos != NULL) {
             	*pos = '2';
            	}
            	char *pozitie = strstr(mesaje, de_eliminat);
            	if (pozitie != NULL) {
        	size_t offset = pozitie - mesaje;
        	size_t lungime_substring = strlen(de_eliminat);
        	memmove(pozitie, pozitie + lungime_substring, strlen(pozitie + lungime_substring) + 1);
    		}	
    		if (strstr(mesaje,"<")!=NULL && strstr(mesaje,">")!=NULL)
		{
		 const char *inceput = strchr(mesaje, '<');  // Găsește prima apariție a caracterului '<'
    const char *sfarsit = strchr(mesaje, '>'); 
    printf("inceput este %s\nsfrasit este%s",inceput,sfarsit);
		if (inceput != NULL && sfarsit != NULL) {
        char numar_str[sfarsit - inceput];
        printf("\n este=%s",numar_str);
    }
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
    		if (comanda<=5)
    		{
    		char text[128];
		printf("%s",mesaje);
		sprintf(text, "\nAi primit %d mesaje de la ultima reincarcare.\n", total);
		strcat(msgrasp,text);
		if (total>0)
		   strcat(msgrasp,mesaje);
    		strcat(msgrasp,"\nComanda ta:");
    		}
    		}
    		printf("\n\n de deschis este %d \n\n", de_deschis);
    		break;
    		}	
    		printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
    		/* returnam mesajul clientului */
    		if (write (client, msgrasp, 1024) <= 0)
    		{   perror ("[server]Eroare la write() catre client.\n");
    			continue;		/* continuam sa ascultam */
    		} 		else
    			printf ("[server]Mesajul a fost trasmis cu succes.\n");
    		}
    		/* am terminat cu acest client, inchidem conexiunea */
    		close (client);
    		char *pozitie = strstr(clienti_online, eu);
    		if (pozitie != NULL) {
        	size_t lungime_sir_extras = strlen(eu);
        size_t offset = pozitie - clienti_online;
        memmove(pozitie, pozitie + lungime_sir_extras, strlen(pozitie + lungime_sir_extras) + 1);
    		exit(0);		}
    	}
    }				/* while */
    munmap(clienti_online, MAX_LENGTH);
    return 0;
}				/* main */
