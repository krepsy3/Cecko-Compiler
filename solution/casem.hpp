#ifndef casem_hpp_
#define casem_hpp_

#include <vector>
#include <iostream>
#include <memory>
#include "cktables.hpp"
#include "ckcontext.hpp"
#include "ckgrptokens.hpp"

namespace casem {

//#pragma region Lexer	
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
//#pragma endregion

	//since some declarations rely on constant expressions, declare the carrier struct in advance
	struct expression_part;

//#pragma region Declarations
	struct decl_specificator {
		bool is_const;
		bool is_type;
		bool is_typedef;
		cecko::CKTypeObs type;
		cecko::loc_t location;
		decl_specificator() : is_const(), is_type(), is_typedef(), type(), location() { }
	};
	using decl_specificators = std::vector<decl_specificator>;

	cecko::CKTypeObs get_ETYPE_type(cecko::context *, const cecko::gt_etype &);
	cecko::CKTypeObs get_TYPEIDF_type(cecko::context *, const cecko::CIName &);

	decl_specificator create_type_specifier(cecko::CKTypeObs, cecko::loc_t);
	decl_specificator create_const_qualifier(cecko::loc_t);
	decl_specificator create_typedef_specifier(cecko::loc_t);

	struct decl_enumerator {
		bool has_value;
		cecko::CKIRConstantIntObs value;
		cecko::CIName identifier;
		cecko::loc_t identifier_loc;
		decl_enumerator() : has_value(), value(), identifier(), identifier_loc() { }
	};
	using decl_enumerators = std::vector<decl_enumerator>;

	decl_enumerator create_enumerator(cecko::context *, const cecko::CIName &, cecko::loc_t);
	decl_enumerator create_enumerator(cecko::context *, const cecko::CIName &, cecko::loc_t, expression_part &);
	decl_specificator create_enum_definition(cecko::context *, const cecko::CIName &, cecko::loc_t, const decl_enumerators &);

	struct decl_struct_type_carrier {
		cecko::CKStructTypeSafeObs type;
		cecko::loc_t struct_identifier_loc;
		decl_struct_type_carrier() : type(), struct_identifier_loc() { }
	};

	cecko::CKTypeRefPack transform_decl_specs(cecko::context *, const decl_specificators &);


	struct decl_pointer {
		bool is_const;
		decl_pointer() : is_const() { }
	};
	using decl_pointer_sequence = std::vector<decl_pointer>;

	decl_pointer create_decl_pointer();
	decl_pointer create_decl_const_pointer();

	cecko::CKTypeRefPack transform_type_pointerize(cecko::context *, const cecko::CKTypeRefPack &, const decl_pointer_sequence &);


	struct singular_declaration;
	using singular_declarations = std::vector<singular_declaration>;

	struct decl_declarator_part {
		bool is_identifier;
		bool is_function;
		bool is_array;

		cecko::CIName identifier;
		cecko::loc_t identifier_loc;
		singular_declarations function_params;
		cecko::CKIRConstantIntObs array_size;
		decl_pointer_sequence pointer;

		decl_declarator_part() : is_identifier(), is_function(), is_array(), identifier(), identifier_loc(), function_params(), array_size(), pointer() { }
	};
	using decl_declarator_parts = std::vector<decl_declarator_part>;
	using decl_declarators = std::vector<decl_declarator_parts>;

	struct singular_declaration {
		decl_specificators specs;
		decl_declarator_parts declarator;
		singular_declaration() : specs(), declarator() { }
	};
	using singular_declarations = std::vector<singular_declaration>;

	void append_pointer_to_declarator(decl_declarator_parts &, const decl_pointer_sequence &);
	decl_declarator_part create_idf_declarator(const cecko::CIName &, cecko::loc_t);
	decl_declarator_part create_array_declarator(cecko::context *, expression_part &, cecko::loc_t);
	decl_declarator_part create_function_declarator(const singular_declarations &);
	decl_declarator_part create_pointer_declarator(const decl_pointer_sequence &);

