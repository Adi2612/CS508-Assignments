/*
* program: Read Grey-Scale Image and create Histogram
* author: Aaditya Arora 
* run : gcc -w p2.c 
*       ./a.out img1 // where img1 is the name of image to load.
* dependencies: output to output.txt
*/

#include<stdio.h>
#include<stdlib.h>

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
  int width, height, channels;
  unsigned char* img = NULL;
  char* img_name = argc > 1 ? argv[1] : "test.jpg"; 
  img = load_image(img_name, &width, &height, &channels);
  if(img == NULL) {
    printf("Error: Image does not exist :/ \n");
    return 0;
  }
  unsigned char** quad = NULL;
  int quad_size[4] = {0,0,0,0};
  quad = split_image_to_quad(img, width, height, quad_size);
  int* histo[4] = {NULL, NULL, NULL, NULL};
  for(int i = 0 ; i < 4 ; i++) {
    histo[i] = (int*)malloc(sizeof(int)*8);
    get_histogram(quad[i], histo[i], quad_size[i]);
  }
  print_to_file("output.txt", histo);
  return 0;
}

