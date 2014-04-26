#include "lexutil.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <math.h>


//Définition des variables externes
char lex_CurrentChar = EOF;
int lex_LineCount = 0;
int lex_EtatOK = 0;

static int anaLog = 0;
static FILE* filePtr = NULL;

bool lex_openFile(char* filename, int log)
{
	filePtr = fopen(filename, "r");
	if(filePtr == NULL)
	{
		fprintf(stderr, "[Lex] Couldn't open file %s\n", filename);
		return false;
	}
	lex_EtatOK = 1;
	lex_LineCount = 1;
	//Lecture du premier caractère, on voit si le fichier est vide ou non
	lex_readChar();
	if(!lex_EtatOK)
	{
		fprintf(stderr, "[Lex] File %s appears to be empty. Closing.\n", filename);
		lex_LineCount = 0;
		fclose(filePtr);
		return false;
	}
	//sinon, OK pour commencer l'analyse !
	anaLog = log;
	
	eg_strToLog(anaLog, "[Lex] File %s opened successfully, ready for lexical analysis.\n", filename);
	return true;
}

bool lex_closeFile()
{
	if(filePtr == NULL)
		return true;
		
	if(fclose(filePtr) != EOF)
	{
		eg_strToLog(anaLog, "[Lex] Analysis terminated, file closed successfully.\n");
		return true;
	}
	else
	{
		eg_strToLog(anaLog, "[Lex] Analysis terminated, but error closing the file.\n");
		return false;
	}
}
	

int lex_readChar()
{
	int c = fgetc(filePtr);
	
	if(c == EOF) lex_EtatOK=0;
	
	lex_CurrentChar = (char)c;
	if(lex_CurrentChar == '\n')	lex_LineCount++;
	return lex_CurrentChar;
}

int lex_readWhites()
{
	while(isspace(lex_CurrentChar) || lex_CurrentChar=='#')
	{
		//saut de ligne si commentaires.
		if(lex_CurrentChar=='#')
			lex_readUntil('\n');
		lex_readChar();
	}		
	
	return lex_EtatOK;
}

int lex_readToNewline()
{
	while(isblank(lex_CurrentChar))
		lex_readChar();
	if(lex_CurrentChar=='#')
		lex_readUntil('\n');
	
	if(lex_CurrentChar == '\n')
	{
		lex_readChar();
		return 1;
	}
	else
		return 0;
}

int lex_readUntil(char c)
{
	while(lex_EtatOK && lex_CurrentChar != c)
		lex_readChar();
		
	return lex_EtatOK;
}

void lex_errorLoop()
{
	while(lex_EtatOK && lex_CurrentChar != '\n')
	{
		//on saute les strings
		if(lex_CurrentChar=='"')
		{
			lex_readChar();
			lex_readUntil('"');
			lex_readChar();
		}
		else if(lex_CurrentChar=='#')
			lex_readUntil('\n');
		else
			lex_readChar();
	}
	lex_readWhites();
}

/*
* Tente de lire un identificateur dans le fichier de config.
* Les identificateurs sont composés de lettres (min ou MAJ),
* de chiffres, d'underscores et comportent au maximum EG_MAX_IDENT_LG
* caractères.
* PARAM:
	chaîne qui recevra l'ident. Doit pointer une location valide.
* RETOURNE:
	1 si ident lu sans erreur, 0 sinon.
*/
bool lex_readIdent(char* ident)
{
	char temp[EG_MAX_IDENT_LG];
	int length = 0, done = 0;
	
	if(!isalpha(lex_CurrentChar))
	{
		eg_strToLog(anaLog, "[Lex] Line %d : error : expected identificator, read %c. Identificators must start with a letter.", lex_LineCount, lex_CurrentChar);
		return 0;
	}
	
	while(!done && length < EG_MAX_IDENT_LG-1)
	{
		
		//le caractère lu est-il autorisé ?
		if(isalnum(lex_CurrentChar) || lex_CurrentChar=='_')
		{
			temp[length] = lex_CurrentChar;
			length++;
			lex_readChar();
		}
		//sinon, on considère que l'ident s'arrête là
		else
			done = 1;
			
	}
	//si done est faux, on a dépassé la limite de taille.
	if(!done)
	{
		eg_strToLog(anaLog, "[Lex] Error at line %d : max identificator size (%d) overrun, discarding value.", lex_LineCount, EG_MAX_IDENT_LG-1);
		return 0;
	}
	//ident valide si on a lu au moins 1 caractère.
	if(length > 0)
	{
		temp[length] = '\0';
		memcpy(ident, temp, length+1);
		return 1;
	}
	
	//sinon, aucun caractère valide lu.
	return 0;
}

