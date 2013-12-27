#include "esc64.i.h"

static Symbol greg[32], gregw;
static Symbol *oreg = &greg[8], *ireg = &greg[24];
static Symbol freg[32], freg2[32];
static Symbol fregw, freg2w;

static int regvars;
static int retstruct;

static int pflag = 0;

static int cseg;

static void progend(void){}
static void progbeg(int argc, char *argv[]) {
        int i;

        {
                union {
                        char c;
                        int i;
                } u;
                u.i = 0;
                u.c = 1;
                swap = ((int)(u.i == 1)) != IR->little_endian;
        }
        parseflags(argc, argv);
        for (i = 0; i < argc; i++)
                if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-pg") == 0)
                        pflag = 1;
        if (IR == &solarisIR)
                stabprefix = ".LL";
        else
                stabprefix = "L";
        for (i = 0; i < 8; i++) {
                greg[i +  0] = mkreg(stringf("g%d", i), i +  0, 1, IREG);
                greg[i +  8] = mkreg(stringf("o%d", i), i +  8, 1, IREG);
                greg[i + 16] = mkreg(stringf("l%d", i), i + 16, 1, IREG);
                greg[i + 24] = mkreg(stringf("i%d", i), i + 24, 1, IREG);
        }
        gregw = mkwildcard(greg);
        for (i = 0; i < 32; i++)
                freg[i]  = mkreg("%d", i, 1, FREG);
        for (i = 0; i < 31; i += 2)
                freg2[i] = mkreg("%d", i, 3, FREG);
        fregw = mkwildcard(freg);
        freg2w = mkwildcard(freg2);
        tmask[IREG] = 0x3fff3e00;
        vmask[IREG] = 0x3ff00000;
        tmask[FREG]  = ~(unsigned)0;
        vmask[FREG]  = 0;
}
static Symbol rmap(int opk) {
        switch (optype(opk)) {
        case I: case U: case P: case B:
                return gregw;
        case F:
                return opsize(opk) == 4 ? fregw : freg2w;
        default:
                return 0;
        }
}
static void target(Node p) {
        assert(p);
        switch (specific(p->op)) {
        case CNST+I: case CNST+U: case CNST+P:
                if (range(p, 0, 0) == 0) {
                        setreg(p, greg[0]);
                        p->x.registered = 1;
                }
                break;
        case CALL+B:
                assert(p->syms[1] && p->syms[1]->type && isfunc(p->syms[1]->type));
                p->syms[1] = intconst(freturn(p->syms[1]->type)->size);
                break;
        case CALL+F: setreg(p, opsize(p->op)==4?freg[0]:freg2[0]);     break;
        case CALL+I: case CALL+P: case CALL+U:
        case CALL+V: setreg(p, oreg[0]);      break;
        case RET+F:  rtarget(p, 0, opsize(p->op)==4?freg[0]:freg2[0]);  break;
        case RET+I: case RET+P: case RET+U:
                rtarget(p, 0, ireg[0]);
                p->kids[0]->x.registered = 1;
                break;
        case ARG+I: case ARG+P: case ARG+U:
                if (p->syms[RX]->u.c.v.i < 6) {
                        rtarget(p, 0, oreg[p->syms[RX]->u.c.v.i]);
                        p->op = LOAD+opkind(p->op);
                        setreg(p, oreg[p->syms[RX]->u.c.v.i]);
                }
                break;
        }
}
static void clobber(Node p) {
        assert(p);
        switch (specific(p->op)) {
        case CALL+B: case CALL+F: case CALL+I:
                spill(~(unsigned)3, FREG, p);
                break;
        case CALL+V:
                spill(oreg[0]->x.regnode->mask, IREG, p);
                spill(~(unsigned)3, FREG, p);
                break;
        case ARG+F:
                if (opsize(p->op) == 4 && p->syms[2]->u.c.v.i <= 6)
                        spill((1<<(p->syms[2]->u.c.v.i + 8)), IREG, p);
                else if (opsize(p->op) == 8 && p->syms[2]->u.c.v.i <= 5)
                        spill((3<<(p->syms[2]->u.c.v.i + 8))&0xff00, IREG, p);
                break;
        }
}
static int imm(Node p) {
        return range(p, -4096, 4091);
}
static void doarg(Node p) {
        assert(p && p->syms[0] && p->op != ARG+B);
        p->syms[RX] = intconst(mkactual(4,
                p->syms[0]->u.c.v.i)/4);
}
static void emit2(Node p) {
        switch (p->op) {
        case INDIR+F+sizeop(8):
                if (generic(p->kids[0]->op) != VREG) {
                        int dst = getregnum(p);
                        print("ld ["); emitasm(p->kids[0], _base_NT); print(  "],%%f%d; ", dst);
                        print("ld ["); emitasm(p->kids[0], _base_NT); print("+4],%%f%d\n", dst+1);
                }
                break;
        case ASGN+F+sizeop(8):
                if (generic(p->kids[0]->op) != VREG) {
                        int src = getregnum(p->kids[1]);
                        print("st %%f%d,[", src);   emitasm(p->kids[0], _base_NT); print("]; ");
                        print("st %%f%d,[", src+1); emitasm(p->kids[0], _base_NT); print("+4]\n");
                }
                break;
        case ARG+F+sizeop(4): {
                int n = p->syms[RX]->u.c.v.i;
                print("st %%f%d,[%%sp+4*%d+68]\n",
                        getregnum(p->x.kids[0]), n);
                if (n <= 5)
                        print("ld [%%sp+4*%d+68],%%o%d\n", n, n);
                break;
        }
        case ARG+F+sizeop(8): {
                int n = p->syms[RX]->u.c.v.i;
                int src = getregnum(p->x.kids[0]);
                print("st %%f%d,[%%sp+4*%d+68]\n", src, n);
                print("st %%f%d,[%%sp+4*%d+68]\n", src+1, n+1);
                if (n <= 5)
                        print("ld [%%sp+4*%d+68],%%o%d\n", n, n);
                if (n <= 4)
                        print("ld [%%sp+4*%d+68],%%o%d\n", n+1, n+1);
                break;
        }
        case LOAD+F+sizeop(8): {
                int dst = getregnum(p);
                int src = getregnum(p->x.kids[0]);
                print("fmovs %%f%d,%%f%d; ", src,   dst);
                print("fmovs %%f%d,%%f%d\n", src+1, dst+1);
                break;
        }
        case NEG+F+sizeop(8): {
                int dst = getregnum(p);
                int src = getregnum(p->x.kids[0]);
                print("fnegs %%f%d,%%f%d; ", src,   dst);
                print("fmovs %%f%d,%%f%d\n", src+1, dst+1);
                break;
        }
        case ASGN+B: {
                static int tmpregs[] = { 1, 2, 3 };
                dalign = salign = p->syms[1]->u.c.v.i;
                blkcopy(getregnum(p->x.kids[0]), 0,
                        getregnum(p->x.kids[1]), 0,
                        p->syms[0]->u.c.v.i, tmpregs);
                break;
        }
        }
}
static void local(Symbol p) {
        if (retstruct) {
                assert(p == retv);
                p->x.name = stringd(4*16);
                p->x.offset = 4*16;
                p->sclass = AUTO;
                retstruct = 0;
                return;
        }
        if (isscalar(p->type) && !p->addressed && !isfloat(p->type))
                p->sclass = REGISTER;
        if (askregvar(p, rmap(ttob(p->type))) == 0)
                mkauto(p);
        else if (p->scope > LOCAL)
                regvars++;
}
static void function(Symbol f, Symbol caller[], Symbol callee[], int ncalls) {
        int autos = 0, i, leaf, reg, varargs;

        if (IR == &solarisIR)
                globalend();
        regvars = 0;
        for (i = 0; callee[i]; i++)
                ;
        varargs = variadic(f->type)
                || i > 0 && strcmp(callee[i-1]->name,
                        "__builtin_va_alist") == 0;
        usedmask[0] = usedmask[1] = 0;
        freemask[0] = freemask[1] = ~(unsigned)0;
        for (i = 0; i < 8; i++)
                ireg[i]->x.regnode->vbl = NULL;
        offset = 68;
        maxargoffset = 24;
        reg = 0;
        for (i = 0; callee[i]; i++) {
                Symbol p = callee[i], q = caller[i];
                int size = roundup(q->type->size, 4);
                assert(q);
                if (isfloat(p->type) || reg >= 6) {
                        p->x.offset = q->x.offset = offset;
                        p->x.name = q->x.name = stringd(offset);
                        p->sclass = q->sclass = AUTO;
                        autos++;
                }
                else if (p->addressed || varargs) {
                        p->x.offset = offset;
                        p->x.name = stringd(p->x.offset);
                        p->sclass = AUTO;
                        q->sclass = REGISTER;
                        askregvar(q, ireg[reg]);
                        assert(q->x.regnode);
                        autos++;
                }
                else {
                        p->sclass = q->sclass = REGISTER;
                        askregvar(p, ireg[reg]);
                        assert(p->x.regnode);
                        q->x.name = p->x.name;
                }
                offset += size;
                reg += isstruct(p->type) ? 1 : size/4;
        }
        assert(caller[i] == 0);
        offset = maxoffset = 0;
        retstruct = isstruct(freturn(f->type));
        gencode(caller, callee);
        maxargoffset = roundup(maxargoffset, 4);
        framesize = roundup(maxoffset + maxargoffset + 4*(16+1), 8);
        assert(!varargs || autos);
        leaf = (!ncalls
                && !maxoffset && !autos && !regvars
                && !isstruct(freturn(f->type))
                && !(usedmask[IREG]&0x00ffff01)
                && !(usedmask[FREG]&~(unsigned)3)
                && !pflag && !glevel);
        print(".align 4\n%s:\n", f->x.name);
        if (leaf) {
                for (i = 0; caller[i] && callee[i]; i++) {
                        Symbol p = caller[i], q = callee[i];
                        if (p->sclass == REGISTER && q->sclass == REGISTER) {
                                assert(q->x.regnode);
                                assert(q->x.regnode->set == IREG);
                                assert(q->x.regnode->number >= 24);
                                assert(q->x.regnode->number <= 31);
                                p->x.name = greg[q->x.regnode->number - 16]->x.name;
                        }
                }
                renameregs();
        } else if (framesize <= 4095)
                print("save %%sp,%d,%%sp\n", -framesize);
        else
                print("set %d,%%g1; save %%sp,%%g1,%%sp\n", -framesize);
        if (varargs)
                for (; reg < 6; reg++)
                        print("st %%i%d,[%%fp+%d]\n", reg, 4*reg + 68);
        else {
                offset = 4*(16 + 1);
                reg = 0;
                for (i = 0; caller[i]; i++) {
                        Symbol p = caller[i];
                        if (isfloat(p->type) && p->type->size == 8 && reg <= 4) {
                                print("st %%r%d,[%%fp+%d]\n",
                                        ireg[reg++]->x.regnode->number, offset);
                                print("st %%r%d,[%%fp+%d]\n",
                                        ireg[reg++]->x.regnode->number, offset + 4);
                        } else if (isfloat(p->type) && p->type->size == 4 && reg <= 5)
                                print("st %%r%d,[%%fp+%d]\n",
                                        ireg[reg++]->x.regnode->number, offset);
                        else
                                reg++;
                        offset += roundup(p->type->size, 4);
                }
        }
        if (pflag) {
                int lab = genlabel(1);
                print("set L%d,%%o0; call mcount; nop\n", lab);
                print(".seg \"data\"\n.align 4; L%d:.word 0\n.seg \"text\"\n", lab);
        }
        emitcode();
        if (isstruct(freturn(f->type)))
                print("jmp %%i7+12; restore\n");
        else if (!leaf)
                print("ret; restore\n");
        else {
                renameregs();
                print("retl; nop\n");
        }
        if (IR == &solarisIR) {
                print(".type %s,#function\n", f->x.name);
                print(".size %s,.-%s\n", f->x.name, f->x.name);
        }
}
#define exch(x, y, t) (((t) = x), ((x) = (y)), ((y) = (t)))

