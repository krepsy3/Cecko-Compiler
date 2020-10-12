/*

context.cpp

context for the compiler

*/

#include <cstdio>

#include "ckcontext.hpp"

namespace {

	const char* err_s_msg[] = {
		"Syntax error: %s",
		"Unknown character '%s'",
		"Unable to open input file %s",
		"Undefined identifier \"%s\"",
	};
	/*
	const char* err_i_msg[] = {
	};
	*/
	const char* err_n_msg[] = {
		"INTERNAL ERROR",
		"expression is void",
		"array expression is not an lvalue",
		"name does not denote a value",
		"expression is not a number",
		"expression is not a pointer",
		"expression is not a number or pointer",
		"Incompatible operand(s)",
	};
}

namespace cecko {

	void context::message(err_s err, loc_t loc, std::string_view msg)
	{
		fprintf(stdout, "Error (line %d): ", loc);
		fprintf(stdout, err_s_msg[err], msg.data());
		fputc('\n', stdout);
	}
	/*
	void context::message(err_i err, loc_t loc, int i)
	{
		fprintf(stdout, "Error (line %d): ", loc);
		fprintf(stdout, err_i_msg[err], i);
		fputc('\n', stdout);
	}
	*/
	void context::message(err_n err, loc_t loc)
	{
		fprintf(stdout, "Error (line %d): ", loc);
		fprintf(stdout, err_n_msg[err]);
		fputc('\n', stdout);
	}

	void CKTables::declare_library()
	{
		auto t_void = typetable_.get_void_type();
		auto t_char = typetable_.get_char_type();
		auto t_int = typetable_.get_int_type();
		auto t_ptr_void = typetable_.get_pointer_type({ t_void, false });
		auto t_cptr_void = typetable_.get_pointer_type({ t_void, true });
		auto t_ptr_char = typetable_.get_pointer_type({ t_char, false });
		auto t_cptr_char = typetable_.get_pointer_type({ t_char, true });

		globtable_.declare_function("printf", module_, typetable_.get_function_type( t_int, { t_cptr_char }, true));
		globtable_.declare_function("scanf", module_, typetable_.get_function_type(t_int, { t_cptr_char }, true));
		globtable_.declare_function("sprintf", module_, typetable_.get_function_type(t_int, { t_ptr_char, t_cptr_char }, true));
		globtable_.declare_function("sscanf", module_, typetable_.get_function_type(t_int, { t_cptr_char, t_cptr_char }, true));
		globtable_.declare_function("memset", module_, typetable_.get_function_type(t_ptr_void, { t_ptr_void, t_int, t_int }, true));
	}

}