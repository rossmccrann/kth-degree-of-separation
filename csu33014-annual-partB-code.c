//
// CSU33014 Summer 2020 Additional Assignment
// Part B of a two-part assignment
//
// Please write your solution in this file

/*

Ross McCrann: 16326617

*** Optimised number within k degrees using Recursion ***

I replaced the Boolean reachable array (reachable[person_get_index(current)] = true;)
with an array of ints (int   reachable[i] = 0;) as the integer value can be utilised
to optimise the code. Each int reachable[i] represents the distance from the intial
person ( person * start) to the ith aquaintance in the array.
If you are at the current person '(person_get_acquaintance(current, i)' it checks the
distance that it is from current starting to the distance of the potential aquaintance that it
wants to get to. If the current distance ('distance - steps_remaining') is less than the
reachable distance of the aquaintance OR the distance of the reachable aquaintance is equal to 0
then the recursive function is called and the steps_remaining is decremented by 1 as all of the
values in the reachable array were initially set to zero. This is represented with an if statement
around the recursive call.

Using these alterations the code is effectively optimised and is much faster than unoptomised
code with all inputs.

*** Parallelisation using OpenMP ***

The best way to parrallelise the code is to use the OpenMP parallel pragma.The block
after the #pragma omp parallel is executed by a group of threads. The index i is automatically
set to private to avoid data corruption. The block is split upand ran simultaneously
on multiple processors with different inputs so that the result is obtained faster.

The #pragma task, is the block of 'work' that is set out to be done by the team of
threads. The task is set in the recursive function.
I use 'omp_set_dynamic(0)' to explicitly disable dynamic teams and then use
'omp_set_num_threads(4)' to override the value of the environmental variable that controls
the upper limit of the size of the thread team that is created for all of the parrallel
segments in the code.

The OpenMP Single ensures that the recursive function (parallel_reachable_recursive) will
only be executed once by single thread. The OpenMp pragma nowait turns off the barrier on
the single however there is already a barrier on the parallel.

Using these OpenMP Pragma the code is effectively parrallelised ensuring that the result is
obtained on average over twice as fast at the already optimised code (less_redundant) and is
much faster on larger input sizes.

*/

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
      find_reachable_recursive(acquaintance, steps_remaining - 1, reachable);
    }
  }
}

void lr_reachable_recursive(struct person * current, int steps_remaining, int * reachable, int distance) {
  int curr = distance - steps_remaining; // current = the distance - the number of steps remaining
  reachable[person_get_index(current)] = curr; // set value of current person
  if (steps_remaining > 0)
  {
    int num_known = person_get_num_known(current);
    for (int i = 0; i < num_known; i++)
    {
      struct person* acquaintance = person_get_acquaintance(current, i);
      // If the current distance is less than the reachable distance of the aquaintace
      //OR the distance of the reachable aquaintance is equal to 0 then the recursive function called
      if (reachable[person_get_index(acquaintance)] ==  0 || curr < reachable[person_get_index(acquaintance)]) {
        lr_reachable_recursive(acquaintance, steps_remaining - 1, reachable, distance);
      }
    }
  }

}

void parallel_reachable_recursive(struct person * current, int steps_remaining, int * reachable, int distance) {
  int curr = distance - steps_remaining;
  reachable[person_get_index(current)] = curr;

  if (steps_remaining > 0)
  {
    int num_known = person_get_num_known(current);
    #pragma omp task
    {
      for (int i = 0; i < num_known; i++)
      {
        struct person* acquaintance = person_get_acquaintance(current, i);

        if (reachable[person_get_index(acquaintance)] ==  0 || curr < reachable[person_get_index(acquaintance)]) {

          parallel_reachable_recursive(acquaintance, steps_remaining - 1, reachable, distance);
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
  reachable = malloc(sizeof(bool) * total_people);
  for ( int i = 0; i < total_people; i++ ) {
    reachable[i] = false;
  }

  // now search for all people who are reachable with k distance
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
int less_redundant_number_within_k_degrees(struct person * start, int total_people, int k) {

  int* reachable;
  reachable = malloc(sizeof(int) * total_people);

  for (int i = 0 ; i < total_people; i++) {
    reachable[i] = 0;
  }
  int distance = k;
  lr_reachable_recursive(start, k, reachable, distance);

// all visited people are marked reachable, so count them
  int count = 0;
  for (int i = 0; i < total_people; i++) {
    if (reachable[i] != 0) {
      count = count + 1;
    }
  }
  return count;
}

// computes the number of people within k degrees of the start person;
// parallel version of the code
int parallel_number_within_k_degrees(struct person * start, int total_people, int k) {
  int * reachable;
// change reachable to ints
  reachable = malloc(sizeof(int) * total_people);

  for (int i = 0 ; i < total_people; i++) {
    reachable[i] = 0;
  }
  int distance = k;

  omp_set_dynamic(0);     // Explicitly disable dynamic teams
  omp_set_num_threads(4); // Use 4 threads for all consecutive parallel regions

  #pragma omp parallel
  {
    #pragma omp single nowait
    {
      parallel_reachable_recursive(start, k, reachable, distance);
    }
  }

// all visited people are marked reachable, so count them
  int count = 0;
  for (int i = 0; i < total_people; i++) {
    if (reachable[i] != 0) {
      count = count + 1;
    }
  }

  return count;
}