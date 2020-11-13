#include "cktables.hpp"
#include <sstream>

namespace cecko {
	// DECLARATION GENERATOR

	CIDecl decl_const(bool is_const)
	{
		return is_const ? "const " : "";
	}

	CIDecl decl_dtor(bool no_space, bool in_suffix, const CIDecl& dtor)
	{
		return in_suffix && !dtor.empty() && dtor[0] == '*'
			? "(" + dtor + ")"
			: (no_space || dtor.empty() || dtor[0] == '*' || dtor[0] == '('
				? dtor
				: " " + dtor);
	}

	std::string generate_dump_name(const CIName& n, loc_t def_loc)
	{
		std::ostringstream oss;
		oss << n;
		oss << "_" << def_loc;
		return std::move(oss).str();
	}

	std::string CINamePtr::get_dump_name() const
	{
		assert(!!name_ptr_);
		return generate_dump_name(*name_ptr_, def_loc_);
	}

	CIDecl CKVoidType::declaration(bool is_const, const CIDecl& dtor) const { return decl_const(is_const) + "void" + decl_dtor(false, false, dtor); }

	CIDecl CKBoolType::declaration(bool is_const, const CIDecl& dtor) const { return decl_const(is_const) + "_Bool" + decl_dtor(false, false, dtor); }

	CIDecl CKCharType::declaration(bool is_const, const CIDecl& dtor) const { return decl_const(is_const) + "char" + decl_dtor(false, false, dtor); }

	CIDecl CKIntType::declaration(bool is_const, const CIDecl& dtor) const { return decl_const(is_const) + "int" + decl_dtor(false, false, dtor); }

	CIDecl CKPtrType::declaration(bool is_const, const CIDecl& dtor) const { return points_to_.type->declaration(points_to_.is_const, "*" + (is_const ? "const" + decl_dtor(false, false, dtor) : decl_dtor(true, false, dtor))); }

	CIDecl CKArrayType::declaration(bool is_const, const CIDecl& dtor) const { return element_type_->declaration(is_const, decl_dtor(true, true, dtor) + "[" + std::to_string(size_->getValue().getZExtValue()) + "]"); }

	CIDecl CKStructType::declaration(bool is_const, const CIDecl& dtor) const { return decl_const(is_const) + "struct " + get_dump_name() + decl_dtor(false, false, dtor); }

	void CKStructType::finalize(const CKStructItemArray& items)
	{
		assert(!defined_);
		CKIRTypeObsArray elements_ir;
		unsigned int idx = 0;
		for (auto&& a : items)
		{
			elements_ir.push_back(a.pack.type->get_ir());
			auto p = elements_.try_emplace(a.name, a.pack, idx, a.loc);
			elements_ordered_.push_back(p);
			++idx;
		}
		irt_->setBody(elements_ir);
		defined_ = true;
	}

	void CKStructType::dump(CIOStream& os, const std::string& indent) const
	{
		if (defined_)
		{
			os << indent << "struct " << get_dump_name() << "{" << CIEndl;
			for (auto&& a : elements_ordered_)
				os << indent << "\t" << a->get_type_pack().type->declaration(a->get_type_pack().is_const, a->get_dump_name()) << ";" << CIEndl;
			os << indent << "};" << CIEndl;
		}
	}

	CIDecl CKEnumType::declaration(bool is_const, const CIDecl& dtor) const { return decl_const(is_const) + "enum " + get_dump_name() + decl_dtor(false, false, dtor); }

	void CKEnumType::finalize(CKConstantObsVector items)
	{
		assert(!defined_);
		elements_ordered_ = std::move(items);
		defined_ = true;
	}

	void CKEnumType::dump(CIOStream& os, const std::string& indent) const
	{
		if (defined_)
		{
			os << indent << "enum " << get_dump_name();
			std::string delim = "{";
			for (auto&& a : elements_ordered_)
			{
				os << delim << CIEndl;
				os << indent << "\t" << a->declaration();
				delim = ",";
			}
			if ( delim == "{" )
				os << delim;
			else
				os << CIEndl << indent;
			os << "};" << CIEndl;
		}
	}

	CKFunctionType::CKFunctionType(CKTypeObs ret_type, CKTypeObsArray arg_types, bool variadic)
		: ret_type_(ret_type), arg_types_(std::move(arg_types)), variadic_(variadic), irt_(nullptr)
	{
		CKIRTypeObsArray arg_ir_types(arg_types_.size());
		std::transform(arg_types_.begin(), arg_types_.end(), arg_ir_types.begin(), [](auto&& a) { return a->get_ir(); });
		irt_ = CKGetFunctionType(ret_type_->get_ir(), std::move(arg_ir_types), variadic_);
	}