static void renameregs(void) {
        int i;

        for (i = 0; i < 8; i++) {
                char *ptmp;
                int itmp;
                if (ireg[i]->x.regnode->vbl)
                        ireg[i]->x.regnode->vbl->x.name = oreg[i]->x.name;
                exch(ireg[i]->x.name, oreg[i]->x.name, ptmp);
                exch(ireg[i]->x.regnode->number,
                        oreg[i]->x.regnode->number, itmp);
        }
}
static void defconst(int suffix, int size, Value v) {
        if (suffix == F && size == 4) {
                float f = v.d;
                print(".word 0x%x\n", *(unsigned *)&f);
        } else if (suffix == F && size == 8) {
                double d = v.d;
                unsigned *p = (unsigned *)&d;
                print(".word 0x%x\n.word 0x%x\n", p[swap], p[!swap]);
        } else if (suffix == P)
                print(".word 0x%x\n", (unsigned)v.p);
        else if (size == 1)
                print(".byte 0x%x\n", (unsigned)((unsigned char)(suffix == I ? v.i : v.u)));
        else if (size == 2)
                print(".half 0x%x\n", (unsigned)((unsigned short)(suffix == I ? v.i : v.u)));
        else if (size == 4)
                print(".word 0x%x\n", (unsigned)(suffix == I ? v.i : v.u));
        else assert(0);
}

