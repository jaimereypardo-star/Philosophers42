#include "philo_bonus.h"

static bool	only_digits(char *str);
static bool	validate_args(int argc, char **argv);
static void	kill_all(t_table *table);
static void	start_simulation(t_table *table);
static long	ft_atol(const char *str);
static bool	init_semaphores(t_table *table);
static bool	init_philos(t_table *table);


int	main(int argc, char **argv)
{
	t_table	table;

	if (!validate_args(argc, argv))
		return (1);
	if (!init_table(&table, argc, argv))
		return (1);
	start_simulation(&table);
	free_table(&table);
	return (0);
}

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

static bool	validate_args(int argc, char **argv)
{
	int	i;

	if (argc < 5 || argc > 6)
		return (printf(ERR_ARGS), false);
	i = 1;
	while (i < argc)
	{
		if (!only_digits(argv[i]))
			return (printf("Error: '%s' is not a valid number.\n",
					argv[i]), false);
		i++;
	}
	return (true);
}

static void	kill_all(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->nb_philos)
	{
		kill(table->philos[i].pid, SIGKILL);
		i++;
	}
}

static void	start_simulation(t_table *table)
{
	int	i;

	i = 0;
	while (i < table->nb_philos)
	{
		table->philos[i].pid = fork();
		if (table->philos[i].pid == 0)
			philo_process(&table->philos[i]);
		i++;
	}
	sem_wait(table->dead_sem);
	kill_all(table);
	i = 0;
	while (i < table->nb_philos)
	{
		waitpid(table->philos[i].pid, NULL, 0);
		i++;
	}
}

/* --------------------------------------------- */

/* INIT PART  */


static long	ft_atol(const char *str)
{
	long	n;

	n = 0;
	while (*str)
	{
		n = n * 10 + (*str - '0');
		str++;
	}
	return (n);
}

static bool	init_semaphores(t_table *table)
{
	sem_unlink(SEM_FORKS);
	sem_unlink(SEM_PRINT);
	sem_unlink(SEM_DEAD);
	table->forks = sem_open(SEM_FORKS, O_CREAT, 0644, table->nb_philos);
	if (table->forks == SEM_FAILED)
		return (printf(ERR_SEM), false);
	table->print_sem = sem_open(SEM_PRINT, O_CREAT, 0644, 1);
	if (table->print_sem == SEM_FAILED)
		return (printf(ERR_SEM), false);
	table->dead_sem = sem_open(SEM_DEAD, O_CREAT, 0644, 0);
	if (table->dead_sem == SEM_FAILED)
		return (printf(ERR_SEM), false);
	return (true);
}

static bool	init_philos(t_table *table)
{
	int	i;

	table->philos = malloc(sizeof(t_philo) * table->nb_philos);
	if (!table->philos)
		return (printf(ERR_MALLOC), false);
	i = 0;
	while (i < table->nb_philos)
	{
		table->philos[i].id = i + 1;
		table->philos[i].meals_eaten = 0;
		table->philos[i].last_meal_time = table->start_time;
		table->philos[i].pid = 0;
		table->philos[i].table = table;
		i++;
	}
	return (true);
}

bool	init_table(t_table *table, int argc, char **argv)
{
	table->nb_philos = (int)ft_atol(argv[1]);
	table->time_to_die = ft_atol(argv[2]);
	table->time_to_eat = ft_atol(argv[3]);
	table->time_to_sleep = ft_atol(argv[4]);
	table->must_eat_count = -1;
	if (argc == 6)
		table->must_eat_count = (int)ft_atol(argv[5]);
	table->start_time = now_ms();
	table->philos = NULL;
	table->forks = NULL;
	table->print_sem = NULL;
	table->dead_sem = NULL;
	if (!init_semaphores(table) || !init_philos(table))
		return (false);
	return (true);
}

/* --------------------------------------------- */

/* Cleanup PART */

void	free_table(t_table *table)
{
	if (table->philos)
	{
		free(table->philos);
		table->philos = NULL;
	}
	if (table->forks)
		sem_close(table->forks);
	if (table->print_sem)
		sem_close(table->print_sem);
	if (table->dead_sem)
		sem_close(table->dead_sem);
	sem_unlink(SEM_FORKS);
	sem_unlink(SEM_PRINT);
	sem_unlink(SEM_DEAD);
}