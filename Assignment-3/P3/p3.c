/*
* program: Read 2 Grey-Scale Image and computing HD
* author: Aaditya Arora 
* compile: gcc -fopenmp -o a.out p3.c 
* run: ./a.out img1 img2 // where img1 and img2 is the name of image to load.
* tested_on: Mac OS
* version: GCC 10.1.0
*/


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
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

int main(int argc, char *argv[]){
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
  int *histo1 = allocate_histo_32();
  int *histo2 = allocate_histo_32();
  int rank;
  #pragma omp parallel num_threads(2)private(rank) 
  {
    rank = omp_get_thread_num();
    if(rank == 0)
      histo_vector(img1, width1, height1, histo1);
    else
      histo_vector(img2, width2, height2, histo2);
  }

  // hellinger distance
  float hd = 0;
  for(int i = 0 ; i < 32 ; i++) {
    hd += sqrt((long long)histo2[i]*histo1[i]);
  }
  printf("HD = %.3f\n", hd/4);

  return 0;
}