	bool CKFunctionType::operator==(const CKFunctionType& b) const
	{
		bool eq = ret_type_ == b.ret_type_
			&& arg_types_.size() == b.arg_types_.size()
			&& variadic_ == b.variadic_;
		for (auto i = std::size_t(0); eq && i < arg_types_.size(); ++i)
			eq = eq && arg_types_[i] == b.arg_types_[i];
		return eq;
	}

	std::size_t CKFunctionType::hash() const
	{
		auto h = ret_type_->hash() ^ compute_hash(arg_types_.size(), variadic_);
		for (auto&& a : arg_types_)
			h ^= a->hash();
		return h;
	}

	CITypeMangle CKFunctionType::mangle() const
	{
		auto m = (variadic_ ? "fv" : "f") + ret_type_->mangle();
		for (auto&& a : arg_types_)
			m += "_" + a->mangle();
		return m;
	}
	CIDecl CKFunctionType::declaration(bool is_const, const CIDecl& dtor) const {
		CIDecl ad = "";
		for (auto&& a : arg_types_)
		{
			if (!ad.empty()) ad += ",";
			ad += a->declaration(false, "");
		}
		if (variadic_)
		{
			if (!ad.empty()) ad += ",";
			ad += "...";
		}
		return ret_type_->declaration(false, decl_dtor(true, true, dtor) + "(" + ad + ")");
	}

	void CKTypeTable::dump(CIOStream& os) const
	{
		/*
		auto dlambda = [&os](auto&& a) {
			os << "typedef " << a->declaration(false, a->mangle()) << ";" << CIEndl;
		};
		strts_.for_each(dlambda);
		dlambda(&boot_);
		dlambda(&chrt_);
		dlambda(&intt_);
		ptrts_.for_each(dlambda);
		arrts_.for_each(dlambda);
		fncts_.for_each(dlambda);
		*/
	}

	std::string CKConstant::declaration() const 
	{ 
		return get_dump_name() + "=" + std::to_string(value_->getValue().getZExtValue());
	}

	CKLocalTableObs CKAbstractScope::get_local()
	{
		return nullptr;
	}

	CKTypedefConstObs CKUniversalTable::declare_typedef(const std::string& name, const CKTypeRefPack& type_pack, loc_t def_loc)
	{
		return typedefs_.try_emplace(name, type_pack, def_loc);
	}
	CKConstantConstObs CKUniversalTable::declare_constant(const std::string& name, CKTypeObs type, CKIRConstantIntObs value, loc_t def_loc)
	{
		return constants_.try_emplace(name, type, value, def_loc);
	}
	CKGlobalVarObs CKGlobalTable::varDefine(CKIRModuleObs M, const std::string& name, const CKTypeRefPack& type_pack, loc_t def_loc)
	{
		auto irtp = type_pack.type->get_ir();
		auto var = CKCreateGlobalVariable(irtp, name, M);
		return vars_.try_emplace(name, type_pack, var, def_loc);
	}
	CKGlobalVarObs CKGlobalTable::declare_extern_variable(CKIRModuleObs M, const std::string& name, const CKTypeRefPack& type_pack)
	{
		auto irtp = type_pack.type->get_ir();
		auto var = CKCreateExternVariable(irtp, name, M);
		return vars_.try_emplace(name, type_pack, var, 0);
	}
	CKFunctionObs CKGlobalTable::declare_function(const CIName& n, CKIRModuleObs M, CKFunctionTypeObs type, loc_t decl_loc)
	{
		return fncs_.try_emplace(n, M, type, n, decl_loc);
	}
	CKFunctionObs CKGlobalTable::declare_function(const CIName& n, CKIRModuleObs M, CKFunctionTypeObs type, const std::string& irname)
	{
		return fncs_.try_emplace(n, M, type, irname, 0);
	}
	CKFunctionObs CKGlobalTable::find_function(const CIName& n)
	{
		auto p = fncs_.find(n);
		return p;
	}
	CKFunctionConstObs CKGlobalTable::find_function(const CIName& n) const
	{
		auto p = fncs_.find(n);
		return p;
	}
	CKTypedefConstObs CKUniversalTable::find_typedef_here(const CIName& n) const
	{
		return typedefs_.find(n);
	}
	CKTypedefConstObs CKGlobalTable::find_typedef(const CIName& n) const
	{
		return find_typedef_here(n);
	}
	CKNamedObs CKUniversalTable::find_constant_here(const CIName& n)
	{
		return constants_.find(n);
	}
	CKNamedObs CKGlobalTable::find(const CIName& n)
	{
		auto pc = find_constant_here(n);
		if (pc)
			return pc;
		auto q = vars_.find(n);
		if (!!q)
			return q;
		auto p = fncs_.find(n);
		return p;
	}

