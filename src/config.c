#include "config.h"
#include "log.h"
#include "controls.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>



static char* configFileName = "config.cfg";
static int ConfigLog = 0;

Eg_configSection Sections[1];
int sectionCount = 1;
static bool init = false;


static int readEntries(Eg_configSection*);
int writeCfgSection(const Eg_configSection* conf, FILE*);


void eg_loadConfig()
{
	ConfigLog = eg_createLog("config.log");
	if(ConfigLog == 0)
		fprintf(stderr, "[Config] Could not create config log.\n");
		
	eg_getControlCfgDesc(&Sections[0]);
	eg_readConfigFile(Sections, 1);
	init = true;
}

/*
* Lecture du fichier de configuration
* Remplit la liste des configs.
* Cette fonction devrait être appelée au début du programme,
* chaque section de configuration ainsi générée étant ensuite
* demandée par les modules en ayant besoin.
* RETOURNE:
	Nombre de sections récupérées
*/
int eg_readConfigFile(Eg_configSection* cfg, int numSections)
{
	int sectionCount = 0, i;
	char currentName[EG_MAX_IDENT_LG];
	
	if(!lex_openFile(configFileName, ConfigLog))
	{
		fprintf(stderr, "[Config] Could not open config file %s. Default settings will be used.\n", configFileName);
		return 0;
	}

	//Lecture du fichier : 1 passage dans le while = 1 section
	while(lex_EtatOK)
	{
		lex_readWhites();
		
		//On doit trouver un début de section bien formé.
		while(lex_CurrentChar != '[' && lex_EtatOK)
		{
			//erreur : test à la ligne suivante.
			eg_strToLog(ConfigLog, "[Config] Error at line %d, expected section entry.", lex_LineCount);
			lex_errorLoop();
		}
		//soit on a trouvé une section, soit on est à EOF !
		if(lex_EtatOK)
		{
			lex_readChar();
			if(lex_readIdent(currentName) && lex_CurrentChar==']')
			{
				//passer le ']'
				lex_readChar();
				//recherche d'une section de nom correspondant
				for(i=0 ; i<numSections ; i++)
					if(strcmp(cfg[i].name, currentName) == 0)	break;
				
				if(i < numSections)
				{
					//il faut un saut de ligne après le nom de section
					if(!lex_readToNewline() && lex_EtatOK)
					{
						eg_strToLog(ConfigLog, "[Config] Error at line %d, Section entry must be followed by newline", lex_LineCount);
						lex_errorLoop();
					}
					//récup des paramètres de la section
					if(readEntries(&cfg[i]) > 0)
						sectionCount++;
					else
						eg_strToLog(ConfigLog, "[Config] Section [%s] held no valid value.", currentName);
				}
				else
				{
					eg_strToLog(ConfigLog, "[Config] Line %d : unknown section : %s.", lex_LineCount, currentName);
					lex_errorLoop();
				}
			}
			else
			{
				eg_strToLog(ConfigLog, "[Config] Line %d : error in section name format", lex_LineCount);
				lex_errorLoop();
			}
		}
		else
			eg_strToLog(ConfigLog, "[Config] unexpected end of file");
	}
	
	eg_strToLog(ConfigLog, "[Config] End of file, loaded %d valid sections.\n", sectionCount);
	
	lex_closeFile();
	return sectionCount;
}



