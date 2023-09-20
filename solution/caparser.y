%language "c++"
%require "3.4"
// NEVER SET THIS INTERNALLY - SHALL BE SET BY CMAKE: %defines "../private/caparser.hpp"
// NEVER SET THIS INTERNALLY - SHALL BE SET BY CMAKE: %output "../private/caparser.cpp"
%locations
%define api.location.type {cecko::loc_t}
%define api.namespace {cecko}
%define api.value.type variant
%define api.token.constructor
%define api.parser.class {parser}
%define api.token.prefix {TOK_}
//%define parse.trace
%define parse.assert
%define parse.error verbose

%code requires
{
// this code is emitted to caparser.hpp

#include "ckbisonflex.hpp"

// adjust YYLLOC_DEFAULT macro for our api.location.type
#define YYLLOC_DEFAULT(res,rhs,N)	(res = (N)?YYRHSLOC(rhs, 1):YYRHSLOC(rhs, 0))

#include "ckgrptokens.hpp"
#include "ckcontext.hpp"
#include "casem.hpp"
}

%code
{
// this code is emitted to caparser.cpp

YY_DECL;

using namespace casem;
}

%param {yyscan_t yyscanner}		// the name yyscanner is enforced by Flex
%param {cecko::context * ctx}

%start translation_unit

%token EOF					0			"end of file"

%token						LBRA		"["
%token						RBRA		"]"
%token						LPAR		"("
%token						RPAR		")"
%token						DOT			"."
%token						ARROW		"->"
%token<cecko::gt_incdec>	INCDEC		"++ or --"
%token						COMMA		","
%token						AMP			"&"
%token						STAR		"*"
%token<cecko::gt_addop>		ADDOP		"+ or -"
%token						EMPH		"!"
%token<cecko::gt_divop>		DIVOP		"/ or %"
%token<cecko::gt_cmpo>		CMPO		"<, >, <=, or >="
%token<cecko::gt_cmpe>		CMPE		"== or !="
%token						DAMP		"&&"
%token						DVERT		"||"
%token						ASGN		"="
%token<cecko::gt_cass>		CASS		"*=, /=, %=, +=, or -="
%token						SEMIC		";"
%token						LCUR		"{"
%token						RCUR		"}"

%token						TYPEDEF		"typedef"
%token						VOID		"void"
%token<cecko::gt_etype>		ETYPE		"_Bool, char, or int"
%token						STRUCT		"struct"
%token						ENUM		"enum"
%token						CONST		"const"
%token						IF			"if"
%token						ELSE		"else"
%token						DO			"do"
%token						WHILE		"while"
%token						FOR			"for"
%token						RETURN		"return"
%token						SIZEOF		"sizeof"

%token<CIName>				IDF			"identifier"
%token<CIName>				TYPEIDF		"type identifier"
%token<int>					INTLIT		"integer literal"
%token<CIName>				STRLIT		"string literal"

%type<casem::decl_specificator> type_specifier_qualifier declaration_specifier enum_specifier struct_specifier struct_definition
%type<casem::decl_specificators> declaration_specifiers specifier_qualifier_list
%type<casem::decl_struct_type_carrier> struct_header
%type<casem::decl_enumerator> enumerator
%type<casem::decl_enumerators> enumerator_list
%type<casem::decl_pointer_sequence> pointer pointer_opt
%type<casem::decl_declarator_parts>
    declarator direct_declarator array_declarator function_declarator
    abstract_declarator abstract_declarator_opt direct_abstract_declarator array_abstract_declarator function_abstract_declarator
%type<casem::decl_declarators> member_declarator_list_opt member_declarator_list
%type<casem::singular_declaration> parameter_declaration type_name
%type<casem::singular_declarations> parameter_list
%type<casem::plural_declaration> declaration_full member_declaration
%type<casem::plural_declarations> member_declaration_list
%type<casem::expression_part>
    expression_opt expression assignment_expression
	logical_OR_expression logical_AND_expression
	equality_expression relational_expression
	additive_expression multiplicative_expression unary_expression
	postfix_expression
