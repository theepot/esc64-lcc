%{
#include "esc64.i.h"
%}
%start stmt

%term CNSTF4=4113
%term CNSTF8=8209
%term CNSTI1=1045
%term CNSTI2=2069
%term CNSTI4=4117
%term CNSTP2=2071
%term CNSTU1=1046
%term CNSTU2=2070
%term CNSTU4=4118

%term ARGB=41
%term ARGF4=4129
%term ARGF8=8225
%term ARGI2=2085
%term ARGI4=4133
%term ARGP2=2087
%term ARGU2=2086
%term ARGU4=4134

%term ASGNB=57
%term ASGNF4=4145
%term ASGNF8=8241
%term ASGNI1=1077
%term ASGNI2=2101
%term ASGNI4=4149
%term ASGNP2=2103
%term ASGNU1=1078
%term ASGNU2=2102
%term ASGNU4=4150

%term INDIRB=73
%term INDIRF4=4161
%term INDIRF8=8257
%term INDIRI1=1093
%term INDIRI2=2117
%term INDIRI4=4165
%term INDIRP2=2119
%term INDIRU1=1094
%term INDIRU2=2118
%term INDIRU4=4166

%term CVFF4=4209
%term CVFF8=8305
%term CVFI2=2165
%term CVFI4=4213
%term CVIF4=4225
%term CVIF8=8321
%term CVII1=1157
%term CVII2=2181
%term CVII4=4229
%term CVIU1=1158
%term CVIU2=2182
%term CVIU4=4230
%term CVPU2=2198
%term CVUI1=1205
%term CVUI2=2229
%term CVUI4=4277
%term CVUP2=2231
%term CVUU1=1206
%term CVUU2=2230
%term CVUU4=4278

%term NEGF4=4289
%term NEGF8=8385
%term NEGI2=2245
%term NEGI4=4293

%term CALLB=217
%term CALLF4=4305
%term CALLF8=8401
%term CALLI2=2261
%term CALLI4=4309
%term CALLP2=2263
%term CALLU2=2262
%term CALLU4=4310
%term CALLV=216

%term RETF4=4337
%term RETF8=8433
%term RETI2=2293
%term RETI4=4341
%term RETP2=2295
%term RETU2=2294
%term RETU4=4342
%term RETV=248

%term ADDRGP2=2311
%term ADDRFP2=2327
%term ADDRLP2=2343

%term ADDF4=4401
%term ADDF8=8497
%term ADDI2=2357
%term ADDI4=4405
%term ADDP2=2359
%term ADDU2=2358
%term ADDU4=4406

%term SUBF4=4417
%term SUBF8=8513
%term SUBI2=2373
%term SUBI4=4421
%term SUBP2=2375
%term SUBU2=2374
%term SUBU4=4422

%term LSHI2=2389
%term LSHI4=4437
%term LSHU2=2390
%term LSHU4=4438

%term MODI2=2405
%term MODI4=4453
%term MODU2=2406
%term MODU4=4454

%term RSHI2=2421
%term RSHI4=4469
%term RSHU2=2422
%term RSHU4=4470

%term BANDI2=2437
%term BANDI4=4485
%term BANDU2=2438
%term BANDU4=4486

%term BCOMI2=2453
%term BCOMI4=4501
%term BCOMU2=2454
%term BCOMU4=4502

%term BORI2=2469
%term BORI4=4517
%term BORU2=2470
%term BORU4=4518

%term BXORI2=2485
%term BXORI4=4533
%term BXORU2=2486
%term BXORU4=4534

%term DIVF4=4545
%term DIVF8=8641
%term DIVI2=2501
%term DIVI4=4549
%term DIVU2=2502
%term DIVU4=4550

%term MULF4=4561
%term MULF8=8657
%term MULI2=2517
%term MULI4=4565
%term MULU2=2518
%term MULU4=4566

%term EQF4=4577
%term EQF8=8673
%term EQI2=2533
%term EQI4=4581
%term EQU2=2534
%term EQU4=4582

%term GEF4=4593
%term GEF8=8689
%term GEI2=2549
%term GEI4=4597
%term GEU2=2550
%term GEU4=4598

%term GTF4=4609
%term GTF8=8705
%term GTI2=2565
%term GTI4=4613
%term GTU2=2566
%term GTU4=4614

