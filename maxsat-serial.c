#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void MAXSAT(int n_clauses, int n_vars, int** clause_matrix, int cur_var, int* prev_comb, int* sat_clauses);
int** parseFile(int * n_clauses, int * n_vars, char name_of_the_file[40]);
int* get_cur_comb(int cur_var, int* prev_comb, int n_vars);
int* alloc_int_array(int n_vars);
int clauses_satisfied(int n_clauses, int** clause_matrix, int cur_var, int* cur_comb, int* unsatisfied, int* sat_clauses);
void print_sol(int* cur_sol, int n_vars);
void free_matrix(int **clause_matrix, int n_clauses);
int* get_cur_sat_clauses(int cur_var, int* prev_sat_clauses, int n_clauses);

int cur_maxsat=0;
int n_solutions=0;
int* cur_sol;

void main(int argc, char** argv){

	if(argc!=2){
		printf("Usage: maxsat-serial input-file.in");
		exit(1);
	}


	int n_clauses, n_vars;
	int **clause_matrix = parseFile(&n_clauses, &n_vars, argv[1]);
	int *first_comb = alloc_int_array(n_vars);
	int *sat_clauses = alloc_int_array(n_clauses);

	MAXSAT(n_clauses, n_vars, clause_matrix,  1, first_comb, sat_clauses); //branch with first variable set to true
	MAXSAT(n_clauses, n_vars, clause_matrix, -1, first_comb, sat_clauses); //branch with first bariable set to false

//	free(first_comb);

	printf("%d %d\n", cur_maxsat, n_solutions);
	print_sol(cur_sol, n_vars);


	free_matrix(clause_matrix, n_clauses);
	free(cur_sol);
	exit(0);
}


void MAXSAT(int n_clauses, int n_vars, int** clause_matrix, int cur_var, int* prev_comb, int* sat_clauses){
	int n_clauses_satisfied;
	int n_clauses_unsatisfied=0;
	int next_var;

	int* cur_comb = get_cur_comb(cur_var, prev_comb, n_vars); //knowing the current variable assignment and the previous combination,
													         //get the current combination
	int* cur_sat_clauses = get_cur_sat_clauses(cur_var, sat_clauses, n_clauses);

	//printf("%f\r", (++prone*100)/pow(2,n_vars) );
	n_clauses_satisfied = clauses_satisfied(n_clauses, clause_matrix, cur_var, cur_comb, &n_clauses_unsatisfied, cur_sat_clauses); //calculate number of clauses satisfied and unsatisfied by current combination
	if(abs(cur_var)<n_vars){ //we're not on the last variable -> we're not on a leaf

		if(n_clauses - n_clauses_unsatisfied < cur_maxsat){ //no need to go further, no better solution we'll be found -> suggestion from the project sheet
			free(cur_comb);
			free(cur_sat_clauses);
		}
		else{ //continue going down the tree
			next_var = abs(cur_var)+1;

			MAXSAT(n_clauses, n_vars, clause_matrix,  next_var, cur_comb, cur_sat_clauses); //branch with next var set to true
			MAXSAT(n_clauses, n_vars, clause_matrix, -next_var, cur_comb, cur_sat_clauses); //branch with next var set to false
		}
		return;
	}
	else{ //we're in a leaf

		if(n_clauses_satisfied == cur_maxsat) //if this combination satisfies the same number of clauses as the current best, increase the number of solutions
			n_solutions++;
		else{
			if(n_clauses_satisfied > cur_maxsat){ //if this combination satisfies more clauses than the previous best...
				cur_sol = cur_comb; //store the solution
				cur_maxsat = n_clauses_satisfied; //update the best score
				n_solutions=1;
				return;
			}
		}
		free(cur_comb);
		free(cur_sat_clauses);
	}
}

int* alloc_int_array(int n){
	int* first_comb = (int *) malloc(n*sizeof(int));

	for(int i=0; i<n; i++)
		first_comb[i]=0;

	return first_comb;
}

int* get_cur_sat_clauses(int cur_var, int* prev_sat_clauses, int n_clauses){
	int* cur_sat_clauses;

	if(cur_var<0)
		cur_sat_clauses = prev_sat_clauses;
	else{
		cur_sat_clauses = (int *) malloc(n_clauses*sizeof(int));
		for(int i=0; i<n_clauses; i++)
			cur_sat_clauses[i]=prev_sat_clauses[i];
	}

	return cur_sat_clauses;
}