static void defaddress(Symbol p) {
        print(".word %s\n", p->x.name);
}

static void defstring(int n, char *str) {
        char *s;

        for (s = str; s < str + n; s++)
                print(".byte %d\n", (*s)&0377);
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
static void export(Symbol p) {
        print(".global %s\n", p->x.name);
}
static void import(Symbol p) {}
static void defsymbol(Symbol p) {
        if (p->scope >= LOCAL && p->sclass == STATIC)
                p->x.name = stringf("%d", genlabel(1));
        else
                assert(p->scope != CONSTANTS || isint(p->type) || isptr(p->type)),
                p->x.name = p->name;
        if (p->scope >= LABELS)
                p->x.name = stringf(p->generated ? "L%s" : "_%s",
                        p->x.name);
}
static void segment(int n) {
        cseg = n;
        switch (n) {
        case CODE: print(".seg \"text\"\n"); break;
        case BSS:  print(".seg \"bss\"\n");  break;
        case DATA: print(".seg \"data\"\n"); break;
        case LIT:  print(".seg \"text\"\n"); break;
        }
}
static void space(int n) {
        if (cseg != BSS)
                print(".skip %d\n", n);
}
static void global(Symbol p) {
        print(".align %d\n", p->type->align);
        assert(p->u.seg);
        if (p->u.seg == BSS
        && (p->sclass == STATIC || Aflag >= 2))
                print(".reserve %s,%d\n", p->x.name, p->type->size);
        else if (p->u.seg == BSS)
                print(".common %s,%d\n",  p->x.name, p->type->size);
        else
                print("%s:\n", p->x.name);
}
static void blkfetch(int k, int off, int reg, int tmp) {
        assert(k == 1 || k == 2 || k == 4);
        assert(salign >= k);
        if (k == 1)
                print("ldub [%%r%d+%d],%%r%d\n", reg, off, tmp);
        else if (k == 2)
                print("lduh [%%r%d+%d],%%r%d\n", reg, off, tmp);
        else
                print("ld [%%r%d+%d],%%r%d\n",   reg, off, tmp);
}
static void blkstore(int k, int off, int reg, int tmp) {
        assert(k == 1 || k == 2 || k == 4);
        assert(dalign >= k);
        if (k == 1)
                print("stb %%r%d,[%%r%d+%d]\n", tmp, reg, off);
        else if (k == 2)
                print("sth %%r%d,[%%r%d+%d]\n", tmp, reg, off);
        else
                print("st %%r%d,[%%r%d+%d]\n",  tmp, reg, off);
}
static void blkloop(int dreg, int doff, int sreg, int soff, int size, int tmps[]) {
        if ((size&~7) < 4096) {
                print("add %%r%d,%d,%%r%d\n", sreg, size&~7, sreg);
                print("add %%r%d,%d,%%r%d\n", dreg, size&~7, tmps[2]);
        } else {
                print("set %d,%%r%d\n", size&~7, tmps[2]);
                print("add %%r%d,%%r%d,%%r%d\n", sreg, tmps[2], sreg);
                print("add %%r%d,%%r%d,%%r%d\n", dreg, tmps[2], tmps[2]);
        }
        blkcopy(tmps[2], doff, sreg, soff, size&7, tmps);
        print("1: dec 8,%%r%d\n", tmps[2]);
        blkcopy(tmps[2], doff, sreg, soff - 8, 8, tmps);
        print("cmp %%r%d,%%r%d; ", tmps[2], dreg);
        print("bgt 1b; ");
        print("dec 8,%%r%d\n", sreg);
}
static void defsymbol2(Symbol p) {
        if (p->scope >= LOCAL && p->sclass == STATIC)
                p->x.name = stringf(".%d", genlabel(1));
        else
                assert(p->scope != CONSTANTS || isint(p->type) || isptr(p->type)),
                p->x.name = p->name;
        if (p->scope >= LABELS)
                p->x.name = stringf(p->generated ? ".L%s" : "%s",
                        p->x.name);
}

static Symbol prevg;

static void globalend(void) {
        if (prevg && prevg->type->size > 0)
                print(".size %s,%d\n", prevg->x.name, prevg->type->size);
        prevg = NULL;
}

static void export2(Symbol p) {
        globalend();
        print(".global %s\n", p->x.name);
}

static void progend2(void) {
        globalend();
}

static void global2(Symbol p) {
        globalend();
        assert(p->u.seg);
        if (!p->generated) {
                print(".type %s,#%s\n", p->x.name,
                        isfunc(p->type) ? "function" : "object");
                if (p->type->size > 0)
                        print(".size %s,%d\n", p->x.name, p->type->size);
                else
                        prevg = p;
        }
        if (p->u.seg == BSS && p->sclass == STATIC)
                print(".local %s\n.common %s,%d,%d\n", p->x.name, p->x.name,
                        p->type->size, p->type->align);
        else if (p->u.seg == BSS && Aflag >= 2)
                print(".align %d\n%s:.skip %d\n", p->type->align, p->x.name,
                        p->type->size);
        else if (p->u.seg == BSS)
                print(".common %s,%d,%d\n", p->x.name, p->type->size, p->type->align);
        else
                print(".align %d\n%s:\n", p->type->align, p->x.name);
}

static void segment2(int n) {
        cseg = n;
        switch (n) {
        case CODE: print(".section \".text\"\n");   break;
        case BSS:  print(".section \".bss\"\n");    break;
        case DATA: print(".section \".data\"\n");   break;
        case LIT:  print(".section \".rodata\"\n"); break;
        }
}

Interface esc64IR = {
        1, 1, 0,  /* char */
        2, 2, 0,  /* short */
        4, 4, 0,  /* int */
        4, 4, 0,  /* long */
        4, 4, 0,  /* long long */
        4, 4, 1,  /* float */
        8, 8, 1,  /* double */
        8, 8, 1,  /* long double */
        4, 4, 0,  /* T * */
        0, 1, 0,  /* struct */
        0,  /* little_endian */
        0,  /* mulops_calls */
        1,  /* wants_callb */
        0,  /* wants_argb */
        1,  /* left_to_right */
        0,  /* wants_dag */
        0,  /* unsigned_char */
        address,
        blockbeg,
        blockend,
        defaddress,
        defconst,
        defstring,
        defsymbol,
        emit,
        export,
        function,
        gen,
        global,
        import,
        local,
        progbeg,
        progend,
        segment,
        space,
        stabblock, 0, 0, stabinit, stabline, stabsym, stabtype,
        {
                1,  /* max_unaligned_load */
                rmap,
                blkfetch, blkstore, blkloop,
                _label,
                _rule,
                _nts,
                _kids,
                _string,
                _templates,
                _isinstruction,
                _ntname,
                emit2,
                doarg,
                target,
                clobber,

        }
};

static char rcsid[] = "$Id$";
