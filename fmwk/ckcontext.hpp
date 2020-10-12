/*

context.hpp

context for the compiler

*/

#ifndef CECKO_CONTEXT_GUARD__
#define CECKO_CONTEXT_GUARD__

#include <string_view>
#include "cktables.hpp"

namespace cecko {

	using loc_t = unsigned;

	namespace errors {
		// messages
		enum err_s {
			SYNTAX,
			UNCHAR,
			NOFILE,
			UNDEF_IDF,
		};

		enum err_i {
		};

		enum err_n {
			INTERNAL,
			VOIDEXPR,
			ARRAY_NOT_LVALUE,
			NAME_NOT_VALUE,
			NOT_NUMBER,
			NOT_POINTER,
			NOT_NUMBER_OR_POINTER,
			INCOMPATIBLE,
		};
	}

	class context : public CKContext {
	public:
		context(CKTablesObs tables) : CKContext(tables), line_(1) {}

		void message(errors::err_s err, loc_t loc, std::string_view msg);
		void message(errors::err_i err, loc_t loc, int i);
		void message(errors::err_n err, loc_t loc);

		static std::string escape(std::string_view s);

		loc_t line() const { return line_; }
		loc_t incline() { return line_++; }		// returns line value before increment
	private:
		loc_t	line_;
	};

	using context_obs = context*;
}

#endif // CECKO_CONTEXT_GUARD__
