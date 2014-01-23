/* x86s running Linux */

#include <string.h>

static char rcsid[] = "$Id$";

/*path to gcc cpp*/
#ifndef CPPPATH
#error CPPPATH is not defined
#endif

/*path to ld-linux.so.2*/
#ifndef LD_LINUX_SO
#error LD_LINUX_SO is not defined
#endif

/*path to crt1.o*/
#ifndef CRT1_O
#error CRT1_O is not defined
#endif

/*path to crti.o*/
#ifndef CRTI_O
#error CRTI_O is not defined
#endif

/*path to crtn.o*/
#ifndef CRTN_O
#error CRTN_O is not defined
#endif

#ifndef LCCDIR
/*#define LCCDIR "/usr/local/lib/lcc/"*/
#error LCCDIR is not defined
#endif

char *suffixes[] = { ".c", ".i", ".s", ".o", ".out", 0 };
char inputs[256] = "";
char *cpp[] = { CPPPATH,
	"-U__GNUC__", "-D_POSIX_SOURCE", "-D__STRICT_ANSI__",
	"-Dunix", "-Di386", "-Dlinux",
	"-D__unix__", "-D__i386__", "-D__linux__", "-D__signed__=signed",
	"$1", "$2", "$3", 0 };
char *include[] = {"-I" LCCDIR "include", "-I" LCCDIR "gcc/include", "-I/usr/include", 0 };
char *com[] = {LCCDIR "rcc", "-target=x86/linux", "$1", "$2", "$3", 0 };
char *as[] = { "/usr/bin/as", "-o", "$3", "$1", "$2", 0 };
char *ld[] = {
	/*  0 */ "/usr/bin/ld", "-m", "elf_i386", "-dynamic-linker",
	/*  4 */ LD_LINUX_SO, "-o", "$3",
	/*  7 */ CRT1_O, CRTI_O,
	/*  9 */ LCCDIR "/gcc/crtbegin.o", 
                 "$1", "$2",
	/* 12 */ "-L" LCCDIR,
	/* 13 */ "-llcc",
	/* 14 */ "-L" LCCDIR "/gcc", "-lgcc", "-lc", "-lm",
	/* 18 */ "",
	/* 19 */ LCCDIR "/gcc/crtend.o", CRTN_O,
	0 };

extern char *concat(char *, char *);

int option(char *arg) {
  	if (strncmp(arg, "-lccdir=", 8) == 0) {
		/*if (strcmp(cpp[0], LCCDIR "gcc/cpp") == 0)
			cpp[0] = concat(&arg[8], "/gcc/cpp");*/
		include[0] = concat("-I", concat(&arg[8], "/include"));
		include[1] = concat("-I", concat(&arg[8], "/gcc/include"));
		ld[9]  = concat(&arg[8], "/gcc/crtbegin.o");
		ld[12] = concat("-L", &arg[8]);
		ld[14] = concat("-L", concat(&arg[8], "/gcc"));
		ld[19] = concat(&arg[8], "/gcc/crtend.o");
		com[0] = concat(&arg[8], "/rcc");
	} else if (strcmp(arg, "-p") == 0 || strcmp(arg, "-pg") == 0) {
		ld[7] = "/usr/lib/gcrt1.o";
		ld[18] = "-lgmon";
	} else if (strcmp(arg, "-b") == 0) 
		;
	else if (strcmp(arg, "-g") == 0)
		;
	else if (strncmp(arg, "-ld=", 4) == 0)
		ld[0] = &arg[4];
	else if (strcmp(arg, "-static") == 0) {
		ld[3] = "-static";
		ld[4] = "";
	} else
		return 0;
	return 1;
}
