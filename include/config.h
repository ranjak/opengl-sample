#ifndef CONFIG_H
#define CONFIG_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include "lexutil.h"
//longueur maximale d'un indentificateur, en comptant le \0
#define EG_MAX_IDENT_LG 48
/*
*Fichier de configuration
*Format :
*[Section]
*paramètre=valeur
*/


/*typedef enum Eg_Type
{
	EG_CHAR,
	EG_STRING,
	EG_INT,
	EG_FLOAT,
	EG_BOOL
}Eg_Type;*/

enum allocation
{
	EG_NOT_ALLOCED,
	EG_STA_ALLOCED,
	EG_DYN_ALLOCED
};

/*Liste de
* couples paramètre=valeur
* (name=value).
* Contient des pointeurs de fonction pour gérer la valeur du paramètre.
*/
typedef struct Eg_cfgEntry {
	char*	label;
	void* 	value;
	void	*min_val,
		*max_val;
		
	enum allocation alloc;
	
	bool (*setValue)(struct Eg_cfgEntry*, void*);
	bool (*valueFromStream)(struct Eg_cfgEntry*);
	const char* (*valToStr)(struct Eg_cfgEntry*);
}Eg_cfgEntry;

/*
* Définit une section de configuration.
* Contient un nom de section,
* le tableau de descripteurs de paramètres,
* le nombre de ces paramètres,
* un booléen indiquant si la section a été lue dans le fichier de config.
*/
typedef struct configSection
{
	char* name;
	int paramCount;
	int wasRead;
	
	Eg_cfgEntry* entries;

}Eg_configSection;

void eg_loadConfig();

void eg_writeConfig();

int eg_readConfigFile(Eg_configSection*, int);

/**Ces fonctions doivent être utilisées dans des structures cfgEntry.
* Les fonctions "set" copient la valeur passée en paramètre,
* cette valeur peut donc être libérée à l'envi par l'appelant.
* Pour int et float, les champs min_val et max_val définissent les bornes.
* Pour bool, ils sont sans effet.
* Pour string, ils bornent la taille de la chaîne.
* Dans tous les cas, les mettre à NULL signifie ne pas imposer de limite.
* Ces fonctions allouent dynamiquement de la mémoire pour stocker la valeur lue
* si aucune n'a été allouée auparavant. (note : pour string, on alloue systématiquement,
* toute mémoire précédemment allouée est libérée).
*/
bool cfg_setInt(Eg_cfgEntry*, void*);
bool cfg_setFloat(Eg_cfgEntry*, void*);
bool cfg_setBool(Eg_cfgEntry*, void*);
bool cfg_setString(Eg_cfgEntry*, void*);

bool cfg_readInt(Eg_cfgEntry*);
bool cfg_readFloat(Eg_cfgEntry*);
bool cfg_readBool(Eg_cfgEntry*);
bool cfg_readString(Eg_cfgEntry*);

#endif
