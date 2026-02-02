/*
 * tm.h
 *
 * Autors: Jan Rusnak.
 * (c) 2023 AZTech.
 */

#ifndef TM_H
#define TM_H

/**
 * init_tm
 */
void init_tm(void);

/**
 * get_uptm
 */
int get_uptm(void);

/**
 * add_tm_clbk
 */
boolean_t add_tm_clbk(void (*clbk)(unsigned int));

#endif
