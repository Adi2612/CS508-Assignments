/*
* program: Selection Sort O(n^2)
* author: Aaditya Arora 
* run : gcc -w p1.c
*       ./a.out 
* dependencies : read from random.txt
*                output to output.txt
*/

#include<stdio.h>
#include<stdlib.h>

#define MAX_SIZE 100000

void _swap(int* a, int* b) {
  int c = *a;
  *a = *b;
  *b = c;
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

int main(int argc, char const *argv[]) {
  int* arr = NULL;
  int size = 0;
  int start = 0;
  arr = load_arr_from_file("random.txt", &size);
  selection_sort(arr, 0, size); // end is not included 
  write_arr_to_file("output.txt", arr, size);
  return 0;
}
