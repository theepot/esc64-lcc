#ifndef ESC64_I_H_INCLUDED
#define ESC64_I_H_INCLUDED

#include "c.h"
#define NODEPTR_TYPE Node
#define OP_LABEL(p) ((p)->op)
#define LEFT_CHILD(p) ((p)->kids[0])
#define RIGHT_CHILD(p) ((p)->kids[1])
#define STATE_LABEL(p) ((p)->x.state)

static void address(Symbol, Symbol, long);
static void blkfetch(int, int, int, int);
static void blkloop(int, int, int, int, int, int[]);
static void blkstore(int, int, int, int);
static void defaddress(Symbol);
static void defconst(int, int, Value);
static void defstring(int, char *);
static void defsymbol(Symbol);
static void doarg(Node);
static void emit2(Node);
static void export(Symbol);
static void clobber(Node);
static void function(Symbol, Symbol [], Symbol [], int);
static void global(Symbol);
static void import(Symbol);
static void local(Symbol);
static void progbeg(int, char **);
static void progend(void);
static void segment(int);
static void space(int);
static void target(Node);
static int imm(Node);
static void renameregs(void);
extern Interface sparcIR, solarisIR;
static void defsymbol2(Symbol);
static void export2(Symbol);
static void globalend(void);
static void global2(Symbol);
static void segment2(int);
static void progend2(void);

extern char *stabprefix;
extern void stabblock(int, int, Symbol*);
extern void stabend(Coordinate *, Symbol, Coordinate **, Symbol *, Symbol *);
extern void stabfend(Symbol, int);
extern void stabinit(char *, int, char *[]);
extern void stabline(Coordinate *);
extern void stabsym(Symbol);
extern void stabtype(Symbol);

#endif
