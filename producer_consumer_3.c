#include "task_struct.h"

///代码的出错处理没有加上 !!!!!!!!!!!

struct thread_buf {
	pthread_t  tid ;
	int 	   status ;
} thread_buf[BUFFER_SIZE] ;

pthread_mutex_t task_queue_lock ; 					//任务队列互斥
sem_t sem ; 								//申请信号量
task_queue_pointer task_queue_hr ; 					//申请任务队列头尾指针
int 	thread_count = 1; 						//初始化创建3个线程

void print_string (const char *str ) {
	printf ("%s" , str ) ;
}

double calculate (const char * str ) {
	double result  = 1 ;

	printf ("This expression is *** , now I will calculate it !\n") ;
	return result ;
}

void init_sem_task_queue () {
	int index = 0 ;

	pthread_mutex_init (&task_queue_lock , NULL) ;
	sem_init (&sem, 0 , 0 ) ; 					//初始化信号量
	task_queue_hr.rear = task_queue_hr.head = NULL ; 		//初始化任务队列为空
	task_queue_hr.task_count = 0 ; 					//初始化任务数量为

	///初始化线程队列为所有线程区状态不可用
	while (index < BUFFER_SIZE) {
		thread_buf[index++].status = INVISIBLE ;
	}
}
///获得当前线程在线程队列中的下标
static inline int get_index (pthread_t argu) {
	int 	i = 0 ;

	while (i < BUFFER_SIZE) {
		if ((thread_buf[i].status != INVISIBLE) && (thread_buf[i].tid == argu)) {
			break ;
		} 
		i ++ ;
	}
	if (i >= BUFFER_SIZE ) {
		printf ("Can't find thread info ! Serious error !\n") ;
		exit (1) ;
	}
	return i ;
} 

void call_back (int argu) {
	int  	index ;
	///信号回调函数 ,如果收到取消信号 , 线程自己退出
	index = get_index (pthread_self()) ;
	printf ("My index is %d \n" , index) ;
	///如果当前收到信号的线程状态为BUSY , 那么不能被取消
	if (thread_buf[index].status == BUSY) {
		return ;
	} else if ( thread_buf[index].status == FREE ) {
		printf ("thread %lu is exit \n" , thread_buf[index].tid) ;
		///设置当前线程为不可用
		thread_buf[index].status = INVISIBLE ;
		pthread_exit ((void*)0) ;
	} 
}

void * exec_th (void * data ) {
	int tid =(int) data ;
	int index ;
	task_queue_p 	task_temp_p ;
	struct task_and_data 	task ;

	///注册信号处理函数
	signal (SIGQUIT , call_back) ;
	printf ("thread %d waitting \n" , tid) ;

	while (1) {
		
		sem_wait (&sem) ; 						//等待信号到来
	
		///锁住任务队列
		pthread_mutex_lock (&task_queue_lock) ;
		if (task_queue_hr.task_count <= 0 ) {
			printf ("任务队列已空 !\n") ;
			pthread_mutex_unlock (&task_queue_lock) ;
			continue ;
		} 
		///获取当前线程下标
		index = get_index (pthread_self ()) ;
		///修改当前线程状态
		thread_buf[index].status = BUSY ;
		printf ("Tread %d is running \n" , data);	
		///当当前任务不是最后一个任务时
		if (task_queue_hr.task_count > 1) {
			task = (task_queue_hr.head)->task ;
			task_temp_p = task_queue_hr.head ;
			///修改队头指针
			task_queue_hr.head = (task_queue_hr.head)->next ;
			(task_queue_hr.task_count ) -- ;
			free (task_temp_p) ;
		} else if (task_queue_hr.task_count <= 0 ) {
				printf ("任务队列已空!\n") ;
				printf ("task_queue count %d \n" , task_queue_hr.task_count) ;
				continue ;
		} else {
			///当为最后一个任务时
			task_temp_p = task_queue_hr.head ;
			task = (task_queue_hr.head)->task ;
			task_queue_hr.head = task_queue_hr.rear = NULL ;
			(task_queue_hr.task_count) -- ;
			free (task_temp_p) ;
		}
			
			///释放锁
			pthread_mutex_unlock (&task_queue_lock) ;
			switch (task.task_type) {
				case 1 : (*(task.task_type_one))(task.task_buffer) ; break;
				case 2 : break ;
			}
			///将自己状态修改为空闲
			thread_buf[index].status = FREE ;
	}
}