%term LEF4=4625
%term LEF8=8721
%term LEI2=2581
%term LEI4=4629
%term LEU2=2582
%term LEU4=4630

%term LTF4=4641
%term LTF8=8737
%term LTI2=2597
%term LTI4=4645
%term LTU2=2598
%term LTU4=4646

%term NEF4=4657
%term NEF8=8753
%term NEI2=2613
%term NEI4=4661
%term NEU2=2614
%term NEU4=4662

%term JUMPV=584

%term LABELV=600

%term VREGP=711

%term LOADI4=4325
%term LOADU4=4326
%term LOADI2=2277
%term LOADU2=2278
%term LOADP2=2279
%term LOADF4=4321
%term LOADB=233
%term LOADF8=8417
%term LOADI1=1253
%term LOADU1=1254
%%
reg8:	INDIRI1(VREGP)			"# read register\n"
reg8:	INDIRU1(VREGP)			"# read register\n"

reg16:	INDIRI2(VREGP)			"# read register\n"
reg16:	INDIRU2(VREGP)			"# read register\n"
reg16:	INDIRP2(VREGP)			"# read register\n"


stmt:	ASGNI1(VREGP, reg8)		"# write register\n"
stmt:	ASGNU1(VREGP, reg8)		"# write register\n"

stmt:	ASGNI2(VREGP, reg16)	"# write register\n"
stmt:	ASGNU2(VREGP, reg16)	"# write register\n"
stmt:	ASGNP2(VREGP, reg16)	"# write register\n"


con:	CNSTI1					"%a"
con:	CNSTU1					"%a"

con:	CNSTI2					"%a"
con:	CNSTU2					"%a"


reg16:	con						"\tmov		%c, %0\t\t\t\t;reg16: con\n"	1


stmt:	reg8					""
stmt:	reg16					""


reg8:	CNSTI1  				"\tmov		%c, %a\n"		1
reg16:	CNSTI2  				"\tmov		%c, %a\n"		1

reg8:	CNSTU1  				"\tmov		%c, %a\n"		1
reg16:	CNSTU2  				"\tmov		%c, %a\n"		1

reg16:	CNSTP2  				"\tmov		%c, %a\n"		1


stmt:	ASGNI1(reg16, reg8)				"\tstb		%0, %1\n"		1
stmt:	ASGNU1(reg16, reg8)				"\tstb		%0, %1\n"		1

stmt:	ASGNI2(reg16, reg16)			"\tst		%0, %1\n"		1
stmt:	ASGNU2(reg16, reg16)			"\tst		%0, %1\n"		1

stmt:	ASGNP2(reg16, reg16)			"\tst		%0, %1\n"		1

stmt:	ASGNB(reg16, INDIRB(reg16))		"\tmov\t\tr2, %a\t\t\t;stmt: ASGNB(reg16, INDIRB(reg16)) (0)\n\tcall\t__memcpy\t\t;stmt: ASGNB(reg16, INDIRB(reg16)) (1)\n"	1


stmt:	ASGNI1(off, reg8)			"# asgn off\n"		1
stmt:	ASGNU1(off, reg8)			"# asgn off\n"		1

stmt:	ASGNI2(off, reg16)			"# asgn off\n"		1
stmt:	ASGNU2(off, reg16)			"# asgn off\n"		1

stmt:	ASGNP2(off, reg16)			"# asgn off\n"		1


reg8:	INDIRI1(reg16)			"\tldb		%c, %0\n"		1
reg8:	INDIRU1(reg16)			"\tldb		%c, %0\t\t;reg8:	INDIRU1(reg16)\n"		1

reg16:	INDIRI2(reg16)			"\tld		%c, %0\n"		1
reg16:	INDIRU2(reg16)			"\tld		%c, %0\n"		1

reg16:	INDIRP2(reg16)			"\tld		%c, %0\t\t;reg16: INDIRP2(reg16)\n"		1


