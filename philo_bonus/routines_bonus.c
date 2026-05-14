
#include "philo_bonus.h"

/* Monitor routines PART */

static void	*death_watcher(void *data)
{
	t_philo	*philo;

	philo = (t_philo *)data;
	while (1)
	{
		if (now_ms() - philo->last_meal_time >= philo->table->time_to_die)
		{
			sem_wait(philo->table->print_sem);
			printf("%ld %d died\n",
				now_ms() - philo->table->start_time, philo->id);
			sem_post(philo->table->dead_sem);
			exit(0);
		}
		usleep(500);
	}
	return (NULL);
}

/* --------------------------------------------- */

/* Philo routines PART*/

static void	print_state(t_philo *philo, const char *msg)
{
	sem_wait(philo->table->print_sem);
	printf("%ld %d %s\n", now_ms() - philo->table->start_time,
		philo->id, msg);
	sem_post(philo->table->print_sem);
}

static void	philo_eat(t_philo *philo)
{
	sem_wait(philo->table->forks);
	print_state(philo, "has taken a fork");
	sem_wait(philo->table->forks);
	print_state(philo, "has taken a fork");
	print_state(philo, "is eating");
	philo->last_meal_time = now_ms();
	philo->meals_eaten++;
	smart_sleep(philo->table->time_to_eat);
	sem_post(philo->table->forks);
	sem_post(philo->table->forks);
}

static void	lonely_philo(t_philo *philo)
{
	print_state(philo, "has taken a fork");
	smart_sleep(philo->table->time_to_die);
	print_state(philo, "died");
	sem_post(philo->table->dead_sem);
}

void	philo_process(t_philo *philo)
{
	pthread_t	watcher;
	long		stagger;

	if (philo->table->must_eat_count == 0)
	{
		if (philo->id == 1)
			sem_post(philo->table->dead_sem);
		exit(0);
	}
	if (philo->table->nb_philos == 1)
	{
		lonely_philo(philo);
		exit(0);
	}
	stagger = (philo->id - 1) * (philo->table->time_to_eat
			/ philo->table->nb_philos + 1);
	smart_sleep(stagger);
	pthread_create(&watcher, NULL, death_watcher, philo);
	pthread_detach(watcher);
	while (1)
	{
		philo_eat(philo);
		if (philo->table->must_eat_count != -1
			&& philo->meals_eaten >= philo->table->must_eat_count)
		{
			sem_post(philo->table->dead_sem);
			exit(0);
		}
		print_state(philo, "is sleeping");
		smart_sleep(philo->table->time_to_sleep);
		print_state(philo, "is thinking");
	}
}

/* --------------------------------------------- */

/* Time PART */

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000 + (long)tv.tv_usec / 1000);
}

void	smart_sleep(long duration_ms)
{
	long	wake_at;

	wake_at = now_ms() + duration_ms;
	while (now_ms() < wake_at)
		usleep(100);
}