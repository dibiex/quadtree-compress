#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/*
 * Structura pentru retinerea rgb
 * folosit la citirea din matrice
 */ 
typedef struct rgb{

	unsigned char red;
	unsigned char green;
	unsigned char blue;

}rgb_t;

/*
 * Structura arborelui
 */
typedef struct QuadtreeNode {
	
	rgb_t rgb;
	long long index;	
	uint32_t area;

	struct QuadtreeNode *top_left,    *top_right;
	struct QuadtreeNode *bottom_left, *bottom_right;
 
 }__attribute__((packed)) QuadtreeNode;

/*
 * Structura vectorului
 */
typedef struct vQuadtreeNode {
	
	unsigned char blue, green ,red;	
	uint32_t area;

	int32_t top_left,    top_right;
	int32_t bottom_left, bottom_right;

 }__attribute__((packed)) vQuadtreeNode;

/*
 * Functia de citire dintr-un fisier
 * returneaza o matrice de culori
 */
rgb_t ** citire(int * height, int * width, char * fisier)
{
	FILE * f;
	char magic_num[3];
	int color;
	int i;

	f = fopen(fisier, "rb");
	
	fscanf(f, "%s"   ,magic_num);
	fscanf(f, "%d "  ,width);
	fscanf(f, "%d\n" ,height);
	fscanf(f, "%d"   ,&color);
	
	char garbage;
	fread(&garbage,sizeof(char),1,f);

	rgb_t ** r = (rgb_t**) malloc(sizeof(rgb_t*) * (*height));
	
	for(i = 0; i < (*height); i++)
	{	
		r[i] = malloc(sizeof(rgb_t) * (*width));
		fread(r[i], sizeof(rgb_t), (*width), f);
	}

	fclose(f);
	return r;
}

/*
 * Functia de compresie
 */
void compresie(rgb_t ** matrix, QuadtreeNode ** nod, int x, int y, int size, int prag)
{
	int i, j;
	unsigned long long int blue=0, green=0, red=0, mean=0;

	(*nod) = malloc(sizeof(QuadtreeNode));
	(*nod)->area = size*size;

// Calculare rgb mediu pe suprafata actuala

	for(i = y; i < y + size; i++)
		for(j = x; j < x + size; j++)
		{
			blue  = blue  + matrix[i][j].blue;
			green = green + matrix[i][j].green;
			red   = red   + matrix[i][j].red;
		}

	blue  = blue  / (size * size);
	red   = red   / (size * size);
	green = green / (size * size);

	(*nod)->rgb.blue  = blue;
	(*nod)->rgb.red   = red;
	(*nod)->rgb.green = green;

// Calculare scor

	for(i = y; i < y + size; i++)
		for(j = x; j < x + size; j++)
		{	
			mean = mean +((red - matrix[i][j].red)     * (red - matrix[i][j].red)) 
						+((green - matrix[i][j].green) * (green - matrix[i][j].green)) 
						+((blue - matrix[i][j].blue)   * (blue - matrix[i][j].blue));
		}

	mean = mean / (3 * size * size);

// Conditia de parcurgere

	if(mean > prag)
	{
		compresie(matrix, &(*nod)->top_left,     x,            y,            size/2, prag);
		compresie(matrix, &(*nod)->top_right,    x + (size/2), y,            size/2, prag);
		compresie(matrix, &(*nod)->bottom_right, x + (size/2), y + (size/2), size/2, prag);
		compresie(matrix, &(*nod)->bottom_left,  x,            y + (size/2), size/2, prag);
		
		return;
	}
	else
	{
		(*nod)->top_right    = NULL;
		(*nod)->top_left     = NULL;
		(*nod)->bottom_left  = NULL;
		(*nod)->bottom_right = NULL;
		
		return;
	}
}

/*
 * Functie ce copiaza adresele nodurilor 
 * din arbore intr-un vector de pointeri
 */
void parcurgere(QuadtreeNode * nod, QuadtreeNode ** vector[], unsigned int * index)
{
	if(nod != NULL)
	{
        if((*index) > 0)
      		(*vector) = realloc((*vector), sizeof(QuadtreeNode*) * ((*index) + 1));
        
        (*vector)[(*index)] = nod;
        nod->index = (*index);
        (*index)++;

        parcurgere(nod->top_left,     vector, index);
        parcurgere(nod->top_right,    vector, index);
        parcurgere(nod->bottom_right, vector, index);
        parcurgere(nod->bottom_left,  vector, index);

	}
	else
		return;
}

/*
 * Folosind vectorul de la functia de mai sus
 * construieste vectorul ce va fi scris
 */