reg8:	CVII1(INDIRI2(reg16))	"\tldb		%c, %0\t\t;reg8:	CVII1(INDIRI2(reg16))\n"		1
reg8:	CVIU1(INDIRI1(reg16))	"\tldb		%c, %0\t\t;reg8:	CVIU1(INDIRI1(reg16))\n"		1
reg8:	CVIU1(INDIRI2(reg16))	"\tldb		%c, %0\t\t;reg8:	CVIU1(INDIRI2(reg16))\n"		1
reg8:	CVUI1(INDIRU2(reg16))	"\tldb		%c, %0\t\t;reg8:	CVUI1(INDIRU2(reg16))\n"		1
reg16:	CVUI2(INDIRU1(reg16))	"\tldb		%c, %0\t\t;reg16:	CVUI2(INDIRU1(reg16))\n"		1
reg8:	CVUU1(INDIRU2(reg16))	"\tldb		%c, %0\t\t;reg8:	CVUU1(INDIRU2(reg16))\n"		1
reg16:	CVUU2(INDIRU1(reg16))	"\tldb		%c, %0\t\t;reg16:	CVUU2(INDIRU1(reg16))\n"		1


reg16:	CVII2(reg8)				"\tcall		__sext8to16\n"					1

reg16:	CVUI2(reg8)				"\tmov\t\t__tmpreg, 0xFF\n\tand\t\t%c, %0, __tmpreg\t;LCC: CVUI1(reg8)\n"	1
reg16:	CVUI2(reg16)			"\tmov\t\t%c, %0\t;LCC: CVUI2(reg16)\n"		1

reg16:	CVUU2(reg8)				"\tmov\t\t__tmpreg, 0xFF\n\tand\t\t%c, %0, __tmpreg\t;LCC: CVUU2(reg8)\n"	1

reg8:	CVII1(reg16)			"\tmov\t\t%c, %0\t;LCC: CVII1(reg16)\n"		1

reg8:	CVUI1(reg8)				"\tmov\t\t%c, %0\t;LCC: CVUI1(reg8)\n"		1
reg8:	CVUI1(reg16)			"\tmov\t\t%c, %0\t;LCC: CVUI1(reg16)\n"		1

reg8:	CVUU1(reg16)			"\tmov\t\t%c, %0\t;LCC: CVUU1(reg16)\n"		1


reg16:	DIVI2(reg16, reg16)			"\tcall		__sdiv16\n"					1
reg16:	DIVU2(reg16, reg16)			"\tcall		__udiv16\n"					1
reg16:	MODI2(reg16, reg16)			"\tcall		__sdiv16\n"					1
reg16:	MODU2(reg16, reg16)			"\tcall		__udiv16\n"					1
reg16:	MULI2(reg16, reg16)			"\tcall		__mul16\n"					1
reg16:	MULU2(reg16, reg16)			"\tcall		__mul16\n"					1


reg16:	ADDI2(reg16, reg16)			"\tadd		%c, %0, %1\n"		1
reg16:	ADDP2(reg16, reg16)			"\tadd		%c, %0, %1\n"		1
reg16:	ADDU2(reg16, reg16)			"\tadd		%c, %0, %1\n"		1
reg16:	BANDI2(reg16, reg16)		"\tand		%c, %0, %1\n"		1
reg16:	BORI2(reg16, reg16)			"\tor		%c, %0, %1\n"		1
reg16:	BXORI2(reg16, reg16)		"\txor		%c, %0, %1\n"		1
reg16:	BANDU2(reg16, reg16)		"\tand		%c, %0, %1\n"		1
reg16:	BORU2(reg16, reg16)			"\tor		%c, %0, %1\n"		1
reg16:	BXORU2(reg16, reg16)		"\txor		%c, %0, %1\n"		1
reg16:	SUBI2(reg16, reg16)			"\tsub		%c, %0, %1\n"		1
reg16:	SUBP2(reg16, reg16)			"\tsub		%c, %0, %1\n"		1
reg16:	SUBU2(reg16, reg16)			"\tsub		%c, %0, %1\n"		1

reg16:	LSHI2(reg16, reg16)			"\tcall\t__shl16\n"				1
reg16:	LSHU2(reg16, reg16)			"\tcall\t__shl16\n"				1
reg16:	RSHI2(reg16, reg16)			"\tcall\t__shr16\n"				1
reg16:	RSHU2(reg16, reg16)			"\tcall\t__shr16\n"				1
reg16:	BCOMI2(reg16)				"\tnot		%c, %0\n"				1
reg16:	BCOMU2(reg16)				"\tnot		%c, %0\n"				1
reg16:	NEGI2(reg16)				"\tnot\t\t%c, %0\n\tinc\t\t%c, %c\n"		1


