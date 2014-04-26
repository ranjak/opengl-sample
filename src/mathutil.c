#include "mathutil.h"
#include <math.h>
#include <stdio.h>

const Mat4 ZeroMat4 = {{0., 0., 0., 0.,
			0., 0., 0., 0.,
			0., 0., 0., 0.,
			0., 0., 0., 0.}};
			
const Mat4 IdMat4 = { {	1., 0., 0., 0.,
			0., 1., 0., 0.,
			0., 0., 1., 0.,
			0., 0., 0., 1.}};

Mat4 egm_mat4Mult(const Mat4* ml, const Mat4* mr)
{
	Mat4 mres = ZeroMat4;
	
	for(int i=0 ; i<4 ; i++)
	{
		for(int j=0 ; j<16 ; j+=4)
		{
			int idx = i+j;
			for(int k=0 ; k<4 ; k++)
				mres.m[idx] += ml->m[4*k+i] * mr->m[j+k];
		}
	}
	
	return mres;
}

float egm_mat4Det(const Mat4* m)
{
	float det = 0.f;
	const float* mt = m->m;
	
	//développement selon la colonne 0
	//0
	/*déterminant  (5,  9, 13,
			6, 10, 14,
			7, 11, 15)*/
	det = mt[0]*((mt[5]*mt[10]*mt[15] + mt[6]*mt[11]*mt[13] + mt[7]*mt[9]*mt[14]) -
		     (mt[7]*mt[10]*mt[13] + mt[5]*mt[11]*mt[14] + mt[6]*mt[9]*mt[15]));
	//1
	/*déterminant	(4,  8, 12,
			 6, 10, 14,
			 7, 11, 15)*/
	det -= mt[1]*((mt[4]*mt[10]*mt[15] + mt[6]*mt[11]*mt[12] + mt[7]*mt[8]*mt[14]) -
		      (mt[7]*mt[10]*mt[12] + mt[4]*mt[11]*mt[14] + mt[6]*mt[8]*mt[15]));
	//2
	/*déterminant	(4,  8, 12,
			 5,  9, 13,
			 7, 11, 15)*/
	det += mt[2]*((mt[4]*mt[9]*mt[15] + mt[5]*mt[11]*mt[12] + mt[7]*mt[8]*mt[13]) -
		      (mt[7]*mt[9]*mt[12] + mt[4]*mt[11]*mt[13] + mt[5]*mt[8]*mt[15]));
	//3
	/*déterminant	(4,  8, 12,
			 5,  9, 13,
			 6, 10, 14)*/
	det -= mt[3]*((mt[4]*mt[9]*mt[14] + mt[5]*mt[10]*mt[12] + mt[6]*mt[8]*mt[13]) -
		      (mt[6]*mt[9]*mt[12] + mt[4]*mt[10]*mt[13] + mt[5]*mt[8]*mt[14]));
		
	return det;
}

Mat4 egm_mat4Translate(const Mat4* m, float x, float y, float z)
{
	//indices de translation : 12, 13, 14
	Mat4 mt = IdMat4;
	mt.m[12] = x;
	mt.m[13] = y;
	mt.m[14] = z;
	
	return egm_mat4Mult(&mt, m);
}

Mat4 egm_mat4Scale(const Mat4* m, float x, float y, float z)
{
	Mat4 ms = IdMat4;
	ms.m[0]  = x;
	ms.m[5]  = y;
	ms.m[10] = z;
	
	return egm_mat4Mult(&ms, m);
}

Mat4 egm_mat4RotX(const Mat4* m, float angle)
{
	float 	cosx = (float)cos(angle),
		sinx = (float)sin(angle);

	Mat4 mx = IdMat4;
	mx.m[5] = cosx;
	mx.m[6] = sinx;
	mx.m[9] = (-sinx);
	mx.m[10] = cosx;
	
	return egm_mat4Mult(&mx, m);
}

Mat4 egm_mat4RotY(const Mat4* m, float angle)
{
	float 	cosy = (float)cos(angle),
		siny = (float)sin(angle);
		
	Mat4 my = IdMat4;
	my.m[0] = cosy;
	my.m[2] = (-siny);
	my.m[8] = siny;
	my.m[10] = cosy;
	
	return egm_mat4Mult(&my, m);
}

Mat4 egm_mat4RotZ(const Mat4* m, float angle)
{
	float 	cosz = (float)cos(angle),
		sinz = (float)sin(angle);
		
	Mat4 mz = IdMat4;
	mz.m[0] = cosz;
	mz.m[1] = sinz;
	mz.m[4] = (-sinz);
	mz.m[5] = cosz;
	
	return egm_mat4Mult(&mz, m);
}



bool egm_mat4Multold(float ma[], float mb[], float mres[])
{
	if(ma == NULL || mb == NULL || mres == NULL)
	{
		fprintf(stderr, "[Mathutil][mat4Mult] Error : all arguments must point to valid locations.\n");
		return false;
	}
	//mres = ma * mb
	//boucle 1 : ligne
	for(int i=0 ; i<4 ; i++)
	{	
		//boucle 2 : colonne
		for(int j=0 ; j<4 ; j++)
		{
			//boucle 3 : valeur [i, j].
			int resId = 4*j + i;
			mres[resId] = 0.f;
			for(int k=0 ; k<4 ; k++)
				mres[resId] += ma[4*k+i] * mb[4*j+k];
		}
	}
	
	return true;
}


void egm_sampleFunction	(	
				float (*evalf)(void*, float),
				void* fparams,
				float fnIntvStart, float fnIntvEnd,
				size_t count,
				float* result, size_t stride
			)
{
	//pas entre chaque valeur à évaluer
	float step = (fnIntvEnd - fnIntvStart) / (float)count;
	//abscisse actuelle, qui augmentera de "step" à chaque calcul de valeur
	float x = fnIntvStart;
	
	//le stride doit être au moins de la taille d'un float
	if(stride == 0)	stride = sizeof(float);
	if(stride < sizeof(float))
	{
		fprintf(stderr, "[Mathutil][sampleFunction] Error : stride parameter must be at least the size of a float (%ld) or 0 for contiguous values, but given %ld\n", sizeof(float), stride);
		return;
	}
	
	//calcul des valeurs jusqu'à itvEnd-step :
	//du fait de l'imprécision de float, le dernier x pourrait être différent de intvEnd
	for(int i=0 ; i<count-1 ; i++)
	{
		*result = evalf(fparams, x);

		result = (float*)((char*)result + stride);
		x += step;
	}
	//calcul de la dernière valeur
	*result = evalf(fparams, fnIntvEnd);
}


float egm_evalPoly(void* polyStruct, float x)
{
	Egm_polynom* poly = (Egm_polynom*)polyStruct;
	float result = 0.f;
	int exp = poly->deg;
	
	while(exp >= 0)
	{
		result += poly->coefs[exp] * pow(x, exp);
		exp--;
	}
	
	return result;
}

void egm_makePolynom(int deg, float coefs[], int count, float* inValues, float* results, size_t stride)
{
	//sert pour l'évaluation d'une valeur individuelle du polynôme
	int calcExp;
	
	for(int i=0 ; i<count ; i++)
	{
		*results = 0.f;
		calcExp = deg;
		while(calcExp >= 0)
		{
			*results += coefs[calcExp]*pow(*inValues, calcExp);
			calcExp--;
		}
		if(stride > 0)
		{
			results += stride;
			inValues += stride;
		}
		else
		{
			results++;
			inValues++;
		}
	}
}