/*
* Lit une chaîne de caractères dans le fichier de config.
* Une chaîne est une valeur qui doit être de forme "chaîne".
* Pas de limite de longueur. La chaîne sera null-terminée.
* PARAM:
	destination pour la chaîne. DOIT ETRE LIBEREE PAR LE CALLER
* RETOURNE:
	nombre de caractères dans la chaîne (vide acceptée).
	-1 si chaîne incorrecte.
*/
int lex_readString(char** str)
{
	int length = 0, i = 0;
	int debutLine;
	fpos_t debutPos;
	
	if(lex_CurrentChar != '"')
		return -1;
		
	fgetpos(filePtr, &debutPos);
	debutLine = lex_LineCount;
	lex_readChar();
	while(lex_EtatOK && lex_CurrentChar != '"')
	{
		length++;
		lex_readChar();
	}
	lex_LineCount = debutLine;
	//Si fin de fichier sans trouver de fin de chaîne, échec.
	if(!lex_EtatOK)
	{
		eg_strToLog(anaLog, "[Lex] Error : string beginning at line %d has no end.", debutLine);
		return -1;
	}
	//Sinon -> ok, récupération de la chaîne.
	*str = malloc((length+1)*sizeof(char));
	fsetpos(filePtr, &debutPos);
	
	while(i < length)
		(*str)[i++] = lex_readChar();
	
	(*str)[length] = '\0';
	//On doit reprendre un caractère d'avance.
	lex_readChar();
	lex_readChar();
	
	return length;
}

bool lex_readInt(int* val)
{
	int res = 0, limitCheck = 0, isNeg = 0;
	
	if(lex_CurrentChar=='-')
	{
		isNeg = 1;
		lex_readChar();
	}
	//lecture digit par digit
	if(isdigit(lex_CurrentChar))
	{
		do {
			//limitCheck = res avec un digit de moins. S'il n'est plus derrière res -> limite atteinte.
			//il faut l'utiliser seulement quand res vaut + que 0, sinon fausse alerte (par ex. avec res = 02).
			if(res > 0)
				limitCheck = res;
			res = res*10 + (lex_CurrentChar-'0');
			lex_readChar();
		} while(isdigit(lex_CurrentChar) && limitCheck < res);
		//si on a un point juste après le nombre, c'est qu'on voulait faire passer un float : pas bien.
		if(lex_CurrentChar=='.')
		{
			eg_strToLog(anaLog, "[Lex] Line %d : expected int, read float.", lex_LineCount);
			return 0;
		}
		//si res n'est pas plus grand que limitCheck, il a atteint la limite d'int.
		if(limitCheck != 0 && limitCheck >= res)
		{
			eg_strToLog(anaLog, "[Lex] Int representation limit overrun at line %d, discarding value.", lex_LineCount);
			return 0;
		}
		//sinon, succès : enregistrement de la valeur.
		if(isNeg)	res = -res;
		
		*val = res;
		return 1;
	}
	
	eg_strToLog(anaLog, "[Lex] Error at line %d : expected int, read %c", lex_LineCount, lex_CurrentChar);
	return 0;
}

bool lex_readFloat(float* val)
{
	float res=0.f, limitCheck=0.f;
	int decDigitNb = 0;
	bool isNeg = false;

	if(lex_CurrentChar=='-')
	{
		isNeg = true;
		lex_readChar();
	}
	
	if(isdigit(lex_CurrentChar))
	{
		do {
			if(res > 0.f)
				limitCheck = res;
			res = res*10.f + (float)(lex_CurrentChar-'0');
			lex_readChar();
		} while(isdigit(lex_CurrentChar) && limitCheck < res);
	}
	if(lex_CurrentChar == '.')
	{
		lex_readChar();
		while(isdigit(lex_CurrentChar) && decDigitNb <= FLT_DIG)
		{
			decDigitNb++;
			res = res + (float)(lex_CurrentChar-'0') / (float)pow(10.d, (double)decDigitNb);
			lex_readChar();
		}
	}
	else
	{
		eg_strToLog(anaLog, "[Lex] Error at line %d : expected floating point number", lex_LineCount);
		return false;
	}
	
	if((limitCheck != 0.f && limitCheck >= res) || decDigitNb > FLT_DIG)
	{
		eg_strToLog(anaLog, "[Lex] Error at line %d : floating point number limit overrun", lex_LineCount);
		return false;
	}
	
	//succès
	if(isNeg)	res = -res;
	*val = res;
	return true;
}

bool lex_readBool(bool* val)
{
	int strsize;
	bool res = 0;
	char temp, tempstr[EG_MAX_IDENT_LG];
	
	//on lit soit 0 ou 1, soit true or false
	temp = lex_CurrentChar;
	if(temp=='0' || temp=='1')
	{
		if(isspace(lex_readChar()) || !lex_EtatOK)
		{
			*val = temp - '0';
			res = 1;
		}
	}
	else if(lex_readIdent(tempstr))
	{
		//on tente de lire true ou false
		strsize = strlen(tempstr);
		if(strsize==4 || strsize==5)
		{
			//on ne veut pas de sensibilité à la casse
			strsize--;
			for(;strsize >= 0 ; strsize--)
				tempstr[strsize] = tolower(tempstr[strsize]);
			
			if(strcmp("true", tempstr) == 0)
			{
				*val = true;
				res = 1;
			}
			else if(strcmp("false", tempstr) == 0)
			{
				*val = false;
				res = 1;
			}
		}
	}
	
	if(res == 0)
		eg_strToLog(anaLog, "[Lex] Error line %d : expected boolean value.", lex_LineCount);
	
	return res;
}

