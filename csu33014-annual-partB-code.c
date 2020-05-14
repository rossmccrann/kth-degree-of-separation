//
// CSU33014 Summer 2020 Additional Assignment
// Part B of a two-part assignment
//
// Please write your solution in this file
#include <pthread.h> 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "csu33014-annual-partB-person.h"

void find_reachable_recursive(struct person * current, int steps_remaining,
            bool * reachable) {
  // mark current root person as reachable
  reachable[person_get_index(current)] = true;
  // now deal with this person's acquaintances
  if ( steps_remaining > 0 ) {
    int num_known = person_get_num_known(current);
    for ( int i = 0; i < num_known; i++ ) {
      struct person * acquaintance = person_get_acquaintance(current, i);
      find_reachable_recursive(acquaintance, steps_remaining-1, reachable);
    }
  }
}

void lr_reachable_recursive(struct person * current, int steps_remaining, int * reachable, int depth){

  reachable[person_get_index(current)] = depth - steps_remaining;
  if(steps_remaining > 0)
  {
    int num_known = person_get_num_known(current);
    for (int i = 0; i < num_known; i++)
    {
      struct person* acquaintance = person_get_acquaintance(current, i);
     if(reachable[person_get_index(acquaintance)] > depth-steps_remaining || reachable[person_get_index(acquaintance)]==  0){
       lr_reachable_recursive(acquaintance, steps_remaining-1, reachable, depth);
    }

}
}

}

void parallel_reachable_recursive(struct person * current, int steps_remaining, int * reachable, int depth){
  reachable[person_get_index(current)] = depth - steps_remaining;

  if(steps_remaining > 0)
  {
    int num_known = person_get_num_known(current);
  #pragma omp task
{

    for (int i = 0; i < num_known; i++)
    {
      struct person* acquaintance = person_get_acquaintance(current, i);
  

     if(reachable[person_get_index(acquaintance)] > depth-steps_remaining || reachable[person_get_index(acquaintance)]==  0){
    
       parallel_reachable_recursive(acquaintance, steps_remaining-1, reachable, depth);
}

    }

}

}

}


// computes the number of people within k degrees of the start person
int number_within_k_degrees(struct person * start, int total_people, int k) {
  bool * reachable;
  int count;

  // maintain a boolean flag for each person indicating if they are visited
  reachable = malloc(sizeof(bool)*total_people);
  for ( int i = 0; i < total_people; i++ ) {
    reachable[i] = false;
  }

  // now search for all people who are reachable with k depth
  find_reachable_recursive(start, k, reachable);

  // all visited people are marked reachable, so count them
  count = 0;
  for ( int i = 0; i < total_people; i++ ) {
    if ( reachable[i] == true ) {
      count++;
    }
  }
  return count;
}


// computes the number of people within k degrees of the start person;
// less repeated computation than the simple original version
int less_redundant_number_within_k_degrees(struct person * start,
             int total_people, int k) {
//pthread_t tid; 

int* reachable;
reachable = malloc(sizeof(int)*total_people);
int depth = k;

for(int i = 0 ; i < total_people; i++){
  reachable[i] = 0;
}
//pthread_create(&tid, NULL, find_reachable_recursive2, (start, k, reachable, depth)) ; 
lr_reachable_recursive(start, k, reachable, depth);
int count = 0;

for(int i =0; i < total_people; i++){
  if(reachable[i] != 0){
    count = count + 1;
  }
}
return count;
}

// computes the number of people within k degrees of the start person;
// parallel version of the code
int parallel_number_within_k_degrees(struct person * start, int total_people, int k) {
int * reachable;
reachable = malloc(sizeof(int)*total_people);

int depth = k;
#pragma omp for
for(int i = 0 ; i < total_people; i++){
  reachable[i] = 0;
}
omp_set_dynamic(0);     // Explicitly disable dynamic teams
omp_set_num_threads(4); // Use 4 threads for all consecutive parallel regions

#pragma omp parallel 
{
#pragma omp single
{
parallel_reachable_recursive(start, k, reachable, depth);
}
}

int count = 0;
#pragma omp for
for(int i =0; i < total_people; i++){

  if(reachable[i] != 0){
    count = count + 1;
  }

}

return count;

}
