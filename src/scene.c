#include "scene.h"
#include "log.h"
#include "renderer.h"
#include "controls.h"
#include <math.h>


//Calcule les FPS et les affiche dans la console
//le void* est un pointeur vers une scène.
static Uint32 updateFps(Uint32, void*);
//MAJ de la scène (déplacement de l'objet). Prend la durée d'une frame en secondes.
static void scene_update(Scene*, double);

//Effectue une action en fonction de la pression d'une touche.
static void scene_performAction(Eg_KeyAction, Scene*);
//Si une touche est relâchée, arrête l'action lui correspondant.
static void scene_stopAction(Eg_KeyAction, Scene*);

Scene* scene_create(char* wdowName, int width, int height)
{
	SDL_Window* wdow = NULL;
	SDL_GLContext ctxt = NULL;
	
	//Initialisation SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "Erreur SDL : %s\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}
	
	//opengl version 3.3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) < 0)	printf("Erreur : %s\n", SDL_GetError());
	
	//création fenêtre et contexte
	wdow = SDL_CreateWindow(wdowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	ctxt = SDL_GL_CreateContext(wdow);
	
	//Chargement des fonctions OpenGL avec GLEW
	GLenum ginit = glewInit();
	if(ginit != GLEW_OK)
	{
		fprintf(stderr, "Erreur GLEW : %s\n", glewGetErrorString(ginit));
		SDL_DestroyWindow(wdow);
		SDL_Quit();
		return NULL;
	}
	//GLEW provoque une erreur GL, on la consomme ici.
	glGetError();
	
	
	if(ctxt == NULL)
	{
		fprintf(stderr, "Erreur de création du contexte : %s\n", SDL_GetError());
		SDL_DestroyWindow(wdow);
		SDL_Quit();
		return NULL;
	}
	
	//attribution des valeurs à la scène
	Scene* scene = (Scene*)malloc(sizeof(Scene));
	if(scene != NULL)
	{
		scene->wdow = wdow;
		scene->ctxt = ctxt;
		scene->wdowTitle = wdowName;
		scene->open = 1;
		scene->movement = 0.f;
		scene->scale = 0.f;
		scene->rotation = 0.f;
		//chargement de la matrice identité ds la matrice de trans
		scene->transMtx = IdMat4;
		scene->wireframe = false;
	}

	if(eg_initRenderer(scene) < 0)
	{
		scene_destroy(scene);
		return NULL;
	}
	
	return scene;
}

	
void scene_destroy(Scene* scene)
{
	eg_destroyRenderer();
	SDL_GL_DeleteContext(scene->ctxt);
	SDL_DestroyWindow(scene->wdow);
	
	eg_closeAllLogs();
	
	free(scene);
	SDL_Quit();
}

void scene_runMainLoop(Scene* scene)
{
	//calcul du temps de la frame. prev et cur en ms, mais frame en s.
	double prevTime, curTime, frameTime;
	scene->frameCount=0.f;
	//affichage des FPS toutes les 250ms.
	SDL_TimerID fpsCounter = SDL_AddTimer(250, updateFps, scene);
	//valeur initiale du temps
	prevTime = (double)SDL_GetTicks();
	while(scene->open)
	{
		//gestion des événements et du rendu
		scene_handleInput(scene);
		scene_update(scene, frameTime);
		eg_render();
		
		//gestion du temps
		scene->frameCount++;
		curTime = (double)SDL_GetTicks();
		frameTime = (curTime - prevTime)/1000.d;
		prevTime = curTime;
	}
	SDL_RemoveTimer(fpsCounter);
}

void scene_handleInput(Scene* scene)
{
	SDL_Event evt;
	Eg_KeyAction act;
	while(SDL_PollEvent(&evt))
	{
		switch(evt.type)
		{
		case SDL_QUIT:
			scene->open = 0;
			return;
		case SDL_KEYDOWN:
			act = eg_getActionFromKey(evt.key.keysym.sym);
			if(act != EG_KEY_NOACTION && !evt.key.repeat)
				scene_performAction(act, scene);
			break;
		case SDL_KEYUP:
			act = eg_getActionFromKey(evt.key.keysym.sym);
			if(act != EG_KEY_NOACTION)
				scene_stopAction(act, scene);
			break;
		default:
			break;
		}
	}
}

void scene_performAction(Eg_KeyAction act, Scene* scene)
{
	switch(act)
	{
	case EG_KEY_CHANGESHAPE:
		printf("Changement de forme !\n");
		break;
	case EG_KEY_EXIT:
		scene->open = 0;
		break;
	case EG_KEY_MOVELEFT:
		scene->movement -= 0.5;
		break;
	case EG_KEY_MOVERIGHT:
		scene->movement += 0.5;
		break;
	case EG_KEY_TGWFRENDER:
		if(scene->wireframe)
		{
			scene->wireframe = false;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
		{
			scene->wireframe = true;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		break;
	case EG_KEY_SCALEUP:
		scene->scale += 0.3;
		break;
	case EG_KEY_SCALEDOWN:
		scene->scale -= 0.3;
		break;
	case EG_KEY_ROTZL:
		scene->rotation += PI/2.f;
		break;
	case EG_KEY_ROTZR:
		scene->rotation -= PI/2.f;
		break;
	default:
		break;
	}
}

void scene_stopAction(Eg_KeyAction act, Scene* scene)
{
	switch(act)
	{
	case EG_KEY_MOVELEFT:
		scene->movement += 0.5;
		break;
	case EG_KEY_MOVERIGHT:
		scene->movement -= 0.5;
		break;
	case EG_KEY_SCALEUP:
		scene->scale -= 0.3;
		break;
	case EG_KEY_SCALEDOWN:
		scene->scale += 0.3;
		break;
	case EG_KEY_ROTZL:
		scene->rotation -= PI/2.f;
		break;
	case EG_KEY_ROTZR:
		scene->rotation += PI/2.f;
		break;
	default:
		break;
	}
}	

void scene_update(Scene* scene, double dt)
{
	float scaleFactor = 1.0 + scene->scale*dt;
	//changement de la position en X.
	scene->transMtx = egm_mat4Translate(&scene->transMtx, dt*scene->movement, 0., 0.);
	//scaling X, Y, Z
	scene->transMtx = egm_mat4Scale(&scene->transMtx, scaleFactor, scaleFactor, 1.0);
	//rotation
	scene->transMtx = egm_mat4RotZ(&scene->transMtx, dt*scene->rotation);
}

Uint32 updateFps(Uint32 interval, void* scene)
{
	float itvSec = (float)interval / 1000.f;
	Scene* sc = (Scene*)scene;

	char title[SCENE_MAX_TITLE_LG];
	//calcul et affichage du framerate dans la barre de titre.
	snprintf(title, SCENE_MAX_TITLE_LG, "%s | %d FPS", sc->wdowTitle, (int)(sc->frameCount / itvSec));
	SDL_SetWindowTitle(sc->wdow, title);
	//réinitialisation du compteur
	sc->frameCount = 0.f;
	
	return interval;
}


char* getGLerrorString(GLenum errCode)
{
	switch(errCode)
	{
	case GL_NO_ERROR:
		return "Pas d'erreur.";
	case GL_INVALID_ENUM:
		return "Argument énuméré incorrect.";
	case GL_INVALID_VALUE:
		return "Argument numérique incorrect.";
	case GL_INVALID_OPERATION:
		return "Opération illégale dans l'état actuel.";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "Framebuffer Object incomplet.";
	case GL_OUT_OF_MEMORY:
		return "Plus assez de mémoire pour exécuter la commande.";
	case GL_STACK_UNDERFLOW:
		return "L'opération demandée causerait un stack underflow.";
	case GL_STACK_OVERFLOW:
		return "L'opération demandée causerait un stack overflow.";
	default:
		return "Valeur d'erreur invalide.";
	}
}

