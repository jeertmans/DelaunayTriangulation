#include "inputs.h"
#include "delaunay.h"
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#define OPTSTR "vi:i:o:n:p:s:a:b:t:di:r:h"
#define USAGE_FMT  "\
Delaunay Triangulation.\n\
Jérome Eertmans, 2020.\n\n\
Usage:\n\
\t./%s [options] [...]\n\
\tpython3 main.py     [options] [...]\n\
\tpython3 main.py     -n 10\n\
\tpython3 main.py     -p uniform-circle -a 2\n\
\tpython3 main.py     -n 1000000 -d -v -r 0\n\
\tpython3 main.py     -n 30 -t 5\n\
\tpython3 main.py     -i input/cowboy.txt\n\
\n\
Options:\n\
\t[-v verbose] adds some verbosity to the program execution\n\
\t[-i input_file=NULL] if present, will read points from this file where first line must be the number of points, and the next lines must match format %%lf%%lf\n\
\t[-o output_file=NULL] if present, will save the last status of the DelaunayTriangulation, where the first line will contain \"(n_points, n_lines_points)\", then all the points (first the points, and the line points)\n\
\t[-n number_of_points=500] number of random points\n\
\t[-p random_process=normal] normal, uniform(-circle), or polygon\n\
\t[-s smoothing_factor=4] smoothing applied on random polygon \n\
\t[-a x_axis=1] x span (double) when generating uniform(-circle) random points\n\
\t[-b y_axis=1] y span (double) when generating uniform(-circle) random points\n\
\t[-t total_time=20] estimated total time (in seconds) for the animation, keep in mind that speed is limited by the refresh time\n\
\t[-d disable_drawing] disables drawing\n\
\t[-r remove_duplicates=1] removes duplicated points (will add overhead), you can disable it to improve performances\n\
\t[-h] displays help and exits\n"
#define ERR_FOPEN_INPUT  "fopen(input, r)"
#define ERR_FOPEN_OUTPUT "fopen(output, w)"
#define ERR_DO_THE_NEEDFUL "do_the_needful blew up"
#define DEFAULT_PROGNAME "lmeca2710_project"

#define WHITE_BACKGROUND 1  // Change to 1 to have a white background (might be better to export images)

extern int errno;
extern char *optarg;
extern int opterr, optind;

typedef struct options_t {
	int v;
	char *i;
	char *o;
	int n;
	int s;
	char *p;
	double a;
	double b;
	double t;
	int d;
	int r;
} options_t;


void usage(char *progname, int opt);

