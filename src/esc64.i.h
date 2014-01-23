#ifndef ESC64_I_H_INCLUDED
#define ESC64_I_H_INCLUDED

//	registers:
//		r0-r3	temporaries
//		r4		frame pointer, reserved
//		r5		reserved
//		sp		reserved
//		pc		reserved

#define REG_SP		6
#define REG_PC		7

#define TREG_MASK	0xF

#define readsreg(p) \
        (generic((p)->op)==INDIR && (p)->kids[0]->op==VREG+P)
#define setsrc(d) ((d) && (d)->x.regnode && \
        (d)->x.regnode->set == src->x.regnode->set && \
        (d)->x.regnode->mask&src->x.regnode->mask)

#define relink(a, b) ((b)->x.prev = (a), (a)->x.next = (b))

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
static int bitcount(unsigned);
static Symbol argreg(int, int, int, int, int);

static Symbol ireg[32];
static Symbol iregw;

//static Symbol reg5;
//static Symbol regsp;
//static Symbol regpc;

//TODO remove me!!!
static Symbol freg2[32];
static Symbol d6;
static int pic;
static int gnum;
static int cseg;
static const char* currentfile;
//end

static int cur_seg;

static void stabinit(char *, int, char *[]);
static void stabline(Coordinate *);
static void stabsym(Symbol);

#endif
