/*

context.cpp

context for the compiler

*/

#include "ckcontext.hpp"

#include <sstream>
#include <iomanip>

namespace cecko {

	namespace errors {

		err_def_s SYNTAX{ "Syntax error: ", "" };
		err_def_s INTOUTRANGE{ "Integer literal \"", "\" out of range" };
		err_def_s BADINT{ "Malformed integer literal \"", "\"" };
		err_def_s BADESCAPE{ "Malformed escape sequence \"", "\"" };
		err_def_s UNCHAR{ "Unknown character '", "'" };
		err_def_s NOFILE{ "Unable to open input file \"", "\"" };
		err_def_s UNDEF_IDF{ "Undefined identifier \"", "\"" };
		err_def_s DUPLICATE_IDF{ "Identifier \"", "\" was already declared in this scope" };
		err_def_s DUPLICATE_TAG{ "Tag \"", "\" was already declared as another kind in this scope" };
		err_def_s DUPLICATE_FUNCTION_DEFINITION{ "Function \"", "\" was already defined" };
		err_def_s DUPLICATE_STRUCT_DEFINITION{ "Struct \"", "\" was already defined" };
		err_def_s DUPLICATE_ENUM_DEFINITION{ "Enum \"", "\" was already defined" };

		err_def_n INTERNAL{ "INTERNAL ERROR" };
		err_def_n EMPTYCHAR{ "Empty character" };
		err_def_n MULTICHAR_LONG{ "Multi-character too long" };
		err_def_n EOLINSTRCHR{ "End of line in string or character literal" };
		err_def_n EOFINSTRCHR{ "End of file in string or character literal" };
		err_def_n EOFINCMT{ "End of file in comment" };
		err_def_n UNEXPENDCMT{ "End of comment outside comment" };
		err_def_n VOIDEXPR{ "Expression is void" };
		err_def_n ARRAY_NOT_LVALUE{ "Array expression is not an lvalue"};
		err_def_n NAME_NOT_VALUE{ "Name does not denote a value" };
		err_def_n NOT_NUMBER{ "Expression is not a number" };
		err_def_n NOT_POINTER{ "Expression is not a pointer" };
		err_def_n NOT_NUMBER_OR_POINTER{ "Expression is not a number or pointer" };
		err_def_n INCOMPATIBLE{ "Incompatible operand(s)" };
		err_def_n INVALID_FUNCTION_TYPE{ "Invalid function type constructed" };
		err_def_n INVALID_ARRAY_TYPE{ "Invalid array type constructed" };
		err_def_n INVALID_VARIABLE_TYPE{ "Invalid variable type" };
	}

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
		return std::move(r).str();
	}

	void context::message(errors::err_s err, loc_t loc, std::string_view msg)
	{
		errors::err_def_s& e = err;
		out() << "Error (line " << loc << "): " << e[0] << escape(msg) << e[1] << std::endl;
	}

	void context::message(errors::err_n err, loc_t loc)
	{
		errors::err_def_n & e = err;
		out() << "Error (line " << loc << "): " << e[0] << std::endl;
	}

	context* CKContext::get_ctx()
	{
		return static_cast<context*>(this);
	}

	CKStructTypeSafeObs CKContext::declare_struct_type(const CIName& n, loc_t loc)
	{
		if (!!conflicting_tag_struct(n))
		{
			get_ctx()->message(errors::DUPLICATE_TAG, loc, n);
		}
		if (!!loctable_)
		{
			return CKStructTypeSafeObs( loctable_->declare_struct_type(n, module_->getContext(), loc));
		}
		else
		{
			return CKStructTypeSafeObs( globtable_->declare_struct_type(n, module_->getContext(), loc));
		}
	}
	CKStructTypeSafeObs CKContext::define_struct_type_open(const CIName& n, loc_t loc)
	{
		if (!!conflicting_tag_struct(n))
		{
			get_ctx()->message(errors::DUPLICATE_TAG, loc, n);
		}
		CKStructTypeObs tp;
		if (!!loctable_)
		{
			tp = loctable_->declare_struct_type_here(n, module_->getContext(), loc);
		}
		else
		{
			tp = globtable_->declare_struct_type_here(n, module_->getContext(), loc);
		}
		if (tp->is_defined())
		{
			get_ctx()->message(errors::DUPLICATE_STRUCT_DEFINITION, loc, n);
		}
		tp->set_def_loc(loc);
		return CKStructTypeSafeObs( tp);
	}
	CKEnumTypeSafeObs CKContext::declare_enum_type(const CIName& n, loc_t loc)
	{
		if (!!conflicting_tag_enum(n))
		{
			get_ctx()->message(errors::DUPLICATE_TAG, loc, n);
		}
		if (!!loctable_)
		{
			return CKEnumTypeSafeObs(loctable_->declare_enum_type(n, get_int_type(), loc));
		}
		else
		{
			return CKEnumTypeSafeObs(globtable_->declare_enum_type(n, get_int_type(), loc));
		}
	}
	CKEnumTypeSafeObs CKContext::define_enum_type_open(const CIName& n, loc_t loc)
	{
		if (!!conflicting_tag_enum(n))
		{
			get_ctx()->message(errors::DUPLICATE_TAG, loc, n);
		}
		CKEnumTypeObs tp;
		if (!!loctable_)
		{
			tp = loctable_->declare_enum_type_here(n, get_int_type(), loc);
		}
		else
		{
			tp = globtable_->declare_enum_type_here(n, get_int_type(), loc);
		}
		if (tp->is_defined())
		{
			get_ctx()->message(errors::DUPLICATE_ENUM_DEFINITION, loc, n);
		}
		tp->set_def_loc(loc);	
		return CKEnumTypeSafeObs(tp);
	}

	CKTypedefConstObs CKContext::define_typedef(const std::string& name, const CKTypeRefPack& type_pack, loc_t loc)
	{
		if (!!conflicting_idf(name))
		{
			get_ctx()->message(errors::DUPLICATE_IDF, loc, name);
		}
		if (!!loctable_)
		{
			return loctable_->declare_typedef(name, type_pack, loc);
		}
		else
		{
			return globtable_->declare_typedef(name, type_pack, loc);
		}
	}
	CKConstantConstObs CKContext::define_constant(const std::string& name, CKIRConstantIntObs value, loc_t loc)
	{
		if (!!conflicting_idf(name))
		{
			get_ctx()->message(errors::DUPLICATE_IDF, loc, name);
		}
		if (!!loctable_)
		{
			return loctable_->declare_constant(name, get_int_type(), value, loc);
		}
		else
		{
			return globtable_->declare_constant(name, get_int_type(), value, loc);
		}
	}
	void CKContext::define_var(const std::string& name, const CKTypeRefPack& type_pack, loc_t loc)
	{
		if (type_pack.type->is_void() || type_pack.type->is_function())
		{
			get_ctx()->message(errors::INVALID_VARIABLE_TYPE, loc);
		}
		if (!!conflicting_idf(name))
		{
			get_ctx()->message(errors::DUPLICATE_IDF, loc, name);
			return;
		}

		if (!!loctable_)
		{
			loctable_->varDefine(alloca_builder_, name, type_pack, loc);
		}
		else
		{
			globtable_->varDefine(module_, name, type_pack, loc);
		}
	}

	CKFunctionObs CKContext::declare_function(const CIName& n, CKTypeObs type, loc_t loc)
	{
		if (!type->is_function())
		{
			get_ctx()->message(errors::INVALID_FUNCTION_TYPE, loc);
		}
		auto function_type = dynamic_cast<CKFunctionTypeObs>(type);

		if (!!conflicting_idf_function(n, function_type))
		{
			get_ctx()->message(errors::DUPLICATE_IDF, loc, n);
		}

		return globtable_->declare_function(n, module_, function_type, loc);
	}

	void CKContext::enter_function(CKFunctionObs f, CKFunctionFormalPackArray pack, loc_t loc)
	{
		if (f->is_defined())
		{
			get_ctx()->message(errors::DUPLICATE_FUNCTION_DEFINITION, loc, f->get_name());
		}
		assert(!loctable_);
		// FUNCTION PROLOG
		auto BB0 = CKCreateBasicBlock("prolog", f->get_function_ir());
		alloca_builder_.SetInsertPoint(BB0);
		loctable_ = f->define(globtable_, alloca_builder_, std::move(pack));
		start_bb_ = CKCreateBasicBlock("start", f->get_function_ir());
		builder_.SetInsertPoint(start_bb_);
	}

}