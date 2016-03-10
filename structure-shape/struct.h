#ifndef STRUCT_H_
#define STRUCT_H_

/* minimum difference (in seconds) */
#define	MINDIFF	60

/*
 * Rules:
 *
 *   soft_sec < hard_sec
 *   soft_kb  < hard_kb
 *   diff(soft_*, hard_*) >= MINDIFF
 *
 */
typedef struct entry_s {
	unsigned timer_hard_sec;
	unsigned timer_soft_sec;
	unsigned timer_hard_kb;
	unsigned timer_soft_kb;
} entry_t;

entry_t
shape(int argc, char *argv[]);

#endif // STRUCT_H_