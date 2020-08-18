/*
* program: Read Grey-Scale Image and create Histogram
* author: Aaditya Arora 
* compile: mpicc -o a.out p2.c 
* run: mpiexec -n 4 ./a.out img1 // where img1 is the name of image to load.
* dependencies: output to output.txt
* tested_on: Mac OS
* version: GCC 10.1.0
*/

#include<stdio.h>
#include<stdlib.h>
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
  int width, height, channels;
  unsigned char* img = NULL;

  if(rank == 0) {
    char* img_name = argc > 1 ? argv[1] : "test.jpg"; 
    img = load_image(img_name, &width, &height, &channels);
  } 
  MPI_Bcast( &width, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast( &height, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast( &channels, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if((width*height)%size) {
    if(rank == 0)
      printf("Error: Number of process should divide %d\n", width*height);
    return 0;
  }
  int send_size = (width*height)/size;
  unsigned char* rec_img = NULL;
  rec_img = (unsigned char*)malloc(sizeof(unsigned char)*send_size);
  MPI_Scatter(img, send_size, MPI_UNSIGNED_CHAR, rec_img, send_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  int *histo = allocate_histo_32();
  histo_vector(rec_img, send_size, rank, width, height, histo);

  int proces_size = size;
  MPI_Status status;

  while(proces_size > 1) {
    int other_rank = proces_size - rank - 1;
    if(rank < proces_size/2) {
      int *other = allocate_histo_32();
      MPI_Recv(other, 32, MPI_INT, other_rank, 5, MPI_COMM_WORLD, &status);
      merge_histogram(histo, other);
      free(other);
    } else if(rank != other_rank && rank < proces_size) {
      MPI_Send(histo, 32, MPI_INT, other_rank, 5, MPI_COMM_WORLD);
      free(histo);
    }
    proces_size = proces_size / 2 + (proces_size % 2 ? 1 : 0);
  }

  if(rank == 0)
    print_to_file("output.txt", histo);

  MPI_Finalize();

  return 0;
}

