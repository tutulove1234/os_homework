#ifndef  _TASK_STRUCT_H
#define  _TASK_STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#define 	BUFFER_SIZE 	256

struct task_and_data {
	void (*task_type_one) (const char *) ; 		//基本的打印工作
	double (*task_type_two) (const char *) ; 	//计算传入的值
	
	char task_buffer[BUFFER_SIZE] ; 		//任务缓冲区

	int task_type ; 				//任务类型标志
} ;

typedef struct task_queue {
	int thread_id ; 				//线程号
	struct task_and_data task ; 			//任务结构
	struct task_queue *next ; 			//指向下一个任务
} task_queue , *task_queue_p ;

typedef struct task_queue_head_rear { 			//队列的头和尾指针
	task_queue_p  head ;
	task_queue_p  rear ;
	
	int task_count ; 				//任务计数
} task_queue_pointer ;

void print_string (const char * str ) ;
double calculate (const char * str ) ;
	
#endif
