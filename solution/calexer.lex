%{

// allow access to YY_DECL macro
#include "ckbisonflex.hpp"

#include INCLUDE_WRAP(BISON_HEADER)

%}

%top{
#include "casem.hpp"
#include <vector>
#include <string>
#define MULTICHAR_MAX 4
#define HEXPREF_LEN 2
}
/* NEVER SET %option outfile INTERNALLY - SHALL BE SET BY CMAKE */

%option noyywrap nounput noinput
%option batch never-interactive reentrant
%option nounistd 

/* AVOID backup perf-report - DO NOT CREATE UNMANAGEABLE BYPRODUCT FILES */

NEWLINE		(\n|\r|\r\n)
DIGIT		[0-9]
HEXDIGIT	[0-9A-Fa-f]
NONDIGIT	[A-Za-z_]
NONHEXDIGIT	[G-Zg-z_]
IDCHAR		({NONDIGIT}|{DIGIT})
HEXPREF		(0x|0X)

%x CHARACTER
%x STRING
%x COMMENT
%x MULTICOMMENT

%%

%{

std::vector<char> string_literal;
int char_literal[MULTICHAR_MAX];
int char_literal_length = 0;
int multi_comment_depth = 0;

%}

{NEWLINE}	ctx->incline();
" "			/* mezery nemaji akci */
\t			/* tabule nemaji akci */

	/* comments */
"//" 			BEGIN(COMMENT);
<COMMENT>{
	{NEWLINE}	{
					ctx->incline();
					BEGIN(INITIAL);
				}
	.	/* bez akce */
	<<EOF>>		return cecko::parser::make_EOF(ctx->line());
}

"/*"	{
			BEGIN(MULTICOMMENT);
			multi_comment_depth = 1;
		}

"*/"	ctx->message(cecko::errors::UNEXPENDCMT, ctx->line());

<MULTICOMMENT>{
	"/*"		multi_comment_depth++;
	"*/"		{
					if(--multi_comment_depth == 0)
						BEGIN(INITIAL);
				}
	{NEWLINE}	ctx->incline();
	.			/* bez akce */
	<<EOF>>		{
					ctx->message(cecko::errors::EOFINCMT, ctx->line());
					return cecko::parser::make_EOF(ctx->line());
				}
}

	/* string and char */
\"	{
		string_literal.clear();
		BEGIN(STRING);
	}

\'\'	{
			ctx->message(cecko::errors::EMPTYCHAR, ctx->line());
			return cecko::parser::make_INTLIT(0, ctx->line());
		}

\'	{
		char_literal_length = 0;
		BEGIN(CHARACTER);
	}

<STRING>{
	\"	{
			std::string result(string_literal.begin(), string_literal.end());
			BEGIN(INITIAL);
			return cecko::parser::make_STRLIT(result, ctx->line());
		}

	{NEWLINE}	{
					ctx->message(cecko::errors::EOLINSTRCHR, ctx->line());
					std::string result(string_literal.begin(), string_literal.end());
					BEGIN(INITIAL);
					return cecko::parser::make_STRLIT(result, ctx->incline());
				}

	<<EOF>>	{
				ctx->message(cecko::errors::EOFINSTRCHR, ctx->line());
				std::string result(string_literal.begin(), string_literal.end());
				BEGIN(INITIAL);
				return cecko::parser::make_STRLIT(result, ctx->line());
			}
	
	\\\'	string_literal.push_back('\x27');
	\\\"	string_literal.push_back('\x22');
	\\\?	string_literal.push_back('\x3f');
	\\\\	string_literal.push_back('\x5c');
	\\a		string_literal.push_back('\x07');
	\\b		string_literal.push_back('\x08');
	\\f		string_literal.push_back('\x0c');
	\\n		string_literal.push_back('\x0a');
	\\r		string_literal.push_back('\x0d');
	\\t		string_literal.push_back('\x09');
	\\v		string_literal.push_back('\x0b');

	\\x{HEXDIGIT}+	{
						if (yyleng > 4) {
							ctx->message(cecko::errors::BADESCAPE, ctx->line(), yytext);
							int result = casem::parse_number_literal(yytext + yyleng - 2, 2, true);
							string_literal.push_back((char)result);
						}

						else {
							int result = casem::parse_number_literal(yytext + 2, yyleng - 2, true);
							string_literal.push_back((char)result);
						}
					}

	\\.	{
			ctx->message(cecko::errors::BADESCAPE, ctx->line(), yytext);
			string_literal.push_back(yytext[1]);
		}

	\\	ctx->message(cecko::errors::BADESCAPE, ctx->line(), yytext);

	.	string_literal.push_back(yytext[0]);
}

