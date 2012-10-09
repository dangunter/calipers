/* Copyright 2012 The Regents of the University of California */
/* See COPYING for information about copying and redistribution.*/

/** \file nl_calipers.c
 * Efficient summarization of measured activites.
 */
static const volatile char rcsid[] = "$Id$";
 
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* time headers */
#if TIME_WITH_SYS_TIME
#    include <sys/time.h>
#    include <time.h>
#else
#    if HAVE_SYS_TIME_H
#        include <sys/time.h>
#    else
#        include <time.h>
#    endif
#endif

/* BSON */
#include "bson.h"

/* Interface */
#include "nl_calipers.h"

/* ---------------------------------------------------------------
 * Utility functions
 */

static int format_iso8601(struct timeval *tv, char *buf);

/**
 * Format a time per ISO8601.
 * Format is: YYYY-MM-DDThh:mm:ss.[fractional]
 *
 * @param tv Time
 * @param buf Buffer to format into (allocated)
 * @return 0 on success, -1 on error
 */
int format_iso8601(struct timeval *tv, char *buf)
{
    int i;
    long usec;
    struct tm *tm_p;
	time_t *time_p;

	time_p = (time_t *)&(tv->tv_sec);
	if ( NULL == time_p ) {
		goto error;
	}
	tm_p = gmtime(time_p);
    if (NULL == tm_p) {
        goto error;
	}

    if (0 == strftime(buf, 21, "%Y-%m-%dT%H:%M:%S.", tm_p))
        goto error;

    /* add in microseconds */
    usec = tv->tv_usec;
    for (i = 0; i < 6; i++) {
        buf[25 - i] = '0' + (usec % 10);
        usec /= 10;
    }

    /* add 'Z'  and trailing NUL */
    buf[26] = 'Z';
    buf[27] = '\0';

    return 27;

  error:
    return -1;
}

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

/* ---------------------------------------------------------------
 * Streaming variance methods
 */

#define WVAR_VARIANCE(W) \
(((W).count < (W).min_items) ? -1 : (W).t / ((W).count - 1))

#define WVAR_SD(W) \
(((W).count < (W).min_items) ? -1 : sqrt((W).t / ((W).count - 1)))

void netlogger_wvar_clear(struct netlogger_wvar_t *self) 
{
    self->m = self->t = 0.0;
    self->count = 0;
}

/* ---------------------------------------------------------------
 * Kahan sum methods
 */

void netlogger_ksum_clear(struct netlogger_ksum_t *self, double x0)
{
    self->s = x0;
    self->c = 0;
}

/* ---------------------------------------------------------------
 * NetLogger calipers methods
 */

#define T nlcali_T

static void
nl_calipers_hist_init(T self, unsigned n, double min, double width);
     
T nlcali_new(unsigned min_items)
{
    T self = (T)malloc(sizeof(struct nlcali_t));
    self->vsm.var.min_items = min_items;
    self->rsm.var.min_items = min_items;
    self->gsm.var.min_items = min_items;
    self->h_state = NL_HIST_OFF;
    self->h_rdata = self->h_gdata = NULL;
    nlcali_clear(self);
    return self;
}

/* Manual histogram */
void nlcali_hist_manual(T self, unsigned n, double min, double max)
{
    double width;
    
    self->h_state = NL_HIST_MANUAL;

    if (n > NL_MAX_HIST_BINS) {
        n = NL_MAX_HIST_BINS;
        /* TODO: Warn user! */
    }
    width = (max - min) / n;
    nl_calipers_hist_init(self, n, min, width);
}

/* Automatic bin-size histogram */
 void nlcali_hist_auto(T self, unsigned n, unsigned m)
 {
     self->h_state = NL_HIST_AUTO_PRE;
     self->h_auto_pre = m;
     nl_calipers_hist_init(self, n, DBL_MAX, 0);
 }


