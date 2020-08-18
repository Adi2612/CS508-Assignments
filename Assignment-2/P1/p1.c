/*
* program: Selection Sort O(n^2)
* author: Aaditya Arora 
* tested-on : Mac OS 
* compile : mpicc -o a.out p1.c
* run: mpiexec -n 4 ./a.out 
* dependencies : read from random.txt
*                output to output.txt
* tested On : Mac OS
* GCC V: 10.1.0
*/

#include<stdio.h>
#include<stdlib.h>
#include <mpi.h>

#define MAX_SIZE 100000

void _swap(int* a, int* b) {
  int c = *a;
  *a = *b;
  *b = c;
}

// merge for concurrent processors
int * _mpi_merge(int * l, int * r, int size_l, int size_r) {
    int i, j, k;
    i = 0;
    j = 0;
    k = 0;
    int * array = (int *)malloc(sizeof(int) * (size_l + size_r));
    while (i < size_l && j < size_r) {
        if (l[i] > r[j]){
            array[k] = r[j];
            k++;
            j++;
        } else {
            array[k] = l[i];
            k++;
            i++;
        }
    }
    while (i < size_l) {
        array[k] = l[i];
        k++;
        i++;
    }
    while (j < size_r) {
        array[k] = r[j];
        k++;
        j++;
    }
    return array;
}


int* load_arr_from_file(char* file_name, int* size) {
  FILE* fi;
  int value,i;
  int* arr = (int*)malloc(sizeof(int)* MAX_SIZE);
  fi = fopen(file_name, "r");
  if(fi == NULL) {
    printf("Error: can not open file :/ \n");
  } else {
    i = 0;
    while (fscanf(fi,"%d", &value) != EOF) {
      arr[i] = value;
      i++;
    }
    *size = i;
  }
  fclose(fi);
  return arr;
}

void write_arr_to_file(char* file_name, int* arr, int size) {
  FILE* fo;
  fo = fopen(file_name, "w");
  if(fo == NULL) {
    printf("Error: can not open file :/ \n");
  } else {
    for(int i = 0 ; i < size ; i++) {
      fprintf(fo, "%d\n", arr[i]);
    }
  }
  fclose(fo);
}

void selection_sort(int* arr, int start, int end) {
  int min_index;
  for(int i = start ; i < end ; i++) {
    min_index = i;
    for(int j = i+1 ; j < end ; j++) {
      if(arr[j] < arr[min_index])
        min_index = j;
    }
    _swap(&arr[i], &arr[min_index]);
  }
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank( MPI_COMM_WORLD , &rank);
  MPI_Comm_size( MPI_COMM_WORLD, &size);

  int* arr = NULL;
  int *rec_arr = NULL;
  int arr_size = 0;
  int start = 0;
  int send_count;
  if(rank == 0) {
    arr = load_arr_from_file("random.txt", &arr_size);
  }
  MPI_Bcast( &arr_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  send_count = arr_size / size;
  if(arr_size % size ) {
    if(rank == 0)
      printf("Error: Number of proceses should be multiple of %d\n", arr_size);
    return 0;
  }
  rec_arr = (int*)malloc(sizeof(int)*send_count);
  MPI_Scatter( arr, send_count, MPI_INT, rec_arr, send_count, MPI_INT, 0, MPI_COMM_WORLD);

  selection_sort(rec_arr, 0, send_count); // end is not included
  // do parallel merging
  
  int merge_size = size;
  MPI_Status status1;
  MPI_Status status2;
  while(merge_size > 1) {
    int other_rank = merge_size - rank - 1;

    if(rank < merge_size/2) {
      int *other_array = NULL;
      int *my_array = rec_arr;
      int other_size;

      MPI_Recv(&other_size, 1, MPI_INT, other_rank, 5, MPI_COMM_WORLD, &status1);
      other_array = (int*)malloc(sizeof(int)*other_size);
      MPI_Recv( other_array, other_size, MPI_INT, other_rank, 7, MPI_COMM_WORLD , &status2);

      rec_arr = _mpi_merge(other_array, my_array, send_count, other_size);
      send_count = send_count + other_size;
      free(other_array);

      free(my_array);
    } else if(other_rank != rank && rank < merge_size) { // in case of merge_size is odd
      // send size of array 

      MPI_Send( &send_count, 1, MPI_INT, other_rank, 5, MPI_COMM_WORLD);
      MPI_Send( rec_arr, send_count, MPI_INT, other_rank, 7, MPI_COMM_WORLD);
      free(rec_arr);
    }
    merge_size = merge_size / 2 + (merge_size % 2 ? 1 : 0);
  }
  if(rank == 0) {
    write_arr_to_file("output.txt", rec_arr, arr_size);
  }
  MPI_Finalize();
  return 0;
}
