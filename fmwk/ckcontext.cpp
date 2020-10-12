/*

context.cpp

context for the compiler

*/

#include "ckcontext.hpp"

#include <sstream>
#include <iomanip>

namespace {

	std::array<const char *,2> err_s_msg[] = {
		{ "Syntax error: ", "" },
		{ "Unknown character '", "'" },
		{ "Unable to open input file \"", "\"" },
		{ "Undefined identifier \"", "\"" },
	};
	/*
	std::array<const char *,2> err_i_msg[] = {
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

	std::string context::escape(std::string_view s)
	{
		std::ostringstream r;
		for (std::uint8_t ch : s)
		{
			if (ch < 32 || ch > 126 || ch == '\'' || ch == '"')
				r << "\\x" << std::hex << std::setfill('0') << std::setw(2) << (int)ch;
			else
				r.put(ch);
		}
		return r.str();
	}

	void context::message(errors::err_s err, loc_t loc, std::string_view msg)
	{
		/*
		fprintf(stdout, "Error (line %d): ", loc);
		fprintf(stdout, err_s_msg[err], msg.data());
		fputc('\n', stdout);
		*/
		auto&& e = err_s_msg[err];
		std::cout << "Error (line " << loc << "): " << e[0] << escape(msg) << e[1] << std::endl;
	}
	/*
	void context::message(errors::err_i err, loc_t loc, int i)
	{
		auto&& e = err_i_msg[err];
		std::cout << "Error (line " << loc << "): " << e[0] << i << e[1] << std::endl;
	}
	*/
	void context::message(errors::err_n err, loc_t loc)
	{
		auto&& e = err_n_msg[err];
		std::cout << "Error (line " << loc << "): " << e << std::endl;
	}
}