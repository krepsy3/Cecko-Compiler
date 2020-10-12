/*

context.cpp

context for the compiler

*/

#include <cstdio>
#include <cstdarg>
#include <sstream>
#include <iomanip>

#include "ckcontext.hpp"


#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define DLL_PUBLIC __attribute__ ((dllexport))
#else
#define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#endif
#else
#if __GNUC__ >= 4
#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#else
#define DLL_PUBLIC
#endif
#endif

extern "C" {
	int DLL_PUBLIC ckrt_printf(const char* s, ...)
	{
		va_list va;
		va_start(va, s);
		int rv = vprintf(s, va);
		va_end(va);
		fflush(stdout);
		return rv;
	}

	int DLL_PUBLIC ckrt_scanf(const char* s, ...)
	{
		va_list va;
		va_start(va, s);
		int rv = vscanf(s, va);
		va_end(va);
		fflush(stdout);
		return rv;
	}

	int DLL_PUBLIC ckrt_sprintf(char *b, const char* s, ...)
	{
		va_list va;
		va_start(va, s);
		int rv = vsprintf(b, s, va);
		va_end(va);
		fflush(stdout);
		return rv;
	}

	int DLL_PUBLIC ckrt_sscanf(const char *b, const char* s, ...)
	{
		va_list va;
		va_start(va, s);
		int rv = vsscanf(b, s, va);
		va_end(va);
		fflush(stdout);
		return rv;
	}

	void DLL_PUBLIC ckrt_memset(void* d, int s, int l)
	{
		memset(d, s, l);
	}
}

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

	void CKTables::declare_library()
	{
		auto t_void = typetable_.get_void_type();
		auto t_char = typetable_.get_char_type();
		auto t_int = typetable_.get_int_type();
		auto t_ptr_void = typetable_.get_pointer_type({ t_void, false });
		auto t_cptr_void = typetable_.get_pointer_type({ t_void, true });
		auto t_ptr_char = typetable_.get_pointer_type({ t_char, false });
		auto t_cptr_char = typetable_.get_pointer_type({ t_char, true });

		globtable_.declare_function("printf", module_, typetable_.get_function_type( t_int, { t_cptr_char }, true), "ckrt_printf");
		globtable_.declare_function("scanf", module_, typetable_.get_function_type(t_int, { t_cptr_char }, true), "ckrt_scanf");
		globtable_.declare_function("sprintf", module_, typetable_.get_function_type(t_int, { t_ptr_char, t_cptr_char }, true), "ckrt_sprintf");
		globtable_.declare_function("sscanf", module_, typetable_.get_function_type(t_int, { t_cptr_char, t_cptr_char }, true), "ckrt_sscanf");
		globtable_.declare_function("memset", module_, typetable_.get_function_type(t_ptr_void, { t_ptr_void, t_int, t_int }, false), "ckrt_memset");
	}

	int CKIREnvironment::run_main(CKIRFunctionObs fnc, int argc, char** argv)
	{
		//auto&& os = llvm::outs();
		auto&& os = std::cout;
		int mainrv = -1;
		{
			// Now we going to create JIT
			std::string errStr;
			auto EE =
				llvm::EngineBuilder(std::move(ckirmoduleptr_))
				.setErrorStr(&errStr)
				.create();

			if (!EE) {
				os << "========== Failed to construct ExecutionEngine: " << errStr << "==========\n";
				return 1;
			}

			if (verifyModule(*ckirmoduleobs_)) {
				os << "========== Error constructing function ==========\n";
				return 1;
			}

			os << "========== starting main() ==========\n";
			os.flush();	// flush before running unsafe code

			std::vector<llvm::GenericValue> Args(2);
			Args[0].IntVal = llvm::APInt(32, argc);
			Args[1].PointerVal = argv;
			auto GV = EE->runFunction(fnc, Args);
			mainrv = GV.IntVal.getSExtValue();

			// hopefully we destroy JIT here
		}
		os << "\n========== main() returned " << mainrv << " ==========\n";
		return mainrv;
	}
}