	void CKGlobalTable::dump(CIOStream& os) const
	{
		os << "// --- GLOBAL TYPEDEFS ---" << std::endl;
		dump_universal(os, "");
		os << "// --- FUNCTION DECLARATIONS ---" << std::endl;
		auto decllambda = [&os](auto&& a) {
			os << a->get_type()->declaration(false, a->get_dump_name()) << ";" << CIEndl;
		};
		fncs_.for_each(decllambda);
		os << "// --- GLOBAL VARIABLES ---" << std::endl;
		auto varlambda = [&os](auto&& a) {
			os << a->get_type()->declaration(false, a->get_dump_name()) << ";" << CIEndl;
		};
		vars_.for_each(varlambda);
		os << "// --- FUNCTION DEFINITIONS ---" << std::endl;
		auto deflambda = [&os](auto&& a) {
			if (a->is_defined())
			{
				a->dump(os);
			}
		};
		fncs_.for_each(deflambda);
	}

	void CKLocalTable::varsFromArgs(CKIRBuilderRef builder, CKFunctionObs f, const CKFunctionFormalPackArray& formal_packs)
	{
		auto f_type = f->get_function_type();
		auto f_ir = f->get_function_ir();
		auto n = f_type->get_function_arg_count();
		assert(n == formal_packs.size());
		assert(n == f_ir->getFunctionType()->getNumParams());
		for (std::size_t ix = 0; ix < n; ++ix)
		{
			auto&& arg_pack = formal_packs[ix];
			if (!!arg_pack.name)	// unnamed arguments are not accessible inside the function
			{
				auto arg_type = f_type->get_function_arg_type(ix);
				auto arg_ir = f_ir->args().begin() + ix;
				auto var = builder.CreateAlloca(arg_type->get_ir(), nullptr, *arg_pack.name);
				builder.CreateStore(arg_ir, var);
				vars_.try_emplace(*arg_pack.name, CKTypeRefPack{ arg_type, arg_pack.is_const }, var, arg_pack.loc, true);
			}
		}
	}

	CKLocalVarObs CKLocalTable::varDefine(CKIRBuilderRef builder, const std::string& name, const CKTypeRefPack& type_pack, loc_t def_loc)
	{
		auto var = builder.CreateAlloca(type_pack.type->get_ir(), nullptr, name);
		return vars_.try_emplace(name, type_pack, var, def_loc, false);
	}

	CKStructTypeObs CKLocalTable::declare_struct_type(const CIName& n, CKIRContextRef Context, loc_t decl_loc)
	{
		auto p = find_struct_type(n);
		if (p)
			return p;
		return declare_struct_type_here(n, Context, decl_loc);
	}
	CKStructTypeObs CKLocalTable::find_struct_type(const CIName& n) 
	{ 
		auto p = find_struct_type_here(n);
		if (p)
			return p;
		return parent_scope_->find_struct_type(n);
	}
	CKEnumTypeObs CKLocalTable::declare_enum_type(const CIName& n, CKTypeObs base_type, loc_t decl_loc)
	{
		auto p = find_enum_type(n);
		if (p)
			return p;
		return declare_enum_type_here(n, base_type, decl_loc);
	}
	CKEnumTypeObs CKLocalTable::find_enum_type(const CIName& n) 
	{ 
		auto p = find_enum_type_here(n); 
		if (p)
			return p;
		return parent_scope_->find_enum_type(n);
	}

	CKTypedefConstObs CKLocalTable::find_typedef(const CIName& n) const
	{
		auto p = find_typedef_here(n);
		if (p)
			return p;
		return parent_scope_->find_typedef(n);
	}
	CKNamedObs CKLocalTable::find(const CIName& n)
	{
		auto pc = find_constant_here(n);
		if (pc)
			return pc;
		auto pv = vars_.find(n);
		if (pv)
			return pv;
		return parent_scope_->find(n);
	}
	CKLocalTableObs CKLocalTable::get_local()
	{
		return this;
	}
	CKLocalTableObs CKLocalTable::create_block()
	{
		auto b = std::make_unique< CKLocalTable>( this);
		auto bo = &*b;
		block_scopes_.push_back(std::move(b));
		return bo;
	}
	CKLocalTableObs CKLocalTable::parent_block() const
	{
		return parent_scope_->get_local();
	}