int main(int argc, char *argv[])
{
	int opt;
	options_t options = {	// Default values
		0,			// Verbosity
		NULL,		// Input file
		NULL,		// Output file
		500,		// Number of points
		4,			// Smoothing factor
		"normal",	// Random process
		1.0,		// By default, will generate points
		1.0,		// in a perfect square (or circle)
		20e6,		// Total animation time in micro seconds
		0,			// By default, we draw
		1,			// We remove duplicates
	};

	// Inspired from:
	// https://opensource.com/article/19/5/how-write-good-c-main-function
	while ((opt = getopt(argc, argv, OPTSTR)) != EOF) {
		switch(opt) {
			case 'p':
				if (strcmp(optarg, "normal") == 0) {
					options.p = optarg;
				}
				else if (strcmp(optarg, "uniform") == 0) {
					options.p = optarg;
				}
				else if (strcmp(optarg, "uniform-circle") == 0) {
					options.p = optarg;
				}
				else if (strcmp(optarg, "polygon") == 0) {
					options.p = optarg;
				}
				else {
					printf("Unknown random process: %s\n"
						   "Please choose one among:\n"
						   "\t- normal\n"
						   "\t- uniform\n"
						   "\t- uniform-circle\n"
						   "\t- polygon\n",
						   optarg);
					exit(EXIT_FAILURE);
				}
        	case 's':
				options.s = atoi(optarg);
              	break;
			case 'n':
				options.n = atoi(optarg);
				break;
			case 'i':
				options.i = optarg;
				break;
			case 'o':
				options.o = optarg;
				break;
			case 'a':
				options.a = strtod(optarg, NULL);
				break;
			case 'b':
				options.b = strtod(optarg, NULL);
				break;
			case 't':
				options.t = strtod(optarg, NULL) * 1e6;
				break;
			case 'd':
				options.d = 1;
				break;
			case 'r':
				options.r = atoi(optarg);
				break;
			case 'v':
				options.v = 1;
				break;
			case 'h':
			default:
          		usage(basename(argv[0]), opt);
              	/* NOTREACHED */
              	break;
			}
		}

	if (options.v) {
		printf("[STEP 1] Points generation.\n");
		if(options.i != NULL) {
			printf("Loading points from input file: %s.\n", options.i);
		}
		else {
			printf("Generating %d random points with a %s random process.\n", options.n, options.p);
		}
	}
	// give a bit of entropy for the seed of rand()
	// or it will always be the same sequence
	int seed = (int) time(NULL);
	srand(seed);

	// we print the seed so you can get the distribution of points back
	if (options.v) printf("seed=%d\n", seed);

	GLsizei n_points;
	GLfloat (*points)[2];

	double x, y;

	if (options.i != NULL) {
		FILE *file_in = fopen(options.i, "r");
		if (file_in == NULL) {
			printf("ERROR: File %s not found\n", options.i);
			exit(EXIT_FAILURE);
		}
		char useless[256];
		if (fscanf(file_in, "%d%[^\n]\n", &n_points, useless) == 0) {
			printf("ERROR: First line doesn't match  %%d format\n");
			exit(EXIT_FAILURE);
		}

		points = malloc(sizeof(points[0]) * n_points);

		for (GLsizei i = 0; i < n_points; i++) {
			if (fscanf(file_in, "%lf %lf", &x, &y) != 2) {
				printf("ERROR: Line %d could not parse x[%d], y[%d]\n", i+1, i, i);
				exit(EXIT_FAILURE);
			}
			points[i][0] = (GLfloat) x;
			points[i][1] = (GLfloat) y;
		}

		fclose(file_in);
	}
	else {
		n_points = (GLsizei) options.n;
		points = malloc(sizeof(points[0]) * n_points);

		if (strcmp(options.p, "normal") == 0) {
			random_points(points, n_points);
		}
		else if (strcmp(options.p, "uniform") == 0) {
			GLfloat min[2] = {0.0, 0.0};
			GLfloat max[2] = {options.a, options.b};
			random_uniform_points(points, n_points, min, max);
		}
		else if (strcmp(options.p, "uniform-circle") == 0) {
			GLfloat min[2] = {0.0, 0.0};
			GLfloat max[2] = {options.a, options.b};
			random_uniform_points_in_circle(points, n_points, min, max);
		}
		else if (strcmp(options.p, "polygon") == 0) {
			random_polygon(points, n_points, options.s);
		}
	}

	if (options.v) printf("[STEP 2] DelaunayTriangulation\n");

	DelaunayTriangulation *delTri;

	clock_t begin = clock();

	delTri = initDelaunayTriangulation(points, n_points, options.r);

	if (options.v) {
		printf("DelaunayTriangulation structure was allocated in %.6f s.\n",
			   (double) (clock() - begin) / CLOCKS_PER_SEC);
	}

	begin = clock();

	triangulateDT(delTri);

	if (options.v) {
		printf("DelaunayTriangulation was computed in %.6f s.\n",
			   (double) (clock() - begin) / CLOCKS_PER_SEC);
	}

	if (options.v) describeDelaunayTriangulation(delTri);

	if (options.v) printf("[STEP 3] Drawing\n");

	if (!options.d) {
		bov_window_t* window = bov_window_new(-1, 1, "DelaunayTriangulation - Jérome Eertmans");

#if WHITE_BACKGROUND
		bov_window_set_color(window, (GLfloat[]){1.0f, 1.f, 1.0f, 1.0f});
#else
		bov_window_set_color(window, (GLfloat[]){0.9f, 0.85f, 0.8f, 1.0f});
#endif
		drawDelaunayTriangulation(delTri, window, options.t);
		bov_window_delete(window);
	}
	else {
		if (options.v) printf("Drawing step skipped.\n");
	}

	if (options.v) printf("[STEP 4] Final state\n");

	if (options.v) describeDelaunayTriangulation(delTri);

	if (options.o != NULL) {
		FILE *file_out = fopen(options.o, "w");

		if (file_out == NULL) {
			printf("ERROR: Couldn't open file %s in write mode.\n", options.o);
		}

		fsaveDelaunayTriangulation(delTri, file_out);

		fclose(file_out);
	}

	freeDelaunayTriangulation(delTri);
	if (options.v) printf("DelaunayTriangulation structure freed.\n");

	free(points);

	return EXIT_SUCCESS;
}

void usage(char *progname, int opt) {
	fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
   	exit(EXIT_FAILURE);
   	/* NOTREACHED */
}