/* Internal method for shared constructor code. */
void nl_calipers_hist_init(T self, unsigned n, double min, double width)
{
    if (NULL != self->h_rdata) {
        free(self->h_rdata);
        self->h_rdata = NULL;
    }
    if (NULL != self->h_gdata) {
        free(self->h_gdata);
        self->h_gdata = NULL;
    }
    if (0 == n) {
        self->h_state = NL_HIST_OFF;
    }
    else {
        self->h_num = n;
        self->h_rmin = min;
        self->h_rwidth = width;
        self->h_rdata = (unsigned *)malloc(sizeof(unsigned) * n);
        memset(self->h_rdata, 0, sizeof(unsigned)*n);
        self->h_gmin = 1/min;
        self->h_gwidth = 1/width;
        self->h_gdata = (unsigned *)malloc(sizeof(unsigned) * n);
        memset(self->h_gdata, 0, sizeof(unsigned)*n);
    }
}

void nlcali_clear(T self)
{
    netlogger_ksum_clear(&self->vsm.ksum, 0);
    netlogger_ksum_clear(&self->rsm.ksum, 0);
    netlogger_ksum_clear(&self->gsm.ksum, 0);
    netlogger_wvar_clear(&self->vsm.var);
    netlogger_wvar_clear(&self->rsm.var);
    netlogger_wvar_clear(&self->gsm.var);
    self->vsm.sd = self->rsm.sd = self->gsm.sd = 0;
    self->vsm.min = self->rsm.min = self->gsm.min = DBL_MAX;
    self->vsm.max = self->rsm.max = self->gsm.max = 0;
    self->dur = self->dur_sum = 0;
    memset(&self->first, 0, sizeof(self->first));
    self->vsm.count = 0;
    self->rsm.count = 0;
    self->is_begun = 0;
    self->dirty = 0;
    if (self->h_state != NL_HIST_OFF) {
        /* clear histogram data */
        memset(self->h_rdata, 0, sizeof(unsigned)*self->h_num);
        memset(self->h_gdata, 0, sizeof(unsigned)*self->h_num);
    }
}

void nlcali_calc(T self)
{
    if (self->dirty && (self->vsm.count > 0)) {
        self->vsm.sum  = self->vsm.ksum.s;
        self->rsm.sum = self->rsm.ksum.s;
        self->gsm.sum = self->gsm.ksum.s;
        self->vsm.mean = self->vsm.sum / self->vsm.count;
        self->rsm.mean = self->rsm.sum / self->rsm.count;
        self->gsm.mean = self->gsm.sum / self->rsm.count;
        self->vsm.sd = WVAR_SD(self->vsm.var);
        self->rsm.sd = WVAR_SD(self->rsm.var);
        self->gsm.sd = WVAR_SD(self->gsm.var);
        self->dur = self->end.tv_sec - self->first.tv_sec + 
            (self->end.tv_usec - self->first.tv_usec) / 1e6;
        self->dirty = 0;
        if (self->h_state == NL_HIST_AUTO_PRE) {
            unsigned n;
            double min, max, width;
            n = self->h_num;
            /* choose min => lower of min and mean -3 standard deviations 
            unless no s.d. calculated yet, then just check vs. mean
            */
            min = self->rsm.sd < 0 ? MIN(self->rsm.min, self->rsm.mean) : 
                                  MIN(self->rsm.min, self->rsm.mean - 3*self->rsm.sd);
            /* merge w/previous min */
            if (self->h_rmin < DBL_MAX) {
                min = MIN(min, self->h_rmin);
            }
            /* choose max => higher of max and mean +3 standard deviations 
            unless no s.d. calculated yet, then just check vs. mean
            */
            max = self->rsm.sd < 0 ? MAX(self->rsm.max, self->rsm.mean) : 
                                  MAX(self->rsm.max, self->rsm.mean 
                                      + 3*self->rsm.sd);
            /* merge with previous max */
            if (self->h_rmin < DBL_MAX) {
                double old_max = self->h_rmin + self->h_rwidth*self->h_num;
                max = MAX(old_max, max);
            }
            /* recalculate binwidth */
            width = (max - min) / (double)n;
            /* check if done with pre-init stage */
            if (0 == --self->h_auto_pre) {
                nl_calipers_hist_init(self, n, min, width);
                self->h_state = NL_HIST_AUTO_READY;
            }
            /* otherwise just record range */
            else {
                self->h_rmin = min;
                self->h_rwidth = width;
            }
        }
        else if (self->h_state == NL_HIST_AUTO_READY) {
            self->h_state = NL_HIST_AUTO_FULL; /* ready to report */
        }
    }
}

