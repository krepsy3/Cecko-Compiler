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

	class context : public CKContext {
	public:
		context(CKTablesObs tables) : CKContext(tables), line_(1) {}

		// messages
		enum err_s {
			SYNTAX,
			UNCHAR,
			NOFILE,
		};

		enum err_i {
		};

		void message(err_s err, loc_t loc, std::string_view msg);
		void message(err_i err, loc_t loc, int i);

		loc_t line() const { return line_; }
		loc_t incline() { return line_++; }		// returns line value before increment
	private:
		loc_t	line_;
	};

	using context_obs = context*;
}

#endif // CECKO_CONTEXT_GUARD__