	void CKUniversalTable::dump_universal(CIOStream& os, const std::string& indent) const
	{
		//os << "// --- ENUMS ---" << std::endl;
		enmts_.for_each([&os, &indent](auto&& a) {
			a->dump(os, indent);
			});
		//os << "// --- STRUCTS ---" << std::endl;
		strts_.for_each([&os, &indent](auto&& a) {
			a->dump(os, indent);
			});
		auto typedeflambda = [&os, &indent](auto&& a) {
			os << indent << "typedef " << a->get_type_pack().type->declaration(a->get_type_pack().is_const, a->get_dump_name()) << ";" << CIEndl;
		};
		typedefs_.for_each(typedeflambda);
	}

	void CKLocalTable::dump(CIOStream& os, const std::string& indent) const
	{
		dump_universal(os, indent);
		auto dlambda = [&os, &indent](auto&& a) {
			if (!a->is_arg())
			{
				a->dump(os, indent);
			}
		};
		vars_.for_each(dlambda);
		auto indent2 = indent + "\t";
		for (auto&& a : block_scopes_)
		{
			os << indent << "{" << std::endl;
			a->dump(os, indent2);
			os << indent << "}" << std::endl;
		}
	}

	CKLocalTableObs CKFunction::define(CKAbstractScopeObs parent, CKIRBuilderRef builder, CKFunctionFormalPackArray formal_packs)
	{
		assert(!loctab_);
		loctab_ = std::make_unique<CKLocalTable>(parent);
		formal_packs_ = std::move(formal_packs);
		loctab_->varsFromArgs(builder, this, formal_packs_);
		return &*loctab_;
	}

	void CKFunction::dump(CIOStream& os) const
	{
		auto f_type = get_function_type();
		std::string args;
		{
			auto n = f_type->get_function_arg_count();
			for (std::size_t ix = 0; ix < n; ++ix)
			{
				auto arg_type = f_type->get_function_arg_type(ix);
				auto&& arg_pack = get_formal_pack(ix);
				if (!args.empty())
					args += ",";
				args += arg_type->declaration(arg_pack.is_const, !!arg_pack.name ? generate_dump_name(*arg_pack.name, arg_pack.loc) : std::string{});
			}
			if (args.empty())
				args += "void";
		}
		os << f_type->get_function_return_type()->declaration(false, get_dump_name() + "(" + args + ")") << "{" << CIEndl;

		loctab_->dump(os, "\t");

		os << "}" << CIEndl;
	}

	void CKTypedef::dump(CIOStream& os) const
	{
		os << "\ttypedef " << get_type_pack().type->declaration(get_type_pack().is_const, get_dump_name()) << ";" << CIEndl;
	}

	void CKVar::dump(CIOStream& os, const std::string& indent) const
	{
		os << indent << get_type()->declaration(is_const(), get_dump_name()) << ";" << CIEndl;
	}

	// CONTEXT

	CKTables::CKTables(CKIREnvironmentObs irenv)
		: irenv_(irenv),
		typetable_(irenv->context()),
		globtable_(),
		module_(irenv->module()),
		data_layout_(irenv->data_layout())
	{
		declare_library();
	}

	void CKTables::dump_tables(std::ostream& os) const
	{
		typetable_.dump(os);
		globtable_.dump(os);
	}

	void CKTables::dump_ir_module(std::ostream& os) const
	{
		irenv_->dump_module(os, module_);
	}

	std::error_code CKTables::write_bitcode_module(const std::string& fname) const
	{
		return irenv_->write_bitcode_module(fname, module_);
	}

	CKContext::CKContext(CKTablesObs tab)
		: typetable_(tab->typetable()),
		globtable_(tab->globtable()),
		loctable_(nullptr),
		module_(tab->module()),
		alloca_builder_(tab->module()->getContext()),
		builder_(tab->module()->getContext()),
		start_bb_(nullptr),
		data_layout_(tab->data_layout())
	{
	}

	void CKContext::enter_function(CKFunctionObs f, CKFunctionFormalPackArray pack, loc_t loc)
	{
		assert(!loctable_);
		// FUNCTION PROLOG
		auto BB0 = CKCreateBasicBlock("prolog", f->get_function_ir());
		alloca_builder_.SetInsertPoint(BB0);
		loctable_ = f->define(globtable_, alloca_builder_, std::move(pack));
		start_bb_ = CKCreateBasicBlock("start", f->get_function_ir());
		builder_.SetInsertPoint(start_bb_);
	}