void copy_to_vector(QuadtreeNode ** vp, vQuadtreeNode ** v, int index)
{
	
	unsigned int i;
	
	for(i=0; i < index; i++)
	{
		(*v)[i].red   = vp[i]->rgb.red;
		(*v)[i].blue  = vp[i]->rgb.blue;
		(*v)[i].green = vp[i]->rgb.green;
		(*v)[i].area  = vp[i]->area;
	
		if(vp[i]->top_left != NULL)	
			(*v)[i].top_left     = vp[i]->top_left->index;
		else
			(*v)[i].top_left     = -1;

		if(vp[i]->top_right != NULL)	
			(*v)[i].top_right    = vp[i]->top_right->index;
		else
			(*v)[i].top_right    = -1;

		if(vp[i]->bottom_right != NULL)	
			(*v)[i].bottom_right = vp[i]->bottom_right->index;
		else
			(*v)[i].bottom_right = -1;

		if(vp[i]->bottom_left != NULL)	
			(*v)[i].bottom_left  = vp[i]->bottom_left->index;
		else
			(*v)[i].bottom_left  = -1;

	}
}

/*
 * Citeste aborele dintr-un vector
 */
void citire_arb(vQuadtreeNode * vec, QuadtreeNode ** nod, int i)
{
	(*nod) = malloc(sizeof(QuadtreeNode));

	(*nod)->rgb.red   = vec[i].red;
	(*nod)->rgb.blue  = vec[i].blue;
	(*nod)->rgb.green = vec[i].green;
	(*nod)->area      = vec[i].area;
	(*nod)->index     = i;

	if(vec[i].top_left != -1 && vec[i].top_right != -1 && vec[i].bottom_left != -1 && vec[i].bottom_right != -1)
	{
		citire_arb(vec, &(*nod)->top_left,     vec[i].top_left);
		citire_arb(vec, &(*nod)->top_right,    vec[i].top_right);
		citire_arb(vec, &(*nod)->bottom_left,  vec[i].bottom_left);
		citire_arb(vec, &(*nod)->bottom_right, vec[i].bottom_right);

	}
	else
	{
		(*nod)->top_left     = NULL;
		(*nod)->top_right    = NULL;
		(*nod)->bottom_left  = NULL;
		(*nod)->bottom_right = NULL;
	}

}

/*
 * Parcurge arborele si construieste pe baza frunzelor
 * matricea de culori
 */
void decompresie(QuadtreeNode * nod, rgb_t *** matrix, int x, int y, int size)
{
	int i, j;

	if(nod->top_left == NULL && nod->top_right == NULL && nod->bottom_right == NULL && nod->bottom_left == NULL)
	{
		for(i = y; i < y + size; i++)
			for(j = x; j < x + size; j++)
			{
				(*matrix)[i][j].red   = nod->rgb.red;
				(*matrix)[i][j].green = nod->rgb.green;
				(*matrix)[i][j].blue  = nod->rgb.blue;
			}
	}
	else
	{
		decompresie(nod->top_left,     matrix, x,          y,          size/2);
		decompresie(nod->top_right,    matrix, x+(size/2), y,          size/2);
		decompresie(nod->bottom_right, matrix, x+(size/2), y+(size/2), size/2);
		decompresie(nod->bottom_left,  matrix, x,          y+(size/2), size/2);
	}

	free(nod);
}

/*
 * Compune fisierul *.ppm pe baza
 * matricei de culori
 */
void scriere_ppm(rgb_t ** mat, char * fisier, int size)
{

	FILE * f = fopen(fisier, "w");
	
	fprintf(f, "P6\n");
	fprintf(f, "%d %d\n", size, size );
	fprintf(f, "255\n");
	
	int i;
	for(i = 0; i < size; i++)
		fwrite(mat[i], sizeof(rgb_t), size, f);
	
	fclose(f);

}
/*
 * Parcurge arborele in adancime 
 * la intoarcere interschimband fii
 */
void orizontala(QuadtreeNode ** nod)
{
	if((*nod)->top_left != NULL && (*nod)->top_left != NULL && (*nod)->top_left != NULL && (*nod)->top_left != NULL)
	{
		orizontala(&(*nod)->top_left);
		orizontala(&(*nod)->top_right);
		orizontala(&(*nod)->bottom_right);
		orizontala(&(*nod)->bottom_left);
		
		QuadtreeNode * aux;

		aux 				 = (*nod)->top_left;
		(*nod)->top_left 	 = (*nod)->bottom_left;
		(*nod)->bottom_left  = aux;
		
		aux					 = (*nod)->top_right;
		(*nod)->top_right	 = (*nod)->bottom_right;
		(*nod)->bottom_right = aux;
	}
}

