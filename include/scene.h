#ifndef SCENEOPENGL_H
#define SCENEOPENGL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#define GL_GLEXT_PROTOTYPES
//#include <GL/glcorearb.h>
#include <GL/glew.h>
#include "SDL.h"
#include "mathutil.h"

#define SCENE_MAX_TITLE_LG 50

/*Structure de scène : contient une fenêtre SDL
 et un contexte opengl.*/
struct Scene
{
	SDL_Window* wdow;
	SDL_GLContext ctxt;
	char* wdowTitle;
	//la fenêtre est-elle ouverte ?
	char open;
	float frameCount;

	double movement;
	float scale;
	float rotation;
	Mat4 transMtx;
	bool wireframe;
};
typedef struct Scene Scene;


/*Création d'une fenêtre SDL et d'un contexte opengl
 PARAMS: nom de la fenetre
 	 largeur
 	 hauteur
 RETOURNE: scène créée, NULL si erreur
*/
Scene* scene_create(char[], int, int);


/*Suppression d'une scène (fenêtre + contexte)
  précédemment créée.
  PARAM: Scène.
*/
void scene_destroy(Scene*);

/*Gestion de l'entrée utilisateur
  PARAM: Scène où recevoir l'info.
*/
void scene_handleInput(Scene*);

void scene_runMainLoop(Scene*);

//message correspondant à un code d'erreur de GL
char* getGLerrorString(GLenum errCode);


#endif
