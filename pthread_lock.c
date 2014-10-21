/*
 * =====================================================================================
 *
 *       Filename:  pthread_lock.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/21/2014 10:35:55 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  windleaves (Litong), tonglovejin1@163.com
 *        Company:  Class 1204 of Computer Science and Technology
 *
 * =====================================================================================
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <ctype.h>

#define 	MAX_THREAD 	3 		//线程总数

unsigned long long main_counter ;
unsigned long long counter[MAX_THREAD] ;
pthread_mutex_t 	sum_lock = PTHREAD_MUTEX_INITIALIZER;

void * thread_worker (void * argu) {
	int thread_num = (int ) argu;

	for ( ; ; ) {
		pthread_mutex_lock (&sum_lock) ;
		counter[thread_num]++ ;
		pthread_mutex_unlock (&sum_lock) ;
	}

	main_counter ++ ;
}

int main(int argc, char *argv[])
{
	int i , rtn , ch ;

	pthread_t pthread_id[MAX_THREAD] = {0} ; 		//存放所有线程id

	for (i = 0 ; i < MAX_THREAD ; ++i ) {
		pthread_create (pthread_id + i , NULL , thread_worker , (void*)i ) ;
	}
	
	do {
		unsigned long long sum = 0 ;
		// 求所有线程的和

		pthread_mutex_lock (&sum_lock) ;
		for (i = 0 ; i < MAX_THREAD ; ++i) {
			sum += counter[i] ;
			printf ("%llu\n" , counter[i]) ;
		}
		printf ("%llu/%llu\n" , main_counter , sum ) ;
		pthread_mutex_unlock (&sum_lock) ;

	} while ((ch = getchar ()) != 'q') ;


	return EXIT_SUCCESS;
}

