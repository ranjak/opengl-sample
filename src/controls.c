#include "controls.h"


static bool cfg_readKey(Eg_cfgEntry* entry);
static const char* cfg_keyToStr(Eg_cfgEntry* entry);

//sert à stocker le résultat de keyToStr. Est remplacé à chaque appel de la fonction.
static char* tempStr = NULL;
/*
* Liste des valeurs associées aux actions
* Valeurs par défaut pouvant être changées
* pendant l'exécution ou au chargement de l'ini
*/
SDL_Keycode Eg_KeyBindings[EG_KEY_NOACTION] = {
	SDLK_t,		//CHANGESHAPE
	SDLK_ESCAPE,	//EXIT
	SDLK_LEFT,	//MOVELEFT
	SDLK_RIGHT,	//MOVERIGHT
	SDLK_w,		//TGWFRENDER
	SDLK_UP,	//SCALEUP
	SDLK_DOWN,	//SCALEDOWN
	SDLK_b,		//ROTZL
	SDLK_n		//ROTZR
};

Eg_cfgEntry keyDesc[EG_KEY_NOACTION] = {
	{
		"ChangeShape",
		&Eg_KeyBindings[0],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"Exit",
		&Eg_KeyBindings[1],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"MoveLeft",
		&Eg_KeyBindings[2],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"MoveRight",
		&Eg_KeyBindings[3],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"ToggleWireframe",
		&Eg_KeyBindings[4],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"ScaleUp",
		&Eg_KeyBindings[5],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"ScaleDown",
		&Eg_KeyBindings[6],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"RotateZleft",
		&Eg_KeyBindings[7],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	},{
		"RotateZright",
		&Eg_KeyBindings[8],
		NULL, NULL,
		EG_STA_ALLOCED,
		NULL, cfg_readKey, cfg_keyToStr
	}
};


void eg_getControlCfgDesc(Eg_configSection* cs)
{
	if(cs != NULL)
	{
		cs->name = "Controls";
		cs->paramCount = EG_KEY_NOACTION;
		cs->wasRead = 0;
		cs->entries = keyDesc;
	}
}


Eg_KeyAction eg_getActionFromKey(SDL_Keycode key)
{
	int i=0;
	while(key != Eg_KeyBindings[i] && i < EG_KEY_NOACTION)
		i++;
	
	return i;
}


void eg_bindKey(Eg_KeyAction act, SDL_Keycode key)
{
	if(act < EG_KEY_NOACTION)
		Eg_KeyBindings[act] = key;
}


bool cfg_readKey(Eg_cfgEntry* entry)
{
	char* temp;
	SDL_Keycode res;
	int strsize = lex_readString(&temp);
	if(strsize < 0)
		return false;
		
	res = SDL_GetKeyFromName(temp);
	//chaîne vide = pas de binding, chaîne non vide mais touche inconnue = erreur !
	if(res == SDLK_UNKNOWN && strsize > 0)
	{
		fprintf(stderr, "[Config] Error setting key for parameter %s : unknown key : '%s'\n", entry->label, temp);
		free(temp);
		return false;
	}
	
	*(SDL_Keycode*)entry->value = res;
	free(temp);
	return true;
}


const char* cfg_keyToStr(Eg_cfgEntry* entry)
{
	const char* sdlval;
	int size;
	
	sdlval = SDL_GetKeyName(*(SDL_Keycode*)entry->value);
	size = strlen(sdlval)+3;
	free(tempStr);
	//la chaîne à renvoyer doit être bornée de guillemets.
	tempStr = malloc(sizeof(char)*size);
	
	snprintf(tempStr, size, "\"%s\"", sdlval);
	return tempStr;
}

