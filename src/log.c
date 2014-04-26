#include "log.h"
#include <stdio.h>
#include <stdlib.h>


/*Liste des logs ouverts*/
typedef struct log {
	FILE* lf;
	struct log* next;
}logList;

static logList  *logs = NULL,
				**nextLog = &logs;

/*Nombre de logs ouverts*/
static int logCount = 0;


int eg_createLog(char* name)
{
	logList* newLog = malloc(sizeof(logList));
	if(newLog == NULL)
	{
		fprintf(stderr, "Log : Erreur : impossible d'allouer de la mémoire\n");
		return 0;
	}
	
	newLog->lf = fopen(name, "w");
	if(newLog->lf == NULL)
	{
		fprintf(stderr, "Log : Erreur : impossible de créer le fichier %s\n", name);
		return 0;
	}
	//En-tête du log
	fprintf(newLog->lf, "[Log : %s]\n\n", name);
	newLog->next = NULL;
	
	//ajout du log dans la liste
	*nextLog = newLog;
	
	logCount++;
	nextLog = &(newLog->next);
	
	return logCount;
}

int eg_bufToLog(int logId, char* data, size_t nbBytes)
{
	if(logId <= 0 || logCount < logId)
	{
		fprintf(stderr, "Log : Erreur : le log %d n'existe pas\n", logId);
		return -1;
	}
	//récupération du log d'ID logId
	logList* p = logs;
	int i = 1;
	while(i < logId)
	{
		i++;
		p = p->next;
	}
	//écriture dans le log récupéré
	if(fwrite(data, sizeof(char), nbBytes, p->lf) != nbBytes)
	{
		fprintf(stderr, "Log : Erreur : toutes les données n'ont pas pu être écrites dans le log %d\n", logId);
		return -1;
	}
		
	fputc('\n', p->lf);
	return 0;
}

int eg_strToLog(int logId, char* fmt, ...)
{
	if(logId <= 0 || logCount < logId)
	{
		fprintf(stderr, "Log : Erreur : le log %d n'existe pas\n", logId);
		return -1;
	}
	
	//récupération du log d'ID logId
	logList* p = logs;
	int i = 1;
	while(i < logId)
	{
		i++;
		p = p->next;
	}
	//écriture de la chaine
	va_list fmt_args;
	va_start(fmt_args, fmt);
	if(vfprintf(p->lf, fmt, fmt_args) < 0)
	{
		fprintf(stderr, "Log : Erreur lors de l'écriture d'une chaîne dans le log %d\n", logId);
		return -1;
	}
	va_end(fmt_args);

	fputc('\n', p->lf);
	return 0;
}

int eg_closeLog(int logId)
{
	if(logId <= 0 || logCount < logId)
	{
		fprintf(stderr, "Log : Erreur : le log %d n'existe pas\n", logId);
		return -1;
	}

	//variables de parcours de la liste
	logList* p = logs;
	FILE* pf = NULL;
	//cas du premier élément
	if(logId == 1)
	{
		pf = logs->lf;
		logs = logs->next;
		free(p);
		
		if(logCount == 1)
			nextLog = &logs;
	}
	else
	{
		//se positionner sur le log d'avant logId
		int i = 1;
		while(i < logId-1)
		{
			i++;
			p = p->next;
		}
		logList* q = p->next;
		pf = q->lf;
		p->next = q->next;
		free(q);
		
		if(logId == logCount)
			nextLog = &(p->next);
	}
	fclose(pf);
	logCount--;
	
	return 0;
}

int eg_closeAllLogs()
{
	if(logCount == 0)
	{
		fprintf(stderr, "Log : Warning : aucun log à fermer\n");
		return -1;
	}
	
	logList *curr = logs, *prec = NULL;
	
	while(curr != NULL)
	{
		fclose(curr->lf);
		prec = curr;
		curr = curr->next;
		free(prec);
	}
	logs = NULL;
	nextLog = &logs;
	
	return 0;
}
	
	
