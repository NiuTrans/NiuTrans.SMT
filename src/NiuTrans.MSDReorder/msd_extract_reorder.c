/* NiuTrans - SMT platform
 * Copyright (C) 2011, NEU-NLPLab (http://www.nlplab.com/). All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/*
 * $Id:
 * MSD reorder model trainer; msd_extract_reorder.c
 *
 * $Version:
 * 0.1.0
 *
 * $Created by:
 * Rushan Chen (email: chenrsster@gmail.com)
 *
 * $Last Modified by:
 * Hao Zhang (email: zhanghao1216@gmail.com); June 24th, 2011; add option "-p", change model output format -- prob => log(prob)
 * Hao Zhang (email: zhanghao1216@gmail.com); June 19th, 2011;
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#ifndef WIN32
#include <getopt.h>
#include <signal.h>
#include <sys/wait.h>
#endif /* WIN32 */

#ifdef WIN32
#define __func__ __FUNCTION__
#endif /* WIN32 */

/*
 * ========================= some function call macros ========================= */

#define FOPEN(fp, filename, mode) \
{ \
        if (((fp) = fopen((filename), (mode))) == NULL) \
                errexit("[ERROR]: failed to open %s in %s: %s\n", filename, \
                                __func__, strerror(errno)); \
}

#define PRINT_PROCESS(prog, big_stop, small_stop) \
{ \
        if ((prog) % (big_stop) == 0) \
                fprintf(stderr, "[%d]", progress + 1); \
        else if ((prog) % (small_stop) == 0) \
                fprintf(stderr, ". "); \
        fflush(stderr); \
}

#define READ_ONE_LINE(fp, buf, buf_len) \
{ \
        char *ret = NULL; \
        if ((ret = read_one_line((fp), &(buf), &(buf_len))) == NULL) \
                errexit("[ERROR]: read_one_line error in %s: %s\n", \
                                        __func__, strerror(errno)); \
        if (ret == (char *)-1) \
                errexit("[ERROR]: read_one_line error in %s: read error\n", \
                                                        __func__); \
}

#define STRSTR_TO_FIELD_SEP(to, from, sep) \
{ \
        if (((to) = strstr((from), (sep))) == NULL) \
                errexit("[ERROR]: no field separator found in %s\n", __func__); \
}

#ifdef WIN32
#define ISSPACE(c) my_isspace((c))
#else
#define ISSPACE(c) isspace((c))
#endif /* WIN32 */

/* =============================================================================
 */

/*
 * ============================ some default values ============================ */

#define FILE_NAME_LEN 1024
#define DEF_MAX_PHRASE_LEN 7
#define DEF_REORDER_TBL_NAME "reordering-table"
#define DEF_REORDER_TBL_FILTER_NAME "reordering-table.filtered"
#define TMP_PHRASE_TABLE_PREFIX "smt_tmp_phrase_table"
#define FIELD_SEP " ||| "
#define FILTER_WITH_TRANTBL 1
#define FILTER_MSD_SUM_EQ_ONE 2

/* =============================================================================
 */

typedef short stridx_t;         /* string index type */

struct phrasepair {
        char *pp_f_phra;
        char *pp_e_phra;
        stridx_t pp_fs, pp_fe;
        stridx_t pp_es, pp_ee;
        stridx_t pp_pM, pp_pS, pp_pD;
        stridx_t pp_nM, pp_nS, pp_nD;
        struct phrasepair *pp_s_next;
        struct phrasepair *pp_e_next;
};

struct sentn_info {
        /* In this struct definition, "size" means the total available entries
           in a given array, "len" means entries that are currently in use */
        char **words;           /* an array with each entry containing a pointer to 
                                   each separate word */
        stridx_t words_size;
        stridx_t words_len;
        stridx_t **alns;        /* 2D array containing alignment info coming from
                                   this sentence, each entry which is itself an
                                   array contains alignment info for word with 
                                   same index as this entry.
                                   Note that "alns_size" is equal to "words_size",
                                   "alns_len" is equal to "words_len", so they're
                                   both omitted */
        stridx_t *aln_sizes;    /* for the align array of one word, the size of this
                                   array is equal to <words_len> */
        stridx_t *aln_lens;
        stridx_t *aln_min;      /* store the min align, if there's no alignment
                                   corresponding to a word, the entry will be -1,
                                   this and <aln_max> are mainly used for 
                                   saving computation time */
        stridx_t *aln_max;
        struct phrasepair **pp_start_arr; 
                                /* each entry stores a list of <struct phrasepair>
                                   whose start point of the e phrase is the same as
                                   the index of this entry. In fact, this array for
                                   f sentn is currently of no use */
        struct phrasepair **pp_end_arr;   
                                /* each entry stores a list of <struct phrasepair>
                                   whose end point of the e phrase is the same as 
                                   the index of this entry. In fact, this array for
                                   f sentn is currently of no use */
};

/*
 * ================================ GLOBAL VARIABLES ============================= */

int if_convert_input_from_gb_to_utf8;
int if_msd_calculate_once;
int if_swap_fe_align;
int if_need_num_of_phrapair;
int if_filter;                  /* set to specify in which way the final reordering
                                   model will be filtered, 0 means no filtering */
int max_phra_len;               /* limit the length of final f and e phrase */
int max_extract_phra_len;       /* limit the length of f and e phrase during 
                                   extracting, INT_MAX means no limit, so that 
                                   the whole sentence is also a phrase */
int msd_model = 1;

FILE *tmp_phratbl_fp;

/* {{{ input files */ 
char f_name[FILE_NAME_LEN], e_name[FILE_NAME_LEN], aln_name[FILE_NAME_LEN];
char filter_trantbl_name[FILE_NAME_LEN]; 
                                /* this file presents all phrase pairs needed,
                                   it's used to filter the reordering model */
/* }}} */

/* {{{ temp files */
char tmp_phratbl_name[FILE_NAME_LEN];
char tmp_phratbl_sorted_name[FILE_NAME_LEN];
char tmp_phratbl_combined_name[FILE_NAME_LEN];
char tmp_phratbl_filtered_name[FILE_NAME_LEN];
char filter_trantbl_sorted_name[FILE_NAME_LEN];
char filter_trantbl_swapped_name[FILE_NAME_LEN];
/* }}} */

/* {{{ output files */
char reordertbl_name[FILE_NAME_LEN];
char reordertbl_filtered_name[FILE_NAME_LEN];
/* }}} */

FILE *f_fp, *e_fp, *aln_fp;
char *f_line, *e_line, *aln_line; /* buffers used for line input */
int f_line_len, e_line_len, aln_line_len;

int pM_ttl, pS_ttl, pD_ttl, nM_ttl, nS_ttl, nD_ttl;
double pM_factor, pS_factor, pD_factor, nM_factor, nS_factor, nD_factor;

long total_tmp_phrase_pair;
long total_phrase_pair;

struct sentn_info f_sent, e_sent;

int step = 1;
char cmd_buf[4096];

char* g_sort = "C:\\cygwin\\bin\\sort.exe";

/* ================================================================================
 */

