#ifndef LOG_H
#define LOG_H

#include <stddef.h>
#include <stdarg.h>
/*
Gestion des logs et des erreurs du moteur
*/

/* Ouverture d'un fichier de log en écriture.
 PARAM : nom du fichier.
 RETOURNE : un numéro de log >= 1. 0 si erreur.
*/
int eg_createLog(char[]);

/* Ecrire dans le log spécifié.
 PARAMS:	n° de log
 			buffer de données à écrire
 			nombre d'octets à écrire
 RETOURNE: 	0 si succès
 			-1 si erreur.
*/
int eg_bufToLog(int, char[], size_t);

/* Ecrire une chaine dans un log.
Wrapper pour fprintf.
 PARAMS: 	n° de log
 	 	format de la chaine
 	 	paramètres format
 RETOURNE: 	0 si succès
 			-1 si échec
*/
int eg_strToLog(int, char[], ...);

/* Fermer le log spécifié.
 PARAM: n° de log.
 RETOURNE: 0 si succès
 		   -1 si erreur.
*/
int eg_closeLog(int);

/* Fermer tous les logs précédemment ouverts.
 RETOURNE: 	0 si succès
 			-1 si erreur
*/
int eg_closeAllLogs();

#endif
