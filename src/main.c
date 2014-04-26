#include "scene.h"
#include "config.h"


int main(int argc, char* argv[])
{
	eg_loadConfig();
	Scene* scene = scene_create("Ma scène", 600, 600);
	if(scene == NULL)
	{
		fprintf(stderr, "Erreur : impossible de créer la scène.\n");
		return 1;
	}
	printf("Version : %s\n", glGetString(GL_VERSION));
	//boucle ppale
	scene_runMainLoop(scene);

	//fin du programme
	eg_writeConfig();
	scene_destroy(scene);
	
	return 0;
}

