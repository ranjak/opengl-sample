#ifndef RENDERER_H
#define RENDERER_H

#include "scene.h"
/*
Définit le programme de rendu, ses étapes, spécifie les éléments à dessiner.
*/

/*
* crée le programme de rendu.
* PARAM: scène sur laquelle le renderer opère
* RETOURNE: 0 si succès, -1 si échec
*/
int eg_initRenderer(Scene*);

/*
* efface et redessine la scène.
*/
void eg_render();

/*
* Détruit le programme de rendu.
* RETOURNE: 0 si succès, -1 si échec
*/
int eg_destroyRenderer();

#endif
