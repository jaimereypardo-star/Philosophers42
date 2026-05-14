# ifndef PHILO_H /* if not yet defined */
# define PHILO_H /* define it */
 
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
#include <stdbool.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
 

# define ERR_ARGS   "Usage: philo N time_to_die time_to_eat time_to_sleep [must_eat]\n"
# define ERR_MALLOC "Error: memory allocation failed.\n"
# define ERR_MUTEX  "Error: mutex init failed.\n"
# define ERR_THREAD "Error: thread creation failed.\n"


typedef struct s_simulation  t_table; /* forward declaration, philo has pointer to table but table is not defined yet */



typedef struct s_philo
{
	int	id;  /* n of the philosopher */              
	int	meals_eaten;  /* how many times this philo has eaten */
	long last_meal_time; /* timestamp (ms) of their most recent meal */
	pthread_t thread; /* philo thread */           
    int fork[2]; /* the two forks this philo needs, as indices into the table's fork array */

	pthread_mutex_t	meal_lock; /* mutex protects last_meal_time and meals_eaten, if not, data race */
    t_table *table; /* back-pointer to shared simulation data, no global variables allowed */
}	t_philo;


struct s_simulation
{
    int nb_philos;  /* number of philosophers */
    long time_to_die; 
    long time_to_eat;
    long time_to_sleep;
    int must_eat_count; 

    long start_time; /* when the simulation starts */
    bool simulation_over; /* when one is dead, when true, shuts down the simulation */

    pthread_t monitor;
    pthread_mutex_t *forks; /* forks -- they are the mutexes, each one represents one fork on the table */
	pthread_mutex_t	print_lock; /* one thread prints at a time*/
	pthread_mutex_t	sim_lock; /* protects reads and writes of sim_over */
	t_philo			*philos;
}  ;

bool	init_table(t_table *table, int argc, char **argv);
void	*dining_routine(void *data);
void	*monitor_routine(void *data);
bool	sim_is_over(t_table *table);
void	set_sim_over(t_table *table);
void	print_state(t_philo *philo, const char *msg);
long	now_ms(void);
void	smart_sleep(t_table *table, long duration_ms);
void	free_table(t_table *table);

#endif