%type<casem::expression_parts> argument_expression_list_opt argument_expression_list
%type<casem::condition_statement_info> 
	cond_statement_header cond_statement_header_core
	cond_m_statement_else cond_m_statement_else_core
%type<casem::loop_statement_info> loop_statement_introduction loop_statement_header loop_statement_header_core
%type<casem::doloop_statement_info>
	doloop_m_statement_body doloop_u_statement_body
	doloop_statement_introduction
%type<casem::iteration_statement_info> 
	iter_statement_inc iter_statement_inc_core
	iter_statement_cond iter_statement_cond_core
	iter_statement_init iter_statement_init_core

/////////////////////////////////

%%
//Core definitions

translation_unit:
	external_declaration
	| translation_unit external_declaration
	;

external_declaration:
	function_definition
	| declaration
	;


//Declarations

function_definition:
	function_definition_header LCUR function_definition_body RCUR
	;

function_definition_header:
	declaration_specifiers declarator { casem::resolve_function_definition(ctx, $1, $2); }
	;

function_definition_body:
	block_item_list_opt { casem::finalize_function_exit(ctx, @1); }
	;

declaration:
	declaration_core SEMIC

declaration_core:
	declaration_full { casem::resolve_declarations(ctx, $1); }

declaration_full:
	declaration_specifiers { $$ = casem::create_declaration($1); }
	| declaration_specifiers declarator { $$ = casem::create_declaration($1, $2); }
	| declaration_full COMMA declarator { casem::append_declaration(ctx, $1, @2, $3); $$ = $1; }


declaration_specifiers:
	declaration_specifier { casem::decl_specificators c; c.push_back($1); $$ = c; }
	| declaration_specifiers declaration_specifier { $1.push_back($2); $$ = $1; }
	;

declaration_specifier:
	TYPEDEF { $$ = casem::create_typedef_specifier(@1); }
	| type_specifier_qualifier { $$ = $1; }
	;

type_specifier_qualifier:
	VOID { $$ = casem::create_type_specifier(ctx->get_void_type(), @1); }
	| ETYPE { $$ = casem::create_type_specifier(casem::get_ETYPE_type(ctx, $1), @1); }
	| struct_specifier { $$ = $1; }
	| enum_specifier { $$ = $1; }
	| TYPEIDF { $$ = casem::create_type_specifier(casem::get_TYPEIDF_type(ctx, $1), @1); }
	| CONST { $$ = casem::create_const_qualifier(@1); }
	;

struct_specifier:
	STRUCT IDF { $$ = casem::create_type_specifier(ctx->declare_struct_type($2, @2), @2); }
	| STRUCT TYPEIDF { $$ = casem::create_type_specifier(ctx->declare_struct_type($2, @2), @2); }
	| struct_definition RCUR { $$ = $1;}
	;

struct_definition:
	struct_header LCUR member_declaration_list { $$ = casem::create_struct_definition(ctx, $1, $3); }
	;

struct_header:
	STRUCT IDF { casem::decl_struct_type_carrier s; s.type = ctx->define_struct_type_open($2, @2); s.struct_identifier_loc = @2; $$ = s; }
	| STRUCT TYPEIDF { casem::decl_struct_type_carrier s; s.type = ctx->define_struct_type_open($2, @2); s.struct_identifier_loc = @2; $$ = s; }
	;

member_declaration_list:
	member_declaration { casem::plural_declarations d; d.push_back($1); $$ = d; }
	| member_declaration_list member_declaration { $1.push_back($2); $$ = $1; }
	;

member_declaration:
	specifier_qualifier_list member_declarator_list_opt SEMIC { $$ = casem::create_declaration($1, $2); }
	;

specifier_qualifier_list:
	type_specifier_qualifier { casem::decl_specificators c; c.push_back($1); $$ = c; }
	| specifier_qualifier_list type_specifier_qualifier { $1.push_back($2); $$ = $1; }
	;

member_declarator_list_opt:
	%empty { casem::decl_declarators d; $$ = d; }
	| member_declarator_list { $$ = $1; }
	;

member_declarator_list:
	declarator { casem::decl_declarators d; d.push_back($1); $$ = d; }
	| member_declarator_list COMMA declarator { $1.push_back($3); $$ = $1; }
	;

