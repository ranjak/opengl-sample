#include "renderer.h"
#include "log.h"
#include "mathutil.h"

/*
* Structure définissant un vertex :
*	Coordonnées X, Y, Z, W
*	Couleur R, G, B, A
*/
typedef struct
{
	GLfloat XYZW[4];
	GLfloat RGBA[4];
} Vertex;


static Scene* target;
static int renderLog;
static GLuint	VAO,
		VBO,
		IBO,
		vtxShader,
		fragShader,
		program,
		mtxUnifLoc;
static GLsizei	vtCount,
		idCount;
		
const GLchar* VertexShader =
{
    "#version 400\n"\
 
    "layout(location=0) in vec3 in_Position;\n"\
    /*"layout(location=1) in vec4 in_Color;\n"\*/
    "uniform mat4 matrix;\n"\
    "out vec4 ex_Color;\n"\
 
    "void main(void)\n"\
    "{\n"\
    "   gl_Position = matrix * vec4(in_Position, 1.0);\n"\
    "   ex_Color = vec4(0.0, 0.5, 1.0, 1.0);\n"\
    "}\n"
};

const GLchar* FragmentShader =
{
    "#version 400\n"\
 
    "in vec4 ex_Color;\n"\
    "out vec4 out_Color;\n"\
 
    "void main(void)\n"\
    "{\n"\
    "   out_Color = ex_Color;\n"\
    "}\n"
};


static GLuint makeShader(GLenum type, const GLchar** source);
static int createProgram();

static int makeRenderData();

int eg_initRenderer(Scene* targetScene)
{
	target = targetScene;
	
	renderLog = eg_createLog("Renderer.log");
	
	if(createProgram() < 0)
	{
		fprintf(stderr, "Renderer : impossible de créer le programme de rendu.\n");
		return -1;
	}
	
	if(makeRenderData() < 0)
	{
		fprintf(stderr, "Renderer : impossible de créer les données de rendu.\n");
		return -1;
	}
	
	eg_strToLog(renderLog, "Renderer initialisé avec succès.");
	return 0;
}

void eg_render()
{
	glUniformMatrix4fv(mtxUnifLoc, 1, GL_FALSE, target->transMtx.m);
	glClear(GL_COLOR_BUFFER_BIT);
	//glDrawArrays(GL_LINE_STRIP, 0, vtCount);
	glDrawElements(GL_TRIANGLE_FAN, idCount, GL_UNSIGNED_BYTE, (GLvoid*)0);
	SDL_GL_SwapWindow(target->wdow);
}

int eg_destroyRenderer()
{
	GLenum errCode;
	glGetError();
	
	//1. suppression des shaders et du programme
	glDeleteShader(vtxShader);
	glDeleteShader(fragShader);
	glUseProgram(0);
	glDeleteProgram(program);
	
	//2. suppression des données de vertices
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &IBO);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);
	
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);
	
	errCode = glGetError();
	if(errCode != GL_NO_ERROR)
	{
		fprintf(stderr, "Erreur destruction du renderer : %s\n", getGLerrorString(errCode));
		return -1;
	}
	
	eg_strToLog(renderLog, "Renderer détruit avec succès");
	return 0;
}