	singular_declaration merge_specificators_declarator(const decl_specificators &, const decl_declarator_parts &);

	struct function_header_data {
		cecko::CKTypeObsArray function_params;
		cecko::CKFunctionFormalPackArray formal_pack_array;
		cecko::CKFunctionTypeSafeObs type;
		function_header_data() : function_params(), formal_pack_array(), type() { }
	};

	struct transformed_singular_declaration {
		cecko::CKTypeRefPack type;
		bool has_identifier;

		cecko::CIName identifier;
		cecko::loc_t identifier_loc;

		function_header_data function_data;

		transformed_singular_declaration() : type(), has_identifier(), identifier(), identifier_loc() { }
	};

	transformed_singular_declaration transform_declaration(cecko::context *, const cecko::CKTypeRefPack &, const decl_declarator_parts &);
	function_header_data transform_function_decl(cecko::context *, cecko::CKTypeObs, const decl_declarator_part &);

	struct plural_declaration {
		decl_specificators specs;
		decl_declarators declarators;
		plural_declaration() : specs(), declarators() { }
	};
	using plural_declarations = std::vector<plural_declaration>;

	plural_declaration create_declaration(const decl_specificators &);
	plural_declaration create_declaration(const decl_specificators &, const decl_declarator_parts &);
	void append_declaration(cecko::context *, plural_declaration &, cecko::loc_t, const decl_declarator_parts &);
	plural_declaration create_declaration(const decl_specificators &, const decl_declarators &);

	void resolve_declarations(cecko::context *, const plural_declaration &);
	void resolve_function_definition(cecko::context *, const decl_specificators &, const decl_declarator_parts &);

	decl_specificator create_struct_definition(cecko::context *, const decl_struct_type_carrier &, const plural_declarations &);
	//#pragma endregion

//#pragma region Expressions
	enum class expression_part_mode { empty, lvalue, rvalue };

	struct expression_part {
		cecko::CKTypeSafeObs type_descriptor;
		cecko::CKIRValueObs value_representation;
		expression_part_mode mode;
		bool is_const;
		cecko::CIName twine_base;

		expression_part() : 
			type_descriptor(nullptr),
			value_representation(nullptr),
			mode(expression_part_mode::empty),
		    is_const(),
		    twine_base() { }

		expression_part(cecko::CKNamedSafeObs descriptor) :
			type_descriptor(descriptor->get_type()),
			value_representation(descriptor->get_type()->is_function() ? descriptor->get_function_ir() : descriptor->get_ir()),
			mode(expression_part_mode::lvalue),
			is_const(descriptor->is_const()),
			twine_base(descriptor->get_name()) { }

		expression_part(cecko::CKIRValueObs value_representation, cecko::CKTypeSafeObs type_descriptor, cecko::CIName &name) :
			type_descriptor(type_descriptor),
			value_representation(value_representation),
			mode(expression_part_mode::rvalue),
			is_const(false),
			twine_base(name) { }

		expression_part(cecko::CKIRValueObs value_representation, cecko::CKTypeSafeObs type_descriptor) :
			type_descriptor(type_descriptor),
			value_representation(value_representation),
			mode(expression_part_mode::rvalue),
			is_const(false),
			twine_base() { }

	};

	using expression_parts = std::vector<expression_part>;

	enum class arith_op { add, sub, mul, div, mod };
	constexpr arith_op get_arith_op(cecko::gt_addop op) {
		switch (op) {
			case cecko::gt_addop::ADD: return arith_op::add;
			case cecko::gt_addop::SUB: return arith_op::sub;
		}
		return arith_op::add;
	}
	constexpr arith_op get_arith_op(cecko::gt_divop op) {
		switch (op) {
			case cecko::gt_divop::DIV: return arith_op::div;
			case cecko::gt_divop::MOD: return arith_op::mod;
		}
		return arith_op::add;
	}
	constexpr arith_op get_arith_op(cecko::gt_incdec op) {
		switch (op) {
			case cecko::gt_incdec::INC: return arith_op::add;
			case cecko::gt_incdec::DEC: return arith_op::sub;
		}
		return arith_op::add;
	}
	constexpr arith_op get_arith_op(cecko::gt_cass op) {
		switch (op) {
			case cecko::gt_cass::ADDA: return arith_op::add;
			case cecko::gt_cass::SUBA: return arith_op::sub;
			case cecko::gt_cass::MULA: return arith_op::mul;
			case cecko::gt_cass::DIVA: return arith_op::div;
			case cecko::gt_cass::MODA: return arith_op::mod;
		}
		return arith_op::add;
	}
	
