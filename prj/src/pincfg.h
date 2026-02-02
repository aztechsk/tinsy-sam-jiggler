/*
 * pincfg.h
 *
 * Autors: Jan Rusnak.
 * (c) 2023 AZTech.
 */

#ifndef PINCFG_H
#define PINCFG_H

enum pincfg {
	PINCFG_INIT,
	PINCFG_SLEEP,
	PINCFG_WAKE
};

/**
 * pincfg
 */
void pincfg(enum pincfg cfg);

#endif