int createProgram()
{
	GLenum errCode;
	glGetError();
	
	program = glCreateProgram();
	
	vtxShader = makeShader(GL_VERTEX_SHADER, &VertexShader);
	fragShader = makeShader(GL_FRAGMENT_SHADER, &FragmentShader);
	
	if(vtxShader < 0 || fragShader < 0)
		return -1;
		
	glAttachShader(program, vtxShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	
	//spécification de la matrice de transformation en uniform du vertex shader
	mtxUnifLoc = glGetUniformLocation(program, "matrix");
	glUseProgram(program);
	glUniformMatrix4fv(mtxUnifLoc, 1, GL_FALSE, target->transMtx.m);
	
	glDetachShader(program, vtxShader);
	glDetachShader(program, fragShader);
	
	errCode = glGetError();
	if(errCode != GL_NO_ERROR)
	{
		fprintf(stderr, "Erreur de création du programme de rendu : %s\n", getGLerrorString(errCode));
		return -1;
	}
	
	eg_strToLog(renderLog, "Programme de rendu créé avec succès.");
	return 0;
}



void mirrorY(Vec3* dest, Vec3* source, int count)
{
	for(int i=0 ; i<count ; i++)
	{
		dest[i].x = -source[count-1-i].x;
		dest[i].y = source[count-1-i].y;
	}
}


int makeRenderData()
{
	//pas entre 2 valeurs = précision du dessin, absc = abscisse actuelle (de 0 à 1)
	float step = 0.3f;
	float absc = 1.f;
	//x^exp, x=1->y=h, x=0->y=c
	int exp = 2, i;
	float coefs[] = {-0.1, 0., -0.3};
	//conteneur des vertices
	Vec3* vertices;
	//nb de vertices qu'on va calculer entre 0 et 1.
	int cvCount = (int)(1.f / step);
	//nb de vertices total = combien de steps entre -1 et 1 + extrémités sup. du pont
	vtCount = cvCount*2 + 3;
	idCount = vtCount+3; //nombre d'indices nécessaires : double passage sur 2 vertices + restart
	GLubyte* indices = malloc(idCount*sizeof(GLubyte));
	vertices = calloc(vtCount, sizeof(Vec3));
	
	vertices[vtCount-2].x = -1.;
	vertices[vtCount-2].y = 0.;
	vertices[vtCount-2].z = 0.;
	vertices[vtCount-1].x = 1.;
	vertices[vtCount-1].y = 0.;
	vertices[vtCount-1].z = 0.;
	indices[0] = vtCount-1;
	
	//génération de la courbe
	for(i=0 ; i<cvCount ; i++)
	{
		indices[i+1] = i;
		vertices[i].x = absc;
		absc -= step;
	}
	egm_makePolynom(exp, coefs, cvCount, &vertices[0].x, &vertices[0].y, 3);
	//ajout du 0 au bout de la courbe
	indices[++i] = cvCount;
	vertices[cvCount].x = 0.;
	vertices[cvCount].y = coefs[0];
	vertices[cvCount].z = 0.;
	indices[++i] = vtCount-2;
	indices[++i] = 255; //max ubyte
	//2e fan
	indices[++i] = vtCount-2;
	int j = 3;
	for(i=i+1 ; i<idCount ; i++)
	{
		indices[i] = vtCount - j;
		j++;
	}
	//miroir de la courbe pour compléter le pont
	mirrorY(&vertices[cvCount+1], vertices, cvCount);
		
	
	GLenum errCode = glGetError();
	const size_t BufferSize = vtCount*sizeof(Vec3);
	/*const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].XYZW);*/
	
	//création du VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	//création du VBO contenant les vertices
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, BufferSize, vertices, GL_STATIC_DRAW);
	free(vertices);
	
	/*Pour chaque sommet, l'attribut coordonnées a l'index 0,
	* et la couleur l'index 1.
	* --------------------------VertexSize---------------------|
	* ----Coordonnées : 4 floats----|----Couleur : 4 floats----|
	*		XYZW			    RGBA
	*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (void*)RgbOffset);
	
	glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	
	//Création de l'IBO qui contient les indices des vertices
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idCount, indices, GL_STATIC_DRAW);
	free(indices);
	glPrimitiveRestartIndex(255);
	glEnable(GL_PRIMITIVE_RESTART);
	
	
	errCode = glGetError();
	if(errCode != GL_NO_ERROR)
	{
		fprintf(stderr, "Erreur dans la spécification des vertices : %s\n", getGLerrorString(errCode));
		return -1;
	}
	
	eg_strToLog(renderLog, "Spécification des données de rendu terminée avec succès.");
	return 0;
}


GLuint makeShader(GLenum type, const GLchar** source)
{
	GLint cmplInfo;
	GLuint shader;
	glGetError();

	//---------------création et compilation du vertex shader-------------------------
	shader = glCreateShader(type);
	//le shader a-t-il été créé ?
	if(shader < 1)
	{
		fprintf(stderr, "Erreur shader : %s\n", getGLerrorString(glGetError()));
		return -1;
	}
	
	glShaderSource(shader, 1, source, NULL);
	glCompileShader(shader);
	//vérification du succès de la compilation
	//DEBUG
		//récup du log s'il existe et envoi dans notre log
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &cmplInfo);
		if(cmplInfo > 0)
		{
			char* shaderLog = malloc(cmplInfo*sizeof(char));
			glGetShaderInfoLog(shader, cmplInfo, NULL, shaderLog);
			eg_strToLog(renderLog, shaderLog);
			free(shaderLog);
		}
		glGetShaderiv(shader, GL_COMPILE_STATUS, &cmplInfo);
		if(cmplInfo == GL_FALSE)
		{
			fprintf(stderr, "Erreur de compilation du shader. Voir log.\n");
			return -1;
		}
	//FINDEBUG
	eg_strToLog(renderLog, "Shader %d compilé avec succès.", shader);
	return shader;
}
	
