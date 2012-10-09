/* Copyright 2012 The Regents of the University of California */
/* See COPYING for information about copying and redistribution.*/

/** \file nl_calipers.h
 * Efficient summarization of measured activites.
 */
 
#include <string.h> /* for memcpy() in macro */
#include "bson.h"

#ifndef NETLOGGER_CALIPERS_INCLUDED
#    define NETLOGGER_CALIPERS_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------
 * Data structures
 */

/* Limit max # of histogram bins */
#define NL_MAX_HIST_BINS 100

struct netlogger_wvar_t {
	double m;
	double t;
	unsigned count;
	unsigned min_items;
};

struct netlogger_ksum_t {
	double s, c, y, t;
};

typedef enum { 
     NL_HIST_OFF=0,
     NL_HIST_AUTO_PRE=1, /* past this point, add data */
     NL_HIST_MANUAL=2,
     NL_HIST_AUTO_READY=3,
     NL_HIST_AUTO_FULL=4
} netlogger_hstate_t;

/**
 * Summary statistics for caliper metrics.
 */
struct summary_t {
    double sum;      /**< sum of values */
    double min;      /**< smallest value */
    double max;      /**< largest value */
    double mean;     /**< mean value (=sum/count) */
    double sd;       /**< standard deviation of value */
    long long count; /**< count of values */
    /* internal variables */
    struct netlogger_wvar_t var; /* streaming variance */
    struct netlogger_ksum_t ksum; /* Kahan sum */
};

#define T nlcali_T

/** Hold current values for a single "caliper".
 * 
 * Each "caliper" tracks statistics for a univariate time-series.
 * Summary statistics tracked include the count, sum, mean, min, max,
 * and standard deviation.
 */
struct nlcali_t {
    struct summary_t vsm; /**< summary of: value */
    struct summary_t rsm; /**< summary of: duration/value (rate) */
    struct summary_t gsm;  /** summary of: value/duration (gap) */
    double dur_sum, dur;
    /* internal variables */
    unsigned is_begun, dirty;
    struct timeval begin, end, first;
    /* histogram */
    netlogger_hstate_t h_state;
    int h_auto_pre; /* pre-init phases left */
    unsigned int h_num; /* #bins, 0=none */
    double h_rmin, h_rwidth, h_gmin, h_gwidth;
    unsigned *h_rdata, *h_gdata;
};

/** Type definition for pointer to Caliper struct.
 */
typedef struct nlcali_t *T;

/* ---------------------------------------------------------------
 * Methods
 */

/**
 * Calculate histogram bin.
 *
 * First arg is the calipers struct, second is the value,
 * third is return value of the bin it belongs in.
 */
#define NL_HBIN_R(S, V, R) do {                                     \
     if ((V) < (S)->h_rmin) { (R) = 0; }                            \
     else {                                                         \
         (R) = (unsigned)(((V) - (S)->h_rmin) / (S)->h_rwidth);     \
         if ((R) >= (S)->h_num) { (R) = (S)->h_num - 1; }           \
     }                                                              \
} while(0)

#define NL_HBIN_G(S, V, R) do {                                     \
     if ((V) < (S)->h_gmin) { (R) = 0; }                            \
     else {                                                         \
         (R) = (unsigned)(((V) - (S)->h_gmin) / (S)->h_gwidth);     \
         if ((R) >= (S)->h_num) { (R) = (S)->h_num - 1; }           \
     }                                                              \
} while(0)

#define NL_WVAR_ADD(W,X) do {                       \
	    double q,r;                                 \
        if ((W).count == 0) {                       \
            (W).m = (X);                            \
            (W).t = 0.0;                            \
        }                                           \
        else {                                      \
            q = (X) - (W).m;                        \
            r = q / ((W).count + 1);                \
            (W).m = (W).m + r;                      \
            (W).t = (W).t + (W).count * q * r;      \
        }                                           \
        (W).count++;                                \
    } while(0)

#define NL_KSUM_ADD( V, X ) do {                \
        (V).y = X - (V).c;                      \
        (V).t = (V).s + (V).y;                  \
        (V).c = ( (V).t - (V).s ) - (V).y;      \
        (V).s = (V).t;                          \
} while(0)

/** 
 * Constructor.
 * Allocates memory and clears values.
 *
 * \param baseline Minimum number of values to get a standard deviation.
 * \return New Calipers object
 */
T nlcali_new(unsigned baseline);

/**
 * \brief Calculate histogram of calculated gap and rate.
 *
 * Bin sizes are given in terms of rate (value / nanosecond).
 * This version sets the histogram bins manually.
 *
 * \param self Calipers object
 * \param n Number of bins, if zero turn off histogram.
 * \param min Value at left edge of first bin
 * \param max Value at right edge of last bin
 * \post May be called multiple times, but destroys
 *       previous data when called.
 */
void nlcali_hist_manual(T self, unsigned n, double min, double max);

/**
 * Calculate histogram of calculated gap and rate.
 *
 * This version sets the histogram bins automatically.
 *
 * May be called multiple times, but destroys
 * previous data when called.
 *
 * \param self Calipers object
 * \param n Number of bins, if zero turn off histogram.
 * \param pre Number of pre-init runs, to set ranges
 */
void nlcali_hist_auto(T self, unsigned n, unsigned pre);

