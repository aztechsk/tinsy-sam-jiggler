/*
 * msgcfg.h
 *
 * Autors: Jan Rusnak.
 * (c) 2020 AZTech.
 */

#ifndef MSGCFG_H
#define MSGCFG_H

enum dbg_level {
        INF,
        DBG
};

#if TERMOUT == 1
 #include "tout.h"
 #ifdef DEBUG
  #define msg(t, ...) add_msg_tout(__VA_ARGS__)
 #else
  #define msg(t, ...) do {if ((t) == INF) {add_msg_tout(__VA_ARGS__);}} while(0)
 #endif
#else
 #define msg(t, ...) do {} while(0)
#endif

#endif
