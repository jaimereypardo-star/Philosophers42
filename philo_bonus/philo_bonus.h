#ifndef PHILO_BONUS_H
# define PHILO_BONUS_H

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <unistd.h>
# include <sys/time.h>
# include <sys/wait.h>
# include <signal.h>
# include <semaphore.h>
# include <fcntl.h>
# include <pthread.h>

# define ERR_ARGS   "Usage: philo_bonus N time_to_die time_to_eat time_to_sleep [must_eat]\n"
# define ERR_MALLOC "Error: memory allocation failed.\n"
# define ERR_SEM    "Error: semaphore failed.\n"

# define SEM_FORKS  "/philo_forks"
# define SEM_PRINT  "/philo_print"
# define SEM_DEAD   "/philo_dead"

typedef struct s_simulation	t_table;

typedef struct s_philo
{
	int		id;
	int		meals_eaten;
	long	last_meal_time;
	pid_t	pid;
	t_table	*table;
}	t_philo;

struct s_simulation
{
	int		nb_philos;
	long	time_to_die;
	long	time_to_eat;
	long	time_to_sleep;
	int		must_eat_count;
	long	start_time;
	t_philo	*philos;
	sem_t	*forks;
	sem_t	*print_sem;
	sem_t	*dead_sem;
};

bool	init_table(t_table *table, int argc, char **argv);
void	free_table(t_table *table);
long	now_ms(void);
void	smart_sleep(long duration_ms);

/* routines_bonus.c */
void	philo_process(t_philo *philo);

#endif