enum_specifier:
	ENUM IDF LCUR enumerator_list RCUR { $$ = casem::create_enum_definition(ctx, $2, @2, $4); }
	| ENUM IDF LCUR enumerator_list COMMA RCUR { $$ = casem::create_enum_definition(ctx, $2, @2, $4); }
	| ENUM IDF { $$ = casem::create_type_specifier(ctx->declare_enum_type($2, @2), @2); }
	| ENUM TYPEIDF LCUR enumerator_list RCUR { $$ = casem::create_enum_definition(ctx, $2, @2, $4); }
	| ENUM TYPEIDF LCUR enumerator_list COMMA RCUR { $$ = casem::create_enum_definition(ctx, $2, @2, $4); }
	| ENUM TYPEIDF { $$ = casem::create_type_specifier(ctx->declare_enum_type($2, @2), @2); }
	;

enumerator_list:
	enumerator { casem::decl_enumerators e; e.push_back($1); $$ = e; }
	| enumerator_list COMMA enumerator { $1.push_back($3); $$ = $1; }
	;

enumerator:
	IDF { $$ = casem::create_enumerator(ctx, $1, @1); }
	| IDF ASGN logical_OR_expression { $$ = casem::create_enumerator(ctx, $1, @1, $3); }
	;

declarator:
	pointer_opt direct_declarator { casem::append_pointer_to_declarator($2, $1); $$ = $2; }
	;

direct_declarator:
	IDF { casem::decl_declarator_parts p; p.push_back(casem::create_idf_declarator($1, @1)); $$ = p; }
	| LPAR declarator RPAR { decl_declarator_part p; $2.push_back(p); $$ = $2; }
	| array_declarator { $$ = $1; }
	| function_declarator { $$ = $1; }
	;

array_declarator:
	direct_declarator LBRA assignment_expression RBRA { $1.push_back(casem::create_array_declarator(ctx, $3, @3)); $$ = $1; }
	;

function_declarator:
	direct_declarator LPAR parameter_list RPAR { $1.push_back(casem::create_function_declarator($3)); $$ = $1; }
	;

pointer_opt:
	%empty { casem::decl_pointer_sequence p; $$ = p; }
	| pointer { $$ = $1; }
	;

pointer:
	STAR { casem::decl_pointer_sequence p; p.push_back(casem::create_decl_pointer()); $$ = p; }
	| STAR CONST { casem::decl_pointer_sequence p; p.push_back(casem::create_decl_const_pointer()); $$ = p; }
	| pointer STAR { $1.push_back(casem::create_decl_pointer()); $$ = $1; }
	| pointer STAR CONST { $1.push_back(casem::create_decl_const_pointer()); $$ = $1; }
	;

parameter_list:
	parameter_declaration { casem::singular_declarations p; p.push_back($1); $$ = p; }
	| parameter_list COMMA parameter_declaration { $1.push_back($3); $$ = $1; }
	;

parameter_declaration:
	declaration_specifiers declarator { $$ = casem::merge_specificators_declarator($1, $2); }
	| declaration_specifiers abstract_declarator_opt { $$ = casem::merge_specificators_declarator($1, $2); }
	;

type_name:
	specifier_qualifier_list abstract_declarator_opt { $$ = casem::merge_specificators_declarator($1, $2); }
	;

abstract_declarator_opt:
	%empty { casem::decl_declarator_parts p; $$ = p; }
	| abstract_declarator { $$ = $1; }
	;

abstract_declarator:
	pointer { casem::decl_declarator_parts p; p.push_back(casem::create_pointer_declarator($1)); $$ = p; }
	| pointer_opt direct_abstract_declarator { casem::append_pointer_to_declarator($2, $1); $$ = $2; }
	;


direct_abstract_declarator:
	LPAR abstract_declarator RPAR { decl_declarator_part p; $2.push_back(p); $$ = $2; }
	|
	array_abstract_declarator { $$ = $1; }
	|
	function_abstract_declarator { $$ = $1; }
	;

