#include "task_struct.h"

pthread_mutex_t task_queue_lock ; 					//任务队列互斥
sem_t sem ; 								//申请信号量
task_queue_pointer task_queue_hr ; 					//申请任务队列头尾指针

void print_string (const char *str ) {
	printf ("%s" , str ) ;
}

double calculate (const char * str ) {
	double result  = 1 ;

	printf ("This expression is *** , now I will calculate it !\n") ;
	return result ;
}

void init_sem_task_queue () {
	pthread_mutex_init (&task_queue_lock , NULL) ;
	sem_init (&sem, 0 , 0 ) ; 					//初始化信号量
	task_queue_hr.rear = task_queue_hr.head = NULL ; 		//初始化任务队列为空
	task_queue_hr.task_count = 0 ; 					//初始化任务数量为0
}

void * exec_th (void * data ) {
	int tid = data ;
	task_queue_p 	task_temp ;
	struct task_and_data 	task ;

	printf ("pthread %d waitting \n" , tid) ;

	while (1) {
		sem_wait (&sem) ; 						//等待信号到来
	
		///锁住任务队列
		pthread_mutex_lock (&task_queue_lock) ;
		printf ("Tread %d is running \n" , data);	
		///当任务队列有任务时, 取出来, 执行
			///取出队头指针
			if (task_queue_hr.head != task_queue_hr.rear) {
				task = (task_queue_hr.head)->task ;
				task_temp = task_queue_hr.head ;
			///修改队头指针
		//	printf ("task pointer value is %p \n" , task_temp) ;
			///当当前任务不是最后一个任务时
				task_queue_hr.head = (task_queue_hr.head)->next ;
				free (task_temp) ;
			} else {
				if (task_queue_hr.head != NULL ) {
					task = (task_queue_hr.head)->task ;
					free (task_queue_hr.head) ;
					task_queue_hr.head = task_queue_hr.rear = NULL ;				
				} else {
					printf ("任务队列已空!\n") ;
				}
			}
			
			///释放队列空间
			///释放锁
			pthread_mutex_unlock (&task_queue_lock) ;
			switch (task.task_type) {
				case 1 : (*(task.task_type_one))(task.task_buffer) ;  break;
				case 2 : break ;
			}
	}
}

void * task_th (void * data ) {
	int count = 100 ;
	task_queue_p temp_task_p ; 					//临时指向申请的任务
	task_queue_p temp_task_before_p ; 				//指向先前一个节点

	while (count) {
		pthread_mutex_lock (&task_queue_lock) ; 		//锁住队列
		temp_task_p = (task_queue_p)malloc (sizeof (task_queue)) ;
		(temp_task_p ->task).task_type_one = print_string ;
		sprintf (((temp_task_p ->task).task_buffer) , "%s%d%c","Hello this is a task " , count,'\n' ) ;
		
		(temp_task_p ->task).task_type = 1 ; 			//表示执行打印函数
		temp_task_p->next = NULL ;	

		///第一次插入时
		if ((task_queue_hr.task_count) == 0) {
			task_queue_hr.head = task_queue_hr.rear = temp_task_p ;			
			temp_task_before_p = temp_task_p ;
			(task_queue_hr.task_count) ++ ;
		} else {
		///插入队列
			task_queue_hr.rear = temp_task_p ;
			temp_task_before_p ->next = temp_task_p ;
			temp_task_before_p = temp_task_p ;
			(task_queue_hr.task_count) ++ ;
		}

		count -- ; 						//添加任务次数减1
		pthread_mutex_unlock (&task_queue_lock) ;
		sem_post (&sem) ; 					//添加信号量
	} 	

	printf ("任务线程放置所有任务完毕\n") ;

	pthread_exit (NULL) ;
}

int main (int argc , char* argv[]) {
	pthread_t 	exec_th1 , exec_th2 , exec_th3 , task_thread ;	

	init_sem_task_queue () ; 					//初始化信号量和互斥量

	pthread_create (&exec_th1 , NULL ,exec_th , (void*)1 ) ;
	pthread_detach (exec_th1) ;
  	pthread_create (&exec_th2 , NULL ,exec_th ,(void*) 2 ) ;
  	pthread_detach (exec_th2) ;
  	pthread_create (&exec_th3 , NULL ,exec_th , (void*)3 ) ;
  	pthread_detach (exec_th3) ;
	pthread_create (&task_thread , NULL , task_th , (void*)4 ) ;
	pthread_detach (task_thread) ;

	sleep (10000) ;

	return 0 ;
}