#ifdef WIN32

/* 
 * ================================================================================
 *                            code for option handling 
 *         (from http://note.sonots.com/EngNote/CompLang/cpp/getopt.html)         */ 

#define ERR(s, c)    if(opterr){\
    char errbuf[2];\
    errbuf[0] = c; errbuf[1] = '\n';\
    fputs(argv[0], stderr);\
    fputs(s, stderr);\
    fputc(c, stderr);}

int opterr = 1;
int optind = 1;
int optopt;
char *optarg;

int getopt(int argc, char **argv, char *opts)
{
    static int sp = 1;
    int c;
    char *cp;

    if(sp == 1) {
        if(optind >= argc ||
           argv[optind][0] != '-' || argv[optind][1] == '\0')
            return -1;
        else if(!strcmp(argv[optind], "--")) {
            optind++;
            return -1;
        }
        }
    optopt = c = argv[optind][sp];
    if(c == ':' || (cp=strchr(opts, c)) == NULL) {
        ERR(": illegal option -- ", c);
        if(argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return('?');
    }
    if(*++cp == ':') {
        if(argv[optind][sp+1] != '\0')
            optarg = &argv[optind++][sp+1];
        else if(++optind >= argc) {
            ERR(": option requires an argument -- ", c);
            sp = 1;
            return('?');
        } else
            optarg = argv[optind++];
        sp = 1;
    } else {
        if(argv[optind][++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    return(c);
}

/* ================================================================================
 */

static int my_isspace(int c)
{
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                return 1;
        return 0;
}

#endif /* WIN32 */

/* 
 * This signal handler is used to delete all the temp files that are left when
 * this program exit abnormally, like being interrupted by Ctrl-C.
 */
void sig_del_tmp_files(int signo)
{
        /* remove temp files
         */
        remove(tmp_phratbl_name);
        remove(tmp_phratbl_sorted_name);
        remove(tmp_phratbl_combined_name);
        remove(tmp_phratbl_filtered_name);
        remove(filter_trantbl_sorted_name);
        remove(filter_trantbl_swapped_name);
        /* remove output files if exist
         */
        remove(reordertbl_name);
        remove(reordertbl_filtered_name);
        exit(1);
}

static void errexit(const char *fmt, ...)
{
        va_list va;
        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);
        sig_del_tmp_files(0);
        exit(1);
}

static char *create_phrase(struct sentn_info *sip, int s, int e)
{
        char *phra = NULL;
        int phra_len = 0;
        int i = 0;

        for (i = s; i <= e; i++)
                phra_len += strlen(sip->words[i]);
        phra_len += e - s + 1; /* spaces and final '\0' */
        if ((phra = calloc(phra_len, sizeof(*phra))) == NULL)
                errexit("[ERROR]: calloc error in %s: %s\n", __func__,
                                                strerror(errno));
        for (i = s; i <= e - 1; i++) {
                strcat(phra, sip->words[i]);
                strcat(phra, " ");
        }
        strcat(phra, sip->words[i]);

        return phra;
}

static void insert_to_phrase_list(char *f_phra, char *e_phra, 
                                  stridx_t fs, stridx_t fe, stridx_t es, stridx_t ee)
{
        struct phrasepair *ppp = NULL;

        if ((ppp = calloc(1, sizeof(*ppp))) == NULL)
                errexit("[ERROR]: calloc error in %s\n", __func__);
        ppp->pp_f_phra = f_phra;
        ppp->pp_e_phra = e_phra;
        ppp->pp_fs = fs; ppp->pp_fe = fe;
        ppp->pp_es = es; ppp->pp_ee = ee;

        /* Insert to list of <e_sent>
         */
        ppp->pp_s_next = e_sent.pp_start_arr[es];
        e_sent.pp_start_arr[es] = ppp;
        ppp->pp_e_next = e_sent.pp_end_arr[ee];
        e_sent.pp_end_arr[ee] = ppp;
}

/* 
 * If e phrase ranging from <min_aln> to <max_aln> is OK, 0 is returned, otherwise,
 * 1 is returned
 */
static int extract_phrase_pairs_given_f(stridx_t fs, stridx_t fe,
                                        stridx_t min_aln, stridx_t max_aln)
{
        char *f_phra = NULL, *e_phra = NULL;
        int i = 0, j = 0;
        int first_ret = 0, ret = 0;

        /* Check whether all words within e phrase ranging from <min_aln> to
         * <max_aln> aligned to f phrase from <fs> to <fe> 
         */
        for (i = min_aln; i <= max_aln; i++) {
                if (e_sent.aln_min[i] == -1)
                        continue;
                if (e_sent.aln_min[i] < fs || e_sent.aln_max[i] > fe)
                        return 1;
        }
        f_phra = create_phrase(&f_sent, fs, fe);
        e_phra = create_phrase(&e_sent, min_aln, max_aln);
        insert_to_phrase_list(f_phra, e_phra, fs, fe, min_aln, max_aln);

        /* Check if we can extend previous e phrase in both directions
         */
        for (i = min_aln; i >= 0; i--) {
                if (i < min_aln && e_sent.aln_min[i] != -1)
                        break;
                for (j = max_aln; j < e_sent.words_len; j++) {
                        if (i == min_aln && j == max_aln)
                                continue;

                        if (j > max_aln && e_sent.aln_min[j] != -1)
                                break;
                        if (j - i + 1 > max_extract_phra_len)
                                break;
                        f_phra = create_phrase(&f_sent, fs, fe);
                        e_phra = create_phrase(&e_sent, i, j);
                        insert_to_phrase_list(f_phra, e_phra, fs, fe, i, j);
                }
        }

        return 0;
}

static void extract_phrase_pairs(void)
{
        int i = 0, j = 0, k = 0;
        int min_aln = 0, max_aln = 0;

        for (i = 0; i < f_sent.words_len; i++) {
                min_aln = max_aln = -1;

                for (j = i; j < f_sent.words_len; j++) {
                        if (j - i + 1 > max_extract_phra_len) /* too long ? */
                                break;

                        /* Update <min_aln> and <max_aln>
                         */
                        if (min_aln == -1) {
                                min_aln = f_sent.aln_min[j];
                                max_aln = f_sent.aln_max[j];
                        } else if (f_sent.aln_min[j] != -1) {
                                if (min_aln > f_sent.aln_min[j])
                                        min_aln = f_sent.aln_min[j];
                                if (max_aln < f_sent.aln_max[j])
                                        max_aln = f_sent.aln_max[j];
                        }

                        if (max_aln - min_aln + 1 > max_extract_phra_len)
                                           /* means f[i..j] corresponds to a too
                                              long e phrase */
                                break;
                        if (min_aln == -1) /* f[i..j] doesn't align to any e words,
                                              which means this phrase can't exist
                                              on its own, it's either a part of some
                                              other phrases or not a phrase at all */
                                continue;
                        extract_phrase_pairs_given_f(i, j, min_aln, max_aln);
                }
        }
}

/* 
 * This function mimics the algorithm that moses uses to calculate M,S,D
 */
static void calculate_MSD_moses(void)
{
        struct phrasepair *p = NULL;
        int i = 0, j = 0;
        int pre_e_end = 0, next_e_start = 0;

        for (i = 0; i < e_sent.words_len; i++) {
                /* Calculate previous M, S, D
                 */
                for (p = e_sent.pp_start_arr[i]; p != NULL; p = p->pp_s_next) {
                        if (i == 0) {
                                if (p->pp_fs != 0)
                                        p->pp_pD = 1, pD_ttl++;
                                else
                                        p->pp_pM = 1, pM_ttl++;
                                continue;
                        }
                        pre_e_end = i - 1;
                        for (j = 0; j < e_sent.aln_lens[pre_e_end]; j++) {
                                if (e_sent.alns[pre_e_end][j] == p->pp_fs - 1) { 
                                                                        /* M */
                                        p->pp_pM = 1, pM_ttl++;
                                        break;
                                }
                                if (e_sent.alns[pre_e_end][j] == p->pp_fe + 1) { 
                                                                        /* S */
                                        p->pp_pS = 1, pS_ttl++;
                                        break;
                                }
                        }
                        if (j == e_sent.aln_lens[pre_e_end])            /* D */
                                p->pp_pD = 1, pD_ttl++;
                }
                /* Calculate next M, S, D
                 */
                for (p = e_sent.pp_end_arr[i]; p != NULL; p = p->pp_e_next) {
                        if (i == e_sent.words_len - 1) {
                                if (p->pp_fe == f_sent.words_len - 1)
                                        p->pp_nM = 1, nM_ttl++;
                                else
                                        p->pp_nD = 1, nD_ttl++;
                                continue;
                        }
                        next_e_start = i + 1;
                        for (j = 0; j < e_sent.aln_lens[next_e_start]; j++) {
                                if (e_sent.alns[next_e_start][j] == p->pp_fe + 1) {
                                                                        /* M */
                                        p->pp_nM = 1, nM_ttl++;
                                        break;
                                }
                                if (e_sent.alns[next_e_start][j] == p->pp_fs - 1) {
                                                                        /* S */
                                        p->pp_nS = 1, nS_ttl++;
                                        break;
                                }
                        }
                        if (j == e_sent.aln_lens[next_e_start])         /* D */
                                p->pp_nD = 1, nD_ttl++;
                }
        }
}

static void calculate_MSD_model2(void)
{
        struct phrasepair *p = NULL, *q = NULL;
        int i = 0;
        int pre_e_end = 0, next_e_start = 0;

        for (i = 0; i < e_sent.words_len; i++) {
                /* Calculate previous M, S, D
                 */
                for (p = e_sent.pp_start_arr[i]; p != NULL; p = p->pp_s_next) {
                        if (i == 0) {
                                if (p->pp_fs != 0)
                                        p->pp_pD++;
                                else
                                        p->pp_pM++;
                                continue;
                        }
                        pre_e_end = i - 1;
                        for (q = e_sent.pp_end_arr[pre_e_end]; q != NULL;
                                                        q = q->pp_e_next) {
                                if (q->pp_fe == p->pp_fs - 1) {
                                        p->pp_pM++;
                                        if (if_msd_calculate_once)
                                                break;
                                } else if (q->pp_fe < p->pp_fs) {
                                        p->pp_pD++;
                                } else if (q->pp_fs == p->pp_fe + 1) {
                                        p->pp_pS++;
                                        if (if_msd_calculate_once)
                                                break;
                                } else if (q->pp_fs > p->pp_fe) {
                                        p->pp_pD++;
                                }
                        }
                        if (if_msd_calculate_once) {
                                if (p->pp_pM > 0)
                                        p->pp_pM = 1, p->pp_pD = p->pp_pS = 0;
                                else if (p->pp_pS > 0)
                                        p->pp_pS = 1, p->pp_pM = p->pp_pD = 0;
                                else 
                                        p->pp_pD = 1, p->pp_pM = p->pp_pS = 0;
                        }
                }
                /* Calculate next M, S, D
                 */
                for (p = e_sent.pp_end_arr[i]; p != NULL; p = p->pp_e_next) {
                        if (i == e_sent.words_len - 1) {
                                if (p->pp_fe == f_sent.words_len - 1)
                                        p->pp_nM++;
                                else
                                        p->pp_nD++;
                                continue;
                        }
                        next_e_start = i + 1;
                        for (q = e_sent.pp_start_arr[next_e_start]; q != NULL;
                                                        q = q->pp_s_next)  {
                                if (q->pp_fs == p->pp_fe + 1) {
                                        p->pp_nM++;
                                        if (if_msd_calculate_once)
                                                break;
                                } else if (q->pp_fs > p->pp_fe) {
                                        p->pp_nD++;
                                } else if (q->pp_fe == p->pp_fs - 1) {
                                        p->pp_nS++;
                                        if (if_msd_calculate_once)
                                                break;
                                } else if (q->pp_fe < p->pp_fs) {
                                        p->pp_nD++;
                                }
                        }
                        if (if_msd_calculate_once) {
                                if (p->pp_nM > 0)
                                        p->pp_nM = 1, p->pp_nS = p->pp_nD = 0;
                                else if (p->pp_nS > 0)
                                        p->pp_nS = 1, p->pp_nM = p->pp_nD = 0;
                                else
                                        p->pp_nD = 1, p->pp_nM = p->pp_nS = 0;
                        }
                }
        }
}

static void calculate_MSD_model3(void)
{
        calculate_MSD_model2();
}

static void calculate_MSD(void)
{
        if (msd_model == 1)
                calculate_MSD_moses();
        else if (msd_model == 2)
                calculate_MSD_model2();
        else if (msd_model == 3)
                calculate_MSD_model3();
}

static void output_pharse_pairs_MSD(void)
{
        struct phrasepair *p = NULL;
        int i = 0;

        for (i = 0; i < e_sent.words_len; i++) {
                for (p = e_sent.pp_start_arr[i]; p != NULL; p = p->pp_s_next) {
                        if (p->pp_fe - p->pp_fs + 1 > max_phra_len
                            || p->pp_ee - p->pp_es + 1 > max_phra_len)
                                continue;
                        total_tmp_phrase_pair++;
                        fprintf(tmp_phratbl_fp, "%s%s%s%s%d %d %d %d %d %d\n",
                                p->pp_f_phra, FIELD_SEP, p->pp_e_phra, FIELD_SEP, 
                                p->pp_pM, p->pp_pS, p->pp_pD, p->pp_nM, p->pp_nS,
                                p->pp_nD);
                }
        }
}

static void free_phrase_pairs(void)
{
        struct phrasepair *p = NULL, *q = NULL;
        int i = 0;

        for (i = 0; i < e_sent.words_len; i++) {
                for (p = e_sent.pp_start_arr[i]; p != NULL; p = q) {
                        q = p->pp_s_next;
                        free(p->pp_f_phra);
                        free(p->pp_e_phra);
                        free(p);
                }
        }
}

/* 
 * @buf: <*buf> should be either allocated on the heap or NULL, in which case 
 *       <*buf_len> should be set to 0 accordingly.
 * This function guarantees to read a complete line. Consider that <*buf> may not be
 * long enough, it will be reallocated in that situation to be able to store one 
 * complete line, and <*buf_len> will be changed accordingly. Note that here the
 * complete line we mention here includes the final '\n' character.
 * Return <*buf> if OK, otherwise, if errors other than read file error happen, NULL
 * is returned with errno set properly, (char *)-1 is returned to signify read file
 * error. When EOF is encountered, (*buf)[0] is set to '\0'
 */
static char *read_one_line(FILE *fp, char **buf, int *buf_len)
{
        if (fp == NULL || buf == NULL || buf_len == NULL) {
                errno = EINVAL;
                return NULL;
        }
        if ((*buf == NULL && *buf_len != 0) || (*buf != NULL && *buf_len == 0)) {
                errno = EINVAL;
                return NULL;
        }
        if (*buf == NULL) {
                *buf_len = 30;
                if ((*buf = calloc(*buf_len, sizeof(**buf))) == NULL) {
                        *buf_len = 0;
                        return NULL;
                }
        }

        if (fgets(*buf, *buf_len, fp) == NULL) {
                if (ferror(fp))
                        return (char *)-1;
                (*buf)[0] = '\0';
                return *buf;
        }

        while ((*buf)[strlen(*buf) - 1] != '\n') {
                char *p = NULL;
                int len = *buf_len + 1;
                *buf_len *= 2;
                if ((*buf = realloc(*buf, *buf_len)) == NULL)
                        return NULL;
                p = *buf + strlen(*buf);
                if (fgets(p, len, fp) == NULL) {
                        if (feof(fp))
                                break;
                        if (ferror(fp))
                                return (char *)-1;
                }
        }

        return *buf;
}

static void parse_fe_line(char *line, struct sentn_info *sip)
{
#define INIT_WORDS_SIZE 10
        char *p = line;
        int i = 0;

        /* Initialization
         */
        sip->words_len = 0;

        while (ISSPACE(*p))
                p++;
        if (*p == '\0') /* blank line */
                return;

        while (*p != '\0') {
                /* memory reallocation
                 */
                if (sip->words_len >= sip->words_size) {
                        int orig_size = sip->words_size;
                        int alloc_size = orig_size == 0 ? INIT_WORDS_SIZE : orig_size;
                        sip->words_size = sip->words_size == 0 ? INIT_WORDS_SIZE :
                                                              2 * sip->words_size;

                        /* Allocate memory
                         */
                        sip->words = realloc(sip->words, 
                                        sip->words_size * sizeof(*sip->words));
                        sip->alns = realloc(sip->alns, 
                                        sip->words_size * sizeof(*sip->alns));
                        sip->aln_sizes = realloc(sip->aln_sizes,
                                        sip->words_size * sizeof(*sip->aln_sizes));
                        sip->aln_lens = realloc(sip->aln_lens,
                                        sip->words_size * sizeof(*sip->aln_lens));
                        sip->aln_min = realloc(sip->aln_min,
                                        sip->words_size * sizeof(*sip->aln_min));
                        sip->aln_max = realloc(sip->aln_max,
                                        sip->words_size * sizeof(*sip->aln_max));
                        sip->pp_start_arr = realloc(sip->pp_start_arr,
                                        sip->words_size * sizeof(*sip->pp_start_arr));
                        sip->pp_end_arr = realloc(sip->pp_end_arr,
                                        sip->words_size * sizeof(*sip->pp_end_arr));

                        if (sip->words == NULL || sip->alns == NULL
                            || sip->aln_sizes == NULL || sip->aln_lens == NULL
                            || sip->aln_min == NULL || sip->aln_max == NULL
                            || sip->pp_start_arr == NULL || sip->pp_end_arr == NULL)
                                errexit("[ERROR]: realloc error in %s\n", __func__);

                        /* Initialize newly allocated memory
                         */
                        memset(sip->words + orig_size, 0, 
                                                alloc_size * sizeof(*sip->words));
                        memset(sip->alns + orig_size, 0, 
                                                alloc_size * sizeof(*sip->alns));
                        memset(sip->aln_sizes + orig_size, 0, 
                                                alloc_size * sizeof(*sip->aln_sizes));
                }
                sip->words[sip->words_len++] = p;
                while (!ISSPACE(*p) && *p != '\0')
                        p++;
                *(p++) = '\0';
                while (ISSPACE(*p))
                        p++;
        }
        /* Putting these initialization here to prevent us from having to call
         * memset every time after realloc 
         */
        memset(sip->aln_lens, 0, sip->words_size * sizeof(*sip->aln_lens));
        for (i = 0; i < sip->words_size; i++)
                sip->aln_min[i] = -1;
        for (i = 0; i < sip->words_size; i++)
                sip->aln_max[i] = -1;
        memset(sip->pp_start_arr, 0, sip->words_size * sizeof(*sip->pp_start_arr));
        memset(sip->pp_end_arr, 0, sip->words_size * sizeof(*sip->pp_end_arr));
}

static void parse_aln_line(char *line , struct sentn_info *f_sip,
                           struct sentn_info *e_sip)
{
#define INIT_ALN_SIZE 5
        char *p = line, *q = NULL;
        int f_idx = 0, e_idx = 0;
        int i = 0;

        /* Initialize
         */
        for (i = 0; i < f_sip->words_len; i++)
                f_sip->aln_lens[i] = 0;
        for (i = 0; i < e_sip->words_len; i++)
                e_sip->aln_lens[i] = 0;

        while (ISSPACE(*p)) p++;
        if (*p == '\0') /* blank line */
                return;

        while (*p != '\0') {
                /* Get <f_idx> and <e_idx> 
                 */
                q = p; while (*p != '-') p++; *(p++) = '\0';
                f_idx = atoi(q);
                q = p; while (!ISSPACE(*p) && *p != '\0') p++; *p = '\0';
                e_idx = atoi(q);

                if (if_swap_fe_align) {
                        int t = f_idx;
                        f_idx = e_idx;
                        e_idx = t;
                }

                /* Store align info into both <f_sip> and <e_sip> 
                 */
                if (f_sip->aln_lens[f_idx] >= f_sip->aln_sizes[f_idx]) {
                        f_sip->aln_sizes[f_idx] = f_sip->aln_sizes[f_idx] == 0 ?
                                       INIT_ALN_SIZE : 2 * f_sip->aln_sizes[f_idx];
                        if ((f_sip->alns[f_idx] = realloc(f_sip->alns[f_idx],
                             f_sip->aln_sizes[f_idx] * sizeof(*f_sip->alns[f_idx])))
                                                          == NULL)
                                errexit("[ERROR]: realloc error in %s 1\n", __func__);
                }
                if (e_sip->aln_lens[e_idx] >= e_sip->aln_sizes[e_idx]) {
                        e_sip->aln_sizes[e_idx] = e_sip->aln_sizes[e_idx] == 0 ?
                                       INIT_ALN_SIZE : 2 * e_sip->aln_sizes[e_idx];
                        if ((e_sip->alns[e_idx] = realloc(e_sip->alns[e_idx],
                             e_sip->aln_sizes[e_idx] * sizeof(*e_sip->alns[e_idx])))
                                                          == NULL)
                                errexit("[ERROR]: realloc error in %s\n", __func__);
                }
                f_sip->alns[f_idx][f_sip->aln_lens[f_idx]++] = e_idx;
                e_sip->alns[e_idx][e_sip->aln_lens[e_idx]++] = f_idx;

                /* Update the <aln_min> and <aln_max> 
                 */
                if (f_sip->aln_min[f_idx] == -1) {
                        f_sip->aln_min[f_idx] = f_sip->aln_max[f_idx] = e_idx;
                } else {
                        if (e_idx < f_sip->aln_min[f_idx])
                                f_sip->aln_min[f_idx] = e_idx;
                        if (e_idx > f_sip->aln_max[f_idx])
                                f_sip->aln_max[f_idx] = e_idx;
                }
                if (e_sip->aln_min[e_idx] == -1) {
                        e_sip->aln_min[e_idx] = e_sip->aln_max[e_idx] = f_idx;
                } else {
                        if (f_idx < e_sip->aln_min[e_idx])
                                e_sip->aln_min[e_idx] = f_idx;
                        if (f_idx > e_sip->aln_max[e_idx])
                                e_sip->aln_max[e_idx] = f_idx;
                }

                /* Skip space
                 */
                p++; while (ISSPACE(*p)) p++;
        }
}

static void process_line(void)
{
#define INIT_LINE_LEN 10
        int progress = 0;

        fprintf(stderr, "======================================================\n");
        fprintf(stderr, "(%d). Process input file to generate temp phrase table\n",
                                                                           step++);

        sprintf(tmp_phratbl_name, "%s.%d", TMP_PHRASE_TABLE_PREFIX, getpid());
        FOPEN(tmp_phratbl_fp, tmp_phratbl_name, "w");

        /* Initialize some global variables
         */
        f_line_len = e_line_len = aln_line_len = INIT_LINE_LEN;
        if ((f_line = calloc(f_line_len, sizeof(*f_line))) == NULL
            || (e_line = calloc(e_line_len, sizeof(*e_line))) == NULL
            || (aln_line = calloc(aln_line_len, sizeof(*aln_line))) == NULL)
                errexit("[ERROR]: calloc error in %s: %s\n", __func__,
                                                strerror(errno));
        memset(&f_sent, 0, sizeof(f_sent));
        memset(&e_sent, 0, sizeof(e_sent));

        while (1) {
                PRINT_PROCESS(progress, 20000, 4000);
                progress++;
                /* Read f line, e line, and aln line individually
                 */
                READ_ONE_LINE(f_fp, f_line, f_line_len);
                READ_ONE_LINE(e_fp, e_line, e_line_len);
                READ_ONE_LINE(aln_fp, aln_line, aln_line_len);
                if (f_line[0] == '\0') /* EOF encountered */
                        break;

                /* Parse each line 
                 */
                parse_fe_line(f_line, &f_sent);
                if (f_sent.words_len == 0) /* blank line */
                        continue;
                parse_fe_line(e_line, &e_sent);
                parse_aln_line(aln_line, &f_sent, &e_sent);

                extract_phrase_pairs();
                calculate_MSD();
                output_pharse_pairs_MSD();
                free_phrase_pairs();
        }
        fprintf(stderr, "[%d]\n", progress - 1);
        fclose(tmp_phratbl_fp);
}

static void sort_tmp_phrase_table(const char *input)
{
        int ret = 0;

        fprintf(stderr, "======================================================\n");
        fprintf(stderr, "(%d). Sort temp phrase table (this may take a while, "
                        "please be patient)\n", step++);
        sprintf(tmp_phratbl_sorted_name, "%s_sorted", tmp_phratbl_name);
#ifndef WIN32
        sprintf(cmd_buf, "LC_ALL=\"C\" sort -T ~ -S 1024M %s > %s", input,
                                        tmp_phratbl_sorted_name);
#else
        sprintf(cmd_buf, "%s -S 1024M %s > %s", g_sort, input,
                                        tmp_phratbl_sorted_name);
#endif /* WIN32 */
        ret = system(cmd_buf);

#ifndef WIN32
        if (ret == -1)
                errexit("[ERROR]: system error: %s\n", strerror(errno));
        else if (WEXITSTATUS(ret) != 0 
                 || (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
                errexit("[ERROR]: run sort command incorrectly\n");
#endif /* WIN32 */
}

static void combine_tmp_phrase_table(const char *input)
{
        FILE *ofp = NULL, *ifp = NULL;
        char *buf = NULL, *_buf = NULL, *p = NULL;
        int buf_len = 0, _buf_len = 0;
        int pM = 0, pS = 0, pD = 0, nM = 0, nS = 0, nD = 0;
        int _pM = 0, _pS = 0, _pD = 0, _nM = 0, _nS = 0, _nD = 0; /* current vals */
        int progress = 0;
        int ncur_phrapair = 0;

        fprintf(stderr, "======================================================\n");
        fprintf(stderr, "(%d). Combine phrases (total %ld)\n", step++, 
                                               total_tmp_phrase_pair);

        FOPEN(ifp, tmp_phratbl_sorted_name, "r");
        sprintf(tmp_phratbl_combined_name, "%s_combined", tmp_phratbl_name);
        FOPEN(ofp, tmp_phratbl_combined_name, "w");
        
        while (1) {
                PRINT_PROCESS(progress, 1000000, 50000);
                progress++;

                READ_ONE_LINE(ifp, _buf, _buf_len);
                if (_buf[0] == '\0')
                        break;

                if (buf == NULL) {
                        total_phrase_pair++;
                        if ((buf = calloc(_buf_len, sizeof(*buf))) == NULL)
                                errexit("[ERROR]: calloc error in %s: %s\n",
                                                __func__, strerror(errno));
                        buf_len = _buf_len;
                        sprintf(buf, "%s", _buf);
                        STRSTR_TO_FIELD_SEP(p, buf, FIELD_SEP);
                        STRSTR_TO_FIELD_SEP(p, p + strlen(FIELD_SEP), FIELD_SEP);
                        *p = '\0';
                        p += strlen(FIELD_SEP);
                        sscanf(p, "%d%d%d%d%d%d", &pM, &pS, &pD, &nM, &nS, &nD);
                        ncur_phrapair = 1;
                        continue;
                }
                STRSTR_TO_FIELD_SEP(p, _buf, FIELD_SEP);
                STRSTR_TO_FIELD_SEP(p, p + strlen(FIELD_SEP), FIELD_SEP);
                *p = '\0';
                p += strlen(FIELD_SEP);
                sscanf(p, "%d%d%d%d%d%d", &_pM, &_pS, &_pD, &_nM, &_nS, &_nD);

                if (!strcmp(_buf, buf)) { /* Current phrase pair is the same as the 
                                             one in <buf> */
                        pM += _pM; pS += _pS; pD += _pD;
                        nM += _nM, nS += _nS; nD += _nD;
                        ncur_phrapair++;
                } else { /* New phrase pair appears */
                        total_phrase_pair++;
                        /* First we need to output previous phrase pair info
                         */
                        if (if_need_num_of_phrapair == 0)
                                fprintf(ofp, "%s%s%d %d %d %d %d %d\n", buf,
                                        FIELD_SEP, pM, pS, pD, nM, nS, nD);
                        else
                                fprintf(ofp, "%s%s%d %d %d %d %d %d%s%d\n", buf,
                                        FIELD_SEP, pM, pS, pD, nM, nS, nD, 
                                        FIELD_SEP, ncur_phrapair);
                        if (_buf_len > buf_len) {
                                buf_len = _buf_len;
                                if ((buf = realloc(buf, buf_len * sizeof(*buf)))
                                                                == NULL)
                                        errexit("[ERROR]: realloc error in %s: %s\n",
                                                        __func__, strerror(errno));
                        }
                        sprintf(buf, "%s", _buf);
                        pM = _pM; pS = _pS; pD = _pD;
                        nM = _nM; nS = _nS; nD = _nD;
                        ncur_phrapair = 1;
                }
        }
        if (if_need_num_of_phrapair == 0)
                fprintf(ofp, "%s%s%d %d %d %d %d %d\n", buf, FIELD_SEP, pM, pS, pD,
                                                                        nM, nS, nD);
        else
                fprintf(ofp, "%s%s%d %d %d %d %d %d%s%d\n", buf, FIELD_SEP, pM, pS,
                                        pD, nM, nS, nD, FIELD_SEP, ncur_phrapair);
        fprintf(stderr, "[%d]\n", progress - 1);

        fclose(ofp);
        fclose(ifp);
}

/* 
 * This function filters out all phrase pairs whose pM + pS + pD == 1
 */
static void filter_MSD_sum_eq_one(const char *input)
{
        FILE *ifp = NULL, *ofp = NULL;
        char *buf = NULL, *p = NULL;
        int buf_len = 0;
        int pM = 0, pS = 0, pD = 0, nM = 0, nS = 0, nD = 0;

        fprintf(stderr, "======================================================\n");
        fprintf(stderr, "(%d). Filter reordering model\n", step++);

        FOPEN(ifp, input, "r");
        sprintf(tmp_phratbl_filtered_name, "%s_filtered", tmp_phratbl_name);
        FOPEN(ofp, tmp_phratbl_filtered_name, "w");

        while (1) {
                READ_ONE_LINE(ifp, buf, buf_len);
                if (buf[0] == '\0')
                        break;

                STRSTR_TO_FIELD_SEP(p, buf, FIELD_SEP);
                STRSTR_TO_FIELD_SEP(p, p + strlen(FIELD_SEP), FIELD_SEP);
                p += strlen(FIELD_SEP);
                sscanf(p, "%d%d%d%d%d%d", &pM, &pS, &pD, &nM, &nS, &nD);
                if (pM + pS + pD == 1)
                        continue;
                else
                        fprintf(ofp, "%s", buf);
        }

        fclose(ifp);
        fclose(ofp);
}

/* TODO */
/* 
 * This function mimics the algorithm moses uses to calculate M, S, D probabilities
 */
static void calculate_MSD_prob_moses(const char *input)
{
        FILE *ifp = NULL, *ofp = NULL;
        char *buf = NULL, *p = NULL;
        int buf_len = 0;
        int pM = 0, pS = 0, pD = 0, nM = 0, nS = 0, nD = 0;
        int progress = 0;
        char num_buf[256];
        int total_pM = 0, total_pS = 0, total_pD = 0;
        double pttl = 0, nttl = 0;

        fprintf(stderr, "======================================================\n");
        fprintf(stderr, "(%d). Calculate the M, S, D probabilities\n", step++);

        FOPEN(ifp, input, "r");
        FOPEN(ofp, reordertbl_name, "w");

        /*
        pM_factor = 0.5 * (pM_ttl + 0.1) / (pM_ttl + pS_ttl + pD_ttl);
        pS_factor = 0.5 * (pS_ttl + 0.1) / (pM_ttl + pS_ttl + pD_ttl);
        pD_factor = 0.5 * (pD_ttl + 0.1) / (pM_ttl + pS_ttl + pD_ttl);
        nM_factor = 0.5 * (nM_ttl + 0.1) / (nM_ttl + nS_ttl + nD_ttl);
        nS_factor = 0.5 * (nS_ttl + 0.1) / (nM_ttl + nS_ttl + nD_ttl);
        nD_factor = 0.5 * (nD_ttl + 0.1) / (nM_ttl + nS_ttl + nD_ttl);
        */
        pM_factor = 0.5;
        pS_factor = 0.5;
        pD_factor = 0.5;
        nM_factor = 0.5;
        nS_factor = 0.5;
        nD_factor = 0.5;

        while (1) {
                PRINT_PROCESS(progress, 1000000, 50000);
                progress++;

                READ_ONE_LINE(ifp, buf, buf_len);
                if (buf[0] == '\0')
                        break;

                STRSTR_TO_FIELD_SEP(p, buf, FIELD_SEP);
                STRSTR_TO_FIELD_SEP(p, p + strlen(FIELD_SEP), FIELD_SEP);
                p += strlen(FIELD_SEP);
                sscanf(p, "%d%d%d%d%d%d", &pM, &pS, &pD, &nM, &nS, &nD);
                total_pM += pM; total_pS += pS; total_pD += pD;
                *p = '\0';
                pttl = pM + pM_factor + pS + pS_factor + pD + pD_factor;
                nttl = nM + nM_factor + nS + nS_factor + nD + nD_factor;
                fprintf(ofp, "%s%g %g %g %g %g %g\n", buf,
                                log((pM + pM_factor) / pttl), log((pS + pS_factor) / pttl),
                                log((pD + pD_factor) / pttl), log((nM + nM_factor) / nttl),
                                log((nS + nS_factor) / nttl), log((nD + nD_factor) / nttl));
        }
        fprintf(stderr, "[%d]\n", progress - 1);
        fprintf(stderr, "total pM, pS, pD: %d %d %d\n", 
                        total_pM, total_pS, total_pD);

        fclose(ofp);
        fclose(ifp);
}

static void filter_given_tran_table(const char *input)
{
        FILE *ifp = NULL, *ofp = NULL, *trantbl_fp = NULL, *trantbl_swap_fp = NULL;
        char *in_buf = NULL, *tt_buf = NULL, *p = NULL;
        int in_buf_len = 0, tt_buf_len = 0;
        char c = '\0';
        char *f1 = NULL, *f2 = NULL, *f3 = NULL;
        int sys_ret = 0;

        fprintf(stderr, "======================================================\n");
        fprintf(stderr, "(%d). Filter reordering model\n", step++);

        /* Swap the filter table if needed
         */
        if (if_swap_fe_align) {
                sprintf(filter_trantbl_swapped_name, "%s.swapped",
                                             filter_trantbl_name);
                FOPEN(trantbl_swap_fp, filter_trantbl_swapped_name, "w");
                FOPEN(trantbl_fp, filter_trantbl_name, "r");
                while (1) {
                        READ_ONE_LINE(trantbl_fp, tt_buf, tt_buf_len);
                        if (tt_buf[0] == '\0')
                                break;
                        /* Set <f1>, <f2>, and <f3> to point to each field
                         */
                        f1 = tt_buf;
                        STRSTR_TO_FIELD_SEP(f2, f1, FIELD_SEP);
                        *f2 = '\0';
                        f2 += strlen(FIELD_SEP);
                        STRSTR_TO_FIELD_SEP(f3, f2, FIELD_SEP);
                        *f3 = '\0';
                        f3 += strlen(FIELD_SEP);

                        fprintf(trantbl_swap_fp, "%s%s%s%s%s", f2, FIELD_SEP,
                                                          f1, FIELD_SEP, f3);
                }
                fclose(trantbl_swap_fp);
                fclose(trantbl_fp);
        }
        /* Sort the tran table 
         */
        sprintf(filter_trantbl_sorted_name, "%s.sorted", filter_trantbl_name);
#ifndef WIN32
        sprintf(cmd_buf, "LC_ALL=\"C\" sort -T ~ -S 1024M %s > %s",
                        if_swap_fe_align == 0 ? filter_trantbl_name :
                                                filter_trantbl_swapped_name,
                        filter_trantbl_sorted_name);
#else
        sprintf(cmd_buf, "%s -S 1024M %s > %s", g_sort,
                        if_swap_fe_align == 0 ? filter_trantbl_name :
                                                filter_trantbl_swapped_name,
                        filter_trantbl_sorted_name);
#endif /* WIN32 */
        sys_ret = system(cmd_buf);
#ifndef WIN32
        if (sys_ret == -1)
                errexit("[ERROR]: system error: %s\n", strerror(errno));
        else if (WEXITSTATUS(sys_ret) != 0 
                 || (WTERMSIG(sys_ret) == SIGINT || WTERMSIG(sys_ret) == SIGQUIT))
                errexit("[ERROR]: run sort command incorrectly\n");
#endif /* WIN32 */

        FOPEN(ifp, input, "r");
        FOPEN(ofp, reordertbl_filtered_name, "w");
        FOPEN(trantbl_fp, filter_trantbl_sorted_name, "r");

        while (1) {
                READ_ONE_LINE(trantbl_fp, tt_buf, tt_buf_len);
                if (tt_buf[0] == '\0')
                        return;
                if (strstr(tt_buf, "<NULL>"))
                        continue;
                STRSTR_TO_FIELD_SEP(p, tt_buf, FIELD_SEP);
                STRSTR_TO_FIELD_SEP(p, p + strlen(FIELD_SEP), FIELD_SEP);
                *p = '\0';
                break;
        }

        while (1) {
                READ_ONE_LINE(ifp, in_buf, in_buf_len);
                if (in_buf[0] == '\0')
                        break;
                STRSTR_TO_FIELD_SEP(p, in_buf, FIELD_SEP);
                STRSTR_TO_FIELD_SEP(p, p + strlen(FIELD_SEP), FIELD_SEP);
                c = *p; *p = '\0';
                if (strcmp(in_buf, tt_buf))
                        continue;
                /* Deal with the situation in which phrase pairs are the same
                 * in <in_buf> and <tt_buf>
                 */
                *p = c;
                fprintf(ofp, "%s", in_buf);
                while (1) { /* read in another filter file line */
                        READ_ONE_LINE(trantbl_fp, tt_buf, tt_buf_len);
                        if (tt_buf[0] == '\0') {
                                remove(filter_trantbl_sorted_name);
                                if (if_swap_fe_align)
                                        remove(filter_trantbl_swapped_name);
                                return;
                        }
                        if (strstr(tt_buf, "<NULL>"))
                                continue;
                        STRSTR_TO_FIELD_SEP(p, tt_buf, FIELD_SEP);
                        STRSTR_TO_FIELD_SEP(p, p + strlen(FIELD_SEP), FIELD_SEP);
                        *p = '\0';
                        break;
                }
        }
        fclose(ifp);
        fclose(ofp);
        fclose(trantbl_fp);
        remove(filter_trantbl_sorted_name);
        if (if_swap_fe_align)
                remove(filter_trantbl_swapped_name);
}

static void usage(const char *cmd)
{
        int i = 0, max = 72;
        for (i = 0; i < max; i++) fprintf(stderr, "*"); putchar('\n');
        fprintf(stderr, "[USAGE]:\n");
        fprintf(stderr, "\t%s [OPTIONS]\n", cmd);
        for (i = 0; i < max; i++) fprintf(stderr, "-"); putchar('\n');
        fprintf(stderr, "[OPTIONS]: (-f -e -a are mandatory)\n");
        fprintf(stderr, "\t-f <source-file>\n");
        fprintf(stderr, "\t-e <target-file>\n");
#ifndef WIN32
        fprintf(stderr, "\t-a, -align <alignment-table>\n");
#else
        fprintf(stderr, "\t-a <alignment-table>\n");
#endif /* WIN32 */
        fprintf(stderr, 
#ifndef WIN32
                        "\t-m, -msd-model <model-number>\n"
#else
                        "\t-m <model-number>\n"
#endif /* WIN32 */
                        "\t\tspecifies which model to use for calculating M, S, D\n"
                        "\t\tproperties of a given phrase pair, it takes 1, 2, or\n"
                        "\t\t3 as its argument, with each number denoting one\n"
                        "\t\tdifferent model\n"
                        "\t\tthe default model to be used is 1\n");
        fprintf(stderr, "\t-p <path-to-tool-sort>\n"
                        "\t\tspecifies the path to the external tool \"sort\" used in program\n" );
        fprintf(stderr, 
#ifndef WIN32
                        "\t-s, -swap-fe-align\n"
#else
                        "\t-s\n"
#endif /* WIN32 */
                        "\t\tsometimes you may want to specify f as e, e as f\n"
                        "\t\twith alignment file being unchanged, then this\n"
                        "\t\toption must be used in this situation\n");
        fprintf(stderr, 
#ifndef WIN32
                        "\t-l, -filter <argument>\n"
#else
                        "\t-l <argument>\n"
#endif /* WIN32 */
                        "\t\t<argument> determine in which way the reordering\n"
                        "\t\tmodel will be filtered. It should be in one of the\n"
                        "\t\tfollowing formats:\n"
                        "\t\t(1). tran-table:tran_table_filename\n"
                        "\t\tthis means user provides a tran-table called\n"
                        "\t\t<tran_table_filename> containing all the phrase pairs\n"
                        "\t\tneeded. All those lines containing phrase pairs that\n"
                        "\t\tdon't occur in this file will be filtered out.\n"
                        "\t\t(2). msd-sum-1\n"
                        "\t\tfilter all phrase pairs for which pM + pS + pD == 1\n");
#ifndef WIN32
        fprintf(stderr, "\t-o, -output <reordering-table>\n");
#else
        fprintf(stderr, "\t-o <reordering-table>\n");
#endif /* WIN32 */
        fprintf(stderr, 
#ifndef WIN32
                        "\t-n, -msd-once\n"
#else
                        "\t-n\n"
#endif /* WIN32 */
                        "\t\tthis option tells the program to only count one for\n"
                        "\t\tany times of occurrences of M, S, D, and if M occurs,\n"
                        "\t\tS and D will be 0, else if S occurs, then M, D will\n"
                        "\t\tbe 0, otherwise, D = 1\n");
        fprintf(stderr,
#ifndef WIN32
                        "\t-c, -count-phrapair\n"
#else
                        "\t-c\n"
#endif /* WIN32 */
                        "\t\tthis option tells the program to also output the\n"
                        "\t\tcount of each phrase pair\n");
#ifndef WIN32
        fprintf(stderr, "\t-max-phrase-len <len>\n"
                        "\t\tthe default value is 7\n");
        fprintf(stderr, "\t-h, -help\n");
#else
        fprintf(stderr, "\t-h\n");
#endif /* WIN32 */
        for (i = 0; i < max; i++) fprintf(stderr, "-"); putchar('\n');
        fprintf(stderr, "[EXAMPLE]:\n");
        fprintf(stderr, "\t%s -f text.zh -e text.en \n"
                        "\t\t-a align -o reordering-table\n", cmd);
        for (i = 0; i < max; i++) fprintf(stderr, "*");
        putchar('\n');
}

int main(int argc, char **argv)
{
#ifndef WIN32
        struct option opts[] = {
                {"f", 1, 0, 'f'},
                {"e", 1, 0, 'e'},
                {"align", 1, 0, 'a'},
                {"msd-model", 1, 0, 'm'},
                {"output", 1, 0, 'o'},
                {"max-phrase-len", 1, 0, 2},
                {"swap-fe-align", 0, 0, 's'},
                {"msd-once", 0, 0, 'n'},
                {"filter", 1, 0, 'l'},
                {"count-phrapair", 1, 0, 'c'},
                {"help", 0, 0, 'h'},
        };
#endif /* WIN32 */
        int optret = 0;
        char *next_step_input = NULL, *p = NULL;
        char name_buf[FILE_NAME_LEN] = { '\0' };
        FILE *fp = NULL;

        if (argc == 1) {
                usage(argv[0]);
                exit(0);
        }

#ifndef WIN32
        while ((optret = getopt_long_only(argc, argv, "f:e:a:ho:m:l:p:cns", opts,
                                                        NULL)) != -1) {
#else
        while ((optret = getopt(argc, argv, "f:e:a:m:p:so:hl:cn")) != -1) {
#endif /* WIN32 */
                switch (optret) {
                case 'f':
                        sprintf(f_name, "%s", optarg);
                        FOPEN(f_fp, optarg, "r");
                        break;
                case 'e':
                        sprintf(e_name, "%s", optarg);
                        FOPEN(e_fp, optarg, "r");
                        break;
                case 'a':
                        sprintf(aln_name, "%s", optarg);
                        FOPEN(aln_fp, optarg, "r");
                        break;
                case 'm':
                        msd_model = atoi(optarg);
                        if (msd_model <= 0 || msd_model > 3)
                                errexit("[ERROR]: the argument of '-msd-model' can"
                                                " only be one of 1, 2, 3\n");
                        if (msd_model == 3)
                                max_extract_phra_len = INT_MAX;
                        break;
                case 's':
                        if_swap_fe_align = 1;
                        break;
                case 'o':
                        if (strlen(optarg) >= FILE_NAME_LEN)
                                errexit("[ERROR]: output file name too long\n");
                        sprintf(name_buf, "%s", optarg);
                        break;
                case 'n':
                        if_msd_calculate_once = 1;
                        break;
                case 'l':
                        if (strstr(optarg, "tran-table") == optarg) {
                                if_filter = FILTER_WITH_TRANTBL;
                                if ((p = strchr(optarg, ':')) == NULL)
                                        errexit("[ERROR]: argument of -l isn't"
                                                                "correct!\n");
                                sprintf(filter_trantbl_name, "%s", p + 1);
                                FOPEN(fp, filter_trantbl_name, "r");
                                fclose(fp);
                        } else if (!strcmp(optarg, "msd-sum-1")) {
                                if_filter = FILTER_MSD_SUM_EQ_ONE;
                        } else {
                                errexit("[ERROR]: wrong argument to '-l'\n");
                        }
                        break;
                case 'c':
                        if_need_num_of_phrapair = 1;
                        break;
                case 'p':
                        g_sort = optarg;
                        break;
                case 'h':
                        usage(argv[0]);
                        exit(1);
                        break;
#ifndef WIN32
                case 2:
                        if ((max_phra_len = atoi(optarg)) == 0)
                                errexit("[ERROR]: 'max-phrase-len' takes as an "
                                                "argument an integer > 0\n");
                        break;
#endif /* WIN32 */
                default:
                        errexit("[ERROR]: unknown option found\n");
                        break;
                }
        }
        if (f_fp == NULL || e_fp == NULL || aln_fp == NULL)
                errexit("[ERROR]: lack of arguments\n");
        sprintf(reordertbl_name, DEF_REORDER_TBL_NAME);
        if (if_filter == FILTER_WITH_TRANTBL)
                sprintf(reordertbl_filtered_name, DEF_REORDER_TBL_FILTER_NAME);
        if (name_buf[0] != '\0') {
                if (if_filter != FILTER_WITH_TRANTBL)
                        sprintf(reordertbl_name, "%s", name_buf);
                else
                        sprintf(reordertbl_filtered_name, "%s", name_buf);
        }
        if (max_phra_len == 0)
                max_phra_len = DEF_MAX_PHRASE_LEN;
        if (max_extract_phra_len == 0)
                max_extract_phra_len = max_phra_len;

#ifndef WIN32
        /* Register signal handler
         */
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sig_del_tmp_files;
        if (sigaction(SIGINT, &sa, NULL) < 0 
            || sigaction(SIGSEGV, &sa, NULL) < 0
            || sigaction(SIGQUIT, &sa, NULL) < 0)
                errexit("[ERROR]: sigaction error: %s\n", strerror(errno));
#endif /* WIN32 */
        
        /* Begin the whole process
         */
        process_line();
        next_step_input = tmp_phratbl_name;

        sort_tmp_phrase_table(next_step_input);
        remove(next_step_input);
        next_step_input = tmp_phratbl_sorted_name;

        combine_tmp_phrase_table(next_step_input);
        // remove(next_step_input);
        next_step_input = tmp_phratbl_combined_name;

        if (if_filter == FILTER_MSD_SUM_EQ_ONE) {
                filter_MSD_sum_eq_one(next_step_input);
                remove(next_step_input);
                next_step_input = tmp_phratbl_filtered_name;
        }

        calculate_MSD_prob_moses(next_step_input);
        // remove(next_step_input);
        next_step_input = reordertbl_name;

        if (if_filter == FILTER_WITH_TRANTBL) {
                filter_given_tran_table(next_step_input);
                remove(next_step_input);
        }

        return 0;
}