<CHARACTER>{
	\'	{
			if (char_literal_length == 0) {
				BEGIN(INITIAL);
				return cecko::parser::make_INTLIT(0, ctx->line());
			}

			if (char_literal_length > 4) {
				char_literal_length = 4;
				ctx->message(cecko::errors::MULTICHAR_LONG, ctx->line());
			}

			int result = 0;
			for (int i = char_literal_length - 1; i >= 0; i--) {
				result *= 256;
				result += char_literal[i];
			}

			BEGIN(INITIAL);
			return cecko::parser::make_INTLIT(result, ctx->line());
		}

	{NEWLINE}	{
					if (char_literal_length == 0) {
						ctx->message(cecko::errors::EOLINSTRCHR, ctx->line());		
						BEGIN(INITIAL);
						return cecko::parser::make_INTLIT(0, ctx->incline());
					}

					if (char_literal_length > 4) {
						char_literal_length = 4;
						ctx->message(cecko::errors::MULTICHAR_LONG, ctx->line());
					}

					int result = 0;
					for (int i = char_literal_length - 1; i >= 0; i--) {
						result *= 256;
						result += char_literal[i];
					}

					ctx->message(cecko::errors::EOLINSTRCHR, ctx->line());		
					BEGIN(INITIAL);
					return cecko::parser::make_INTLIT(result, ctx->incline());
				}

	<<EOF>>	{
				if (char_literal_length == 0) {
					BEGIN(INITIAL);
					return cecko::parser::make_INTLIT(0, ctx->line());
				}

				if (char_literal_length > 4) {
					char_literal_length = 4;
					ctx->message(cecko::errors::MULTICHAR_LONG, ctx->line());
				}

				int result = 0;
				for (int i = char_literal_length - 1; i >= 0; i--) {
					result *= 256;
					result += char_literal[i];
				}
				ctx->message(cecko::errors::EOFINSTRCHR, ctx->line());
				BEGIN(INITIAL);
				return cecko::parser::make_INTLIT(result, ctx->line());
			}
	
	\\\'	casem::shift_enregister(39, char_literal, MULTICHAR_MAX, char_literal_length);
	\\\"	casem::shift_enregister(34, char_literal, MULTICHAR_MAX, char_literal_length);
	\\\?	casem::shift_enregister(63, char_literal, MULTICHAR_MAX, char_literal_length);
	\\\\	casem::shift_enregister(92, char_literal, MULTICHAR_MAX, char_literal_length);
	\\a		casem::shift_enregister( 7, char_literal, MULTICHAR_MAX, char_literal_length);
	\\b		casem::shift_enregister( 8, char_literal, MULTICHAR_MAX, char_literal_length);
	\\f		casem::shift_enregister(12, char_literal, MULTICHAR_MAX, char_literal_length);
	\\n		casem::shift_enregister(10, char_literal, MULTICHAR_MAX, char_literal_length);
	\\r		casem::shift_enregister(13, char_literal, MULTICHAR_MAX, char_literal_length);
	\\t		casem::shift_enregister( 9, char_literal, MULTICHAR_MAX, char_literal_length);
	\\v		casem::shift_enregister(11, char_literal, MULTICHAR_MAX, char_literal_length);

	\\x{HEXDIGIT}+	{
						if (yyleng > 4) {
							ctx->message(cecko::errors::BADESCAPE, ctx->line(), yytext);
							int result = casem::parse_number_literal(yytext + yyleng - 2, 2, true);
							casem::shift_enregister((int)result, char_literal, MULTICHAR_MAX, char_literal_length);
						}

						else {
							int result = casem::parse_number_literal(yytext + 2, yyleng - 2, true);
							casem::shift_enregister((int)result, char_literal, MULTICHAR_MAX, char_literal_length);
						}
					}

	\\.	{
			ctx->message(cecko::errors::BADESCAPE, ctx->line(), yytext);
			casem::shift_enregister((int)yytext[1], char_literal, MULTICHAR_MAX, char_literal_length);
		}

	\\		ctx->message(cecko::errors::BADESCAPE, ctx->line(), yytext);

	.		casem::shift_enregister((int)yytext[0], char_literal, MULTICHAR_MAX, char_literal_length);
}

	/* integers */
{HEXPREF}{HEXDIGIT}+	{
							if (casem::number_literal_outrange(yytext + HEXPREF_LEN, yyleng - HEXPREF_LEN, true)) {
								ctx->message(cecko::errors::INTOUTRANGE, ctx->line(), yytext);
							}
							return cecko::parser::make_INTLIT(casem::parse_number_literal(yytext + HEXPREF_LEN, yyleng - HEXPREF_LEN, true), ctx->line());
						}
{DIGIT}+	{
				if (casem::number_literal_outrange(yytext, yyleng, false)) {
					ctx->message(cecko::errors::INTOUTRANGE, ctx->line(), yytext);
				}
				return cecko::parser::make_INTLIT(casem::parse_number_literal(yytext, yyleng, false), ctx->line());
			}

{HEXPREF}{HEXDIGIT}+{NONHEXDIGIT}{IDCHAR}*	{
												char* numtext = yytext + HEXPREF_LEN;
												int numlen = 0;
												while(casem::is_numeral_char(numtext[numlen], true)) {
													numlen++;
												}
												
												ctx->message(cecko::errors::BADINT, ctx->line(), yytext);

												if (casem::number_literal_outrange(numtext, numlen, true)) {
													ctx->message(cecko::errors::INTOUTRANGE, ctx->line(), yytext);
												}

												int result = casem::parse_number_literal(numtext, numlen, true);
												return cecko::parser::make_INTLIT(result, ctx->line());
											}