	enum class rel_op { EQ, NEQ, LT, LEQ, GEQ, GT };
	constexpr rel_op get_rel_op(cecko::gt_cmpe op) {
		switch (op) {
			case cecko::gt_cmpe::EQ: return rel_op::EQ;
			case cecko::gt_cmpe::NE: return rel_op::NEQ;
		}
		return rel_op::EQ;
	}
	constexpr rel_op get_rel_op(cecko::gt_cmpo op) {
		switch (op) {
			case cecko::gt_cmpo::LT: return rel_op::LT;
			case cecko::gt_cmpo::LE: return rel_op::LEQ;
			case cecko::gt_cmpo::GE: return rel_op::GEQ;
			case cecko::gt_cmpo::GT: return rel_op::GT;
		}
		return rel_op::EQ;
	}

	bool has_implicit_conversion(cecko::CKTypeObs, cecko::CKTypeSafeObs);
	bool is_comparable(cecko::CKTypeObs, cecko::CKTypeSafeObs);
	expression_part generate_implicit_conversion(cecko::context *, expression_part &, cecko::CKTypeSafeObs);
	expression_part generate_lvalue_load(cecko::context *, expression_part &);
	expression_part generate_number_arith(cecko::context *, expression_part &, expression_part &, arith_op, cecko::loc_t);
	expression_part generate_pointer_add(cecko::context *, expression_part &, expression_part &, arith_op, cecko::loc_t);
	expression_part generate_relational_expression(cecko::context *, expression_part &, expression_part &, rel_op, cecko::loc_t);
	
	expression_part generate_assign_expression(cecko::context *, expression_part &, expression_part &, cecko::loc_t);
	expression_part generate_assign_arith_expression(cecko::context *, expression_part &, expression_part &, cecko::gt_cass, cecko::loc_t);
	expression_part generate_or_expression(cecko::context *, expression_part &, expression_part &, cecko::loc_t);
	expression_part generate_and_expression(cecko::context *, expression_part &, expression_part &, cecko::loc_t);
	expression_part generate_eq_expression(cecko::context *, expression_part &, expression_part &, cecko::gt_cmpe, cecko::loc_t);
	expression_part generate_compare_expression(cecko::context *, expression_part &, expression_part &, cecko::gt_cmpo, cecko::loc_t);
	expression_part generate_add_expression(cecko::context *, expression_part &, expression_part &, cecko::gt_addop, cecko::loc_t);
	expression_part generate_mult_expression(cecko::context *, expression_part &, expression_part &, cecko::loc_t);
	expression_part generate_div_expression(cecko::context *, expression_part &, expression_part &, cecko::gt_divop, cecko::loc_t);
	expression_part generate_incdec_expression(cecko::context *, expression_part &, cecko::gt_incdec, cecko::loc_t, bool);
	expression_part generate_ref_expression(cecko::context *, expression_part &, cecko::loc_t);
	expression_part generate_deref_expression(cecko::context *, expression_part &, cecko::loc_t);
	expression_part generate_sign_expression(cecko::context *, expression_part &, cecko::gt_addop, cecko::loc_t);
	expression_part generate_not_expression(cecko::context *, expression_part &, cecko::loc_t);
	expression_part generate_sizeof_expression(cecko::context *, singular_declaration &, cecko::loc_t);
	expression_part generate_arrow_expression(cecko::context *, expression_part &, cecko::CIName &, cecko::loc_t);
	expression_part generate_access_expression(cecko::context *, expression_part &, cecko::CIName &, cecko::loc_t);
	expression_part generate_call_expression(cecko::context *, expression_part &, expression_parts &, cecko::loc_t);
	expression_part generate_index_expression(cecko::context *, expression_part &, expression_part &, cecko::loc_t);
	expression_part generate_identifier_expression(cecko::context *, cecko::CIName &, cecko::loc_t);
	expression_part generate_intlit_expression(cecko::context *, int, cecko::loc_t);
	expression_part generate_strlit_expression(cecko::context *, cecko::CIName &, cecko::loc_t);

//#pragma endregion

//#pragma region Control Flow
	void finalize_function_exit(cecko::context *, cecko::loc_t);
	void generate_explicit_return(cecko::context *, expression_part &, cecko::loc_t);