#define LOG_BUFSZ 1024
char *nlcali_log(T self, const char *event)
{
    struct timeval now;
    char *msg;
    char *ts, *p;
    int msg_size, len;

    gettimeofday(&now, NULL);
    if (self->dirty) {
        nlcali_calc(self);
    }    
    msg_size = LOG_BUFSZ;
    p = msg = malloc(msg_size);
    if (NULL == p) {
        return NULL;
    }
    strcpy(p, "ts=");
    p += 3;
    len = format_iso8601(&now, p);
    if ( -1 == len ) goto error;
    p += len;
#define SF self /* alias, cosmetic */
    len = sprintf(p, " event=%s "
            "v.sum=%lf v.min=%lf v.max=%lf v.mean=%lf v.sd=%lf "
            "r.sum=%lf r.min=%lf r.max=%lf r.mean=%lf r.sd=%lf "
            "g.sum=%lf g.min=%lf g.max=%lf g.mean=%lf g.sd=%lf "
            "count=%lld dur=%lf dur.i=%lf",
            event,
            SF->vsm.sum, SF->vsm.min, SF->vsm.max, SF->vsm.mean, SF->vsm.sd, 
            SF->rsm.sum, SF->rsm.min, SF->rsm.max, SF->rsm.mean, SF->rsm.sd,
            SF->gsm.sum, SF->gsm.min, SF->gsm.max, SF->gsm.mean, SF->gsm.sd,
            SF->vsm.count, SF->dur, SF->dur_sum);
#undef SF
    if (-1 == len) goto error;
    p += len;
    /* histogram */
    if (NL_HIST_HAS_DATA(self)) {
        int i, need, avail;
        char numbuf[32];
        double h_rmax = self->h_rmin + self->h_rwidth * self->h_num;
        double h_gmax = self->h_gmin + self->h_gwidth * self->h_num;
        
        /* see if all values will fit */
        avail = msg_size - (int)(p - msg) - 1;
        need = 6*4; /* space + h.rd=, h.rw=, h.rm= h.rx= */
        need += sprintf(numbuf, "%lf", self->h_rmin);
        need += sprintf(numbuf, "%lf", h_rmax);
        need += sprintf(numbuf, "%lf", self->h_rwidth);
        for (i=0; i < self->h_num; i++) {
            need += 1 + sprintf(numbuf, "%d", self->h_rdata[i]);
        }
        need += 6*4; /* space + h.gd=, h.gw=, h.gm= h.gx= */
        need += sprintf(numbuf, "%lf", self->h_gmin);
        need += sprintf(numbuf, "%lf", h_gmax);
        need += sprintf(numbuf, "%lf", self->h_gwidth);
        for (i=0; i < self->h_num; i++) {
            need += 1 + sprintf(numbuf, "%d", self->h_gdata[i]);
        }
        /* if they fit, then add histogram */
        if (need <= avail) {
            /* - rate - */
            p += sprintf(p, " h.rm=%lf", self->h_rmin);
            p += sprintf(p, " h.rx=%lf", h_rmax);
            p += sprintf(p, " h.rw=%lf", self->h_rwidth);
            p += sprintf(p, " h.rd=");
            /* print each buckets' size */
            for (i=0; i < self->h_num-1; i++) {
                p += sprintf(p, "%d,", self->h_rdata[i]);
            }
            /* final one without trailing delimiter */
            p += sprintf(p, "%d", self->h_rdata[self->h_num - 1]);
            /* - gap - */
            p += sprintf(p, " h.gm=%lf", self->h_gmin);
            p += sprintf(p, " h.gx=%lf", h_gmax);
            p += sprintf(p, " h.gw=%lf", self->h_gwidth);
            p += sprintf(p, " h.gd=");
            /* print each buckets' size */
            for (i=0; i < self->h_num-1; i++) {
                p += sprintf(p, "%d,", self->h_gdata[i]);
            }
            /* final one without trailing delimiter */
            p += sprintf(p, "%d", self->h_gdata[self->h_num - 1]);
        }
    }
    return msg;
error:
    if (msg) free(msg);
    return NULL;
}

