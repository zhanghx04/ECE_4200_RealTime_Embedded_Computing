#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#define MAX_LINE 500


int row;
int col;
int *matrix;
int num;

struct thread_data{
    int thread_id;
    int sum;
};

struct timeval start, end;
/*
    *(matrix+i*row + j is the address of point(i,j)
    the address of matrix is an 1_D array. 
    like row1col1 | row1col2 | row1col3 | row2col1 | row2col2 | row2col3
*/

// One thread for searching the entire matrix
void* oneT_all(void* threadptr){
    struct thread_data *td;
    td = (struct thread_data*) threadptr;
    //printf("prepare to searching %d\n", num);
    int i;
    int j;
    //printf("the number found in: \n");
    for (i=0; i<row; i++) {
        for (j=0; j<col; j++) {
            if(*(matrix+i*col + j) == num) {
                //printf("(%d, %d) ", i, j);
                td->sum += 1;
            }
        }
        //printf("\n");
    }
    pthread_exit(0);
}

// One thread for searching each row of the matrix
void* oneT_row(void *threadptr){
    struct thread_data *td;
    td = (struct thread_data*) threadptr;
    int j;
    for (j = 0; j < col; j++) {
        if (*(matrix + td->thread_id*col + j) == num) {
            printf("(%d, %d) ", td->thread_id, j);
            td->sum += 1;
        }
    }
    printf("\n");
    pthread_exit(0);
}
// One thread for searching each column of the matrix
void* oneT_col(void *threadptr){
    struct thread_data *td;
    td = (struct thread_data*) threadptr;
    int i;
    for (i = 0; i < row; i++) {
        if (*(matrix + i*col + td->thread_id) == num) {
            printf("(%d, %d) ", i, td->thread_id);
            td->sum += 1;
        }
    }
    pthread_exit(0);
}
// One thread for searching each element of the matrix
void* oneT_one(void *threadptr){
    struct thread_data *td;
    td = (struct thread_data*) threadptr;
    if (*(matrix + td->thread_id) == num) {
        int r = (int)td->thread_id/(col-1);
        int c = (int)td->thread_id%(col-1);
        printf("(%d, %d)\n", r,c);
        td->sum += 1;
    }
    pthread_exit(0);
}


/*
    main function
 */
int main (int argc, char **argv){
    /* 
     * Task1: reading data from file
     */
    FILE *fptr;
    char line[MAX_LINE];
    char *result; // line
    char *p;

    
    char filename[15] = "2x100.txt";
    
    printf("reading from < %s > .......\n", filename);
    fptr = fopen(filename, "r");
    // check the access for file
    if(fptr == NULL){                                           
        printf("Cannot open file < %s > \n", filename);
        exit(0);
    }
    result  =fgets(line, MAX_LINE, fptr);
    // reading
    /*
	strtok(string, split_value)
	strtok read one word each time. (everytime meet space will stop)
    */
    p = strtok(result, " ");
    if(p != NULL){
        row = atoi(p);
        p = strtok(NULL, " ");
        col = atoi(p);
    }
    printf("%d x %d\n", row, col);
    
    int m[row][col];
    
    // reading matrix
    int i;
    int j;
    for (i = 0; i < row; i++){
        result = fgets(line, MAX_LINE, fptr);
        p = strtok(result, "\t");
        if(p != NULL){
            m[i][0] = atoi(p);
        }
        for (j = 1; j < col; j++){
            p = strtok(NULL, "\t");
            if(p != NULL){
                m[i][j] = atoi(p);
            }
        }
    }
    matrix = &m[0][0];
    
    // printing m

    for(i = 0; i < row; i++){
        for(j = 0; j < col; j++){
            printf("%d   ", m[i][j]);
        }
        printf("\n");
    }
    
    
    fclose(fptr);
    printf("============================ Start ============================\n");
    /* 
     *  Task2
     */
    
    printf("Please enter a number you want to search:\n");
    scanf("%d", &num);
    
    printf("Searching %d ...\n", num);
    
    printf("argc: %d\n", argc);

    float time_total[10];
    for (i = 0; i < 10; i++){
	    gettimeofday(&start, NULL);
        
        
        // thread 1
	    if (argc == 1) {
            /*
            * One thread to search the entire matrix 
            */
            int sum_num = 0;
            // create structure
            struct thread_data threaddata;
            // setup params
            threaddata.thread_id = 1;
            threaddata.sum = 0;
            // thread id
            pthread_t thread;
            // start thread
            pthread_create(&thread, NULL, oneT_all, (void*)&threaddata);
            pthread_join(thread, NULL);
            
            sum_num += threaddata.sum;
            
            printf("sum is %d\n", sum_num);
	    } 
	    
	    
	    // thread 2
	    else if (argc == 2){
            /*
            * One thread for searchingeach row of the matrix
            */
            int sum = 0;
            // create structure
            struct thread_data threaddata[row];
            // thread id
            pthread_t thread[row];
            long t;
            for (t=0; t<row; t++) {
                // setup params
                threaddata[t].thread_id = t;
                threaddata[t].sum = 0;
                // start thread
                pthread_create(&thread[t], NULL, oneT_row, (void *) &threaddata[t]);
            }
            for (t=0; t<row; t++) {
                pthread_join(thread[t], NULL);
                sum += threaddata[t].sum;
            }
            printf("sum is %d\n", sum);
	    }
	    
	    // thread 3
	    else if (argc == 3){
            /*
            * One   thread   for   searchingeach column of the matrix
            */
            int sum = 0;
            // create structure
            struct thread_data threaddata[col];
            // thread id
            pthread_t thread[col];
            long t;
            for (t=0; t<col; t++) {
                // setup params
                threaddata[t].thread_id = t;
                threaddata[t].sum = 0;
                // start thread
                pthread_create(&thread[t], NULL, oneT_col, (void *)&threaddata[t]);
            }
            printf("\n");
            for (t=0; t<col; t++) {
                pthread_join(thread[t], NULL);
                sum += threaddata[t].sum;
            }
            printf("sum is %d\n", sum);
	    }
	    
	    // thread 4
	    else if (argc == 4){
            /*
            * One thread for each elementof the matrix
            */
            long l = row * col;
            int sum = 0;
            // create structure
            struct thread_data threaddata[l];
            // thread id
            pthread_t thread[l];
            long t;
            // pass the id of thread. 
            // because the id of thread is same position of the index of matrix pointer.
            for (t=0; t<l; t++) {
                // setup params
                threaddata[t].thread_id = t;
                threaddata[t].sum = 0;
                // start thread
                pthread_create(&thread[t], NULL, oneT_one, (void *)&threaddata[t]);
            }
            
            for (t=0; t<l; t++) {
                pthread_join(thread[t], NULL);
                sum += threaddata[t].sum;
            }
            printf("sum is %d\n", sum);
            
        } else {
            printf("Wrong argc... done!");
            exit(0);
        }
	    
        // get the end time
	    gettimeofday(&end, NULL);
	    float time = (end.tv_sec - start.tv_sec) + ((float)end.tv_usec - (float)start.tv_usec)/1000000;
	    time_total[i] = time;
    }
    float sum_time = 0;
    for (i = 0; i < 10; i++) {
	sum_time += time_total[i];
    }
    printf("time used: %lfs\n", sum_time/10);

    
    
    /*
     * Task3
     */
    
    // exit
    pthread_exit(0);


	return 0;
}