	struct condition_statement_info {
		using BB = cecko::CKIRBasicBlockObs;
		BB trueblock;
		BB falseblock;
		BB endblock;
		cecko::loc_t if_location;

		condition_statement_info() :
			trueblock(), falseblock(), endblock(), if_location() { }

		condition_statement_info(BB trueblock, BB falseblock, BB endblock, cecko::loc_t loc) :
			trueblock(trueblock), falseblock(falseblock), endblock(endblock), if_location(loc) { }

		condition_statement_info(BB trueblock, BB falseblock, cecko::loc_t loc) :
			condition_statement_info(trueblock, falseblock, falseblock, loc) { }
	};

	condition_statement_info parse_condition_if(cecko::context *, expression_part &, cecko::loc_t);
	void parse_condition_else(cecko::context *, condition_statement_info &);
	void parse_condition_finalize(cecko::context *, condition_statement_info &);


	struct loop_statement_info {
		cecko::CKIRBasicBlockObs condblock;
		cecko::CKIRBasicBlockObs endblock;
		cecko::CKIRBasicBlockObs loopblock;
		cecko::loc_t while_location;

		loop_statement_info() :
			condblock(), endblock(), loopblock(), while_location() { }

		loop_statement_info(cecko::loc_t loc) :
			condblock(), endblock(), loopblock(), while_location(loc) { }
	};

	loop_statement_info parse_loop_while(cecko::context *, cecko::loc_t);
	void parse_loop_cond(cecko::context *, loop_statement_info &, expression_part &);
	void parse_loop_finalize(cecko::context *, loop_statement_info &);


	struct doloop_statement_info {
		cecko::CKIRBasicBlockObs loopblock;
		cecko::CKIRBasicBlockObs endblock;
		cecko::loc_t do_location;
		cecko::loc_t while_location;

		doloop_statement_info() :
			loopblock(), endblock(), do_location(), while_location() { }
	};

	doloop_statement_info parse_doloop_do(cecko::context *, cecko::loc_t);
	void parse_doloop_while(cecko::context *, doloop_statement_info &, cecko::loc_t);
	void parse_doloop_finalize(cecko::context *, doloop_statement_info &, expression_part &);


	struct iteration_statement_info {
		cecko::CKIRBasicBlockObs condblock;
		cecko::CKIRBasicBlockObs endblock;
		cecko::CKIRBasicBlockObs incblock;
		cecko::CKIRBasicBlockObs loopblock;
		cecko::loc_t for_location;

		iteration_statement_info() :
			condblock(), endblock(), incblock(), loopblock(), for_location() { }

		iteration_statement_info(cecko::loc_t loc) :
			condblock(), endblock(), incblock(), loopblock(), for_location(loc) { }
	};

	iteration_statement_info parse_iteration_init(cecko::context *, cecko::loc_t);
	void parse_iteration_cond(cecko::context *, iteration_statement_info &, expression_part &);
	void parse_iteration_inc(cecko::context *, iteration_statement_info &);
	void parse_iteration_finalize(cecko::context *, iteration_statement_info &);
//#pragma endregion
}

#endif
