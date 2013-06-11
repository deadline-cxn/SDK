$L
/*
 * Automaton to interpret LALR(1) tables.
 *
 *	Macros available in actions:
 *		yyclearin - clear the lookahead token.
 *		yyerrok - forgive a pending error
 *		YYERROR - simulate an error
 *		YYACCEPT - halt and return 0
 *		YYABORT - halt and return 1
 *		YYRETURN(value) - halt and return value.  You should use this
 *			instead of return(value).
 *		YYREAD - ensure yychar contains a lookahead token by reading
 *			one if it does not.  See also YYSYNC.
 *
 *	Macros set by user, used internally:
 *		YYDEBUG - includes debug code.  The parser will print
 *			 a travelogue of the parse if this is defined
 *			 and yydebug is non-zero. (default undef)
 *		YYSSIZE - size of state and value stacks (default 150).
 *		YYSTATIC - By default, the state stack is an automatic array.
 *			If this is defined, the stack will be static.
 *			In either case, the value stack is static.
 *		YYALLOC - Dynamically allocate both the state and value stacks
 *			by calling malloc() and free(). If YYALLOC >= 10, the stacks 
 *			will be grown by YYALLOC elements. This means the user cannot
 *			keep pointer into the stack, if he wants to use this feature.
 *		YYLR2 - defined if lookahead is needed to resolve R/R or S/R conflicts
 *		YYSYNC - if defined, Yay guarantees to fetch a lookahead token
 *			before any action, even if it doesnt need it for a decision.
 *			If YYSYNC is defined, YYREAD will never be necessary unless
 *			the user explicitly sets yychar = -1
 *		YYSHIFT - called when a succesful shift occurs on the last value
 *			returned by yylex. Defaults to nothing.
 *
 *	Copyright (c) 1983, by the University of Waterloo
 */

#include <stdlib.h>		/* for malloc and realloc */

#ifndef YYSSIZE
	#define YYSSIZE	150
#endif
#ifndef YYSHIFT
	#define YYSHIFT	/* nothing */
#endif
#define YYERROR		goto yyerrlabel
#define yyerrok		yyerrflag = 0
#define yyclearin	yychar = -1
#define YYACCEPT	YYRETURN(0)
#define YYABORT		YYRETURN(1)
#ifdef YYALLOC
	#define YYRETURN(val)	{ yyretval = (val); goto yyReturn; }
#else
	#define YYRETURN(val)	return(val)
#endif
#ifdef YYDEBUG
	#define YYREAD	do if (yychar < 0) {									\
						if ((yychar = yylex()) < 0)							\
							yychar = 0;										\
						if (yydebug)										\
							printf("read %8s (%3d)\n", yyptok(yychar), yychar);\
					} while(0)
#else
	#define YYREAD	do if (yychar < 0) {									\
						if ((yychar = yylex()) < 0)							\
							yychar = 0;										\
					} while(0)
#endif

#ifdef YYSYNC
	#define YYSYNC_READ 1
#else
	#define YYSYNC_READ 0
#endif

#define YYERRCODE	256		/* value of `error' */

YYSTYPE	yyval,			/* $$ */
	*yypvt,				/* $n */
	yylval;				/* yylex() sets this */

int	yychar,				/* current token */
	yyerrflag,			/* error flag */
	yynerrs;			/* error count */

#ifdef YYDEBUG
	int yydebug = YYDEBUG-0;		/* debug flag & tables */
	extern YYSCTAB char	* YYSCTAB yysvar[], * YYSCTAB yystoken[];
	extern const char *yyptok(int);
	extern YYSCTAB short	yyrmap[], yysmap[];
	extern int	yynstate, yynvar, yyntoken, yynrule;
# define yyassert(condition, msg, arg) \
	if (!(condition)) { printf("\nYay bug: "); printf(msg, arg); YYABORT; }
#else /* !YYDEBUG */
# define yyassert(condition, msg, arg)
#endif

	static int
