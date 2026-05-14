#include "philo.h"

/* Mutex PART */
/* if two threads touch the exact variable at the same exact microsecond, the program will crash*/ 


bool	sim_is_over(t_table *table) /* simulation ends sign, is simulation over? */
{
	bool	result;

	pthread_mutex_lock(&table->sim_lock);  /* locking the simulation mutex, so if another thread tries to finish the simulation, he will wait until the lock is available */
	result = table->simulation_over; /* save what you read andn return the lock, as someone else could change so you want a copy */
	pthread_mutex_unlock(&table->sim_lock);
	return (result);
}

void	set_sim_over(t_table *table) /* writing the sign "simulation over", only the monitor call this */
{
	pthread_mutex_lock(&table->sim_lock);
	table->simulation_over = true;
	pthread_mutex_unlock(&table->sim_lock);
}

void	print_state(t_philo *philo, const char *msg) /* only one philo speaks at a time - avoiding everyone printing at the same time */
{
	pthread_mutex_lock(&philo->table->print_lock);
	if (!sim_is_over(philo->table)) /* check if someone died, and the simulation ended, stay quiet */
		printf("%ld %d %s\n", now_ms() - philo->table->start_time,
			philo->id, msg);
	pthread_mutex_unlock(&philo->table->print_lock);
}


/* --------------------------------------------- */

/* Time PART */

long	now_ms(void)
{
	struct timeval	tv;
 
	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000 + (long)tv.tv_usec / 1000);
}
 
void	smart_sleep(t_table *table, long duration_ms) /* sleep but one eye open - need to wake up early as maybe one philo died while you were sleeping*/ 
{
	long	wake_at; /* calculate the exact moment we should wake up, two conditions- 1. simulation is over, 2. time is up */
 
	wake_at = now_ms() + duration_ms; 
	while (now_ms() < wake_at)
	{
		if (sim_is_over(table))
			break ;
		usleep(100); /* mini naps of 100ms as usleep is not a perfect alarm clock. you cant interrupt it */
	}
}
 

/* --------------------------------------------- */

/* Philo routines PART*/

static void	*lonely_philo(t_philo *philo);
static void	philo_eat(t_philo *philo);
static void	philo_sleep(t_philo *philo);
static void	philo_think(t_philo *philo);

void	*dining_routine(void *data)
{
	t_philo	*philo;

	philo = (t_philo *)data; /* personal data */
	if (philo->table->must_eat_count == 0)
		return (NULL);
	if (philo->table->nb_philos == 1) /* special case, only one philo */
		return (lonely_philo(philo));
	if (philo->id % 2 != 0) /* special case if odd philos, wait 1ms to grab the fork so prevent deadlock of taking all the right one */
		smart_sleep(philo->table, 1);
	while (!sim_is_over(philo->table)) /* philo life */
	{
		philo_eat(philo);
		philo_sleep(philo);
		philo_think(philo);
	}
	return (NULL);
}

static void	philo_eat(t_philo *philo)
{
	t_table	*t;

	t = philo->table;
	pthread_mutex_lock(&t->forks[philo->fork[0]]);
	print_state(philo, "has taken a fork");
	pthread_mutex_lock(&t->forks[philo->fork[1]]);
	print_state(philo, "has taken a fork");
	print_state(philo, "is eating");
	pthread_mutex_lock(&philo->meal_lock); /* updating meal data in lock */
	philo->last_meal_time = now_ms();
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_lock);
	smart_sleep(t, t->time_to_eat); /* eating duration, forks still held as you eating  */
	pthread_mutex_unlock(&t->forks[philo->fork[1]]); /* puting both forks down */
	pthread_mutex_unlock(&t->forks[philo->fork[0]]);
}

static void	philo_sleep(t_philo *philo)
{
	print_state(philo, "is sleeping");
	smart_sleep(philo->table, philo->table->time_to_sleep);
}

static void	philo_think(t_philo *philo)
{
	print_state(philo, "is thinking");
	smart_sleep(philo->table, 1); /* 1ms pause to be polite with the CPU */
}

static void	*lonely_philo(t_philo *philo) /* sad  */
{
	pthread_mutex_lock(&philo->table->forks[0]);
	print_state(philo, "has taken a fork");
	smart_sleep(philo->table, philo->table->time_to_die); /* take the fork and wait till death */
	print_state(philo, "died");
	pthread_mutex_unlock(&philo->table->forks[0]);
	return (NULL);
}


/* --------------------------------------------- */


/* Monitor routines PART */


static bool	philo_has_died(t_philo *philo);
static bool	all_ate_enough(t_table *table);

void	*monitor_routine(void *data) /* monitor shift: for each philo, asks if they have died or eaten enough. If yes, stop, wait 1ms and do it all again*/
{ 
	t_table	*table;
	int		i;

	table = (t_table *)data;
	while (true)
	{
		i = 0;
		while (i < table->nb_philos)
		{
			if (philo_has_died(&table->philos[i]))
				return (NULL);
			i++;
		}
		if (all_ate_enough(table))
		{
			set_sim_over(table);
			return (NULL);
		}
		usleep(1000); /* without this, the monitor would consume 100% of the CPU */
	}
	return (NULL);
}

static bool	philo_has_died(t_philo *philo)  
{
	long	since_last_meal;

	pthread_mutex_lock(&philo->meal_lock); /* read safely under meal lock */ 
	since_last_meal = now_ms() - philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_lock);
	if (since_last_meal >= philo->table->time_to_die) /* if gap time last meal bigger than time to die, they are dead*/ 
	{
		pthread_mutex_lock(&philo->table->print_lock);
		printf("%ld %d died\n",
			now_ms() - philo->table->start_time, philo->id);
		set_sim_over(philo->table);
		pthread_mutex_unlock(&philo->table->print_lock); /* only unlock now has the death message is the last one sent */ 
		return (true);
	}
	return (false);
}

static bool	all_ate_enough(t_table *table) /* is everyone full*/ 
{
	int	i;
	int	done;

	if (table->must_eat_count == -1) /* if meal limit not given, return false */ 
		return (false);
	i = 0;
	done = 0;
	while (i < table->nb_philos) /* go through every philosopher and count how many have eaten enough times */ 
	{
		pthread_mutex_lock(&table->philos[i].meal_lock);
		if (table->philos[i].meals_eaten >= table->must_eat_count)
			done++;
		pthread_mutex_unlock(&table->philos[i].meal_lock);
		i++;
	}
	return (done == table->nb_philos); /* Only when every single philo has eaten enough,return true. One person not being done means the party continue */
}