int readEntries(Eg_configSection* cfg)
{
	char entryLabel[EG_MAX_IDENT_LG];
	int i;
	int entryCount = 0;
	
	do
	{
		if(!lex_readWhites())
			continue;
		//Lecture d'un label d'entrée :
		//(blancs)label(blancs)=(blancs)valeur(blancs)\n
		//....
		if(!lex_readIdent(entryLabel))
		{
			eg_strToLog(ConfigLog, "[Config] Error at line %d : invalid parameter label.", lex_LineCount);
			lex_errorLoop();
			continue;
		}
		
		lex_readWhites();
		if(lex_CurrentChar != '=')
		{
			eg_strToLog(ConfigLog, "[Config] Error at line %d : expected '=', read '%c'.", lex_LineCount, lex_CurrentChar);
			lex_errorLoop();
			continue;
		}
		//On est arrivé à label=
		//parcours des descripteurs de paramètres à la recherche du label
		lex_readChar();
		i = 0;
		while(i < cfg->paramCount
		&& strcmp(cfg->entries[i].label, entryLabel) != 0)
			i++;
		
		if(i == cfg->paramCount)
		{
			eg_strToLog(ConfigLog, "[Config] Error at line %d : no parameter %s in section %s.", lex_LineCount, entryLabel, cfg->name);
			lex_errorLoop();
			continue;
		}
		//Récupération de la valeur
		lex_readWhites();
		if(cfg->entries[i].valueFromStream(&cfg->entries[i]))
			entryCount++;
		else
		{
			eg_strToLog(ConfigLog, "[Config] Error at line %d : incorrect value for parameter %s.", lex_LineCount, entryLabel);
			lex_errorLoop();
			continue;
		}
		
		//1 entrée max par ligne
		if(!lex_readToNewline() && lex_EtatOK)
		{
			eg_strToLog(ConfigLog, "[Config] Error at line %d : unexpected characters found after parameter affectation.", lex_LineCount);
			lex_errorLoop();
		}
		lex_readWhites();
		
	} while(lex_CurrentChar != '[' && lex_EtatOK);
	
	//fin de lecture de la section
	cfg->wasRead = 1;
	eg_strToLog(ConfigLog, "[Config] End of section %s, loaded %d parameter(s) out of %d.", cfg->name, entryCount, cfg->paramCount);
	return entryCount;
}


void eg_writeConfig()
{
	int i;
	
	if(init == false)
	{
		fprintf(stderr, "[Config] Warning : configuration was not initialized. Cannot save config file.\n");
		return;
	}
	
	FILE* cfg = fopen(configFileName, "w");
	if(cfg == NULL)
	{
		eg_strToLog(ConfigLog, "[Config] Could not open config file for writing. Changes in options will not be saved.");
		return;
	}
	
	for(i=0 ; i < sectionCount ; i++)
		writeCfgSection(&Sections[i], cfg);
	
	eg_strToLog(ConfigLog, "[Config] Wrote cofiguration to file %s.\n", configFileName);
	fclose(cfg);
}


int writeCfgSection(const Eg_configSection* conf, FILE* stream)
{
	int i;
	Eg_cfgEntry* entries = conf->entries;
	//début section
	fprintf(stream, "[%s]\n\n", conf->name);
	
	//écriture de chaque couple paramètre=valeur
	for(i=0 ; i<conf->paramCount ; i++)
	{
		fprintf(stream,
			"%s = %s\n", entries[i].label, entries[i].valToStr(&entries[i]));
	}
	
	fputc('\n', stream);
	return 0;
}
	
	

bool cfg_setInt(Eg_cfgEntry* entry, void* newval)
{
	if(newval == NULL)
	{
		fprintf(stderr, "[Config] Warning : tried to set NULL value for parameter %s", entry->label);
		return false;
	}
	if(entry->min_val != NULL && *(int*)newval < *(int*)entry->min_val)
	{
		eg_strToLog(ConfigLog, "[Config] Error : value %d out of bounds for parameter %s, "\
		"expects a value of at least %d", *(int*)newval, entry->label, *(int*)entry->min_val);
		return 0;
	}
	if(entry->max_val != NULL && *(int*)newval > *(int*)entry->max_val)
	{
		eg_strToLog(ConfigLog, "[Config] Error : read value %d out of bounds for parameter %s, "\
		"expects a value of at most %d", *(int*)newval, entry->label, *(int*)entry->max_val);
		return 0;
	}
	
	//allouer la mémoire si elle ne l'est pas !
	if(entry->alloc == EG_NOT_ALLOCED)
	{
		entry->value = malloc(sizeof(int));
		entry->alloc = EG_DYN_ALLOCED;
	}
		
	//on a passé tous les tests, affectation !
	*(int*)entry->value = *(int*)newval;
	return 1;
}