void * task_th (void * data ) {
	int count = 15 ;
	int index ;
	task_queue_p temp_task_p ; 					//临时指向申请的任务
	task_queue_p temp_task_before_p ; 				//指向先前一个节点

	index = get_index (pthread_self()) ;
	thread_buf[index].status = BUSY ;

task_put:
	while (count) {
		pthread_mutex_lock (&task_queue_lock) ; 		//锁住队列
		if (task_queue_hr.task_count >= BUFFER_SIZE) {
			printf ("Task queue is full !\n") ;
			pthread_mutex_unlock (&task_queue_lock) ;	//解锁队列
			continue ;
		}
		temp_task_p = (task_queue_p)malloc (sizeof (task_queue)) ;
		(temp_task_p ->task).task_type_one = print_string ;
		sprintf (((temp_task_p ->task).task_buffer) , "%s%d%c","Task " , count,'\n' ) ;
		
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
	printf ("当前任务总数 %d\n" , task_queue_hr.task_count) ;
//	pthread_exit (NULL) ;
	count = 4 ;
	sleep (1) ;
	goto task_put ;
}

void create_thread ( void*(*func)(void*) , void *argu) {
	pthread_t tid ;
	int index = 0 ;
	
	pthread_create (&tid , NULL , func , (void *)argu) ;
	pthread_detach (tid) ;

	while (index < BUFFER_SIZE) {
		if (thread_buf[index].status == INVISIBLE )
		      break ;
		index ++ ;
	}
	if ( index >= BUFFER_SIZE ) {
		printf ("I can't find a place ! Serious error !\n") ;
		exit (1) ;
	}
	///将线程号加入到数组中
	thread_buf[index].tid = tid ;
	///1 为任务放置线程 2 为管理线程
	if ((argu == 1) || (argu == 2) ) { 					//管理线程
		thread_buf[index].status = BUSY ;
	} else {
		thread_buf[index].status = FREE ;
	}
	thread_count ++ ;
}

///管理线程函数
void * manage_thread (void *data) {
	int  	index = 0 ;
	
	///管理线程 , 实现对当前线程的管理 包括任务多时 动态添加 任务少时 动态取消
	while (1) {
		///当任务队列中的任务大于10 时 , 动态创建线程加入到数组中
		if ( (task_queue_hr.task_count > 10) && (thread_count < 7 ) ) {
				printf ("当前任务较多,添加新线程 !\n") ;
				create_thread (exec_th , (void*)thread_count) ;
				create_thread (exec_th , (void*)thread_count) ;
				create_thread (exec_th , (void*)thread_count) ;
				create_thread (exec_th , (void*)thread_count) ;
				create_thread (exec_th , (void*)thread_count) ;
				printf ("当前线程总数 %d \n" , thread_count ) ;
		} else if ( (task_queue_hr.task_count < 5 ) && (thread_count > 5) ) {
			///当前任务数小于5 但是 线程数大于5 取消一些线程 直到线程数 等于5
				while ( thread_count > 5 ) {
					if (thread_buf[index].status == FREE )	{
						if ( 0 == pthread_kill (thread_buf[index].tid , SIGQUIT) )
						{
							printf ("线程 %lu 已经取消\n" , thread_buf[index].tid) ;
							thread_count -- ;
						} else {
							printf ("Pthread_kill error !\n") ;
							exit (1) ;
						}
					}
					index ++ ;
					if (index >= BUFFER_SIZE) {
						index = index % BUFFER_SIZE ;
					}
				}
		}
		printf ("当前线程总数 %d \n" , thread_count ) ;
		sleep (1) ;
	}
}	 

int main (int argc , char* argv[]) {
	init_sem_task_queue () ; 					//初始化信号量和互斥量
	
	create_thread (task_th , (void*)1);
 	sleep (2) ;
// 	创建4个线程先用
	create_thread (manage_thread , (void*)2 ) ;
	sleep (10000) ;

	return 0 ;
}
