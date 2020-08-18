/*
* program: Selection Sort O(n^2)
* author: Aaditya Arora 
* compile : gcc -fopenmp p1.c -o a.out
* exec : ./a.out 
* dependencies : read from random.txt
*                output to output.txt
*/

#include<stdio.h>
#include<stdlib.h>
#include <omp.h>  

struct comp_ { int val; int index; };

#pragma omp declare reduction(minimum: struct comp_ : omp_out  = omp_in.val < omp_out.val ? omp_in : omp_out) initializer(omp_priv=omp_orig)

#define MAX_SIZE 100000
#define OUTER_THREAD_COUNT 4
void _swap(int* a, int* b) {
  int c = *a;
  *a = *b;
  *b = c;
}

// merge for concurrent processors
void _merge(int* arr, int a1, int b1, int a2, int b2) {
    int* l;
    int* r;
    int size_l; 
    int size_r;
    size_l = b1-a1;
    size_r = b2-a2;

    l = (int *)malloc(sizeof(int) * (size_l));
    r = (int *)malloc(sizeof(int) * (size_r));
    int z = 0;
    for(int i = a1 ; i < b1 ; i++){
      l[z] = arr[i];
      z++;
    }
    z = 0;
    for(int i = a2 ; i < b2 ; i++){
      r[z] = arr[i];
      z++;
    }
    

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
    z = 0;
    for(int i = a1 ; i < b1 ; i++) {
      arr[i] = array[z];
      z++;
    }
    for(int i = a2 ; i < b2 ; i++) {
      arr[i] = array[z];
      z++;
    }
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
  for(int i = start ; i < end ; i++) {
    struct comp_ minn;
    minn.val = arr[i];
    minn.index = i;
    #pragma omp parallel for reduction(minimum:minn)
    for(int j = i+1 ; j < end ; j++) {
      if(arr[j] < minn.val) {
        minn.index = j;
        minn.val = arr[j];
      }
    }
    _swap(&arr[i], &arr[minn.index]);
  }
}

int main(int argc, char const *argv[]) {
  int* arr = NULL;
  int size = 0;
  int start = 0;
  arr = load_arr_from_file("random.txt", &size);
  if(size % OUTER_THREAD_COUNT) {
    printf("Error: Outer thread size should divide array size :/");
    return 0;
  }
  int split = size/OUTER_THREAD_COUNT;
  int rank;
  #pragma omp parallel for num_threads(OUTER_THREAD_COUNT) private(rank)
  for(int i = 0 ; i < OUTER_THREAD_COUNT ; i++) {
    rank = omp_get_thread_num();
    selection_sort(arr, split*rank, split*(rank+1)); // end is not included     
  } 
  int loop = OUTER_THREAD_COUNT;
  while(loop > 1) {
    #pragma opm parallel for num_threads(loop/2)
    for(int i = 0 ; i < loop ; i+=2) {
      int a1 = split*i;
      int b1 = split*(i+1);
      int a2 = split*(i+1);
      int b2 = split*(i + 2);
      _merge(arr, a1,b1,a2,b2);
    }
    loop /= 2;
    split*= 2;
  }


  write_arr_to_file("output.txt", arr, size);
  return 0;
}
