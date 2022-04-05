#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
	const int N_TAG = 0;
	const int ID_DATA_TAG = 1;
	const int GRADE_DATA_TAG = 2;
	int my_rank; /* rank of process		*/
	int p;		 /* number of process	*/
	int dest;	 /* rank of reciever		*/
	int size = 2;
	int *students = malloc(size * sizeof(int));
	int *grades = malloc(size * sizeof(int));
	int passed = 0;
	int totalPassed = 0;
	int studentsNum = 0;
	int i;
	int j = 0;
	int studentsPP;
	MPI_Status status; /* return status for 	*/
					   /* recieve				*/
	FILE *file;

	/* Start up MPI */
	MPI_Init(&argc, &argv);

	/* Find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out number of process */
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	if (my_rank == 0)
	{
		file = fopen("./students.txt", "r");

		while (fscanf(file, "%d %d", students + studentsNum, grades + studentsNum) == 2)
		{
			studentsNum++;

			// Expand array
			if (studentsNum > size)
			{
				size *= 2;
				students = realloc(students, size * sizeof(int));
				grades = realloc(grades, size * sizeof(int));
			}
		}

		fclose(file);

		studentsPP = (studentsNum / (p - 1)) + 1;

		for (dest = 1; dest < p; dest++)
		{
			if (dest > studentsNum % (p - 1)) 
				studentsPP = studentsNum / (p - 1);
			
			MPI_Send(&studentsPP, 1, MPI_INT, dest, N_TAG, MPI_COMM_WORLD);

			for (i = 0; i < studentsPP; i++)
			{
				MPI_Send(students + j, 1, MPI_INT, dest, ID_DATA_TAG, MPI_COMM_WORLD);
				MPI_Send(grades + j, 1, MPI_INT, dest, GRADE_DATA_TAG, MPI_COMM_WORLD);
				j++;
			}
		}

		for (dest = 1; dest < p; dest++)
		{
			MPI_Recv(&passed, 1, MPI_INT, dest, N_TAG, MPI_COMM_WORLD, &status);
			totalPassed += passed;
		}

		printf("Total Number of students passed the exam is %d out of %d\n", totalPassed, studentsNum);
	}
	else
	{
		MPI_Recv(&size, 1, MPI_INT, 0, N_TAG, MPI_COMM_WORLD, &status);
		students = realloc(students, size * sizeof(int));
		grades = realloc(grades, size * sizeof(int));

		for (i = 0; i < size; i++)
		{
			MPI_Recv(students + i, 1, MPI_INT, 0, ID_DATA_TAG, MPI_COMM_WORLD, &status);
			MPI_Recv(grades + i, 1, MPI_INT, 0, GRADE_DATA_TAG, MPI_COMM_WORLD, &status);
			
			if (grades[i] >= 60) {
				printf("%d Passed The Exam\n", students[i]);
				passed++;
			}
			else {
				printf("%d Failed. Please Repeat The Exam\n", students[i]);
			}
		}

		MPI_Send(&passed, 1, MPI_INT, 0, N_TAG, MPI_COMM_WORLD);
	}

	/* shutdown MPI */
	MPI_Finalize();
	return 0;
}
