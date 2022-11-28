#ifndef casem_hpp_
#define casem_hpp_

#include "cktables.hpp"
#include "ckcontext.hpp"
#include "ckgrptokens.hpp"

namespace casem {
	using namespace cecko;

	//Lexer Auxiliary functions
	
		//prepends number to first position in array, shifting already present values right
		void shift_enregister(int, int*, int, int&);

		//converts char literal to number as a digit 
		int char_to_int(char);

		//checks if text interpreted as number literal is inside integer type
		bool number_literal_outrange(char*, int, bool);

		//converts text to integer as a numeric literal
		int parse_number_literal(char*, int, bool);

		//checks if a character can be interpreted as a number literal digit
		bool is_numeral_char(char, bool);


	//Declaration analysis auxiliary
	struct decl_specifier {
		bool intSpecifier;
		bool charSpecifier;
		bool voidSpecifier;
		bool typeSpecifier;
		CIName typeSpecifierName;
	};

	using spec_qualf_list = std::vector<decl_specifier>;

}

#endif
