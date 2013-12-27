#include "esc64.i.h"

static void progend(void) {
}

static void progbeg(int argc, char *argv[]) {
	int i;

	{
		union {
			char c;
			int i;
		} u;
		u.i = 0;
		u.c = 1;
		swap = ((int) (u.i == 1)) != IR->little_endian;
	}
	print(".set reorder\n");
	pic = !IR->little_endian;
	parseflags(argc, argv);
	for (i = 0; i < argc; i++)
		if (strncmp(argv[i], "-G", 2) == 0)
			gnum = atoi(argv[i] + 2);
		else if (strcmp(argv[i], "-pic=1") == 0
				|| strcmp(argv[i], "-pic=0") == 0)
			pic = argv[i][5] - '0';
	for (i = 0; i < 31; i += 2)
		freg2[i] = mkreg("%d", i, 3, FREG);
	for (i = 0; i < 32; i++)
		ireg[i] = mkreg("%d", i, 1, IREG);
	ireg[29]->x.name = "sp";
	d6 = mkreg("6", 6, 3, IREG);
	freg2w = mkwildcard(freg2);
	iregw = mkwildcard(ireg);
	tmask[IREG] = INTTMP;
	tmask[FREG] = FLTTMP;
	vmask[IREG] = INTVAR;
	vmask[FREG] = FLTVAR;
	blkreg = mkreg("8", 8, 7, IREG);
}

static Symbol rmap(int opk) {
	switch (optype(opk)) {
	case I:
	case U:
	case P:
	case B:
		return iregw;
	case F:
		return freg2w;
	default:
		return 0;
	}
}

static void target(Node p) {
	assert(p);
	switch (specific(p->op)) {
	case CNST + I:
	case CNST + U:
	case CNST + P:
		if (range(p, 0, 0) == 0) {
			setreg(p, ireg[0]);
			p->x.registered = 1;
		}
		break;
	case CALL + V:
		rtarget(p, 0, ireg[25]);
		break;
	case CALL + F:
		rtarget(p, 0, ireg[25]);
		setreg(p, freg2[0]);
		break;
	case CALL + I:
	case CALL + P:
	case CALL + U:
		rtarget(p, 0, ireg[25]);
		setreg(p, ireg[2]);
		break;
	case RET + F:
		rtarget(p, 0, freg2[0]);
		break;
	case RET + I:
	case RET + U:
	case RET + P:
		rtarget(p, 0, ireg[2]);
		break;
	case ARG + F:
	case ARG + I:
	case ARG + P:
	case ARG + U: {
		static int ty0;
		int ty = optype(p->op);
		Symbol q;

		q = argreg(p->x.argno, p->syms[2]->u.c.v.i, ty, opsize(p->op), ty0);
		if (p->x.argno == 0)
			ty0 = ty;
		if (q && !(ty == F && q->x.regnode->set == IREG))
			rtarget(p, 0, q);
		break;
	}
	case ASGN + B:
		rtarget(p->kids[1], 0, blkreg);
		break;
	case ARG + B:
		rtarget(p->kids[0], 0, blkreg);
		break;
	}
}

static void clobber(Node p) {
	assert(p);
	switch (specific(p->op)) {
	case CALL + F:
		spill(INTTMP | INTRET, IREG, p);
		spill(FLTTMP, FREG, p);
		break;
	case CALL + I:
	case CALL + P:
	case CALL + U:
		spill(INTTMP, IREG, p);
		spill(FLTTMP | FLTRET, FREG, p);
		break;
	case CALL + V:
		spill(INTTMP | INTRET, IREG, p);
		spill(FLTTMP | FLTRET, FREG, p);
		break;
	}
}

