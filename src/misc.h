/* @file misc.h
**
** miscellaneous definitions and function prototypes
** @@
******************************************************************************/

#ifndef MISC_H
#define MISC_H


#include <sys/resource.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include "error.h"

void* pthread_single(void* voidargs);
void pthread_db(core_t* core, db_t* db, void (*func)(core_t*,db_t*,int));



/* models */
uint32_t read_model(model_t* model, const char* file, uint32_t type);
uint32_t set_model(model_t* model, uint32_t model_id);

event_table getevents(size_t nsample, float* rawptr, int8_t rna);

refsynth_t *gen_ref(const char *genome, model_t *pore_model, uint32_t kmer_size, uint32_t rna, int32_t query_size);
void free_ref(refsynth_t *ref);

// taken from minimap2/misc
static inline double realtime(void) {
    struct timeval tp;
    struct timezone tzp;
    gettimeofday(&tp, &tzp);
    return tp.tv_sec + tp.tv_usec * 1e-6;
}

// taken from minimap2/misc
static inline double cputime(void) {
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_utime.tv_sec + r.ru_stime.tv_sec +
           1e-6 * (r.ru_utime.tv_usec + r.ru_stime.tv_usec);
}

//taken from minimap2
static inline long peakrss(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
#ifdef __linux__
	return r.ru_maxrss * 1024;
#else
	return r.ru_maxrss;
#endif
}

// Prints to the provided buffer a nice number of bytes (KB, MB, GB, etc)
//from https://www.mbeckler.org/blog/?p=114
static inline void print_size(const char* name, uint64_t bytes)
{
    const char* suffixes[7];
    suffixes[0] = "B";
    suffixes[1] = "KB";
    suffixes[2] = "MB";
    suffixes[3] = "GB";
    suffixes[4] = "TB";
    suffixes[5] = "PB";
    suffixes[6] = "EB";
    uint64_t s = 0; // which suffix to use
    double count = bytes;
    while (count >= 1024 && s < 7)
    {
        s++;
        count /= 1024;
    }
    if (count - floor(count) == 0.0)
        fprintf(stderr, "[%s] %s : %d %s\n", __func__ , name, (int)count, suffixes[s]);
    else
        fprintf(stderr, "[%s] %s : %.1f %s\n", __func__, name, count, suffixes[s]);
}

//replace u with t in a string
static inline void replace_char(char *str, char u, char t){
    while(*str){
        if(*str == u){
            *str = t;
        }
        str++;
    }
}

//read bed file

static inline char **read_bed_regions(char *bedfile, int64_t *count){

    FILE *bedfp = fopen(bedfile,"r");
    F_CHK(bedfp,bedfile);

    char* buffer = (char*)malloc(sizeof(char) * (100)); //READ+newline+nullcharacter
    MALLOC_CHK(buffer);

    int64_t reg_capcacity = 1024;
    int64_t reg_i = 0;
    char **reg_list = (char **)malloc(reg_capcacity * sizeof(char *));
    MALLOC_CHK(reg_list);


    size_t bufferSize = 100;
    ssize_t readlinebytes = 0;
    int64_t line_no = 0;


    while ((readlinebytes = getline(&buffer, &bufferSize, bedfp)) != -1) {

        char *ref = (char *)malloc(sizeof(char)*readlinebytes);
        MALLOC_CHK(ref);
        int64_t beg=-1;
        int64_t end=-1;

        //TODO can optimised though strtok etc later
        int ret=sscanf(buffer,"%s\t%ld\t%ld",ref,&beg, &end);
        if(ret!=3 || end<beg){
            ERROR("Malformed bed entry at line %ld",line_no);
            exit(EXIT_FAILURE);
        }

        if(reg_i>=reg_capcacity){
            if(reg_capcacity>1000000){
                WARNING("The region bed file has over %ld regions. To reduce memory usage, you may consider merging bed regions.",reg_i);
            }
            reg_capcacity=reg_capcacity*2;
            reg_list = (char **)realloc((void *)reg_list,reg_capcacity * sizeof(char *));
            MALLOC_CHK(reg_list);

        }

        reg_list[reg_i] = (char *)malloc(sizeof(char)*readlinebytes);
        sprintf(reg_list[reg_i],"%s:%ld-%ld",ref, beg, end);
        reg_i++;


        free(ref);



        line_no++;
    }

    fclose(bedfp);
    free(buffer);
    *count = reg_i;

    return reg_list;
}

static inline float *signal_in_picoamps(slow5_rec_t *rec){
    int16_t* rawptr = rec->raw_signal;
    float range = rec->range;
    float digitisation = rec->digitisation;
    float offset = rec->offset;
    int32_t nsample = rec->len_raw_signal;

    // convert to pA
    float *current_signal = (float*)malloc(sizeof(float) * nsample);
    MALLOC_CHK(current_signal);

    float raw_unit = range / digitisation;
    for (int32_t j = 0; j < nsample; j++) {
        current_signal[j] = ((float)rawptr[j] + offset) * raw_unit;
    }

    return current_signal;
}


#define TO_PICOAMPS(RAW_VAL,DIGITISATION,OFFSET,RANGE) (((RAW_VAL)+(OFFSET))*((RANGE)/(DIGITISATION)))

#define SIGFISH_MEAN_VAL 104.6
#define SIGFISH_STDV_VAL 20.39
#define SIGFISH_WINDOW_SIZE 2000
#define SIGFISH_SIZE 1000

typedef struct {
    int64_t x;
    int64_t y;
} pair_t;

pair_t find_polya(float *raw, int64_t nsample, float top, float bot);
pair_t find_adaptor(slow5_rec_t *rec);
float meanf(float *x, int n);
float stdvf(float *x, int n);
float medianf(float *x, int n);
#endif
