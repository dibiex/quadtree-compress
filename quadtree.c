#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/*
 *	RGB struct used for 
 *	reading from a matrix
 */ 
typedef struct rgb{

	unsigned char red;
	unsigned char green;
	unsigned char blue;

}rgb_t;

/*
 *	Tree struct
 */
typedef struct QuadtreeNode {
	
	rgb_t rgb;
	long long index;	
	uint32_t area;

	struct QuadtreeNode *top_left,    *top_right;
	struct QuadtreeNode *bottom_left, *bottom_right;
 
 }__attribute__((packed)) QuadtreeNode;

/*
 *	Vector struct
 */
typedef struct vQuadtreeNode {
	
	unsigned char blue, green ,red;	
	uint32_t area;

	int32_t top_left,    top_right;
	int32_t bottom_left, bottom_right;

 }__attribute__((packed)) vQuadtreeNode;

/*
 *	Function used for reading from a file
 *	returns a rgb matrix
 */
rgb_t ** read(int * height, int * width, char * file)
{
	FILE * f;
	char magic_num[3];
	int color;
	int i;

	f = fopen(file, "rb");
	
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
 *	Compression function
 */
void compression(rgb_t ** matrix, QuadtreeNode ** node, int x, int y, int size, int threshold)
{
	int i, j;
	unsigned long long int blue=0, green=0, red=0, mean=0;

	(*node) = malloc(sizeof(QuadtreeNode));
	(*node)->area = size*size;

// Compute medium rgb on the current area

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

	(*node)->rgb.blue  = blue;
	(*node)->rgb.red   = red;
	(*node)->rgb.green = green;

// Compute score

	for(i = y; i < y + size; i++)
		for(j = x; j < x + size; j++)
		{	
			mean = mean +((red - matrix[i][j].red)     * (red - matrix[i][j].red)) 
						+((green - matrix[i][j].green) * (green - matrix[i][j].green)) 
						+((blue - matrix[i][j].blue)   * (blue - matrix[i][j].blue));
		}

	mean = mean / (3 * size * size);

// Traversal condition

	if(mean > threshold)
	{
		compression(matrix, &(*node)->top_left,     x,            y,            size/2, threshold);
		compression(matrix, &(*node)->top_right,    x + (size/2), y,            size/2, threshold);
		compression(matrix, &(*node)->bottom_right, x + (size/2), y + (size/2), size/2, threshold);
		compression(matrix, &(*node)->bottom_left,  x,            y + (size/2), size/2, threshold);
		
		return;
	}
	else
	{
		(*node)->top_right    = NULL;
		(*node)->top_left     = NULL;
		(*node)->bottom_left  = NULL;
		(*node)->bottom_right = NULL;
		
		return;
	}
}

/*
 * Function used to copy nodes addresses 
 * from the tree into a pointer vector
 */
void traversal(QuadtreeNode * node, QuadtreeNode ** vector[], unsigned int * index)
{
	if(node != NULL)
	{
        if((*index) > 0)
      		(*vector) = realloc((*vector), sizeof(QuadtreeNode*) * ((*index) + 1));
        
        (*vector)[(*index)] = node;
        node->index = (*index);
        (*index)++;

        traversal(node->top_left,     vector, index);
        traversal(node->top_right,    vector, index);
        traversal(node->bottom_right, vector, index);
        traversal(node->bottom_left,  vector, index);

	}
	else
		return;
}

/*
 *	Using the vector from above 
 *	computes the vector that needs to be written
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
 *	Read a tree struct from a vector
 */
void read_tree(vQuadtreeNode * vec, QuadtreeNode ** node, int i)
{
	(*node) = malloc(sizeof(QuadtreeNode));

	(*node)->rgb.red   = vec[i].red;
	(*node)->rgb.blue  = vec[i].blue;
	(*node)->rgb.green = vec[i].green;
	(*node)->area      = vec[i].area;
	(*node)->index     = i;

	if(vec[i].top_left != -1 && vec[i].top_right != -1 && vec[i].bottom_left != -1 && vec[i].bottom_right != -1)
	{
		read_tree(vec, &(*node)->top_left,     vec[i].top_left);
		read_tree(vec, &(*node)->top_right,    vec[i].top_right);
		read_tree(vec, &(*node)->bottom_left,  vec[i].bottom_left);
		read_tree(vec, &(*node)->bottom_right, vec[i].bottom_right);

	}
	else
	{
		(*node)->top_left     = NULL;
		(*node)->top_right    = NULL;
		(*node)->bottom_left  = NULL;
		(*node)->bottom_right = NULL;
	}

}

/*
 *	Traverses tree and computes based on leafes
 *	the rgb matrix
 */
void decompression(QuadtreeNode * node, rgb_t *** matrix, int x, int y, int size)
{
	int i, j;

	if(node->top_left == NULL && node->top_right == NULL && node->bottom_right == NULL && node->bottom_left == NULL)
	{
		for(i = y; i < y + size; i++)
			for(j = x; j < x + size; j++)
			{
				(*matrix)[i][j].red   = node->rgb.red;
				(*matrix)[i][j].green = node->rgb.green;
				(*matrix)[i][j].blue  = node->rgb.blue;
			}
	}
	else
	{
		decompression(node->top_left,     matrix, x,          y,          size/2);
		decompression(node->top_right,    matrix, x+(size/2), y,          size/2);
		decompression(node->bottom_right, matrix, x+(size/2), y+(size/2), size/2);
		decompression(node->bottom_left,  matrix, x,          y+(size/2), size/2);
	}

	free(node);
}

/*
 *	Computes *.ppm file based on
 *	the rgb matrix
 */
void write_ppm(rgb_t ** mat, char * file, int size)
{

	FILE * f = fopen(file, "w");
	
	fprintf(f, "P6\n");
	fprintf(f, "%d %d\n", size, size );
	fprintf(f, "255\n");
	
	int i;
	for(i = 0; i < size; i++)
		fwrite(mat[i], sizeof(rgb_t), size, f);
	
	fclose(f);

}
/*
 *	Reverse tree horizontally
 */
void horizontal(QuadtreeNode ** node)
{
	if((*node)->top_left != NULL && (*node)->top_left != NULL && (*node)->top_left != NULL && (*node)->top_left != NULL)
	{
		horizontal(&(*node)->top_left);
		horizontal(&(*node)->top_right);
		horizontal(&(*node)->bottom_right);
		horizontal(&(*node)->bottom_left);
		
		QuadtreeNode * aux;

		aux 				 = (*node)->top_left;
		(*node)->top_left 	 = (*node)->bottom_left;
		(*node)->bottom_left  = aux;
		
		aux					 = (*node)->top_right;
		(*node)->top_right	 = (*node)->bottom_right;
		(*node)->bottom_right = aux;
	}
}

/*
 *	Reverse tree vertically 
 */
void vertical(QuadtreeNode ** node)
{
	if((*node)->top_left != NULL && (*node)->top_left != NULL && (*node)->top_left != NULL && (*node)->top_left != NULL)
	{
		vertical(&(*node)->top_left);
		vertical(&(*node)->top_right);
		vertical(&(*node)->bottom_right);
		vertical(&(*node)->bottom_left);

		QuadtreeNode * aux;

		aux					 = (*node)->top_left;
		(*node)->top_left	 = (*node)->top_right;
		(*node)->top_right	 = aux;
		
		aux					 = (*node)->bottom_left;
		(*node)->bottom_left	 = (*node)->bottom_right;
		(*node)->bottom_right = aux;
	}
}

/* 
 *	Return minimum
 */
uint32_t min(int a, int b)
{
	return ((a)<(b))?(a):(b);
}

/*
 * 	Traverses both trees node1 and node2
 * 	computing for each node the average of the colours
 * 	In case a tree has reached a leaf it waits for 
 * 	the other one to return
 */
void overlay(QuadtreeNode * node1, QuadtreeNode * node2, QuadtreeNode ** rez)
{
	(*rez) = malloc(sizeof(QuadtreeNode));
	
	if(node1->top_left != NULL && node2->top_left != NULL)
	{	
		overlay(node1->top_left, 	node2->top_left,		&(*rez)->top_left);
		overlay(node1->top_right,	node2->top_right,	&(*rez)->top_right);
		overlay(node1->bottom_right, node2->bottom_right, &(*rez)->bottom_right);
		overlay(node1->bottom_left,	node2->bottom_left,	&(*rez)->bottom_left);

	}
	else
		if(node1->top_left == NULL && node2->top_left != NULL)
		{
		overlay(node1, node2->top_left,     &(*rez)->top_left);
		overlay(node1, node2->top_right,    &(*rez)->top_right);
		overlay(node1, node2->bottom_right, &(*rez)->bottom_right);
		overlay(node1, node2->bottom_left,  &(*rez)->bottom_left);
		}
		else
		
			if(node1->top_left != NULL && node2->top_left == NULL)
			{
				overlay(node1->top_left,     node2, &(*rez)->top_left);
				overlay(node1->top_right,    node2, &(*rez)->top_right);
				overlay(node1->bottom_right, node2, &(*rez)->bottom_right);
				overlay(node1->bottom_left,  node2, &(*rez)->bottom_left);
			}
			else
				{
					(*rez)->top_right	 = NULL;
					(*rez)->top_left	 = NULL;
					(*rez)->bottom_left  = NULL;
					(*rez)->bottom_right = NULL;
				}
	
	(*rez)->area      = min(node1->area, node2->area);
	(*rez)->rgb.blue  = (node1->rgb.blue  + node2->rgb.blue)  / 2;
	(*rez)->rgb.red   = (node1->rgb.red   + node2->rgb.red)   / 2;
	(*rez)->rgb.green = (node1->rgb.green + node2->rgb.green) / 2;

}

/*
 *	Free tree from memory
 */
void free_tree(QuadtreeNode ** node)
{
	if((*node)->top_left != NULL && (*node)->top_left != NULL && (*node)->top_left != NULL && (*node)->top_left != NULL)
	{
		free_tree(&(*node)->top_left);
		free_tree(&(*node)->top_right);
		free_tree(&(*node)->bottom_right);
		free_tree(&(*node)->bottom_left);
	}
	free((*node));
}

int main(int argc, char * argv[])
{

	if (strcmp(argv[1], "-c") == 0)
	{
		unsigned int index=0;
		unsigned int i;
		int threshold, width, height;
		
		rgb_t **r;
		QuadtreeNode * node=NULL;

		r = read(&height, &width, argv[3]);
		threshold = atoi(argv[2]);
		compression(r, &node, 0, 0, width, threshold);

		// Vector of pointers to the tree nodes
		QuadtreeNode ** vector = malloc(sizeof(QuadtreeNode*));
		traversal(node, &vector, &index);

		vQuadtreeNode * vec=malloc(sizeof(vQuadtreeNode)*index);
		copy_to_vector(vector,&vec,index);

		for(i=0; i < index; i++)
			free(vector[i]);
		free(vector);

		// Write vector in a file
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

		// Free memory
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
			// Read number of nodes
			fread(&index,  sizeof(int), 1, f);			

			vQuadtreeNode * vec = malloc(sizeof(vQuadtreeNode) * index);

			// Read nodes vector
			fread(vec, sizeof(vQuadtreeNode), index, f);
			fclose(f);

			QuadtreeNode *node = NULL;
			read_tree(vec, &node, 0);
			free(vec);

			uint32_t size = sqrt(node->area);
			
			// Alloc rgb matrix						
			rgb_t ** mat = (rgb_t**) malloc(sizeof(rgb_t*) * size); 
			for(i = 0; i < size; i++)	
				mat[i] = malloc(sizeof(rgb_t) * size);
			
			decompression(node, &mat, 0, 0, size);
			write_ppm(mat, argv[3], size);

			// Free memory
			for(i = 0; i < size; i++)
				free(mat[i]);
			free(mat);

		}
		else
			if (strcmp(argv[1], "-m") == 0)
			{
				unsigned int i;
				int threshold, width, height;
				rgb_t ** r;
				QuadtreeNode * node = NULL;
				
				threshold = atoi(argv[3]);
				r = read(&height, &width, argv[4]);

				compression(r, &node, 0, 0, width, threshold);
				
				// Check mirror type
				if(strcmp(argv[2], "v") == 0)
					horizontal(&node);
				else
					if(strcmp(argv[2], "h") == 0)
						vertical(&node);
					else
						if(strcmp(argv[2], "n") != 0)
						printf("Wrong type\n");
							
			
				// Alloc rgb matrix			
				rgb_t ** mat = (rgb_t**)malloc(sizeof(rgb_t*) * width);
				for(i = 0; i < width; i++)		
					mat[i] = malloc(sizeof(rgb_t) * width);
			
				decompression(node, &mat, 0, 0, width);
				write_ppm(mat, argv[5], width);
				
				// Free
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
					int threshold, width, height;

					rgb_t ** r1;
					rgb_t ** r2;
					
					QuadtreeNode * node1=NULL;
					QuadtreeNode * node2=NULL;

					threshold = atoi(argv[2]);

					r1 = read(&height, &width, argv[3]);
					r2 = read(&height, &width, argv[4]);

					compression(r1, &node1, 0, 0, width, threshold);
					compression(r2, &node2, 0, 0, width, threshold);

					QuadtreeNode * node_overlay = NULL;
					overlay(node1, node2, &node_overlay);
 
 					// Alloc rgb matrix			
					rgb_t ** mat = (rgb_t**) malloc(sizeof(rgb_t*) * width);
					for(i = 0; i < width; i++)	
						mat[i] = malloc(sizeof(rgb_t) * width);
					
					decompression(node_overlay, &mat, 0, 0, width);
					write_ppm(mat, argv[5], width);
					
					// Free
					free_tree(&node1);
					free_tree(&node2);

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