/**
 * Build perfSONAR data block.
 */
bson *nlcali_psdata(T self, const char *event, const char *m_id,
                                int32_t sample_num)
{
    struct timeval now;
    bson_buffer bb;
    bson *bp = NULL;
   
    assert(self && event && m_id);

    gettimeofday(&now, NULL);
    if (self->dirty) {
        nlcali_calc(self);
    }
    
    bson_buffer_init(&bb);
    bson_ensure_space(&bb, LOG_BUFSZ);
    
    bson_append_string(&bb, "mid", m_id);
    bson_append_start_array(&bb, "data");
    bson_append_double(&bb, "ts", now.tv_sec + now.tv_usec/1e6);
    bson_append_int(&bb, "_sample", sample_num);
    bson_append_double(&bb, "sum_v", self->vsm.sum);
    bson_append_double(&bb, "min_v", self->vsm.min);
    bson_append_double(&bb, "max_v", self->vsm.max);
    bson_append_double(&bb, "mean_v", self->vsm.mean);
    bson_append_double(&bb, "sd_v", self->vsm.sd);
    bson_append_double(&bb, "sum_r", self->rsm.sum);
    bson_append_double(&bb, "min_r", self->rsm.min);
    bson_append_double(&bb, "max_r", self->rsm.max);
    bson_append_double(&bb, "sd_r", self->rsm.sd);
    bson_append_double(&bb, "sum_g", self->gsm.sum);
    bson_append_double(&bb, "min_g", self->gsm.min);
    bson_append_double(&bb, "max_g", self->gsm.max);
    bson_append_double(&bb, "sd_g", self->gsm.sd);
    bson_append_int(&bb, "count", self->vsm.count);
    bson_append_double(&bb, "dur", self->dur);
    bson_append_double(&bb, "dur_inst", self->dur_sum);
    /* add histogram data, if being recorded */
    if (NL_HIST_HAS_DATA(self)) {
        int i;
        char idx[16];
        /* rate hist */
        bson_append_double(&bb, "h_rm", self->h_rmin);
        bson_append_double(&bb, "h_rw", self->h_rwidth);
        bson_append_start_array(&bb, "h_rd");
        for (i=0; i < self->h_num; i++) {
            sprintf(idx, "%d", i);
            bson_append_int(&bb, idx, self->h_rdata[i]);            
        }
        bson_append_finish_object(&bb);
        /* gap hist */
        bson_append_double(&bb, "h_gm", self->h_gmin);
        bson_append_double(&bb, "h_gw", self->h_gwidth);
        bson_append_start_array(&bb, "h_gd");
        for (i=0; i < self->h_num; i++) {
            sprintf(idx, "%d", i);
            bson_append_int(&bb, idx, self->h_gdata[i]);            
        }
        bson_append_finish_object(&bb);
    }

    bson_append_finish_object(&bb);

    bp = malloc(sizeof(bson));
    bson_from_buffer(bp, &bb);

    return(bp);

 error:
    if (bp) {
        bson_destroy(bp);
        free(bp);
    }
    bson_buffer_destroy(&bb);
    return(NULL);
}

void nlcali_free(T self)
{
    if (self) {
        free(self);
    }
}

#undef T
