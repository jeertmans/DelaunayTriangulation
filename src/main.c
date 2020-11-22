#include "inputs.h"
#include "geometry_structure.h"
#include "delaunay.h"
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#define OPTSTR "vi:i:o:n:p:s:a:b:di:h"
#define USAGE_FMT  "%s [-param value] ...\n\
All the parameters below are optionnal:\n\
\t[-v verbose] adds some verbosity to the program execution\n\
\t[-i input_file] if present, will read points from this file where first line must be the number of points, and the next lines must match format %%lf%%lf\n\
\t[-o output_file]\n\
\t[-n number_of_points] number of random points\n\
\t[-p random_process] normal, uniform(-circle), or polygon\n\
\t[-s smoothing_factor] smoothing applied on random polygon \n\
\t[-a x_axis] x span (double) when generating uniform(-circle) random points\n\
\t[-b y_axis] y span (double) when generating uniform(-circle) random points\n\
\t[-d disable_drawing] disable drawing\n\
\t[-h] displays help and exits\n"
#define ERR_FOPEN_INPUT  "fopen(input, r)"
#define ERR_FOPEN_OUTPUT "fopen(output, w)"
#define ERR_DO_THE_NEEDFUL "do_the_needful blew up"
#define DEFAULT_PROGNAME "lmeca2710_project"

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
	int d;
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
		0,			// By default, we draw
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
						   "please choose one among:\n"
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
			case 'd':
				options.d = 1;
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
		}
		if (fscanf(file_in, "%d", &n_points) != 1) {
			printf("ERROR: First line doesn't match  %%d format\n");
		}

		points = malloc(sizeof(points[0]) * n_points);

		for (GLsizei i = 0; i < n_points; i++) {
			if (fscanf(file_in, "%lf%lf", &x, &y) != 2) {
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
#if 1
	if (options.v) printf("[STEP 2] DelaunayTriangulation\n");

	DelaunayTriangulation *delTri;

	clock_t begin = clock();

	delTri = initDelaunayTriangulation(points, n_points);

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
		//glDisable(GL_CULL_FACE);
		bov_window_set_color(window, (GLfloat[]){0.9f, 0.85f, 0.8f, 1.0f});
		drawDelaunayTriangulation(delTri, window);
		bov_window_delete(window);
	}
	else {
		printf("Drawing step skipped.\n");
	}

	if (options.v) printf("[STEP 4] Final state\n");

	if (options.v) describeDelaunayTriangulation(delTri);

	freeDelaunayTriangulation(delTri);
	if (options.v) printf("DelaunayTriangulation structure freed.\n");
#else

	bov_points_t *coordDraw = bov_points_new(coord, nPoints, GL_STATIC_DRAW);
	bov_points_set_color(coordDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_outline_color(coordDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});

	while(!bov_window_should_close(window)){
		bov_points_set_width(coordDraw, 0.003);
		bov_points_set_outline_width(coordDraw, 0.002);


		// points_set_width(coordDraw, 0.003);
		bov_points_set_outline_width(coordDraw, -1.);
		bov_points_draw(window, coordDraw, 0, nPoints);

		bov_window_update(window);
	}

	bov_points_delete(coordDraw);
	//free(coord);
#endif
	free(points);

	return EXIT_SUCCESS;
}

void usage(char *progname, int opt) {
	fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
   	exit(EXIT_FAILURE);
   	/* NOTREACHED */
}