/*
 * Parcurge arborele in adancime 
 * la intoarcere interschimband fii
 */
void verticala(QuadtreeNode ** nod)
{
	if((*nod)->top_left != NULL && (*nod)->top_left != NULL && (*nod)->top_left != NULL && (*nod)->top_left != NULL)
	{
		verticala(&(*nod)->top_left);
		verticala(&(*nod)->top_right);
		verticala(&(*nod)->bottom_right);
		verticala(&(*nod)->bottom_left);

		QuadtreeNode * aux;

		aux					 = (*nod)->top_left;
		(*nod)->top_left	 = (*nod)->top_right;
		(*nod)->top_right	 = aux;
		
		aux					 = (*nod)->bottom_left;
		(*nod)->bottom_left	 = (*nod)->bottom_right;
		(*nod)->bottom_right = aux;
	}
}

/* 
 * Returneaza minimul dintre a si b
 */
uint32_t min(int a, int b)
{
	return ((a)<(b))?(a):(b);
}

/*
 * Parcurge ambii arbori nod1 si nod2
 * calculand pentru fiecare nod suma algebrica a culorilor.
 * In cazul in care unul dintre arbori a ajuns la o frunza acesta il asteapta
 * pe celalt arbore sa se intoarca
 */
void suprapunere(QuadtreeNode * nod1, QuadtreeNode * nod2, QuadtreeNode ** rez)
{
	(*rez) = malloc(sizeof(QuadtreeNode));
	
	if(nod1->top_left != NULL && nod2->top_left != NULL)
	{	
		suprapunere(nod1->top_left, 	nod2->top_left,		&(*rez)->top_left);
		suprapunere(nod1->top_right,	nod2->top_right,	&(*rez)->top_right);
		suprapunere(nod1->bottom_right, nod2->bottom_right, &(*rez)->bottom_right);
		suprapunere(nod1->bottom_left,	nod2->bottom_left,	&(*rez)->bottom_left);

	}
	else
		if(nod1->top_left == NULL && nod2->top_left != NULL)
		{
		suprapunere(nod1, nod2->top_left,     &(*rez)->top_left);
		suprapunere(nod1, nod2->top_right,    &(*rez)->top_right);
		suprapunere(nod1, nod2->bottom_right, &(*rez)->bottom_right);
		suprapunere(nod1, nod2->bottom_left,  &(*rez)->bottom_left);
		}
		else
		
			if(nod1->top_left != NULL && nod2->top_left == NULL)
			{
				suprapunere(nod1->top_left,     nod2, &(*rez)->top_left);
				suprapunere(nod1->top_right,    nod2, &(*rez)->top_right);
				suprapunere(nod1->bottom_right, nod2, &(*rez)->bottom_right);
				suprapunere(nod1->bottom_left,  nod2, &(*rez)->bottom_left);
			}
			else
				{
					(*rez)->top_right	 = NULL;
					(*rez)->top_left	 = NULL;
					(*rez)->bottom_left  = NULL;
					(*rez)->bottom_right = NULL;
				}
	
	(*rez)->area      = min(nod1->area, nod2->area);
	(*rez)->rgb.blue  = (nod1->rgb.blue  + nod2->rgb.blue)  / 2;
	(*rez)->rgb.red   = (nod1->rgb.red   + nod2->rgb.red)   / 2;
	(*rez)->rgb.green = (nod1->rgb.green + nod2->rgb.green) / 2;

}

/*
 * Dealoca un arbore
 */
void free_arb(QuadtreeNode ** nod)
{
	if((*nod)->top_left != NULL && (*nod)->top_left != NULL && (*nod)->top_left != NULL && (*nod)->top_left != NULL)
	{
		free_arb(&(*nod)->top_left);
		free_arb(&(*nod)->top_right);
		free_arb(&(*nod)->bottom_right);
		free_arb(&(*nod)->bottom_left);
	}
	free((*nod));
}