{DIGIT}+{NONDIGIT}{IDCHAR}*	{
								int numlen = 0;
								while(casem::is_numeral_char(yytext[numlen], false)) {
									numlen++;
								}
								
								ctx->message(cecko::errors::BADINT, ctx->line(), yytext);

								if (casem::number_literal_outrange(yytext, numlen, false)) {
									ctx->message(cecko::errors::INTOUTRANGE, ctx->line(), yytext);
								}

								int result = casem::parse_number_literal(yytext, numlen, false);
								return cecko::parser::make_INTLIT(result, ctx->line());
							}

	/* keywords */
const			return cecko::parser::make_CONST(ctx->line());
do				return cecko::parser::make_DO(ctx->line());
else			return cecko::parser::make_ELSE(ctx->line());
enum			return cecko::parser::make_ENUM(ctx->line());
for				return cecko::parser::make_FOR(ctx->line());
if				return cecko::parser::make_IF(ctx->line());
return			return cecko::parser::make_RETURN(ctx->line());
sizeof			return cecko::parser::make_SIZEOF(ctx->line());
struct			return cecko::parser::make_STRUCT(ctx->line());
typedef			return cecko::parser::make_TYPEDEF(ctx->line());
void			return cecko::parser::make_VOID(ctx->line());
while			return cecko::parser::make_WHILE(ctx->line());

char			return cecko::parser::make_ETYPE(cecko::gt_etype::CHAR, ctx->line());
int				return cecko::parser::make_ETYPE(cecko::gt_etype::INT, ctx->line());
_Bool			return cecko::parser::make_ETYPE(cecko::gt_etype::BOOL, ctx->line());

	/* punctuators */
"["				return cecko::parser::make_LBRA(ctx->line());
"]"				return cecko::parser::make_RBRA(ctx->line());
"("				return cecko::parser::make_LPAR(ctx->line());
")"				return cecko::parser::make_RPAR(ctx->line());
"{"				return cecko::parser::make_LCUR(ctx->line());
"}"				return cecko::parser::make_RCUR(ctx->line());
"."				return cecko::parser::make_DOT(ctx->line());
"->"			return cecko::parser::make_ARROW(ctx->line());

"++"			return cecko::parser::make_INCDEC(cecko::gt_incdec::INC, ctx->line());
"--"			return cecko::parser::make_INCDEC(cecko::gt_incdec::DEC, ctx->line());
"&"				return cecko::parser::make_AMP(ctx->line());
"*"				return cecko::parser::make_STAR(ctx->line());
"+"				return cecko::parser::make_ADDOP(cecko::gt_addop::ADD, ctx->line());
"-"				return cecko::parser::make_ADDOP(cecko::gt_addop::SUB, ctx->line());
"!"				return cecko::parser::make_EMPH(ctx->line());

"/"				return cecko::parser::make_DIVOP(cecko::gt_divop::DIV, ctx->line());
"%"				return cecko::parser::make_DIVOP(cecko::gt_divop::MOD, ctx->line());
"<"				return cecko::parser::make_CMPO(cecko::gt_cmpo::LT, ctx->line());
">"				return cecko::parser::make_CMPO(cecko::gt_cmpo::GT, ctx->line());
"<="			return cecko::parser::make_CMPO(cecko::gt_cmpo::LE, ctx->line());
">="			return cecko::parser::make_CMPO(cecko::gt_cmpo::GE, ctx->line());
"=="			return cecko::parser::make_CMPE(cecko::gt_cmpe::EQ, ctx->line());
"!="			return cecko::parser::make_CMPE(cecko::gt_cmpe::NE, ctx->line());
"&&"			return cecko::parser::make_DAMP(ctx->line());
"||"			return cecko::parser::make_DVERT(ctx->line());

";"				return cecko::parser::make_SEMIC(ctx->line());
","				return cecko::parser::make_COMMA(ctx->line());

"="				return cecko::parser::make_ASGN(ctx->line());
"*="			return cecko::parser::make_CASS(cecko::gt_cass::MULA, ctx->line());
"/="			return cecko::parser::make_CASS(cecko::gt_cass::DIVA, ctx->line());
"%="			return cecko::parser::make_CASS(cecko::gt_cass::MODA, ctx->line());
"+="			return cecko::parser::make_CASS(cecko::gt_cass::ADDA, ctx->line());
"-="			return cecko::parser::make_CASS(cecko::gt_cass::SUBA, ctx->line());

	/* other */

{NONDIGIT}{IDCHAR}*	{
						std::string result(yytext, yyleng);
						if(ctx->is_typedef(yytext)) {
							return cecko::parser::make_TYPEIDF(result, ctx->line());
						}
						return cecko::parser::make_IDF(result, ctx->line());
					}

.			ctx->message(cecko::errors::UNCHAR, ctx->line(), yytext);

<INITIAL><<EOF>>	return cecko::parser::make_EOF(ctx->line());

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