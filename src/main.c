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
#define USAGE_FMT  "%s [-v] [-p number_of_points] [-n smoothing_factor] [-h]\n"
#define ERR_FOPEN_INPUT  "fopen(input, r)"
#define ERR_FOPEN_OUTPUT "fopen(output, w)"
#define ERR_DO_THE_NEEDFUL "do_the_needful blew up"
#define DEFAULT_PROGNAME "lmeca2710_project"

extern int errno;
extern char *optarg;
extern int opterr, optind;

typedef struct options_t {
	int p;
	int n;
} options_t;


void usage(char *progname, int opt);

int main(int argc, char *argv[])
{
	int opt;
	options_t options = {500, 4};

	// Inspired from:
	// https://opensource.com/article/19/5/how-write-good-c-main-function
	while ((opt = getopt(argc, argv, OPTSTR)) != EOF) {
		switch(opt) {
        	case 'p':
				options.p = atoi(optarg);
              	break;
			case 'n':
				options.n = atoi(optarg);
				break;
			case 'h':
			default:
          		usage(basename(argv[0]), opt);
              	/* NOTREACHED */
              	break;
			}
       }

	unsigned int n = (unsigned int) options.p;

	Point *points = newRandomPoints(n);

	for(int i = 0; i < n; i++) {
		printPoint(&points[i]);
	}

	printf("Sorted\n");

	qsort(points, n, sizeof(Point), comparePoints);

	for(int i = 0; i < n; i++) {
		printPoint(&points[i]);
	}

	free(points);

	// give a bit of entropy for the seed of rand()
	// or it will always be the same sequence
	int seed = (int) time(NULL);
	srand(seed);

	// we print the seed so you can get the distribution of points back
	printf("seed=%d\n", seed);

	bov_window_t* window = bov_window_new(800, 800, "My first BOV program");
	bov_window_set_color(window, (GLfloat[]){0.9f, 0.85f, 0.8f, 1.0f});

	const GLsizei nPoints = (GLsizei) options.p;
	GLfloat (*coord)[2] = malloc(sizeof(coord[0])*nPoints);
#if 1 // put 1 for random polygon
	random_polygon(coord, nPoints, options.n);
#else
	random_points(coord, nPoints);
#endif

	printf("BEGIN\n");

	DelaunayTriangulation *delTri = initDelaunayTriangulation(coord, nPoints);
	drawDelaunayTriangulation(delTri, window);

	printf("END\n");

	bov_points_t *coordDraw = bov_points_new(coord, nPoints, GL_STATIC_DRAW);
	bov_points_set_color(coordDraw, (GLfloat[4]) {0.0, 0.0, 0.0, 1.0});
	bov_points_set_outline_color(coordDraw, (GLfloat[4]) {0.3, 0.12, 0.0, 0.25});

	while(!bov_window_should_close(window)){
		bov_points_set_width(coordDraw, 0.003);
		bov_points_set_outline_width(coordDraw, 0.002);
		bov_line_loop_draw(window, coordDraw, 0, nPoints);

		// points_set_width(coordDraw, 0.003);
		bov_points_set_outline_width(coordDraw, -1.);
		bov_points_draw(window, coordDraw, 0, nPoints);

		bov_window_update(window);
	}

	bov_window_screenshot(window, "test.png");

	bov_points_delete(coordDraw);
	free(coord);
	bov_window_delete(window);

	return EXIT_SUCCESS;
}

void usage(char *progname, int opt) {
	fprintf(stderr, USAGE_FMT, progname ? progname : DEFAULT_PROGNAME);
   	exit(EXIT_FAILURE);
   	/* NOTREACHED */
}
