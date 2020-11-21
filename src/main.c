#include "inputs.h"
#include "geometry_structure.h"
#include "delaunay.h"
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#define OPTSTR "vi:p:n:h"
#define USAGE_FMT  "%s \
[-v verbose] \
[-i input_file] \
[-o output_file] \
[-n number_of_points] \
[-p random_process] \
[-s smoothing_factor] \
[-a range_x for random points]\
[-b range_y for random points]\
[-h]\n"
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
		if(options.i != NULL) {
			printf("- Loading points from input file: %s\n", options.i);
		}
		else {
			printf("- Generating %d random points with a %s random process\n", options.n, options.p);
		}
		if(options.o != NULL) {
			printf("- Saving results to file: %s\n", options.o);
		}
	}
	// give a bit of entropy for the seed of rand()
	// or it will always be the same sequence
	int seed = (int) time(NULL);
	srand(seed);

	// we print the seed so you can get the distribution of points back
	if (options.v) printf("seed=%d\n", seed);

	bov_window_t* window = bov_window_new(800, 800, "My first BOV program");
	//glDisable(GL_CULL_FACE);
	bov_window_set_color(window, (GLfloat[]){0.9f, 0.85f, 0.8f, 1.0f});

	const GLsizei nPoints = (GLsizei) options.n;
	GLfloat (*coord)[2] = malloc(sizeof(coord[0]) * nPoints);

	if (strcmp(options.p, "normal") == 0) {
		random_points(coord, nPoints);
	}
	else if (strcmp(options.p, "uniform") == 0) {
		GLfloat min[2] = {0.0, 0.0};
		GLfloat max[2] = {1.0, 1.0};
		random_uniform_points(coord, nPoints, min, max);
	}
	else if (strcmp(options.p, "uniform-circle") == 0) {
		GLfloat min[2] = {0.0, 0.0};
		GLfloat max[2] = {1.0, 1.0};
		random_uniform_points_in_circle(coord, nPoints, min, max);
	}
	else if (strcmp(options.p, "polygon") == 0) {
		random_polygon(coord, nPoints, options.s);
	}
#if 1
	if (options.v) printf("BEGIN\n");

	DelaunayTriangulation *delTri;
	delTri = initDelaunayTriangulation(coord, nPoints);
	triangulateDT(delTri);

	if (options.v) describeDelaunayTriangulation(delTri);

	if (options.v) printf("DONE, will draw\n");

	drawDelaunayTriangulation(delTri, window);

	if (options.v) printf("END\n");

	if (options.v) describeDelaunayTriangulation(delTri);

	freeDelaunayTriangulation(delTri);
	if (options.v) printf("Freed the DelaunayTriangulation\n");
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
	bov_window_delete(window);

	return EXIT_SUCCESS;
}

void usage(char *progname, int opt) {
	fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
   	exit(EXIT_FAILURE);
   	/* NOTREACHED */
}