int main(int argc, char * argv[])
{

	if (strcmp(argv[1], "-c") == 0)
	{
		unsigned int index=0;
		unsigned int i;
		int prag, width, height;
		
		rgb_t **r;
		QuadtreeNode * nod=NULL;

		r = citire(&height, &width, argv[3]);
		prag = atoi(argv[2]);
		compresie(r, &nod, 0, 0, width, prag);

		// Vector de pointeri catre nodurile arborelui
		QuadtreeNode ** vector = malloc(sizeof(QuadtreeNode*));
		parcurgere(nod, &vector, &index);

		vQuadtreeNode * vec=malloc(sizeof(vQuadtreeNode)*index);
		copy_to_vector(vector,&vec,index);

		for(i=0; i < index; i++)
			free(vector[i]);
		free(vector);

		//Scrierea vectorului in fisier
		FILE * f = fopen(argv[4], "wb");

		unsigned int count = 0;
		for(i = 0; i < index; i++)
		{
			if(vec[i].top_left == -1)
				count++;
		}

		fwrite(&count, sizeof(int), 1, f);
		fwrite(&index, sizeof(int), 1, f);

		for(i = 0; i < index; i++)
		{
			fwrite(&vec[i], sizeof(vQuadtreeNode), 1, f);
			if(vec[i].top_left == -1)
					count++;
		}
		free(vec);

		// Dealocare
		for(i=0;i<height;i++)
			free(r[i]);
		free(r);

		fclose(f);

	}
	else 
		if (strcmp(argv[1], "-d") == 0)
		{
			
			unsigned int index,i;
			unsigned int colors;
			
			FILE * f = fopen(argv[2], "rb");
			fread(&colors, sizeof(int), 1, f);
			// Citire numar de noduri
			fread(&index,  sizeof(int), 1, f);			

			vQuadtreeNode * vec = malloc(sizeof(vQuadtreeNode) * index);

			// Citire vectorul de noduri
			fread(vec, sizeof(vQuadtreeNode), index, f);
			fclose(f);

			QuadtreeNode *nod = NULL;
			citire_arb(vec, &nod, 0);
			free(vec);

			uint32_t size = sqrt(nod->area);
			
			// Alocare matrice de culori			
			rgb_t ** mat = (rgb_t**) malloc(sizeof(rgb_t*) * size); 
			for(i = 0; i < size; i++)	
				mat[i] = malloc(sizeof(rgb_t) * size);
			
			decompresie(nod, &mat, 0, 0, size);
			scriere_ppm(mat, argv[3], size);

			// Dealocare
			for(i = 0; i < size; i++)
				free(mat[i]);
			free(mat);

		}
		else
			if (strcmp(argv[1], "-m") == 0)
			{
				unsigned int i;
				int prag, width, height;
				rgb_t ** r;
				QuadtreeNode * nod = NULL;
				
				prag = atoi(argv[3]);
				r = citire(&height, &width, argv[4]);

				compresie(r, &nod, 0, 0, width, prag);
				
				// Verificare tip oglindire
				if(strcmp(argv[2], "v") == 0)
					orizontala(&nod);
				else
					if(strcmp(argv[2], "h") == 0)
						verticala(&nod);
					else
						if(strcmp(argv[2], "n") != 0)
						printf("Wrong type\n");
							
			
				// Alocare matrice de culori			
				rgb_t ** mat = (rgb_t**)malloc(sizeof(rgb_t*) * width);
				for(i = 0; i < width; i++)		
					mat[i] = malloc(sizeof(rgb_t) * width);
			
				decompresie(nod, &mat, 0, 0, width);
				scriere_ppm(mat, argv[5], width);
				
				// Dealocare
				for(i = 0; i < width; i++)
					free(mat[i]);
				free(mat);

				for(i = 0; i < height; i++)
					free(r[i]);
				free(r);
			}
			else
				if (strcmp(argv[1], "-o") == 0)
				{
					unsigned int i;
					int prag, width, height;

					rgb_t ** r1;
					rgb_t ** r2;
					
					QuadtreeNode * nod1=NULL;
					QuadtreeNode * nod2=NULL;

					prag = atoi(argv[2]);

					r1 = citire(&height, &width, argv[3]);
					r2 = citire(&height, &width, argv[4]);

					compresie(r1, &nod1, 0, 0, width, prag);
					compresie(r2, &nod2, 0, 0, width, prag);

					QuadtreeNode * nodsup = NULL;
					suprapunere(nod1, nod2, &nodsup);
 
 					// Alocare matrice de culori			
					rgb_t ** mat = (rgb_t**) malloc(sizeof(rgb_t*) * width);
					for(i = 0; i < width; i++)	
						mat[i] = malloc(sizeof(rgb_t) * width);
					
					decompresie(nodsup, &mat, 0, 0, width);
					scriere_ppm(mat, argv[5], width);
					
					// Dealocari
					free_arb(&nod1);
					free_arb(&nod2);

					for(i = 0; i < width; i++)
						free(mat[i]);
					free(mat);

					for(i = 0; i < height; i++)
						free(r1[i]);
					free(r1);

					for(i = 0; i < height; i++)
						free(r2[i]);  
					free(r2);
				}
}
