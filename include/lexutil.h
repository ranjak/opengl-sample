#ifndef LEXUTIL_H
#define LEXUTIL_H

#include <stdbool.h>

#define EG_MAX_IDENT_LG 48
/*
* Analyseur lexical, conçu pour l'analyse de fichiers de configuration.
* Cet analyseur travaille sur un fichier à la fois, son but est d'être
* utilisé par d'autres modules qui se chargeront de l'analyse syntaxique.
*/


/*****************************************************************************
*********Variables maintenues au cours de l'analyse par l'analyseur***********
******************************************************************************/

/*Caractère actuel lu avec la fonction lex_readChar.*/
extern char lex_CurrentChar;

/*Ligne actuelle de l'analyse lexicale dans le fichier*/
extern int lex_LineCount;

/*Si on est arrivé à la fin du fichier ou s'il y a une erreur, ceci vaudra 0.*/
extern int lex_EtatOK;


/*****************************************************************************
************************FONCTIONS D'ANALYSE LEXICALE**************************
******************************************************************************/

/*Tente d'ouvrir le fichier indiqué.
* Le fichier est ouvert en vue d'une analyse lexicale,
* c'est à lui que feront référence les appels aux fonctions de ce module.
* PARAM:
	fichier au format texte à ouvrir.
	id de log où écrire les messages (0 = aucun).
* RETOURNE:
	true(1) si succès, false(0) si échec.
*/
bool lex_openFile(char*, int);

/*Ferme le fichier actuellement lu par l'analyseur.
* RETOURNE:
	1 si OK, 0 sinon.
	renvoie aussi 1 si aucun fichier n'était ouvert par l'analyseur.
*/
bool lex_closeFile();

/*lire un caractère. Augmente le compteur de lignes si saut détecté.
* Retourne la valeur de etatOK*/
int lex_readChar();

/*lire des caractères blancs jusqu'à un non-blanc (1) ou EOF (0).
* lit aussi les commentaires (précédés par #) jusqu'à newline.*/
int lex_readWhites();

/*sauter une ligne, s'arrêter si on tombe sur un caractère non commenté.
* Retourne 1 si saut OK, 0 si caractère rencontré.*/
int lex_readToNewline();

/*Lire jusqu'à rencontrer le caractère sépcifié (ne le passe pas).
* Retourne 1 si caractère atteint, 0 sinon (EOF)*/
int lex_readUntil(char);

//En cas d'erreur syntaxique, saute au moins une ligne.
void lex_errorLoop();

//tente de lire un identificateur, stocké dans la chaîne passée si OK.
//retourne 0 si erreur, 1 si OK.
bool lex_readIdent(char*);

//lit une chaîne comprenant n'importe quel caractère, ALLOUE LA MEMOIRE.
//retourne la taille de la chaîne, -1 si erreur.
int lex_readString(char**);

//lit un nombre. 1 si succès, 0 si erreur. Pas d'allocation.
bool lex_readInt(int*);

//lit un flottant. Requiert la présence d'un '.' dans le nombre.
bool lex_readFloat(float*);

//lit un booléen. 0 ou 1, true ou false (insensible à la casse)
bool lex_readBool(bool*);

#endif