static void emit2(Node p) {
	int dst, n, src, sz, ty;
	static int ty0;
	Symbol q;

	switch (specific(p->op)) {
	case ARG + F:
	case ARG + I:
	case ARG + P:
	case ARG + U:
		ty = optype(p->op);
		sz = opsize(p->op);
		if (p->x.argno == 0)
			ty0 = ty;
		q = argreg(p->x.argno, p->syms[2]->u.c.v.i, ty, sz, ty0);
		src = getregnum(p->x.kids[0]);
		if (q == NULL && ty == F && sz == 4)
			print("s.s $f%d,%d($sp)\n", src, p->syms[2]->u.c.v.i);
		else if (q == NULL && ty == F)
			print("s.d $f%d,%d($sp)\n", src, p->syms[2]->u.c.v.i);
		else if (q == NULL )
			print("sw $%d,%d($sp)\n", src, p->syms[2]->u.c.v.i);
		else if (ty == F && sz == 4 && q->x.regnode->set == IREG)
			print("mfc1 $%d,$f%d\n", q->x.regnode->number, src);
		else if (ty == F && q->x.regnode->set == IREG)
			print("mfc1.d $%d,$f%d\n", q->x.regnode->number, src);
		break;
	case ASGN + B:
		dalign = salign = p->syms[1]->u.c.v.i;
		blkcopy(getregnum(p->x.kids[0]), 0, getregnum(p->x.kids[1]), 0,
				p->syms[0]->u.c.v.i, tmpregs);
		break;
	case ARG + B:
		dalign = 4;
		salign = p->syms[1]->u.c.v.i;
		blkcopy(29, p->syms[2]->u.c.v.i, getregnum(p->x.kids[0]), 0,
				p->syms[0]->u.c.v.i, tmpregs);
		n = p->syms[2]->u.c.v.i + p->syms[0]->u.c.v.i;
		dst = p->syms[2]->u.c.v.i;
		for (; dst <= 12 && dst < n; dst += 4)
			print("lw $%d,%d($sp)\n", (dst / 4) + 4, dst);
		break;
	}
}

static Symbol argreg(int argno, int offset, int ty, int sz, int ty0) {
	assert((offset&3) == 0);
	if (offset > 12)
		return NULL ;
	else if (argno == 0 && ty == F)
		return freg2[12];
	else if (argno == 1 && ty == F && ty0 == F)
		return freg2[14];
	else if (argno == 1 && ty == F && sz == 8)
		return d6; /* Pair! */
	else
		return ireg[(offset / 4) + 4];
}
static void doarg(Node p) {
	static int argno;
	int align;

	if (argoffset == 0)
		argno = 0;
	p->x.argno = argno++;
	align = p->syms[1]->u.c.v.i < 4 ? 4 : p->syms[1]->u.c.v.i;
	p->syms[2] = intconst(mkactual(align, p->syms[0]->u.c.v.i));
}

static void local(Symbol p) {
	if (askregvar(p, rmap(ttob(p->type))) == 0)
		mkauto(p);
}

