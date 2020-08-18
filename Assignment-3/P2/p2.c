/*
* program: Read Grey-Scale Image and create Histogram
* author: Aaditya Arora 
* compile: gcc -fopenmp p2.c -o a.out 
* run: ./a.out img1 // where img1 is the name of image to load.
* dependencies: output to output.txt
* tested_on: Mac OS
* version: GCC 10.1.0
*/

#include<stdio.h>
#include<stdlib.h>
#include <omp.h>   

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

unsigned char* load_image(char* file_name, int* width, int* height, int* channels) {
  unsigned char *img = stbi_load(file_name, width, height, channels, 0);
  return img;
}

int* allocate_histo_32() {
  int *histo = (int*)malloc(sizeof(int)*32);
  for(int i = 0 ; i < 32 ; i++) {
    histo[i] = 0;
  }
  return histo;
}

void print_to_file(char* file_name, int* histo) {
  FILE* fo;
  fo = fopen(file_name, "w");
  if(fo == NULL) {
    printf("Error: can not open file :/ \n");
  } else {
    for(int i = 0 ; i < 4 ; i++) {
      for(int j = 0 ; j < 8 ; j++) {
        fprintf(fo, "%d ", histo[i*8 + j]);
      }
      fprintf(fo,"\n");
    }
  }
}

void histo_vector(unsigned char* img, int width, int height, int *histo) {

  #pragma omp parallel for reduction(+:histo[:32])
  for(int i = 0 ; i < width*height ; i++) {
    int t_i = i;
    int row = t_i/width;
    int column = t_i%width;
    if(row < height/2 && column < width/2) {
      histo[(int)img[i]/32]++;
    }
    if(row < height/2 && column >= width/2 ){
      histo[8 + (int)img[i]/32]++;
    } 
    if(row >= height/2 && column < width/2) {
      histo[16 + (int)img[i]/32]++;
    }
    if(row >= height/2 && column >= width/2) {
      histo[24 + (int)img[i]/32]++;
    }
  }
}

void merge_histogram(int* mine, int* other) {
  for(int i = 0 ; i < 32 ; i++) {
    mine[i] += other[i];
  }
}

int main(int argc, char *argv[]){
  int width, height, channels;
  unsigned char* img = NULL;
  char* img_name = argc > 1 ? argv[1] : "test.jpg"; 
  img = load_image(img_name, &width, &height, &channels);
  if(img == NULL) {
    printf("Error: Image does not exist :/ \n");
    return 0;
  }
  int *histo = allocate_histo_32();
  histo_vector(img, width, height, histo);
  print_to_file("output.txt", histo);
  return 0;
}