	void CKContext::exit_function()
	{
		assert(loctable_);
		loctable_ = nullptr;
		builder_.ClearInsertionPoint();
		alloca_builder_.CreateBr(start_bb_);
		start_bb_ = nullptr;
		alloca_builder_.ClearInsertionPoint();
	}

	void CKContext::enter_block()
	{
		assert(loctable_);
		loctable_ = loctable_->create_block();
	}

	void CKContext::exit_block()
	{
		assert(loctable_);
		loctable_ = loctable_->parent_block();
		assert(loctable_);
	}

	CKStructTypeObs CKContext::declare_struct_type(const CIName& n, loc_t loc)
	{
		if (!!loctable_)
		{
			return loctable_->declare_struct_type(n, module_->getContext(), loc);
		}
		else
		{
			return globtable_->declare_struct_type(n, module_->getContext(), loc);
		}
	}
	CKStructTypeObs CKContext::define_struct_type_open(const CIName& n, loc_t loc)
	{ 
		CKStructTypeObs tp;
		if (!!loctable_)
		{
			tp = loctable_->declare_struct_type_here(n, module_->getContext(), loc);
		}
		else
		{
			tp = globtable_->declare_struct_type_here(n, module_->getContext(), loc);
		}
		tp->set_def_loc(loc);	// !!! DUPLICITE DEFINITION ???
		return tp;
	}
	void CKContext::define_struct_type_close(CKStructTypeObs type, const CKStructItemArray& items)
	{
		type->finalize(items);
	}

	CKEnumTypeObs CKContext::declare_enum_type(const CIName& n, loc_t loc)
	{
		if (!!loctable_)
		{
			return loctable_->declare_enum_type(n, get_int_type(), loc);
		}
		else
		{
			return globtable_->declare_enum_type(n, get_int_type(), loc);
		}
	}
	CKEnumTypeObs CKContext::define_enum_type_open(const CIName& n, loc_t loc)
	{ 
		CKEnumTypeObs tp;
		if (!!loctable_)
		{
			tp = loctable_->declare_enum_type_here(n, get_int_type(), loc);
		}
		else
		{
			tp = globtable_->declare_enum_type_here(n, get_int_type(), loc);
		}
		tp->set_def_loc(loc);	// !!! DUPLICITE DEFINITION ???
		return tp;
	}
	void CKContext::define_enum_type_close(CKEnumTypeObs type, CKConstantObsVector items)
	{
		type->finalize(std::move(items));
	}

	CKTypedefConstObs CKContext::define_typedef(const std::string& name, const CKTypeRefPack& type_pack, loc_t loc)
	{
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
		if (!!loctable_)
		{
			loctable_->varDefine(alloca_builder_, name, type_pack, loc);
		}
		else
		{
			globtable_->varDefine(module_, name, type_pack, loc);
		}
	}
	CKNamedObs CKContext::find(const CIName& n)
	{
		if (!!loctable_)
		{
			return loctable_->find(n);
		}
		else
		{
			return globtable_->find(n);
		}
	}

	CKTypedefConstObs CKContext::find_typedef(const CIName& n) const
	{
		if (!!loctable_)
		{
			return loctable_->find_typedef(n);
		}
		else
		{
			return globtable_->find_typedef(n);
		}
	}

	bool CKContext::is_typedef(const CIName& n) const
	{
		if (!!loctable_)
		{
			return !! loctable_->find_typedef(n);
		}
		else
		{
			return !! globtable_->find_typedef(n);
		}
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

		auto t_file_s = globtable_.declare_struct_type("_file_s", module_->getContext(), 0);

		globtable_.declare_typedef("FILE", { t_file_s, false }, 0);

		globtable_.declare_function("printf", module_, typetable_.get_function_type(t_int, { t_cptr_char }, true), "ckrt_printf");
		globtable_.declare_function("scanf", module_, typetable_.get_function_type(t_int, { t_cptr_char }, true), "ckrt_scanf");
		globtable_.declare_function("sprintf", module_, typetable_.get_function_type(t_int, { t_ptr_char, t_cptr_char }, true), "ckrt_sprintf");
		globtable_.declare_function("sscanf", module_, typetable_.get_function_type(t_int, { t_cptr_char, t_cptr_char }, true), "ckrt_sscanf");
		globtable_.declare_function("memset", module_, typetable_.get_function_type(t_ptr_void, { t_ptr_void, t_int, t_int }, false), "ckrt_memset");
	}
}