static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls) {
	int i, saved, sizefsave, sizeisave, varargs;
	Symbol r, argregs[4];

	usedmask[0] = usedmask[1] = 0;
	freemask[0] = freemask[1] = ~(unsigned) 0;
	offset = maxoffset = maxargoffset = 0;
	for (i = 0; callee[i]; i++)
		;
	varargs = variadic(f->type)
			|| i > 0 && strcmp(callee[i - 1]->name, "va_alist") == 0;
	for (i = 0; callee[i]; i++) {
		Symbol p = callee[i];
		Symbol q = caller[i];
		assert(q);
		offset = roundup(offset, q->type->align);
		p->x.offset = q->x.offset = offset;
		p->x.name = q->x.name = stringd(offset);
		r = argreg(i, offset, optype(ttob(q->type)), q->type->size,
				optype(ttob(caller[0]->type)));
		if (i < 4)
			argregs[i] = r;
		offset = roundup(offset + q->type->size, 4);
		if (varargs)
			p->sclass = AUTO;
		else if (r && ncalls == 0 && !isstruct(q->type) && !p->addressed
				&& !(isfloat(q->type) && r->x.regnode->set == IREG)) {
			p->sclass = q->sclass = REGISTER;
			askregvar(p, r);
			assert(p->x.regnode && p->x.regnode->vbl == p);
			q->x = p->x;
			q->type = p->type;
		} else if (askregvar(p, rmap(ttob(p->type))) && r != NULL
				&& (isint(p->type) || p->type == q->type)) {
			assert(q->sclass != REGISTER);
			p->sclass = q->sclass = REGISTER;
			q->type = p->type;
		}
	}
	assert(!caller[i]);
	offset = 0;
	gencode(caller, callee);
	if (ncalls)
		usedmask[IREG] |= ((unsigned) 1) << 31;
	usedmask[IREG] &= 0xc0ff0000;
	usedmask[FREG] &= 0xfff00000;
	if (pic && ncalls)
		usedmask[IREG] |= 1 << 25;
	maxargoffset = roundup(maxargoffset, usedmask[FREG] ? 8 : 4);
	if (ncalls && maxargoffset < 16)
		maxargoffset = 16;
	sizefsave = 4 * bitcount(usedmask[FREG]);
	sizeisave = 4 * bitcount(usedmask[IREG]);
	framesize = roundup(maxargoffset + sizefsave
			+ sizeisave + maxoffset, 16);
	segment(CODE);
	print(".align 2\n");
	print(".ent %s\n", f->x.name);
	print("%s:\n", f->x.name);
	i = maxargoffset + sizefsave - framesize;
	print(".frame $sp,%d,$31\n", framesize);
	if (pic)
		print(".set noreorder\n.cpload $25\n.set reorder\n");
	if (framesize > 0)
		print("addu $sp,$sp,%d\n", -framesize);
	if (usedmask[FREG])
		print(".fmask 0x%x,%d\n", usedmask[FREG], i - 8);
	if (usedmask[IREG])
		print(".mask 0x%x,%d\n", usedmask[IREG], i + sizeisave - 4);
	saved = maxargoffset;
	for (i = 20; i <= 30; i += 2)
		if (usedmask[FREG] & (3 << i)) {
			print("s.d $f%d,%d($sp)\n", i, saved);
			saved += 8;
		}

	for (i = 16; i <= 31; i++)
		if (usedmask[IREG] & (1 << i)) {
			if (i == 25)
				print(".cprestore %d\n", saved);
			else
				print("sw $%d,%d($sp)\n", i, saved);
			saved += 4;
		}
	for (i = 0; i < 4 && callee[i]; i++) {
		r = argregs[i];
		if (r && r->x.regnode != callee[i]->x.regnode) {
			Symbol out = callee[i];
			Symbol in = caller[i];
			int rn = r->x.regnode->number;
			int rs = r->x.regnode->set;
			int tyin = ttob(in->type);

			assert(out && in && r && r->x.regnode);
			assert(out->sclass != REGISTER || out->x.regnode);
			if (out->sclass == REGISTER
					&& (isint(out->type) || out->type == in->type)) {
				int outn = out->x.regnode->number;
				if (rs == FREG && tyin == F + sizeop(8))
					print("mov.d $f%d,$f%d\n", outn, rn);
				else if (rs == FREG && tyin == F + sizeop(4))
					print("mov.s $f%d,$f%d\n", outn, rn);
				else if (rs == IREG && tyin == F + sizeop(8))
					print("mtc1.d $%d,$f%d\n", rn, outn);
				else if (rs == IREG && tyin == F + sizeop(4))
					print("mtc1 $%d,$f%d\n", rn, outn);
				else
					print("move $%d,$%d\n", outn, rn);
			} else {
				int off = in->x.offset + framesize;
				if (rs == FREG && tyin == F + sizeop(8))
					print("s.d $f%d,%d($sp)\n", rn, off);
				else if (rs == FREG && tyin == F + sizeop(4))
					print("s.s $f%d,%d($sp)\n", rn, off);
				else {
					int i, n = (in->type->size + 3) / 4;
					for (i = rn; i < rn + n && i <= 7; i++)
						print("sw $%d,%d($sp)\n", i, off + (i - rn) * 4);
				}
			}
		}
	}
	if (varargs && callee[i - 1]) {
		i = callee[i - 1]->x.offset + callee[i - 1]->type->size;
		for (i = roundup(i, 4) / 4; i <= 3; i++)
			print("sw $%d,%d($sp)\n", i + 4, framesize + 4 * i);
	}
	emitcode();
	saved = maxargoffset;
	for (i = 20; i <= 30; i += 2)
		if (usedmask[FREG] & (3 << i)) {
			print("l.d $f%d,%d($sp)\n", i, saved);
			saved += 8;
		}
	for (i = 16; i <= 31; i++)
		if (usedmask[IREG] & (1 << i)) {
			print("lw $%d,%d($sp)\n", i, saved);
			saved += 4;
		}
	if (framesize > 0)
		print("addu $sp,$sp,%d\n", framesize);
	print("j $31\n");
	print(".end %s\n", f->x.name);
}

