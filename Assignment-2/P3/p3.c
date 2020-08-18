/*
* program: Read 2 Grey-Scale Image and computing HD
* author: Aaditya Arora 
* compile: mpicc -o a.out p3.c 
* run: mpiexec -n 4 ./a.out img1 img2 // where img1 and img2 is the name of image to load.
* tested_on: Mac OS
* version: GCC 10.1.0
*/


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include <mpi.h>


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

void histo_vector(unsigned char* img, int size, int rank, int width, int height, int *histo) {

  for(int i = 0 ; i < size ; i++) {
    int t_i = size*rank + i;
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
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank( MPI_COMM_WORLD , &rank);
  MPI_Comm_size( MPI_COMM_WORLD, &size);
  int conf_1[3]; // 0 : width, 1: height, 2: channels;
  int conf_2[3];
  unsigned char* img_1 = NULL;
  unsigned char* img_2 = NULL;

  if(rank == 0) {
    char* img_name1 = argc > 1 ? argv[1] : "test1.jpg";
    char* img_name2 = argc > 1 ? argv[2] : "test2.jpg";
    img_1 = load_image(img_name1, &conf_1[0], &conf_1[1], &conf_1[2]);
    img_2 = load_image(img_name2, &conf_2[0], &conf_2[1], &conf_2[2]);
  } 
  MPI_Bcast(conf_1, 3, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(conf_2, 3, MPI_INT, 0, MPI_COMM_WORLD);
  if((conf_1[0]*conf_1[1])%size || (conf_2[0]*conf_2[1])%size ) {
    if(rank == 0)
      printf("Error: Number of process should divide %d and %d\n", conf_1[0]*conf_1[1] , conf_2[0]*conf_2[1]);
    return 0;
  }
  int send_size1 = (conf_1[0]*conf_1[1])/size;
  int send_size2 = (conf_2[0]*conf_2[1])/size;

  unsigned char* rec_img1 = NULL;
  unsigned char* rec_img2 = NULL;
  rec_img1 = (unsigned char*)malloc(sizeof(unsigned char)*send_size1);
  rec_img2 = (unsigned char*)malloc(sizeof(unsigned char)*send_size2);
  
  MPI_Scatter(img_1, send_size1, MPI_UNSIGNED_CHAR, rec_img1, send_size1, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  MPI_Scatter(img_2, send_size2, MPI_UNSIGNED_CHAR, rec_img2, send_size2, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  int *histo1 = allocate_histo_32();
  int *histo2 = allocate_histo_32();
  histo_vector(rec_img1, send_size1, rank, conf_1[0], conf_1[1], histo1);
  histo_vector(rec_img2, send_size2, rank, conf_2[0], conf_2[1], histo2);

  int proces_size = size;
  MPI_Status status;

  while(proces_size > 1) {
    int other_rank = proces_size - rank - 1;
    if(rank < proces_size/2) {
      int *other = allocate_histo_32();
      MPI_Recv(other, 32, MPI_INT, other_rank, 5, MPI_COMM_WORLD, &status);
      MPI_Send(histo2, 32, MPI_INT, other_rank, 7, MPI_COMM_WORLD);
      merge_histogram(histo1, other);
      MPI_Recv(histo2, 32, MPI_INT, other_rank, 9, MPI_COMM_WORLD, &status);
      free(other);
    } else if(rank != other_rank && rank < proces_size) {
      int *other = allocate_histo_32();
      MPI_Send(histo1, 32, MPI_INT, other_rank, 5, MPI_COMM_WORLD);
      MPI_Recv(other, 32, MPI_INT, other_rank, 7, MPI_COMM_WORLD, &status);
      merge_histogram(histo2, other);
      MPI_Send(histo2, 32, MPI_INT, other_rank, 9, MPI_COMM_WORLD);
      free(other);
      free(histo2);
      free(histo1);
    }
    proces_size = proces_size / 2 + (proces_size % 2 ? 1 : 0);
  }

  if(rank == 0) {
    float hd = 0;
    for(int i = 0 ; i < 32 ; i++) {
      hd += sqrt((long long)histo2[i]*histo1[i]);
    }
    printf("HD = %.3f\n", hd/4);
  }
    
  MPI_Finalize();

  return 0;
}

