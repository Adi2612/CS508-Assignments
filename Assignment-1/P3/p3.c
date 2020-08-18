/*
* program: Read Grey-Scale Image and do computation 
* author: Aaditya Arora 
* run: gcc p3.c -w 
*      ./a.out img1 img2 // where img1 and img2 are name of two images.
* dependencies: output to stdout.
*/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

unsigned char* load_image(char* file_name, int* width, int* height, int* channels) {
  unsigned char *img = stbi_load(file_name, width, height, channels, 0);
  return img;
}

unsigned char** split_image_to_quad(unsigned char* img, int width, int height, int* quad_size) {
  unsigned char** quad = (unsigned char**)malloc(sizeof(unsigned char*)*4);
  for(int i = 0 ; i < 4; i++)
    quad[i] = (unsigned char*)malloc(sizeof(unsigned char)*width*height/4);
  int j=0,k=0,l=0,m=0;
  // load them into different blocks :-- 
  for(int i = 0 ; i < width*height ; i++) {
    int row = i/width;
    int column = i%width;
    if(row < height/2 && column < width/2) {
      quad[0][j++] = img[i];
    }
    if(row < height/2 && column >= width/2 ){
      quad[1][k++] = img[i];
    } 
    if(row >= height/2 && column < width/2) {
      quad[2][l++] = img[i];
    }
    if(row >= height/2 && column >= width/2) {
      quad[3][m++] = img[i];
    }
  }
  quad_size[0] = j;
  quad_size[1] = k;
  quad_size[2] = l;
  quad_size[3] = m;
  return quad;
}

void get_histogram(unsigned char* quad, int* histo, int quad_size) {
  for(int i = 0 ; i < 8 ; i++)
    histo[i] = 0;
  int size = 0;
  for(int i = 0 ; i < quad_size ; i++) {
    histo[((int)quad[i])/32]++;
  }
}

void print_to_file(char* file_name, int** histo) {
  FILE* fo;
  fo = fopen(file_name, "w");
  if(fo == NULL) {
    printf("Error: can not open file :/ \n");
  } else {
    for(int i = 0 ; i < 4 ; i++) {
      for(int j = 0 ; j < 8 ; j++) {
        fprintf(fo, "%d ", histo[i][j]);
      }
      fprintf(fo,"\n");
    }
  }
}

int main(int argc, char const *argv[]){
  int width1, height1, channels1;
  int width2, height2, channels2;
  unsigned char* img1 = NULL;
  unsigned char* img2 = NULL;
  char* img_name1 = argc > 2 ? argv[1] : "test1.jpg"; 
  char* img_name2 = argc > 2 ? argv[2] : "test2.jpg";
  img1 = load_image(img_name1, &width1, &height1, &channels1);
  img2 = load_image(img_name2, &width2, &height2, &channels2); 
  if(img1 == NULL || img2 == NULL) {
    printf("Error: Images does not exist :/\n");
    return 0;
  }
  unsigned char** quad2 = NULL;
  unsigned char** quad1 = NULL;
  int quad_size1[4] = {0,0,0,0};
  int quad_size2[4] = {0,0,0,0};
  quad1 = split_image_to_quad(img1, width1, height1, quad_size1);
  quad2 = split_image_to_quad(img2, width2, height2, quad_size2);

  int* histo1[4] = {NULL, NULL, NULL, NULL};
  int* histo2[4] = {NULL, NULL, NULL, NULL};
  for(int i = 0 ; i < 4 ; i++) {
    histo1[i] = (int*)malloc(sizeof(int)*8);
    histo2[i] = (int*)malloc(sizeof(int)*8);
    get_histogram(quad1[i], histo1[i], quad_size1[i]);
    get_histogram(quad2[i], histo2[i], quad_size2[i]);
  }
  // hellinger distance
  float hd = 0;
  for(int i = 0 ; i < 4 ; i++) {
    for(int j = 0 ; j < 8 ; j++) {
      hd += sqrt((long long)histo2[i][j]*histo1[i][j]);
    }
  }
  printf("HD = %.3f\n", hd/4);
  return 0;
}

