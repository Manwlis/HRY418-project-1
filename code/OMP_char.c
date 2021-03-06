/*
 ============================================================================
 Name        : OMP_char.c
 Author      : Manolis P, Dimitris K
 Description : Task is set as: Compare one character from a group A string with another from a group B string.
 If they are different, add 1 to the diffs[i][j].
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/time.h>


/*	Random string Generator	*/
void rand_str(char *dest, size_t length) {

	char charset[] = "0123456789";

	while (length-- > 0) {
		size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
		*dest++ = charset[index];
	}
	//	*dest = '\0';	//	End of string.
}


long algori8mos(int m, int n, int l, int threads) {

	//	Array to store distances. Preset all values to 0.
	int** diffs = (int**) malloc( m * sizeof(int*));
	for (int i = 0; i < m; i++)
	   diffs[i] = (int*) calloc(l, n * sizeof(int));

	//	Arrays for the random strings.
	char** StringsA = (char**) malloc( m * sizeof(char*));
	for (int i = 0; i < m; i++)
	   StringsA[i] = (char*) malloc( l * sizeof(char));

	char** StringsB = (char**) malloc( n * sizeof(char*));
	for (int i = 0; i < n; i++)
	   StringsB[i] = (char*) malloc( l * sizeof(char));

	//	Total hamming distance.
	long totalDiff = 0;	

	struct timeval start,finish;
	double totalTime;

	/*~~~~~~~~~~~~~~	Initialize arrays.		~~~~~~~~~~~~~~~~*/

	//	Create random strings.
	for(int i = 0; i < m; i++){
		rand_str(StringsA[i], l);
	}
	for(int i = 0; i < n; i++){
		rand_str(StringsB[i], l);
	}

	/*~~~~~~~~~~~~~~	Find hamming distances.	~~~~~~~~~~~~~~~~*/

	//	Get time of start.
	gettimeofday(&start, NULL);

	#pragma omp parallel num_threads(threads)
	{
		//	Every thread has a local diff in order to lower communication between them.
		long threadDiff = 0;

		/*	Threads pick a pair of character from any pair of strings.
			"nowait" clause because threads don't need to syncronise at the end.
			"ordered" clause because omp acts weird with huge number of loops without it.	*/
		#pragma omp for schedule(static) collapse(3) ordered nowait
		for(int i = 0; i < m; i++){
			for(int j = 0; j < n; j++){
				for(int pshfio = 0; pshfio < l; pshfio++){
					
					if(StringsA[i][pshfio] != StringsB[j][pshfio]){
						threadDiff++;

					/*	Race condition.	Many threads may try to increment the same cell.
						Atomic is faster than critical and can be used here.*/
						#pragma omp atomic
							diffs[i][j]++;
					}
				}
			}
		}

		//	Add local diffs to find the total.
		#pragma omp atomic
			totalDiff += threadDiff;
	}

	//	Get time of finish.
	gettimeofday(&finish, NULL);

	/*~~~~~~~~~~~~~~~	Free memory. Finish.	~~~~~~~~~~~~~~~~*/

	free(diffs);
	free(StringsA);
	free(StringsB);
	
	//	Calculate and print total time.
	totalTime = 
	(double)(finish.tv_usec - start.tv_usec) / 1000.0L + 
	(double)(finish.tv_sec - start.tv_sec) * 1000.0L;
	printf("Total time: %f ms\n", totalTime);

	return totalDiff;
}


int main(int argc, const char** argv) {

	int m = atoi(argv[1]);
	int n = atoi(argv[2]);
	int l = atoi(argv[3]);
	int threads = atoi(argv[4]);

	/*	Seed the pseudo-random algorithm.	*/
	srand (0);

	puts("\nTask size: 1 character.");
	printf("Total humming distance: %ld\n", algori8mos(m, n, l, threads));

	return EXIT_SUCCESS;
}
