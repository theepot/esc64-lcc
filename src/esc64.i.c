#include "esc64.i.h"

static void progend(void)
{
}

static void progbeg(int argc, char *argv[])
{
	int i;

	{
		union { char c; int i; } u;
		u.i = 0;
		u.c = 1;
		swap = ((int)(u.i == 1)) != IR->little_endian;
	}

	parseflags(argc, argv);

	for(i = 0; i < 4; ++i)
	{
		ireg[i] = mkreg("r%d", i, 1, IREG);
	}

	iregw = mkwildcard(ireg);

	tmask[IREG] = TREG_MASK;
	vmask[IREG] = 0;
	tmask[FREG] = 0;
	vmask[FREG] = 0;

	print(
		";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n"
		";; ESC64 assembly generated by LCC 4.2 ;;\n"
		";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n"
		"\n"
		"#include <esccrt.inc>\n\n");

	cur_seg = -1;
}

static Symbol rmap(int opk)
{
	switch (optype(opk))
	{
		case I:
		case U:
		case P:
		case B:
			return iregw;
		case F:
			assert(0 && "rmap(): case F");
			return 0;
		default:
			return 0;
	}
}

static void target(Node p)
{
	assert(p);

	switch (specific(p->op))
	{
	case CALL+I:
	case CALL+U:
	case CALL+P: //case CALL+V:
		setreg(p, ireg[0]); //return value is in r0
		break;

	case RET+I:
	case RET+U:
	case RET+P:
		rtarget(p, 0, ireg[0]); //always return in r0
		break;

	case CVI+I:
		if(opsize(p->op) == 2)
		{
			assert(opsize(p->kids[0]->op) == 1 && "target(): CVI+I");
			rtarget(p, 0, ireg[0]);
			setreg(p, ireg[0]);
		}
		break;

	case ASGN+B:
		rtarget(p, 0, ireg[0]);
		rtarget(p->kids[1], 0, ireg[1]);
		break;

	case DIV+I:
	case DIV+U:
	case MUL+I:
	case MUL+U:
	case LSH+I:
	case LSH+U:
	case RSH+I:
	case RSH+U:
		assert(opsize(p->op) == 2 && "target(): div/mul/shift opsize != 2");
		setreg(p, ireg[0]);
		rtarget(p, 0, ireg[0]);
		rtarget(p, 1, ireg[1]);
		break;
		
	case MOD+I:
	case MOD+U:
		assert(opsize(p->op) == 2 && "target(): mod opsize != 2");
		setreg(p, ireg[1]);
		rtarget(p, 0, ireg[0]);
		rtarget(p, 1, ireg[1]);
		break;

	default:
		break;
	}
}

static void clobber(Node p)
{
	assert(p);

	switch (specific(p->op))
	{
	case ASGN+B:
		spill(1 << 2, IREG, p);
		break;
		
	case DIV+I:
	case DIV+U:
		spill(1 << 1, IREG, p);
		break;
		
	case MOD+I:
	case MOD+U:
		spill(1 << 0, IREG, p);
		break;
//	case CALL + I:
//	case CALL + P:
//	case CALL + U:
//		spill(TREG_MASK & ~1, IREG, p); //dont spill r0 (return reg)
//		break;
//	case CALL + V:
//		spill(TREG_MASK, IREG, p);
//		break;
	}
}

static void emit2(Node p)
{
	switch(specific(p->op))
	{
	case LABEL+V:
		assert(p->syms[0]);
		if(p->syms[0]->sclass != STATIC && !p->syms[0]->generated)	{ print(".global "); }
		print("%s:\t;emit2(): case LABEL\n", p->syms[0]->x.name);
		break;

	case ASGN+I:
	case ASGN+U:
	case ASGN+P:
		if(p->kids[0]->op == VREG+P)
		{
			fprint(stderr, "emit2(): case ASGN: p->kids[0]->op == VREG+P\n");
			break;
		}
		assert(specific(p->kids[0]->op) == ADDRL+P || specific(p->kids[0]->op) == ADDRF+P);
		assert(opsize(p->op) == 1 || opsize(p->op) == 2);

		print("\tmov\t\t__tmpreg, %d\t\t;emit2a\n", p->kids[0]->syms[0]->x.offset);
		print("\tadd\t\t__tmpreg, __bp, __tmpreg\t\t;emit2b\n");
		print("\t%s\t\t__tmpreg, %s\t\t;emit2c\n", opsize(p->op) == 2 ? "st" : "stb", p->kids[1]->syms[RX]->x.name);
		break;
	}
}

