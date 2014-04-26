#ifndef CONTROLS_H
#define CONTROLS_H
#include <SDL2/SDL.h>
#include "config.h"

/*Controles au clavier/souris ou autres.
* Liste des actions possibles.
* Gère le chargement des bindings et leur valeur par défaut.
*/


/*
* Liste des actions devant être affectées à une touche.
* EG_KEY_NOACTION est toujours à la fin de l'énumération,
* il représente à la fois l'action nulle
* et le nombre d'actions non-nulles (toutes les valeurs sauf lui).
*/
typedef enum Eg_KeyAction
{
	EG_KEY_CHANGESHAPE,
	EG_KEY_EXIT,
	EG_KEY_MOVELEFT,
	EG_KEY_MOVERIGHT,
	EG_KEY_TGWFRENDER,
	EG_KEY_SCALEUP,
	EG_KEY_SCALEDOWN,
	EG_KEY_ROTZL,
	EG_KEY_ROTZR,
	EG_KEY_NOACTION
}Eg_KeyAction;


/*
* Liste des bindings : associe une touche à chaque action.
* Ce module fournit des valeurs par défaut
* si aucune configuration n'est spécifiée.
*/
extern SDL_Keycode Eg_KeyBindings[];

/*
* Associe une action à une touche.
* PARAM: 
	code SDL de la touche
* RETOURNE: 
	action liée à la touche,
	EG_KEY_NOACTION si touche non liée.
*/
Eg_KeyAction eg_getActionFromKey(SDL_Keycode);

/*
* Change un key binding.
* PARAMS:
	action à rebinder
	touche à associer.
*/
void eg_bindKey(Eg_KeyAction, SDL_Keycode);

/*
* Fournit la structure correspondant à la section 
* de configuration des contrôles.
* PARAM:
	structure configSection vide mais allouée.
*/
void eg_getControlCfgDesc(Eg_configSection* cs);

#endif