static void defconst(int suffix, int size, Value v) {
	if (suffix == F && size == 4) {
		float f = v.d;
		print(".word 0x%x\n", *(unsigned *) &f);
	} else if (suffix == F && size == 8) {
		double d = v.d;
		unsigned *p = (unsigned *) &d;
		print(".word 0x%x\n.word 0x%x\n", p[swap], p[!swap]);
	} else if (suffix == P)
		print(".word 0x%x\n", (unsigned) v.p);
	else if (size == 1)
		print(".byte 0x%x\n",
				(unsigned) ((unsigned char) (suffix == I ? v.i : v.u)));
	else if (size == 2)
		print(".half 0x%x\n",
				(unsigned) ((unsigned short) (suffix == I ? v.i : v.u)));
	else if (size == 4)
		print(".word 0x%x\n", (unsigned) (suffix == I ? v.i : v.u));
}

static void defaddress(Symbol p) {
	if (pic && p->scope == LABELS)
		print(".gpword %s\n", p->x.name);
	else
		print(".word %s\n", p->x.name);
}

static void defstring(int n, char *str) {
	char *s;

	for (s = str; s < str + n; s++)
		print(".byte %d\n", (*s) & 0377);
}

static void export(Symbol p) {
	print(".globl %s\n", p->x.name);
}

static void import(Symbol p) {
	if (!isfunc(p->type))
		print(".extern %s %d\n", p->name, p->type->size);
}

static void defsymbol(Symbol p) {
	if (p->scope >= LOCAL && p->sclass == STATIC)
		p->x.name = stringf("L.%d", genlabel(1));
	else if (p->generated)
		p->x.name = stringf("L.%s", p->name);
	else
		assert(p->scope != CONSTANTS || isint(p->type) || isptr(p->type)), p->x.name =
				p->name;
}

static void address(Symbol q, Symbol p, long n) {
	if (p->scope == GLOBAL || p->sclass == STATIC || p->sclass == EXTERN)
		q->x.name = stringf("%s%s%D", p->x.name, n >= 0 ? "+" : "", n);
	else {
		assert(n <= INT_MAX && n >= INT_MIN);
		q->x.offset = p->x.offset + n;
		q->x.name = stringd(q->x.offset);
	}
}

static void global(Symbol p) {
	if (p->u.seg == BSS) {
		if (p->sclass == STATIC || Aflag >= 2)
			print(".lcomm %s,%d\n", p->x.name, p->type->size);
		else
			print(".comm %s,%d\n", p->x.name, p->type->size);
	} else {
		if (p->u.seg == DATA && (p->type->size == 0 || p->type->size > gnum))
			print(".data\n");
		else if (p->u.seg == DATA)
			print(".sdata\n");
		print(".align %c\n", ".01.2...3"[p->type->align]);
		print("%s:\n", p->x.name);
	}
}

static void segment(int n) {
	cseg = n;
	switch (n) {
	case CODE:
		print(".text\n");
		break;
	case LIT:
		print(".rdata\n");
		break;
	}
}

