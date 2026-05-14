#include "philo.h"

/* declaring functions in this file - son privadas para main*/
static bool	only_digits(char *str);
static bool	validate_args(int argc, char **argv);
static bool	start_simulation(t_table *table);
static void	stop_simulation(t_table *table);


/* literally alquilando espacio de el heap to the stack, as we dont know how big will be the simulation for holding the data */
int	main(int argc, char **argv)
{
	t_table	table;

	if (!validate_args(argc, argv))
		return (1);
	if (!init_table(&table, argc, argv))
		return (1);
	if (!start_simulation(&table))   
	{
		free_table(&table);
		return (1);
	}
	stop_simulation(&table);
	return (0);
}

/* checking if the input is not a digit  */
static bool	only_digits(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (false);
		i++;
	}
	return (str[0] != '\0');
}

/* handles having the correct number of arguments */
static bool	validate_args(int argc, char **argv)
{
	int	i;

	if (argc < 5 || argc > 6)
	{
		printf(ERR_ARGS);
		return (false);
	}
	i = 1;
	while (i < argc)
	{
		if (!only_digits(argv[i]))
		{
			printf("Error: '%s' is not a valid positive number.\n", argv[i]);
			return (false);
		}
		i++;
	}
	return (true);
}

/* despertando a cada filosofo, creas threads y enciendes monitor so watch everyone and check if they die */
static bool	start_simulation(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->nb_philos)
	{
		if (pthread_create(&table->philos[i].thread, NULL,
				dining_routine, &table->philos[i]) != 0)
		{
			printf(ERR_THREAD);
			return (false);
		}
		i++;
	}
	if (table->nb_philos > 1)
	{
		if (pthread_create(&table->monitor, NULL,
				monitor_routine, table) != 0)
		{
			printf(ERR_THREAD);
			return (false);
		}
	}
	return (true);
}
 /* once everyone is done, free memory */
static void	stop_simulation(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->nb_philos)
	{
		pthread_join(table->philos[i].thread, NULL);
		i++;
	}
	if (table->nb_philos > 1)
		pthread_join(table->monitor, NULL);
	free_table(table);
}

/* --------------------------------------------- */

/* INIT PART  */

static long		ft_atol(const char *str);
static void		assign_forks(t_philo *philo, int nb_philos);
static bool		init_philos(t_table *table);
static bool		init_forks(t_table *table);


/* setting up. the table (world) before anyone sits down. Literally just preparing the table for the dinner */

bool	init_table(t_table *table, int argc, char **argv)  /* returns true if setup success. pointer to the table, int argc is just the number of arguments y el otro seria para setear cada argumento un index  */
{
	table->nb_philos = (int)ft_atol(argv[1]);
	table->time_to_die = ft_atol(argv[2]);
	table->time_to_eat = ft_atol(argv[3]);
	table->time_to_sleep = ft_atol(argv[4]);
	table->must_eat_count = -1;
	if (argc == 6) /* in case Kenny test the last argument */
		table->must_eat_count = (int)ft_atol(argv[5]);
	table->simulation_over = false;
	table->start_time = now_ms();
	table->forks = NULL; /* we use NULL because MALLOC */
	table->philos = NULL;
	if (pthread_mutex_init(&table->print_lock, NULL) != 0
		|| pthread_mutex_init(&table->sim_lock, NULL) != 0)
	{
		printf(ERR_MUTEX);
		return (false);
	}
	if (!init_forks(table) || !init_philos(table)) /* calls forks and philos to build them (los reales), si fallan, you stop. */ 
		return (false);
	return (true);
}

/* seat each philosopher and give them the information */
static bool	init_philos(t_table *table)
{
	int	i;

	table->philos = malloc(sizeof(t_philo) * table->nb_philos);
	if (!table->philos)
	{
		printf(ERR_MALLOC);
		return (false);
	}
	i = 0;
	while (i < table->nb_philos)
	{
		table->philos[i].id = i + 1; /* numbering the philos, as index is 0, add plus 1 because one cannot be the zero philosopher*/
		table->philos[i].meals_eaten = 0; /* anyone has eaten */
		table->philos[i].last_meal_time = table->start_time; /* set the moment the simulations starts that all the philos have eaten, if not everyone will die as the monitor dont know where they last eat   */
		table->philos[i].table = table; /* given them the game, rules and map. with this pointer, any philo can access shared rules, forks, everything. FIXING GLOBAL VARIABLES    */
		assign_forks(&table->philos[i], table->nb_philos); /* decider of which two forks will be assigned to each philo   */
		if (pthread_mutex_init(&table->philos[i].meal_lock, NULL) != 0)
		{
			printf(ERR_MUTEX);
			return (false);
		}
		i++;
	}
	return (true);
}


/* putting the forks in the table*/

static bool	init_forks(t_table *table)
{
	int	i;

	table->forks = malloc(sizeof(pthread_mutex_t) * table->nb_philos); /* using the memory allocation MALLOC, fork is a mutex, ONLY1 can have it */ 
	if (!table->forks)
	{
		printf(ERR_MALLOC);
		return (false);
	}
	i = 0;
	while (i < table->nb_philos)
	{
		if (pthread_mutex_init(&table->forks[i], NULL) != 0) /* creating the lock for each fork */
		{
			printf(ERR_MUTEX);
			return (false);
		}
		i++;
	}
	return (true);
}

/*  decide which forks cada filosofo reaches for y en que ORDEN(preventing deadlock) */
/* basically setting that ODD nb_philos grab RIGHT and then LEFT. EVEN nb_philos grab LEFT and then RIGHT */
static void	assign_forks(t_philo *philo, int nb_philos)
{
	int	id;

	id = philo->id - 1;
	philo->fork[0] = id;
	philo->fork[1] = (id + 1) % nb_philos;
	if (philo->id % 2 != 0)
	{
		philo->fork[0] = (id + 1) % nb_philos;
		philo->fork[1] = id;
	}
}

/* turn a word(string) into number. ejemplo cuando escriben 800 en terminal, the program lo recibe como una palabra, no como un numero.  */
static long	ft_atol(const char *str)
{
	long	n;

	n = 0;
	while (*str)
	{
		n = n * 10 + (*str - '0');  /* if input 800, then the word will be 8, so 800 will be 8 * 100 + 0 * 10 + 0 */
		str++;
	}
	return (n);
}











/* --------------------------------------------- */

/* Cleanup PART */

/* give everything back, everylock destroyed as it could end up in memory leak*/

static void	destroy_philos(t_table *table) /* pack up philos */
{
	int	i;
 
	if (!table->philos) /* if no philos, leave */
		return ;
	i = 0;
	while (i < table->nb_philos) /*  destroy each philo's meal lock BEFORE freeing the memory */
	{
		pthread_mutex_destroy(&table->philos[i].meal_lock);
		i++;
	}
	free(table->philos); /* return the rented memory block */
	table->philos = NULL; /* set pointer to null to avoid dangling pointer, as the memory is not ours anymore, we dont want to accidentally use it */
}
 
static void	destroy_forks(t_table *table)
{
	int	i;
 
	if (!table->forks)
		return ;
	i = 0;
	while (i < table->nb_philos)
	{
		pthread_mutex_destroy(&table->forks[i]);
		i++;
	}
	free(table->forks);
	table->forks = NULL;
}
 
void	free_table(t_table *table) /* master cleaner. there is no free table as the table lives on the stack */
{
	destroy_philos(table);
	destroy_forks(table);
	pthread_mutex_destroy(&table->print_lock);
	pthread_mutex_destroy(&table->sim_lock);
}
 