array_abstract_declarator:
	LBRA assignment_expression RBRA { casem::decl_declarator_parts p; p.push_back(casem::create_array_declarator(ctx, $2, @2)); $$ = p; }
	| direct_abstract_declarator LBRA assignment_expression RBRA { $1.push_back(casem::create_array_declarator(ctx, $3, @3)); $$ = $1; }
	;

function_abstract_declarator:
	LPAR parameter_list RPAR { casem::decl_declarator_parts p; p.push_back(casem::create_function_declarator($2)); $$ = p; }
	| direct_abstract_declarator LPAR parameter_list RPAR { $1.push_back(casem::create_function_declarator($3)); $$ = $1; }
	;


//Expressions

postfix_expression:
	IDF { $$ = casem::generate_identifier_expression(ctx, $1, @1); }
	| INTLIT { $$ = casem::generate_intlit_expression(ctx, $1, @1); }
	| STRLIT { $$ = casem::generate_strlit_expression(ctx, $1, @1); }
	| LPAR expression RPAR { $$ = $2; }
	| postfix_expression LBRA expression RBRA { $$ = casem::generate_index_expression(ctx, $1, $3, @2); }
	| postfix_expression LPAR argument_expression_list_opt RPAR { $$ = casem::generate_call_expression(ctx, $1, $3, @2); }
	| postfix_expression DOT IDF { $$ = casem::generate_access_expression(ctx, $1, $3, @2); }
	| postfix_expression ARROW IDF { $$ = casem::generate_arrow_expression(ctx, $1, $3, @2); }
	| postfix_expression INCDEC { $$ = casem::generate_incdec_expression(ctx, $1, $2, @2, true); }
	;

argument_expression_list_opt:
	%empty { casem::expression_parts p; $$ = p; }
	| argument_expression_list { $$ = $1; }
	;

argument_expression_list:
	assignment_expression { casem::expression_parts p; p.push_back($1); $$ = p; }
	| argument_expression_list COMMA assignment_expression { $1.push_back($3); $$ = $1; }
	;

unary_expression:
	postfix_expression { $$ = $1; }
	| INCDEC unary_expression { $$ = casem::generate_incdec_expression(ctx, $2, $1, @1, false); }
	| AMP unary_expression { $$ = casem::generate_ref_expression(ctx, $2, @1); }
	| STAR unary_expression { $$ = casem::generate_deref_expression(ctx, $2, @1); }
	| ADDOP unary_expression { $$ = casem::generate_sign_expression(ctx, $2, $1, @1); }
	| EMPH unary_expression { $$ = casem::generate_not_expression(ctx, $2, @1); }
	| SIZEOF LPAR type_name RPAR { $$ = casem::generate_sizeof_expression(ctx, $3, @1); }
	;

multiplicative_expression:
	unary_expression { $$ = $1; }
	| multiplicative_expression STAR unary_expression { $$ = casem::generate_mult_expression(ctx, $1, $3, @2); }
	| multiplicative_expression DIVOP unary_expression { $$ = casem::generate_div_expression(ctx, $1, $3, $2, @2); }
	;

additive_expression:
	multiplicative_expression { $$ = $1; }
	| additive_expression ADDOP multiplicative_expression { $$ = casem::generate_add_expression(ctx, $1, $3, $2, @2); }
	;

relational_expression:
	additive_expression { $$ = $1; }
	| relational_expression CMPO additive_expression { $$ = casem::generate_compare_expression(ctx, $1, $3, $2, @2); }
	;

equality_expression:
	relational_expression { $$ = $1; }
	| equality_expression CMPE relational_expression { $$ = casem::generate_eq_expression(ctx, $1, $3, $2, @2); }
	;

logical_AND_expression:
	equality_expression { $$ = $1; }
	| logical_AND_expression DAMP equality_expression{ $$ = casem::generate_and_expression(ctx, $1, $3, @2); }
	;

logical_OR_expression:
	logical_AND_expression { $$ = $1; }
	| logical_OR_expression DVERT logical_AND_expression { $$ = casem::generate_or_expression(ctx, $1, $3, @2); }
	;