static void space(int n) {
	if (cseg != BSS)
		print(".space %d\n", n);
}

static void blkloop(int dreg, int doff, int sreg, int soff, int size,
		int tmps[]) {
	int lab = genlabel(1);

	print("addu $%d,$%d,%d\n", sreg, sreg, size & ~7);
	print("addu $%d,$%d,%d\n", tmps[2], dreg, size & ~7);
	blkcopy(tmps[2], doff, sreg, soff, size & 7, tmps);
	print("L.%d:\n", lab);
	print("addu $%d,$%d,%d\n", sreg, sreg, -8);
	print("addu $%d,$%d,%d\n", tmps[2], tmps[2], -8);
	blkcopy(tmps[2], doff, sreg, soff, 8, tmps);
	print("bltu $%d,$%d,L.%d\n", dreg, tmps[2], lab);
}

static void blkfetch(int size, int off, int reg, int tmp) {
	assert(size == 1 || size == 2 || size == 4);
	if (size == 1)
		print("lbu $%d,%d($%d)\n", tmp, off, reg);
	else if (salign >= size && size == 2)
		print("lhu $%d,%d($%d)\n", tmp, off, reg);
	else if (salign >= size)
		print("lw $%d,%d($%d)\n", tmp, off, reg);
	else if (size == 2)
		print("ulhu $%d,%d($%d)\n", tmp, off, reg);
	else
		print("ulw $%d,%d($%d)\n", tmp, off, reg);
}

static void blkstore(int size, int off, int reg, int tmp) {
	if (size == 1)
		print("sb $%d,%d($%d)\n", tmp, off, reg);
	else if (dalign >= size && size == 2)
		print("sh $%d,%d($%d)\n", tmp, off, reg);
	else if (dalign >= size)
		print("sw $%d,%d($%d)\n", tmp, off, reg);
	else if (size == 2)
		print("ush $%d,%d($%d)\n", tmp, off, reg);
	else
		print("usw $%d,%d($%d)\n", tmp, off, reg);
}

static int bitcount(unsigned mask) {
	unsigned i, n = 0;

	for (i = 1; i; i <<= 1)
		if (mask & i)
			n++;
	return n;
}

/* stabinit - initialize stab output */
static void stabinit(char *file, int argc, char *argv[]) {
	if (file) {
		print(".file 2,\"%s\"\n", file);
		currentfile = file;
	}
}

/* stabline - emit stab entry for source coordinate *cp */
static void stabline(Coordinate *cp) {
	if (cp->file && cp->file != currentfile) {
		print(".file 2,\"%s\"\n", cp->file);
		currentfile = cp->file;
	}
	print(".loc 2,%d\n", cp->y);
}

/* stabsym - output a stab entry for symbol p */
static void stabsym(Symbol p) {
	if (p == cfunc && IR->stabline)
		(*IR->stabline)(&p->src);
}

Interface esc64IR = { 1, 1, 0, /* char */
2, 2, 0, /* short */
4, 4, 0, /* int */
4, 4, 0, /* long */
4, 4, 0, /* long long */
4, 4, 1, /* float */
8, 8, 1, /* double */
8, 8, 1, /* long double */
4, 4, 0, /* T * */
0, 1, 0, /* struct */
1, /* little_endian */
0, /* mulops_calls */
0, /* wants_callb */
1, /* wants_argb */
1, /* left_to_right */
0, /* wants_dag */
0, /* unsigned_char */
address, blockbeg, blockend, defaddress, defconst, defstring, defsymbol, emit,
		export, function, gen, global, import, local, progbeg, progend, segment,
		space, 0, 0, 0, stabinit, stabline, stabsym, 0, { 4, /* max_unaligned_load */
		rmap, blkfetch, blkstore, blkloop, _label, _rule, _nts, _kids, _string,
				_templates, _isinstruction, _ntname, emit2, doarg, target,
				clobber,

		} };

static char rcsid[] = "$Id$";
