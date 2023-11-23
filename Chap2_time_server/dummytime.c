#include <stdio.h>
#include <time.h>

/*****************
 * main - entry point
 * Before going on network program, it is always useful
 * to have a simple CLI functionning program of what we intend to do
 * Here trivial timer display
 */

int main()
{
	time_t timer;
	time(&timer);

	/* Getting the time with built-in time() function */
	/* Converts to string with ctime() */

	printf("Local time is: %s\n", ctime(&timer));

	return (0);
}