//to obtain the combination, we copy the previous combination and assign the current variable accordingly. actually we only copy half of the time. half of the children reuse the parent's array, the other's copy it.
int* get_cur_comb(int cur_var, int* prev_comb, int n_vars){
	int* cur_comb;
	int i;

	if(cur_var<0)
		cur_comb = prev_comb;
	else{
		cur_comb = (int *) malloc(n_vars*sizeof(int));
		for(i=0; i<n_vars; i++)
			cur_comb[i]=prev_comb[i];
	}

	cur_comb[abs(cur_var)-1]=cur_var;

	return cur_comb;
}

//for a given combination and current variable, calculate the number of clauses satisfied (and unsatisfied)
int clauses_satisfied(int n_clauses, int** clause_matrix, int cur_var, int* cur_comb, int* unsatisfied, int* sat_clauses){
	int i, j, n_clauses_satisfied=0;
	int unsat = 0;
	int var;

	for(i=0; i<n_clauses; i++){ //for each clause
		if(sat_clauses[i]==-1){
			*unsatisfied = *unsatisfied + 1;
			unsat=0;
			continue;
		}
		if(sat_clauses[i]==1){
			n_clauses_satisfied++;
			unsat=0;
			continue;
		}

		for(j=0; j<20; j++){ //for each variable
			var = abs(clause_matrix[i][j]);

			if(var==0 || var>abs(cur_var)) // end of clause or we don't know the next variable assignments
				break;

			if(cur_comb[var-1] == clause_matrix[i][j]){ //if the variable corresponds
				sat_clauses[i]=1;
				n_clauses_satisfied++;
				unsat=0;
				break; //only one variable needs to match for the clause to be satisfied
			}
			else
				unsat=1;
		}

		if(unsat==1 && clause_matrix[i][j]==0){
			sat_clauses[i]=-1;
			*unsatisfied = *unsatisfied + 1;
			unsat=0;
		}
	}

	return n_clauses_satisfied;
}

void print_sol(int* cur_sol, int n_vars){
	int i;
	for(i=0; i<n_vars; i++)
		printf("%d ", cur_sol[i]);

	printf("\n");
}

void free_matrix(int** clause_matrix, int n_clauses){
	int i;
	for(i=0; i<n_clauses; i++)
		free(clause_matrix[i]);

	free(clause_matrix);
}

int **parseFile(int * n_clauses, int * n_vars, char name_of_the_file[40]){
	int n,j,i = 0;
   	long elapsed_seconds;
  	char line[80];
   	int matrix_line = 0, matrix_column = 0;
   	int field = -1;
	FILE *fr;
	char *start;
	int ** clause_matrix;
   	fr = fopen (name_of_the_file, "rt");  /* open the file for reading */

   	/*Read n_vars and n_clauses*/
	fgets(line,80,fr);
	sscanf(line, "%d" "%d", n_vars, n_clauses);
	//printf("N_vars =  %d  N_clauses = %d \n",*n_vars,*n_clauses);



    clause_matrix = (int**)malloc( *n_clauses * sizeof( int* ));

    for (i = 0; i < *n_clauses; i++){
    	clause_matrix[i] = (int *)malloc(20 * sizeof(int));
 	}

 	for (i = 0; i < *n_clauses; i++){
 		for (j = 0; j < 20; j++)
 		{
 			clause_matrix[i][j] = 0;
 		}
 	}
 	//printf("-------clauses---------------\n");
   	while(fgets(line, 80, fr) != NULL){
   		start = line;

   		while (sscanf(start, "%d%n", &field, &n) == 1 && field != 0) {
   			clause_matrix[matrix_line][matrix_column] = field;
        	//printf("%d ", clause_matrix[matrix_line][matrix_column]);
        	start += n;
        	matrix_column ++;
    	}

    //printf("\n");
    matrix_column= 0;
    matrix_line ++;
   	}
   	//printf("----------------------------\n" );

   	fclose(fr);

   	return clause_matrix;
}