static void doarg(Node p)
{
	assert(p && p->syms[0]);
	mkactual(2, p->syms[0]->u.c.v.i);
}

static void local(Symbol p)
{
	if(askregvar(p, rmap(ttob(p->type))) == 0)
	{
		mkauto(p);
	}
}

//CALL FRAME//
//
//	|                    | HIGH ADDRESSES
//	|  caller frame      |
//	|____________________|
//	|                    |
//	|  arguments         |
//	|____________________| 10*  NOTE: if the function returns a structure, the last actual argument will be
//	|  return  (lo)      | 9         a pointer to an area of memory to return the structure in.
//	|__________(hi)______| 8         the last "real" argument will be at bp+14
//	|  saves r1(lo)      | 7
//	|          (hi)      | 6
//	|        r2(lo)      | 5
//	|          (hi)      | 4
//	|        r3(lo)      | 3
//	|          (hi)      | 2
//	|        bp(lo)      | 1
//	|__________(hi)______| 0  <- base pointer
//	|                    |
//	|  locals + temps    |
//	|____________________|
//	|                    |
//	|  callee frame      |
//	|                    | LOW ADDRESSES
//
//////////////
//* if the function returns a structure, the last actual argument will be
//	a pointer to an area of memory to return the structure in.
//	the last "real" argument will be at 2 higher
//
//  if the function doesn't return anything, r0 is also saved, the last actual argument
//  will be at 2 higher
static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls)
{
	int i;

	//function start
	print(".pad 2\n");
	if(f->sclass != STATIC)	{ print(".global "); }
	print("%s:\t;function()\n", f->x.name);

	int voidfunc = f->type->type->op == VOID;

	//save temps + frame pointer, set up frame pointer
	if(voidfunc)	{ print("\tpush\tr0\n"); }
	print(
		"\tpush\tr1\n"
		"\tpush\tr2\n"
		"\tpush\tr3\n"
		"\tpush\t__bp\n"
		"\tmov\t\t__bp, sp\n");

	//arguments
	usedmask[0] = usedmask[1] = 0;
	freemask[0] = freemask[1] = ~(unsigned)0;
	offset = 10; //saved registers + return address
	offset = voidfunc ? 12 : 10;

	for (i = 0; callee[i]; i++)
	{
		Symbol p = callee[i];
		Symbol q = caller[i];
		assert(q);
		p->x.offset = q->x.offset = offset;
		p->x.name = q->x.name = stringf("%d", p->x.offset);
		p->sclass = q->sclass = AUTO;
		offset += roundup(q->type->size, 2);
	}
	assert(caller[i] == 0);
	offset = maxoffset = 0;

	//generate code
	gencode(caller, callee);

	//allocate space in frame
	framesize = roundup(maxoffset, 2);
	if(framesize > 0)
	{
		print(
			"\tmov\t\t__tmpreg, %d\n"
			"\tsub\t\tsp, sp, __tmpreg\t\t;allocate space for locals\n",
			framesize);
	}

	print("\n");

	//emit code
	emitcode();

	//restore sp, frame pointer and temps
	print(
		"\tmov\t\tsp, __bp\n"
		"\tpop\t\t__bp\n"
		"\tpop\t\tr3\n"
		"\tpop\t\tr2\n"
		"\tpop\t\tr1\n");
	if(voidfunc)	{ print("\tpop\tr0\n"); }
	print(
		"\tret\n"
		";;end %s\n\n", f->x.name);
}