bool cfg_setFloat(Eg_cfgEntry* entry, void* newval)
{
	if(newval == NULL)
	{
		fprintf(stderr, "[Config] Warning : tried to set NULL value for parameter %s", entry->label);
		return false;
	}
	if(entry->min_val != NULL && *(float*)newval < *(float*)entry->min_val)
	{
		eg_strToLog(ConfigLog, "[Config] Error : value %f out of bounds for parameter %s, "\
		"expects a value of at least %f", *(float*)newval, entry->label, *(float*)entry->min_val);
		return false;
	}
	if(entry->max_val != NULL && *(float*)newval > *(float*)entry->max_val)
	{
		eg_strToLog(ConfigLog, "[Config] Error : value %f out of bounds for parameter %s, "\
		"expects a value of at most %f", *(float*)newval, entry->label, *(float*)entry->max_val);
		return false;
	}
	
	//allouer la mémoire si elle ne l'est pas !
	if(entry->alloc == EG_NOT_ALLOCED)
	{
		entry->value = malloc(sizeof(float));
		entry->alloc = EG_DYN_ALLOCED;
	}
	
	*(float*)entry->value = *(float*)newval;
	return true;
}

bool cfg_setString(Eg_cfgEntry* entry, void* newval)
{
	if(newval == NULL)
	{
		fprintf(stderr, "[Config] Warning : tried to set NULL value for parameter %s", entry->label);
		return false;
	}
	int size = strlen((char*)newval);
	//min et max sont ici des entiers restreignant la taille de la chaîne.
	if(entry->min_val != NULL && size < *(int*)entry->min_val)
	{
		eg_strToLog(ConfigLog, "[Config] Error : string : \n%s\n is too short for parameter %s, "\
		"expects a string of at least %d characters", (char*)newval, entry->label, *(int*)entry->min_val);
		return false;
	}
	if(entry->max_val != NULL && size > *(int*)entry->max_val)
	{
		eg_strToLog(ConfigLog, "[Config] Error : string : \n%s\n is too long for parameter %s, "\
		"expects a string of at most %d characters", (char*)newval, entry->label, *(int*)entry->max_val);
		return false;
	}
	//s'il y avait déjà une chaîne allouée dynamiquement, on la libère.
	if(entry->alloc == EG_DYN_ALLOCED)
		free(entry->value);
	
	entry->value = malloc(sizeof(char)*(size+1));
	memcpy(entry->value, newval, size+1);
	entry->alloc = EG_DYN_ALLOCED;
	return true;
}

bool cfg_setBool(Eg_cfgEntry* entry, void* newval)
{
	if(newval == NULL)
	{
		fprintf(stderr, "[Config] Warning : tried to set NULL value for parameter %s", entry->label);
		return false;
	}
	if(*(bool*)newval != true && *(bool*)newval != false)
	{
		fprintf(stderr, "[Config] Warning : tried to affect a non-bool value to bool parameter %s", entry->label);
		return false;
	}
	
	if(entry->alloc == EG_NOT_ALLOCED)
	{
		entry->value = malloc(sizeof(bool));
		entry->alloc = EG_DYN_ALLOCED;
	}
	
	*(bool*)entry->value = *(bool*)newval;
	return true;
}


bool cfg_readInt(Eg_cfgEntry* entry)
{
	int temp;
	if(!lex_readInt(&temp))
		return 0;
	
	if(entry->setValue(entry, &temp))
		return true;
	else
		return false;
}

bool cfg_readFloat(Eg_cfgEntry* entry)
{
	float temp;
	if(!lex_readFloat(&temp))
		return false;
	
	if(entry->setValue(entry, &temp))
		return true;
	else
		return false;
}

bool cfg_readString(Eg_cfgEntry* entry)
{
	char* temp;
	bool res;
	if(lex_readString(&temp) < 0)
		return false;
	
	if(entry->setValue(entry, &temp))
		res = true;
	else
		res = false;
	
	free(temp);
	return res;
}

bool cfg_readBool(Eg_cfgEntry* entry)
{
	bool temp;
	if(!lex_readBool(&temp))
		return false;
	
	if(entry->setValue(entry, &temp))
		return true;
	else
		return false;
}