/* Check if histogram has data to show */ 
#define NL_HIST_HAS_DATA(X) (\
 (X)->h_state == NL_HIST_MANUAL || \
 (X)->h_state == NL_HIST_AUTO_FULL)

/** 
 * \brief Begin a timed event.
 * Modifies the input argument in-place.
 * Defined as a macro for performance.
 *
 * \param S Calipers obj
 * \return None
 */
#define nlcali_begin(S)  do {                               \
        gettimeofday(&(S)->begin, NULL);                                \
        if ((S)->vsm.count == 0) {                                          \
            memcpy(&(S)->first, &(S)->begin, sizeof((S)->first));       \
        }                                                               \
        (S)->is_begun = 1;                                              \
    } while(0)

/** 
 * End a timed event.
 * Modifies the input argument in-place.
 * Defined as a macro for performance.
 *
 * \param S Calipers obj
 * \param V Value of event
 * \return None
 */
#define nlcali_end(S,V) do {                                    \
    if ((S)->is_begun) {                                        \
        register double dur, rate, gap;                         \
        gettimeofday(&(S)->end, NULL);                          \
        dur = (S)->end.tv_sec - (S)->begin.tv_sec +             \
            ((S)->end.tv_usec - (S)->begin.tv_usec) / 1e6;      \
        (S)->dur_sum += dur;                                    \
        NL_KSUM_ADD(((S)->vsm.ksum), (V));                      \
        NL_WVAR_ADD((S)->vsm.var, (V));                         \
        if ((V) < (S)->vsm.min) (S)->vsm.min = (V);             \
        if ((V) > (S)->vsm.max) (S)->vsm.max = (V);             \
        if ((V) != 0 && dur > 0) {                              \
            gap = (1e9*dur) / (V);                              \
            rate = (V) / (1e9*dur);                             \
            NL_KSUM_ADD(((S)->rsm.ksum), rate);                 \
            NL_WVAR_ADD((S)->rsm.var, rate);                    \
            NL_KSUM_ADD(((S)->gsm.ksum), gap);                  \
            NL_WVAR_ADD((S)->gsm.var, gap);                     \
            if (rate < (S)->rsm.min) (S)->rsm.min = rate;       \
            if (gap < (S)->gsm.min) (S)->gsm.min = gap;         \
            if (rate > (S)->rsm.max) (S)->rsm.max = rate;       \
            if (gap > (S)->gsm.max) (S)->gsm.max = gap;         \
            (S)->rsm.count++;                                   \
            if ((S)->h_state > NL_HIST_AUTO_PRE) {              \
                int i;                                          \
                NL_HBIN_R(S, rate, i);                          \
                (S)->h_rdata[i]++;                              \
                NL_HBIN_G(S, gap, i);                           \
                (S)->h_gdata[i]++;                              \
            }                                                   \
        }                                                       \
        (S)->vsm.count++;                                       \
        (S)->is_begun = 0;                                      \
        (S)->dirty = 1;                                         \
    }                                                           \
} while(0)

/**
 * \brief Calculate values for all events so far.
 *
 * Need to call this before accessing sums, means and deviations.
 *
 * \post Sums, means, deviations, etc. are ready for retrieval
 *        and/or output. Calling this multiple times has no
 *        effect.
 * \param self Calipers object
 * \return None
 */
void nlcali_calc(T self);

/**
 * \brief Return NetLogger BP log message.
 *
 * For general BP format, see (...). All the statistics
 * are for a single interval.
 *
 * Values in the log message have the following attributes:
 *
 *   - ts: Time of start of event
 *
 *   - event: Name of event, given by user
 * 
 *   - {metric}.sum: Sum of metric
 *
 *   - {metric}.min: Minimum of metric
 *
 *   - {metric}.max: Maximum of metric
 *
 *   - {metric}.mean: Mean of metric
 *
 *   - {metric}.sd: Standard deviation of metric
 *     where {metric} is one of:
 *   
 *        - 'v' = value
 *       
 *        - 'g' = gap: time_interval(ns)/value
 *     
 *        - 'r' = rate: value/time_interval(ns)
 *
 *   - count: Number of samples
 * 
 *   - dur: Wallclock duration (seconds)
 *
 *   - dur.inst: Total time spent between calipers start/end (seconds)
 *
 * \post As if nlcali_calc() was called
 * \param self Calipers
 * \param event NetLogger event name
 * \return Heap-allocated string, does NOT have a newline at the end.
 */
char *nlcali_log(T self, const char *event);

/**
 * \brief Build perfSONAR data block.
 *
 * This block is a BSON object with the following structure:
 *
 * The attributes have the same meanings as in the log message
 * returned by nlcali_log().
 *
 * \post As if nlcali_calc was called
 * \param self Calipers
 * \param event perfSONAR event type
 * \param m_id Metadata ID
 * \param sample_num Sample number 
 * \return BSON buffer
 */
bson *nlcali_psdata(T self, const char *event, const char *m_id,
                            int32_t sample_num);

/** 
 * Clear all values in bucket.
 * Do this before restarting a new time-series.
 *
 * \param self Calipers
 * \return None
 */
void nlcali_clear(T self);

/** 
 * Free memory for bucket.
 *
 * \param self Calipers
 * \return None
 */
void nlcali_free(T self);

/** @} */

#undef T

#ifdef __cplusplus
}
#endif
#endif /* NETLOGGER_CALIPERS_INCLUDED */