static void defconst(int suffix, int size, Value v)
{
	switch(suffix)
	{
	case F:
		assert(0 && "defconst(): no floating point support");
		break;

	case I:
	case U:
		switch(size)
		{
		case 1:
			print(".byte\t0x%X\n", (unsigned)(unsigned char)(suffix == I ? v.i : v.u));
			break;

		case 2:
			print(".word\t0x%X\n", (unsigned)(unsigned short)(suffix == I ? v.i : v.u));
			break;

		case 4:
			assert(0 && "defconst(): no int32 support");
			break;

		default:
			assert(0 && "defconst(): I or U with size != 1, 2 or 4");
			break;
		}
		break;

	case P:
		assert(size == 2 && "defconst(): case P: size != 2");
		print(".word\t0x%X\n", (unsigned)v.p);
		break;

	default:
		assert(0 && "defconst(): unknown suffix");
		break;
	}
}

static void defaddress(Symbol p)
{
	print(".word\t%s\n", p->x.name);
}

static void defstring(int n, char *str)
{
	char *s;
	for (s = str; s < str + n; ++s)
	{
		print(".byte %d\n", (*s) & 0xFF);
	}
}

static void export(Symbol p)
{
}

static void import(Symbol p)
{
}

static void defsymbol(Symbol p)
{
	if(p->scope >= LOCAL && p->sclass == STATIC)
	{
		p->x.name = stringf("__L%d", genlabel(1));
	}
	else if (p->generated)
	{
		p->x.name = stringf("__L%s", p->name);
	}
	else
	{
		assert(p->scope != CONSTANTS || isint(p->type) || isptr(p->type)); //what's this for?
		p->x.name = p->name;
	}
}

static void address(Symbol q, Symbol p, long n)
{
	fprint(stderr, "address(): q=%s is at p->x.offset(%d) + n(%d)\n", p->x.name, p->x.offset, n);

	if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN)
	{
		q->x.name = stringf("%s%s%D", p->x.name, n >= 0 ? "+" : "", n);
	}
	else
	{
		assert(n <= INT_MAX && n >= INT_MIN);
		q->x.offset = p->x.offset + n;
		q->x.name = stringd(q->x.offset);
	}
}

static void global(Symbol p)
{
	print(".pad\t%d\n", p->type->align);
	if(p->sclass != STATIC && !p->generated)	{ print(".global "); }
	print("%s:\t;global()\n", p->x.name);
}

static void segment(int n)
{
	switch(n)
	{
	case CODE:
	case DATA:
	case LIT:
		if(cur_seg == CODE || cur_seg == DATA || cur_seg == LIT) { return; }
		print(".data\n");
		print(".align 2\n");
		break;
	case BSS:
		if(cur_seg == BSS) { return; }
		print(".bss\n");
		print(".align 2\n");
		break;
	default:
		assert(0 && "segment(): unknown segment");
		break;
	}

	cur_seg = n;
}

static void space(int n)
{
	if(cseg != BSS)
	{
		print(".resb\t%d\n", n);
	}
}

static void blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[])
{
	assert(0 && "blkloop(): should never be called");
}

static void blkfetch(int size, int off, int reg, int tmp)
{
	assert(0 && "blkfetch(): should never be called");
}

static void blkstore(int size, int off, int reg, int tmp)
{
	assert(0 && "blkstore(): should never be called");
}

Interface esc64IR =
{
	/* type metrics
	size, align, outofline */
	1, 1, 0, /* char */
	2, 2, 0, /* short */
	2, 2, 0, /* int */
	2, 2, 0, /* long */
	2, 2, 0, /* long long */
	4, 4, 1, /* float */
	8, 8, 1, /* double */
	8, 8, 1, /* long double */
	2, 2, 0, /* T* */
	0, 1, 0, /* struct */

	1, /* little_endian */
	0, /* mulops_calls */
	0, /* wants_callb */
	0, /* wants_argb */
	0, /* left_to_right */
	0, /* wants_dag */
	0, /* unsigned_char */

	/* routines */
	address, blockbeg, blockend, defaddress, defconst, defstring, defsymbol, emit,
	export, function, gen, global, import, local, progbeg, progend, segment,
	space,
	0, 0, 0, 0, 0, 0, 0, /* stabX routines */

	/* Xinterface */
	{
		1, /* max_unaligned_load */
		/* BURM routines */
		rmap, blkfetch, blkstore, blkloop, _label, _rule, _nts, _kids, _string,
		_templates, _isinstruction, _ntname, emit2, doarg, target,
		clobber,
	}
};

static char rcsid[] = "$Id$";
