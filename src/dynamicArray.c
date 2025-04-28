#include "dynamicArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct dynamicArray init_array(size_t capacity){
    struct dynamicArray tab;
    tab.arr = (char**) malloc(capacity*sizeof(char[1024]));
    if(tab.arr==NULL){
        perror("erreur malloc");
        exit(1);
    }
    else{
        tab.capacity = capacity;
        tab.size=0;
    }
    return tab; 
}

void add_capacity(struct dynamicArray *Array){
    char **NewTab = (char**) malloc(2*Array->capacity*sizeof(char[1024]));
    if(NewTab == NULL){
        perror("erreur malloc");
        exit(1);
    }
    else{
        memcpy(NewTab, Array->arr, Array->size*sizeof(char));
        free(Array->arr);
        Array->arr = NewTab;
        Array->capacity = 2*Array->capacity;
        printf("Capacite augmentée !! \n");
    }
}

void add_element(struct dynamicArray *Array,char* element ){
    if(Array->capacity == Array->size){
        add_capacity(Array);
    }
    Array->arr[Array->size] = element;
    Array->size ++;
}

void pop_element(struct dynamicArray *Array, int index){
    if(index> Array->size){
        perror("Index out of range");
    }
    else{
       for(size_t i=index;i<Array->size-1;i++){
        Array->arr[i]==Array->arr[i+1];
       }
       Array->arr[Array->size] = '\0';
       Array->size --;
    }
}

void print_elements(struct dynamicArray *Array){
    printf("Size : %zu\n", Array->size);
    printf("Elements :\n");
    for(int i =0;i<Array->size;i++){
        printf("%c\n", Array->arr[i]);
    }
}