yyparse() {

	register short			yyi;	/* for table lookup */
	register YYSCTAB short *yyp;	/* for table lookup */
	register short	      *yyps;	/* top of state stack */
	register short		yystate;	/* current state */
	register YYSTYPE	  *yypv;	/* top of value stack */
	register YYSCTAB short *yyq;
	register int			yyj;
	int						yylerr = 0;

	#ifdef YYSTATIC
		static short	yys[YYSSIZE + 1];
		static YYSTYPE	yyv[YYSSIZE + 1];
	#elif defined(YYALLOC)
		YYSTYPE *yyv;
		short	*yys;
		YYSTYPE yysave_lval, yysave_val, *yysave_pvt;
		int yysave_char, yysave_errflag, yysave_nerrs;
		int yyretval;
	#else
		short			yys[YYSSIZE + 1];
		static YYSTYPE	yyv[YYSSIZE + 1];	/* historically static */
	#endif
	
	#if defined(YYALLOC) && YYALLOC-0 >= 10
		int yyssize = YYSSIZE;
	#else
		#define	yyssize		(YYSSIZE)
	#endif
$A
$L
	#ifdef YYALLOC
		yys = (short *) malloc((YYSSIZE + 1) * sizeof(short));
		yyv = (YYSTYPE *) malloc((YYSSIZE + 1) * sizeof(YYSTYPE));
		if (yys == (short *)0 || yyv == (YYSTYPE *)0) {
			yyerror("Not enough space for parser stacks");
			return 1;
		}
		yysave_lval = yylval;
		yysave_val = yyval;
		yysave_pvt = yypvt;
		yysave_char = yychar;
		yysave_errflag = yyerrflag;
		yysave_nerrs = yynerrs;
	#endif

	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;
	yyps = yys;
	yypv = yyv;
	yystate = YYS0;		/* start state */

  yyStack:
	yyassert((unsigned)yystate < yynstate, "state %4d\n", yystate);
	if (++yyps > &yys[yyssize]) {
		#if defined(YYALLOC) && YYALLOC-0 >= 10
			short *newyys;
			YYSTYPE *newyyv;
			newyys = (short *)realloc( (void *)yys,
				(yyssize+YYALLOC+1) * sizeof(short));
			newyyv = (YYSTYPE *) realloc((void *) yyv,
				(yyssize+YYALLOC+1) * sizeof(YYSTYPE));
			if( !newyys || !newyyv ) {
				yyerror("Parser stack overflow");
				YYABORT;
			}
			yyps = newyys + (yyps-yys);
			yypv = newyyv + (yypv-yyv);
			yys = newyys;
			yyv = newyyv;
			yyssize += YYALLOC;
		#else
			yyerror("Parser stack overflow");
			YYABORT;
		#endif
	}
	*yyps = yystate;	/* stack current state */
	*++yypv = yyval;	/* ... and value */

	#ifdef YYDEBUG
		if (yydebug)
			printf("state %4d (%4d), char %8s (%3d)\n", yysmap[yystate],
				yystate, yyptok(yychar), yychar);
	#endif

	if( 0 ) goto yyerrlabel;	/* suppress an error message */
	/*
	 *	Look up next action in action table.
	 */
  yyEncore:
	if( YYSYNC_READ ) YYREAD;

	if ((unsigned)yystate >= sizeof yypact/sizeof yypact[0]) /* simple state */
		yyi = yystate - YYDELTA;	/* reduce in any case */
	else {
		if( *(yyp = &yyact[yypact[yystate]]) >= 0 ) {
			/* Look for a shift on yychar */
			
			if( !YYSYNC_READ ) YYREAD;
			yyq = yyp;
			yyi = yychar;
			while (yyi < *yyp++);
			if (yyi == yyp[-1]) {
				yystate = ~yyq[yyq-yyp];
				#ifdef YYDEBUG
					if (yydebug)
						printf("shift %4d (%4d)\n", yysmap[yystate], yystate);
				#endif
				yyval = yylval;		/* stack what yylex() set */
				YYSHIFT;			/* Tell user about shift */
				yychar = -1;		/* clear token */
				if (yyerrflag) {
					yyerrflag--;	/* successful shift */
					yylerr = 0;		/* no restrictions on error shifts */
				}
				goto yyStack;
			}
		}

		/*
	 	 *	Fell through - take default action
	 	 */

		if ((unsigned)yystate >= sizeof yydef /sizeof yydef[0] )
			goto yyError;
		if ((yyi = yydef[yystate]) < 0)	 { /* default == reduce? */
											/* Search exception table */
			yyassert((unsigned)~yyi < sizeof yyex/sizeof yyex[0],
				"exception %d\n", yystate);
			yyp = &yyex[~yyi];

			if( !YYSYNC_READ) YYREAD;

			while( (yyi = *yyp) >= 0 && yyi != yychar)
				yyp += 2;
			yyi = yyp[1];
			yyassert(yyi >= 0,"Ex table not reduce %d\n", yyi);
		}
	}

	#ifdef YYLR2
	  yyReduce:	/* reduce yyi */
	#endif
	yyassert((unsigned)yyi < yynrule, "reduce %d\n", yyi);
	yyj = yyrlen[yyi];
	#ifdef YYDEBUG
		if (yydebug) printf("reduce %3d (%3d), pops %4d (%4d)\n", yyrmap[yyi],
			yyi, yysmap[yyps[-yyj]], yyps[-yyj]);
	#endif
	yyps -= yyj;		/* pop stacks */
	yypvt = yypv;		/* save top */
	yypv -= yyj;
	yyval = yypv[1];	/* default action $$ = $1 */
	switch (yyi) {		/* perform semantic action */
		$A
$L
	case YYrACCEPT:
		YYACCEPT;
	case YYrERROR:
		yystate = *--yyps;
		yypv--;
		goto yyError;
	#ifdef YYLR2
		case YYrLR2:
			if( !YYSYNC_READ) YYREAD;
			yyj = 0;
			while( yylr2[yyj] >= 0 ) {
				if( yylr2[yyj] == yystate && yylr2[yyj+1] == yychar
					&& yylook(yys+1,yyps,yystate,yychar,yy2lex(),yylr2[yyj+2]) )
						break;
				yyj += 3;
			}
			if( yylr2[yyj] < 0 )
				goto yyError;
			if( yylr2[yyj+2] < 0 ) {
				yystate = ~ yylr2[yyj+2];
				goto yyStack;
			}
			yyi = yylr2[yyj+2];
			goto yyReduce;
	#endif
	}

	/*
	 *	Look up next state in goto table.
	 */

	yyp = &yygo[yypgo[yyi]];
	yyq = yyp++;
	yyi = *yyps;
	do
		;
	while (yyi < *yyp++);
	yystate = ~(yyi == *--yyp? yyq[yyq-yyp]: *yyq);
	goto yyStack;

  yyerrlabel:
	#pragma used yyerrlabel			/* from YYERROR */
	yyerrflag = 1;
  yyError:

	switch (yyerrflag) {

		default:		/* new error */
			yynerrs++;
			yyi = yychar;
			yyerror("Syntax error");
			if (yyi != yychar) {
				/* user has changed the current token */
				/* try again */
				yynerrs--;	/* assume not really an error */
				#ifdef YYDEBUG
					if( yydebug )
						printf("yychar changed from %8s (%3d) to %8s (%3d)\n",
							yyptok(yyi), yyi, yyptok(yychar), yychar);
				#endif
				goto yyEncore;
			}

		case 1:		/* partially recovered */
		case 2:
			yyerrflag = 3;	/* need 3 valid shifts to recover */
			
			/*
			 *	Pop states, looking for a
			 *	shift on `error'.
			 */

			for ( ; yyps > yys; yyps--, yypv--) {
				if ((size_t)*yyps >= sizeof yypact/sizeof yypact[0])
					continue;
				yyp = &yyact[yypact[*yyps]];
				yyq = yyp;
				do
					;
				while (YYERRCODE < *yyp++);
				if (YYERRCODE == yyp[-1] && yylerr != ~yyq[yyq-yyp] ) {
					yylerr = yystate = ~yyq[yyq-yyp];
					goto yyStack;
				}
				
				/* no shift in this state */
				#ifdef YYDEBUG
					if (yydebug && yyps > yys+1)
						printf("Error recovery pops state %4d (%4d),"
								" uncovers %4d (%4d)\n",
							yysmap[yyps[0]], yyps[0],
							yysmap[yyps[-1]], yyps[-1]);
				#endif
				/* pop stacks; try again */
			}
			/* no shift on error - abort */
			break;

		case 3:
			/*
			 *	Erroneous token after
			 *	an error - discard it.
			 */

			if (yychar == 0)  /* but not EOF */
				break;
			#ifdef YYDEBUG
				if (yydebug)
					printf("Error recovery discards %8s (%3d), ",
						yyptok(yychar), yychar);
			#endif
			yyclearin;
			goto yyEncore;	/* try again in same state */
	}
	YYABORT;

	#ifdef YYALLOC
	  yyReturn:
		yylval = yysave_lval;
		yyval = yysave_val;
		yypvt = yysave_pvt;
		yychar = yysave_char;
		yyerrflag = yysave_errflag;
		yynerrs = yysave_nerrs;
		free((char *)yys);
		free((char *)yyv);
		return(yyretval);
	#endif
}

