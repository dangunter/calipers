/*
 * Example of netlogger calipers usage.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "nl_calipers.h"

static const volatile char rcsid[] = "$Id: nlcali_example.c 32903 2012-10-03 00:23:22Z dang $";

void usage(const char *s) {
    fprintf(stderr,"%s\n"
            "usage: nlcali_example [N=num. loops] [M=work/loop]\n", s);
}

#define MAX_WORK 1000
int *do_something(int n)
{
	static int a[MAX_WORK];
	int i,j;

    for (i=0; i < n; a[i++] = 1);
    for (i=0; i < n; i++) {
		for (j=0; j < i; j++) {
			a[(i+j) % n ] = a[(i*j) % n ] + 1;
		}
	}
	return a;
}

void run(nlcali_T c, int numloops, int workperloop)
{
    int i, *p;
    for (i=0; i < numloops; i++) {
        nlcali_begin(c);
        p = do_something(workperloop);
        nlcali_end(c, 1.234);
    }
}

void report(nlcali_T c)
{
    char *log_event;
    double d;
    int i;
    bson *bdata;

    nlcali_calc(c);
    printf("Value:\n");
    printf("    sum=%lf mean=%lf\n", c->sum, c->mean);
    log_event = nlcali_log(c, "example");
    assert(log_event);
    printf("Log event:\n%s\n", log_event);
    /* XXX: why? fwrite(log_event, 1024, 1, stdout); */
    printf("\n");
    free(log_event);
    bdata = nlcali_psdata(c, "example", "METAID", 1);
    assert(bdata);
    printf("Perfsonar data block (ASCII):\n");
    bson_print(bdata);
    fflush(stdout);
    bson_destroy(bdata);
    free(bdata);
    /* printf("Histogram:\n");
    for (i=0; i < c->h_num; i++) {
        unsigned count = c->h_data[i];
        double p = c->h_min + c->h_width * i;
        printf("%d [%lf - %lf]: %d\n", i, p, p + c->h_width, count);
    }*/
    printf("Overhead:\n");
    d = c->dur - c->dur_sum;
    printf("    begin/end pairs per sec: %lf\n", c->count / d);
    printf("    usec per begin/end pair: %lf\n", d / c->count * 1e6);
    printf("    %%overhead: %lf\n", d / c->dur * 100.);
}

int main(int argc, char **argv)
{
    int n = 0;
	int m = 0;
    int min_items = 0;
    int i;
    nlcali_T calipers;

    if (argc > 1) {
        if (sscanf(argv[1], "%d", &n) != 1 || n < 0) {
            usage("first argument (iterations), "
                  "if present, must be a positive integer");
            goto ERROR;
        }
    }
    else {
        n = 100;
    }
	if (argc > 2) {
		if (sscanf(argv[2], "%d", &m) != 1 || m < 1 || m >= MAX_WORK) {
			usage("second argument (work per iteration), "
			      "if present, must be a positive integer less than 1000");
			goto ERROR;
		}
	}
	else {
		m = 100;
	}
    printf("Run %d iterations\n", n);
    calipers = nlcali_new(1);
    
    printf("With manual histogram\n");
    /* note: rate = 1e9*dur / value, so if value =~ 1 and dur
       expected up to 0.001, rate =~ 1e6 */
    nlcali_hist_manual(calipers, 100, 0, 1e6);
    run(calipers, n, m);
    printf("Results, with manual histogram\n");
    report(calipers);

    printf("With auto histogram (2 runs)\n");
    nlcali_hist_auto(calipers, 100, 1);
    run(calipers, n, m);
    nlcali_calc(calipers);
    nlcali_clear(calipers);
    run(calipers, n, m);    
    printf("Results, with auto histogram\n");
    report(calipers);

    printf("Without histogram\n");
    nlcali_hist_manual(calipers, 0, 0, 0);
    run(calipers, n, m);
    printf("Results, without histogram\n");
    report(calipers);

	nlcali_free(calipers);
    return 0;

 ERROR:
    return -1;
}