assignment_expression:
	logical_OR_expression { $$ = $1; }
	| unary_expression ASGN assignment_expression { $$ = casem::generate_assign_expression(ctx, $1, $3, @2); }
	| unary_expression CASS assignment_expression { $$ = casem::generate_assign_arith_expression(ctx, $1, $3, $2, @2);}
	;

expression:
	assignment_expression { $$ = $1; }
	;

expression_opt:
	%empty { casem::expression_part p; $$ = p; }
	| expression { $$ = $1; }
	;


//Statements

statement: 
	m_statement
    | u_statement
    
m_statement:
	cond_m_statement_else m_statement { casem::parse_condition_finalize(ctx, $1); }
	| loop_statement_header m_statement { casem::parse_loop_finalize(ctx, $1); }
	| doloop_m_statement_body LPAR expression RPAR SEMIC { casem::parse_doloop_finalize(ctx, $1, $3); }
	| iter_statement_inc m_statement { casem::parse_iteration_finalize(ctx, $1); }
	| nonrecursive_statement
    ;

u_statement:
	cond_statement_header statement { casem::parse_condition_finalize(ctx, $1); }
    | cond_m_statement_else u_statement { casem::parse_condition_finalize(ctx, $1); }
	| loop_statement_header u_statement { casem::parse_loop_finalize(ctx, $1); }
	| doloop_u_statement_body LPAR expression RPAR SEMIC { casem::parse_doloop_finalize(ctx, $1, $3); }
	| iter_statement_inc u_statement { casem::parse_iteration_finalize(ctx, $1); }
    ;


cond_m_statement_else:
	cond_m_statement_else_core ELSE { $$ = $1; }

cond_m_statement_else_core:
	cond_statement_header m_statement { casem::parse_condition_else(ctx, $1); $$ = $1; }

cond_statement_header:
	cond_statement_header_core RPAR { $$ = $1; }

cond_statement_header_core:
	IF LPAR expression { $$ = casem::parse_condition_if(ctx, $3, @1); }


loop_statement_header:
	loop_statement_header_core RPAR { $$ = $1; }

loop_statement_header_core:
	loop_statement_introduction LPAR expression { casem::parse_loop_cond(ctx, $1, $3); $$ = $1; }

loop_statement_introduction:
	WHILE { $$ = casem::parse_loop_while(ctx, @1); }


doloop_m_statement_body:
	doloop_statement_introduction m_statement WHILE { casem::parse_doloop_while(ctx, $1, @3); $$ = $1; }
	
doloop_u_statement_body:
	doloop_statement_introduction u_statement WHILE { casem::parse_doloop_while(ctx, $1, @3); $$ = $1; }

doloop_statement_introduction:
	DO { $$ = casem::parse_doloop_do(ctx, @1); }


iter_statement_inc:
	iter_statement_inc_core RPAR { $$ = $1; }

iter_statement_inc_core:
	iter_statement_cond expression_opt { parse_iteration_inc (ctx, $1); $$ = $1; }

iter_statement_cond:
	iter_statement_cond_core SEMIC { $$ = $1; }

iter_statement_cond_core:
	iter_statement_init expression_opt { parse_iteration_cond(ctx, $1, $2); $$ = $1; }

iter_statement_init:
	iter_statement_init_core SEMIC { $$ = $1; }

iter_statement_init_core:
	FOR LPAR expression_opt { $$ = casem::parse_iteration_init(ctx, @1); }


nonrecursive_statement:
	expression_statement
	| compound_statement_header LCUR compound_statement_body RCUR
	| RETURN expression_opt SEMIC { casem::generate_explicit_return(ctx, $2, @1); }
	;

compound_statement_header:
	%empty { ctx->enter_block(); }

compound_statement_body:
	block_item_list_opt { ctx->exit_block(); }

block_item_list_opt:
	%empty
	| block_item_list
	;

block_item_list:
	block_item
	| block_item_list block_item
	;

block_item:
	declaration
	| statement
	;

expression_statement:
	expression_opt SEMIC
	;
%%

namespace cecko {

	void parser::error(const location_type& l, const std::string& m)
	{
		ctx->message(cecko::errors::SYNTAX, l, m);
	}

}