#ifdef YYLR2

static int yylook(s,rsp,state,c1,c2,i)
	short *s;							/* stack				*/
	short *rsp;							/* real top of stack	*/
	int state;							/* current state		*/
	int c1;								/* current char			*/
	int c2;								/* next char			*/
	int i;								/* action S < 0, R >= 0	*/
{
	int j;
	short *p,*q;
	short *sb,*st;
	#ifdef YYDEBUG
		if( yydebug ) {
			printf("LR2 state %4d (%4d) char %8s (%3d) lookahead %8s (%3d)",
				yysmap[state],state,yyptok(c1),c1,yyptok(c2),c2);
			if( i > 0 )
				printf( "reduce %3d (%3d)\n", yyrmap[i], i);
			else
				printf( "shift %4d (%4d)\n", yysmap[i], i);
		}
	#endif
	st = sb = rsp+1;
	if( i >= 0 ) goto reduce;
  shift:
	state = ~i;
	c1 = c2;
	if( c1 < 0 ) return 1;
	c2 = -1;

  stack:
  	if( ++st >= &s[yyssize] ) {
		yyerror("Parser Stack Overflow");
		return 0;
	}
	*st = state;
	if( (unsigned)state >= sizeof yypact/sizeof yypact[0] )
		i = state- YYDELTA;
	else {
		p = &yyact[yypact[state]];
		q = p;
		i = c1;
		while( i < *p++ );
		if( i == p[-1] ) {
			state = ~q[q-p];
			c1 = c2;
			if( c1 < 0 ) return 1;
			c2 = -1;
			goto stack;
		}
		if( (unsigned)state >= sizeof yydef/sizeof yydef[0] )
			return 0
		if( (i = yydef[state]) < 0 ) {
			p = &yyex[~i];
			while( (i = *p) >= 0 && i != c1)
				p += 2;
			i = p[1];
		}
	}
  reduce:
  	j = yyrlen[i];
	if( st-sb >= j )
		st -= j;
	else {
		rsp -= j+st-sb;
		st = sb;
	}
	switch( i ) {
	  case YYrERROR:
		return 0;
	  case YYrACCEPT:
		return 1;
	  case YYrLR2:
		j = 0;
		while( yylr2[j] >= 0 ) {
			if( yylr2[j] == state && yylr2[j+1] == c1 )
				if( (i = yylr2[j+2]) < 0 )
					goto shift;
				else
					goto reduce;
		}
		return 0;
	}
	p = &yygo[yypgo[i]];
	q = p++;
	i = st==sb ? *rsp : *st;
	while( i < *p++ );
	state = ~( i == *--p? q[q-p]: *q);
	goto stack;
}
#endif
		
#ifdef YYDEBUG
	
	/*
	 *	Print a token legibly.
	 *	This won't work if you roll your own token numbers,
	 *	but I've found it useful.
	 */

	const char *
yyptok(int i) {
	static char	buf[10];

	if (i >= YYERRCODE)
		return yystoken[i-YYERRCODE];
	if (i < 0)
		return "";
	if (i == 0)
		return "$end";
	if (i < ' ')
		sprintf(buf, "'^%c'", i+'@');
	else
		sprintf(buf, "'%c'", i);
	return buf;
}
#endif
