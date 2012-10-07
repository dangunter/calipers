/*
 * Simple benchmark
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "nl_calipers.h"

static const volatile char rcsid[] = "$Id: ps_calipers_bench.c 27253 2011-02-26 17:46:48Z dang $";

#define MAX_WORK 1000

char *prog = NULL;

/* Subtract timeval 'S' from 'E' and return the 
 * number of seconds.
 */
#define SUBTRACT_TV(E,S) \
(((E).tv_sec - (S).tv_sec) + ((E).tv_usec - (S).tv_usec)/1e6)

void usage(const char *s) {
    fprintf(stderr,"%s\n"
            "usage: %s <total_sec> <report_sec> <work>(0..%d)\n", 
            s,  prog, MAX_WORK);
}

int *do_something(int n)
{
    static int a[MAX_WORK + 1];
    int i,j;
    
    for (i=0; i < n; a[i++] = 1);
    for (i=0; i < n; i++) {
        for (j=0; j < i; j++) {
            a[(i+j) % n ] = a[(i*j) % n ] + 1;
        }
    }
    return a;
}

void run(nlcali_T c, double dur, int work)
{
    int *p;
    
    do {
        nlcali_begin(c);
        p = do_something(work);
        nlcali_end(c, 12345);
    } while (SUBTRACT_TV(c->end, c->first) < dur);
}

void report(nlcali_T c, int is_first, int work)
{
    char *log_event;
    double d;
    bson *bdata;

    bdata = nlcali_psdata(c, "example", "METAID", 1);
    assert(bdata);
    if (is_first) {
        printf("wall,inst,noinst,rate,usec,pctovhd,work\n");
    }
    d = c->dur - c->dur_sum;
    printf("%lf,%lf,%lf,%lf,%lf,%lf,%d\n",
           c->dur, c->dur_sum, d, c->count/d, d / c->count*1e6, d/c->dur*100., work);
}

int main(int argc, char **argv)
{
    double sec = 0.0;
    double ttl = 0.0;
    double elapsed;
    int work = 0;
    int is_first = 1;
    struct timeval start;
    nlcali_T calipers;

    prog = argv[0];

    if (argc != 4) {
        usage("wrong num. of args");
        goto ERROR;
    }
    if (sscanf(argv[1], "%lf", &ttl) != 1 || ttl < 0) {
        usage("bad value for <total_sec>");
        goto ERROR;
    }
    if (sscanf(argv[2], "%lf", &sec) != 1 || sec < 0) {
        usage("bad value for <report_sec>");
        goto ERROR;
    }
    if (sscanf(argv[3], "%d", &work) != 1 || work < 1 || work >= MAX_WORK) {
        usage("bad value for <work>");
        goto ERROR;
    }

    gettimeofday(&start, 0);
    calipers = nlcali_new(1);
    do {
        run(calipers, sec, work);
        report(calipers, is_first, work);
        elapsed = SUBTRACT_TV(calipers->end, start);
        nlcali_clear(calipers);
        is_first = 0;
    } while(elapsed < ttl);
    nlcali_free(calipers);

    return 0;

 ERROR:
    return -1;
}
