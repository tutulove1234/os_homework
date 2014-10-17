/*
 * =====================================================================================
 *
 *       Filename:  philosopher_thinking.c
 *
 *    Description:  test write philosopher thinking problem
 *
 *        Version:  1.0
 *        Created:  10/16/2014 04:32:30 PM
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

#define 	PHI_COUNT 	5 
#define 	THINKING 	0
#define 	HUNGRY 		1
#define 	EATING 		2
#define 	LEFT(i)    	((i-1+PHI_COUNT)%PHI_COUNT)
#define 	RIGHT(i)        ((i+1)%PHI_COUNT)
#define 	TRUE 		1 
#define 	FALSE 		0

struct philosopher {
	pthread_t 		tid ;
	pthread_cond_t 		cond ;
	int 			status ;
} phi_buf[PHI_COUNT] ;

pthread_mutex_t 	phi_lock ;

void print_status () {
	int i  ;

	for (i = 0 ; i < PHI_COUNT ; ++i ) {
		if (phi_buf[i].status == THINKING  )
			printf ("哲学家 %d 正在思考\n" , i) ;
		else if (phi_buf[i].status == HUNGRY ) {
			printf ("哲学家 %d 很饿 \n" , i) ;
		} else {
			printf ("哲学家 %d 正在吃吃吃吃吃吃 ~~~  \n" , i) ;
		}
	}
}

void init_philosopher () {
	int 	i ;
	//初始化互斥锁 , 为一般快速互斥锁
	if (pthread_mutex_init (&phi_lock , NULL ) != 0 ) {
		perror ("Pthread_mutx_init error !") ;
		exit (1) ;
	}
	//初始化所有线程初始状态为思考
	for ( i = 0 ; i < PHI_COUNT ; ++i ) {
		phi_buf[i].status = THINKING ;
	}
}

int test (int argu ) {
	if ( (phi_buf[argu].status == HUNGRY )&& (phi_buf[LEFT(argu)].status != EATING) && (phi_buf[RIGHT(argu)].status != EATING) ) {
		return TRUE ;
	} else {
		return FALSE ;
	}
} 

void take_fork (int argu) {
	//锁住共享区 即那组哲学家信息数组
	pthread_mutex_lock (&phi_lock) ;
//	index = get_index (pthread_self () ) ;
	//修改自己当前状态
	phi_buf[argu].status = HUNGRY ;
	//测试左边 , 右边是否都不处于EATING状态 , 如果是 , 那么就可以吃啦!
	if ( test (argu) ) {
		//修改自己的状态
		phi_buf[argu].status = EATING ;
		printf ("哲学家 %d  正在进食 ~~~ \n" , argu) ;
		print_status () ;
		phi_buf[argu].status = THINKING ;
	
		//判断左边能否进食
		if (test (LEFT (argu))) {
			phi_buf[LEFT(argu)].status = EATING ;
			printf ("哲学家 %d  正在进食 ~~~ \n" , LEFT(argu)) ;
			pthread_cond_signal (&(phi_buf[LEFT(argu)].cond)) ;
			print_status () ;
		} 
		if (test (RIGHT (argu))) {
			phi_buf[RIGHT(argu)].status = EATING ;
			printf ("哲学家 %d  正在进食 ~~~ \n" , RIGHT(argu)) ;
			print_status () ;
			pthread_cond_signal (&(phi_buf[RIGHT(argu)].cond)) ;
		} 
//		pthread_mutex_unlock (&phi_lock) ;
		//告诉自己不用再阻塞了
		pthread_mutex_unlock (&phi_lock) ;
	//	pthread_cond_broadcast (&(phi_buf[argu].cond)) ;
	} else {
		//如果左边和右边有一个在吃 那么就再等待
//		pthread_mutex_unlock (&phi_lock) ;
		pthread_cond_wait (&(phi_buf[argu].cond) , &phi_lock) ;
	}
}

/*void put_fork (int argu) {
	//锁住临界区
	pthread_mutex_lock (&phi_lock) ;
	
	//放下叉子 , 同时判断放下后的身边哲学家能不能吃 ~ 能吃的话 就吃 .

//	pthread_cond_wait (&(phi_buf[argu].cond), &phi_lock) ;
}*/

void * func (void * argu) {
	while (1 ) {
		take_fork (argu) ;	
//		put_fork (argu) ;
		sleep (1) ;
	}
} 

void create_pthread (void * (*fun)(void*) , int data) {
	pthread_t 	tid ;

	if (pthread_create (&tid , NULL , fun , (void*)data ) != 0 ) {
		perror ("Pthread_create error !") ;
		exit (1) ;
	}
	phi_buf[data].tid = tid ;
	if (pthread_cond_init(&(phi_buf[data].cond) , NULL ) != 0 ) {
		perror ("Pthread_cond_init error !") ;
		exit (1) ;
	}
	pthread_detach (tid) ;
}

int main(int argc, char *argv[])
{
	//初始化
	init_philosopher () ;

	create_pthread (func , 0) ;
	create_pthread (func , 1) ;
	create_pthread (func , 2) ;
	create_pthread (func , 3) ;
	create_pthread (func , 4) ;

	sleep (100000) ;

	return EXIT_SUCCESS;
}

