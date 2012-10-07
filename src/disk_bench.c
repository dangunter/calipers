/**
 * Test nl calipers on some local disk perf.
 *
 * Dan Gunter, Sep 2012
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netlogger_calipers.h"

 static const volatile char rcsid[] = "$Id: disk_bench.c 32915 2012-10-06 11:53:27Z dang $";

#define BLOCK1MB 1048576
#define BLOCK 131072
#define RPT_INTERVAL 1000

typedef enum { DISK=0, MEMORY=1 } device_t;
typedef enum { CSV=0, LOG=1 } output_t;

static void usage(const char *msg)
{
    float rpt_mb = 1. * RPT_INTERVAL * BLOCK / BLOCK1MB;
    
    fprintf(stderr,"%s\n", msg);
    fprintf(stderr,"Usage: disk_bench MODE SIZE OUTPUT\n", msg);
    fprintf(stderr,"-   MODE: dd=disk/disk, dm=disk/mem, md=mem/disk, "
                   "mm=mem/mem\n");
    fprintf(stderr,"-   SIZE: data set size in MB\n");
    fprintf(stderr,"-   OUTPUT: output type c=csv, n=netlogger\n");
    fprintf(stderr,"Reports will occur every %.1fMB (%d operations)\n",
             rpt_mb, RPT_INTERVAL);
}


static void write_output(struct netlogger_calipers_t **nl, output_t outp)
{
    static int write_num = 0;
    int i, j;

    write_num++;
    
    if (outp == CSV && write_num == 1) {
        printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
               "op","i","ts", "dur","nbin","bin",
               "rbs","rbe","rcount",
               "gbs","gbe","gcount");
    }
    
    for (i=0; i < 2; i++) {
        struct netlogger_calipers_t *nlp = nl[i];
        netlogger_calipers_calc(nlp);
        if (NL_HIST_HAS_DATA(nlp)) {
            if (outp == LOG) {
                printf("%s\n",netlogger_calipers_log(nlp,
                    i ? "dbench.write" : "dbench.read"));
            }
            else {
                for (j=0; j < nlp->h_num; j++) {
                    double rbin_s, rbin_e, gbin_s, gbin_e;
                    rbin_s = nlp->h_rmin + nlp->h_rwidth * j;
                    rbin_e = rbin_s + nlp->h_rwidth;
                    gbin_s = nlp->h_gmin + nlp->h_gwidth * j;
                    gbin_e = gbin_s + nlp->h_gwidth;
                    printf("%s,%d,%lf,%lf,%d,%d,%lf,%lf,%d,%lf,%lf,%d\n",
                           i ? "write" : "read",
                           write_num,
                           nlp->begin.tv_sec + nlp->begin.tv_usec/1e6,
                           nlp->dur,
                           nlp->h_num,
                           j + 1,
                           rbin_s,
                           rbin_e,
                           nlp->h_rdata[j],
                           gbin_s,
                           gbin_e,
                           nlp->h_gdata[j]);
                }
            }
            netlogger_calipers_clear(nlp);
        }
    }
}

static int run(device_t src, device_t dst, int size, output_t outp)
{
    FILE *fp, *src_fp, *dst_fp;
    char buf1[BLOCK1MB];
    char src_buf[BLOCK], dst_buf[BLOCK];
    int i, j;
    struct netlogger_calipers_t *nl[2];
    
    /* init */
    if (src == DISK) {
        fp = fopen("/tmp/disk_bench_read.dat","w");
        for (i=0; i < size; i++) {
            fwrite(buf1, BLOCK1MB, 1, fp);
        }
        fclose(fp);
        src_fp = fopen("/tmp/disk_bench_read.dat", "r");
    }
    if (dst == DISK) {
        dst_fp = fopen("/tmp/disk_bench_write.dat","w");
    }
    /* NL init, src + dst */
    for (i=0; i < 2; i++) {
        nl[i] = netlogger_calipers_new(RPT_INTERVAL - 1);
        netlogger_calipers_hist_auto(nl[i], 20, 3);
    }

    /* go */
    for (i=0; i < size * (BLOCK1MB / BLOCK); i++) {
        /* read/write operation */
        if (src == DISK) {
            netlogger_calipers_begin(nl[0]);
            fread(src_buf, BLOCK, 1, src_fp);
            netlogger_calipers_end(nl[0], BLOCK*1.);
        }
        else {
            netlogger_calipers_begin(nl[0]);
            memset(src_buf, BLOCK, i);
            netlogger_calipers_end(nl[0], BLOCK*1.);
        }
        if (dst == DISK) {
            netlogger_calipers_begin(nl[1]);
            fwrite(src_buf, BLOCK, 1, dst_fp);
            netlogger_calipers_end(nl[1], BLOCK*1.);
        }
        else {
            netlogger_calipers_begin(nl[1]);
            memcpy(dst_buf, src_buf, BLOCK);
            netlogger_calipers_end(nl[1], BLOCK*1.);      
        }
        /* report */
        if (i > 0 && 0 == (i % RPT_INTERVAL)) {
            write_output(nl, outp);
        }
    }
    
    /* final report */
    write_output(nl, outp);
    
    return 0;
}

int main(int argc, char **argv)
{
    device_t dev[2];
    int size = -1;
    int status = 0;
    output_t outp;
    
    /* parse args */
    if (argc != 4) {
        usage("Wrong # arguments");
    } 
    else if (!strcmp(argv[1], "-h")) {
        usage("Show help");
    }
    else if (strlen(argv[1]) != 2){
        usage("Bad mode");
    }
    else {
        int i;
        /* parse mode */
        for (i=0; i < 2; i++) {
            switch(argv[1][i]) {
                case 'd': dev[i] = DISK; break;
                case 'm': dev[i] = MEMORY; break;
                default: usage("Bad mode");
            }
        }
        /* parse size */
        size = atoi(argv[2]);
        if (size <= 0) {
            usage("Bad size, must be positive integer");
        }
    }
    /* parse output */
    switch(argv[3][0]) {
        case 'c':
        case 'C': outp = CSV; break;
        case 'n':
        case 'N': outp = LOG; break;
        default: usage("Bad output type");
    }
    /* If parsing was OK, then run */
    if (size > 0) {
        status = run(dev[0], dev[1], size, outp);
    }
    return(status);
}
