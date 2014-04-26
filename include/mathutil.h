#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <stddef.h>
#include <stdbool.h>

#define PI 3.14159265

/*Utilitaires de calcul pour maths analytiques (fonctions)*/

/*
* Vecteur à 3 composantes flottantes
*/
typedef struct Vec3
{
	float x, y, z;
} Vec3;

typedef struct Vec4
{
	float v[4];
} Vec4;

typedef struct Mat4
{
	float m[16];
} Mat4;

/*Matrice nulle (pleine de 0)*/
extern const Mat4 ZeroMat4;

/*Matrice identité*/
extern const Mat4 IdMat4;

/*
* Produit matriciel de matrices carrées 4.
* PARAMS:
	matrices à multiplier (gauche et droite)
* RETOURNE:
	matrice résultat
*/
Mat4 egm_mat4Mult(const Mat4* ml, const Mat4* mr);

/*
* Déterminant d'une matrice 4.
* PARAM:
	matrice dont calculer le déterminant
* RETOURNE:
	déterminant de la matrice.
*/
float egm_mat4Det(const Mat4* m);

/*
* Appliquer une translation sur une matrice.
* PARAMS:
	matrice sur laquelle appliquer la translation.
	3 composantes de la translation.
* RETOURNE:
	Nouvelle matrice correspondant à celle passée translatée.
*/
Mat4 egm_mat4Translate(const Mat4* m, float x, float y, float z);

/*
* Appliquer une échelle sur une matrice.
* PARAMS:
	matrice à scaler
	3 composantes d'échelle.
* RETOURNE:
	matrice dupliquée scalée.
*/
Mat4 egm_mat4Scale(const Mat4* m, float x, float y, float z);

/*
* Applique une rotation à une matrice.
* PARAMS:
	matrice à faire pivoter
	angle de rotation
* RETOURNE:
	nouvelle matrice = rotation * m.
*/
Mat4 egm_mat4RotX(const Mat4* m, float angle);

Mat4 egm_mat4RotY(const Mat4* m, float angle);

Mat4 egm_mat4RotZ(const Mat4* m, float angle);

/*
* Multiplier 2 matrices carrées 4x4 de floats.
* En ordre par colonne.
* PARAMS:
	matrice gauche
	matrice droite
	matrice pour contenir gauche*droite
	(chacune un tableau de 16 floats alloué par l'appelant)
* RETOURNE:
	true si OK,
	false si échec (un paramètre NULL)
	
*/
bool egm_mat4Multold(float ma[], float mb[], float mres[]);


/*
* Polynôme : décrit par un degré et des coefficients réels.
* Le nombre de coefficients doit être égal à deg+1.
* Ils sont rangés par exposant croissant.
* Ex : 2x² + 3x - 5 :
	deg = 2
	coefs = {-5, 3, 2}
*/
typedef struct polynom
{
	int deg;
	float coefs[];
} Egm_polynom;


/*
* Calculer "count" valeurs de la fonction "evalf" sur l'intervalle [fnIntvStart, fnIntvEnd],
* stocker ces valeurs dans "results", avec un espacement de "stride" octets entre chaque valeur.
* PARAMS:
	Fonction à évaluer. Le void* représente des paramètres spécifiques à cette fonction,
	ex. des coefficients pour un polynôme.
	Paramètres spécifiques en question, qui seront passés à la fonction.
	Bornes inférieure et supérieure (inclues) de l'intervalle de calcul.
	Nombre de valeurs à calculer dans l'intervalle, le pas de calcul est fixé en fonction.
	Tableau dans lequel seront stocké les "count" valeurs résultantes, il doit pouvoir contenir ces valeurs.
	Nombres d'octets à sauter dans le tableau entre chaque valeur (si <=4, valeurs contigües).
*/
void egm_sampleFunction	(	
				float (*evalf)(void*, float),
				void* fparams,
				float fnIntvStart, float fnIntvEnd,
				size_t count,
				float* result, size_t stride
			);
			

/*
* Calcule la valeur polyStruct(x).
* PARAMS:
	Structure de type Egm_polynom
	Valeur dont l'image doit être évaluée
* RETOURNE:
	Image de x.
*/
float egm_evalPoly(void* polyStruct, float x);

/*calcule les images de count valeurs par le poly spécifié par son degré et ses coefs.
results doit être déjà alloué. stride marche à la fois pour invalues et results.*/
void egm_makePolynom(int deg, float coefs[], int count, float* inValues, float* results, size_t stride);

#endif

