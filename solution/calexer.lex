%{

// allow access to YY_DECL macro
#include "ckbisonflex.hpp"

#include INCLUDE_WRAP(BISON_HEADER)

%}

/* NEVER SET THIS INTERNALLY - SHALL BE SET BY CMAKE: %option outfile="../private/cecko-scn.cpp" */

%option noyywrap nounput noinput
%option batch never-interactive reentrant
%option nounistd 

/* AVOID THIS - DO NOT CREATE UNMANAGEABLE BYPRODUCT FILES: backup perf-report */


%%

\n				ctx->incline();

.				ctx->message(cecko::errors::UNCHAR, ctx->line(), yytext);

<<EOF>>			return cecko::parser::make_EOF(ctx->line());

%%

namespace cecko {

	yyscan_t lexer_init(FILE * iff)
	{
		yyscan_t scanner;
		yylex_init(&scanner);
		yyset_in(iff, scanner);
		return scanner;
	}

	void lexer_shutdown(yyscan_t scanner)
	{
		yyset_in(nullptr, scanner);
		yylex_destroy(scanner);
	}

}