reg8:	LOADI1(reg16)				"\tmov		%c, %0\t\t;reg8:	LOADI1(reg16)\n"				move(a)
reg8:	LOADU1(reg16)				"\tmov		%c, %0\t\t;reg8:	LOADU1(reg16)\n"				move(a)
reg16:	LOADI2(reg16)				"\tmov		%c, %0\t\t;reg16:	LOADI2(reg16)\n"				move(a)
reg16:	LOADU2(reg16)				"\tmov		%c, %0\t\t;reg16:	LOADU2(reg16)\n"				move(a)
reg16:	LOADP2(reg16)				"\tmov		%c, %0\t\t;reg16:	LOADP2(reg16)\n"				move(a)


stmt:	LABELV						"# labelv\n"


stmt:	JUMPV(reg16)				"\tjmp		%0\n"   					1
stmt:	JUMPV(con)					"\tjmp		%0\n"   					1

stmt:	EQI2(reg16, reg16)			"\tscmp\t\t%0, %1\n\tjeq\t\t%a\n"		1
stmt:	EQU2(reg16, reg16)			"\tcmp\t\t%0, %1\n\tjeq\t\t%a\n"		1

stmt:	GEI2(reg16, reg16)			"\tscmp\t\t%0, %1\n\tjge\t\t%a\n"		1
stmt:	GEU2(reg16, reg16)			"\tcmp\t\t%0, %1\n\tjge\t\t%a\n"		1

stmt:	GTI2(reg16, reg16)			"\tscmp\t\t%0, %1\n\tjgt\t\t%a\n"		1
stmt:	GTU2(reg16, reg16)			"\tcmp\t\t%0, %1\n\tjgt\t\t%a\n"		1

stmt:	LEI2(reg16, reg16)			"\tscmp\t\t%0, %1\n\tjle\t\t%a\n"		1
stmt:	LEU2(reg16, reg16)			"\tcmp\t\t%0, %1\n\tjle\t\t%a\n"		1

stmt:	LTI2(reg16, reg16)			"\tscmp\t\t%0, %1\n\tjlt\t\t%a\n"		1
stmt:	LTU2(reg16, reg16)			"\tcmp\t\t%0, %1\n\tjlt\t\t%a\n"		1

stmt:	NEI2(reg16, reg16)			"\tscmp\t\t%0, %1\n\tjne\t\t%a\n"		1
stmt:	NEU2(reg16, reg16)			"\tcmp\t\t%0, %1\n\tjne\t\t%a\n"		1


con:	ADDRGP2     				"%a"
reg16:	ADDRGP2     				"\tmov\t\t%c, %a\t\t;reg16: ADDRGP2\n"	1

reg16:	ADDRFP2						"\tmov\t\t__tmpreg, %a\t\t\t;reg16: ADDRFP2(0)\n\tadd\t\t%c, __bp, __tmpreg\t\t;reg16: ADDRFP2(1)\n"		1
off:	ADDRFP2						"# addrfp2"

reg16:	ADDRLP2						"\tmov\t\t__tmpreg, %a\t\t\t;reg16: ADDRLP2(0)\n\tadd\t\t%c, __bp, __tmpreg\t\t;reg16: ADDRLP2(1)\n"		1
off:	ADDRLP2						"# addrlp2"


reg16:	CALLI2(reg16)			"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1
reg16:	CALLP2(reg16)			"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1
reg16:	CALLU2(reg16)			"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1

reg16:	CALLI2(con)				"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1
reg16:	CALLP2(con)				"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1
reg16:	CALLU2(con)				"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1

stmt:	CALLV(reg16)			"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1
stmt:	CALLV(con)				"\tcall\t%0\n\tmov\t\t__tmpreg, %a\n\tadd\t\tsp, sp, __tmpreg\n"		1


con:	CNSTP2					"%a"


stmt:	RETI2(reg16)			"# reti2\n"				1
stmt:	RETU2(reg16)			"# retu2\n"				1
stmt:	RETP2(reg16)			"# retp2\n"				1
stmt:	RETV					"# retv\n"				1


stmt:	ARGI2(reg16)			"\tpush\t%0\n"			1
stmt:	ARGP2(reg16)			"\tpush\t%0\n"			1
stmt:	ARGU2(reg16)			"\tpush\t%0\n"			1
%%

#include "esc64.i.c"
