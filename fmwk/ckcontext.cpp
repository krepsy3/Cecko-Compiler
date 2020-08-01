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
	};

}

namespace cecko {

	void context::message(err_s err, loc_t loc, std::string_view msg)
	{
		fprintf(stderr, "Error (line %d): ", loc);
		fprintf(stderr, err_s_msg[err], msg.data());
		fputc('\n', stderr);
	}

	void context::message(err_i err, loc_t loc, int i)
	{

	}
}