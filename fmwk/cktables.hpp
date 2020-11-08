/** @file

cktables.hpp

Semantic tables.

*/

#ifndef cktables_hpp_
#define cktables_hpp_

#include "ckir.hpp"

#include <memory>
#include <functional>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <optional>

namespace cecko {
	class CIAbstractType;

	using CKTypeObs = const CIAbstractType*;	// use something smarter for safety

	struct CKTypeRefPack;

	/// @cond INTERNAL
	template< typename ... TL>
	inline std::size_t compute_hash(TL&& ... l)
	{
		return (0 ^ ... ^ std::hash<std::remove_cv_t<std::remove_reference_t<TL>>>{}(std::forward<TL>(l)));
	}

	inline bool compute_equal()
	{
		return true;
	}

	template< typename TA, typename TB, typename ... TL>
	inline bool compute_equal(TA&& a, TB&& b, TL&& ... l)
	{
		return a == b && compute_equal(std::forward<TL>(l)...);
	}
	/// @endcond
}

/// @cond INTERNAL
template<>
class std::hash<cecko::CKIRAPInt> {
public:
	std::size_t operator()(const cecko::CKIRAPInt& a) const
	{
		return cecko::CKHashValue(a);
	}
};

template<>
class std::hash<cecko::CKTypeObs> {
public:
	std::size_t operator()(const cecko::CKTypeObs& a) const;
};

template<>
class std::hash<cecko::CKTypeRefPack> {
public:
	std::size_t operator()(const cecko::CKTypeRefPack& a) const;
};
///  @endcond

namespace cecko {
	// DUMP OSTREAM

	/// @cond INTERNAL
	using CIOStream = std::ostream;

	inline CIOStream& (*CIEndl)(CIOStream&) = &std::endl;

	// IMMOVABLE CLASS HELPER

	class CIImmovable {
	public:
		// COPYING AND MOVING DENIED
		CIImmovable() = default;
		CIImmovable(const CIImmovable&) = delete;
		CIImmovable(CIImmovable&&) = delete;
		CIImmovable& operator=(const CIImmovable&) = delete;
		CIImmovable& operator=(CIImmovable&&) = delete;
	};

	// HASHED STORAGE

	template< typename T>
	class CIHashedStorage : CIImmovable {
	public:
		template< typename ... TL>
		const T* emplace(TL&& ... l)
		{
			auto rv = data_.emplace(std::forward<TL>(l)...);
			return &*rv.first;
		}

		template<typename F>
		void for_each(F&& f) const
		{
			for (auto&& a : data_)
			{
				f(&a);
			}
		}
	private:
		struct hasher_t {
			std::size_t operator()(const T& a) const { return a.hash(); }
		};
		/*
			struct comparator_t {
				bool operator()(const T& a, const T& b) const { return a == b; }
			};
		*/
		using container_type_ = std::unordered_set< T, hasher_t>;
		container_type_ data_;
	};

	// NAMED STORAGE

	using CIName = std::string;

	template< typename T>
	class CINamedStorage : CIImmovable {
	public:
		template< typename ... TL>
		T* try_emplace(const CIName& n, TL&& ... l)
		{
			auto rv = data_.try_emplace(n, std::forward<TL>(l)...);
			rv.first->second.set_name_ptr(&rv.first->first);
			return &rv.first->second;
		}

		T* find(const CIName& n)
		{
			auto it = data_.find(n);
			return it == data_.end() ? nullptr : &it->second;
		}

		const T* find(const CIName& n) const
		{
			auto it = data_.find(n);
			return it == data_.end() ? nullptr : &it->second;
		}

		template<typename F>
		void for_each(F&& f) const
		{
			using container_pair_obs = const std::pair< const CIName, T>*;
			auto pair_less = [](auto a, auto b) { 
				return a->first < b->first; 
			};
			using ordering_set = std::vector< container_pair_obs>;
			ordering_set os(data_.size(), nullptr);
			std::transform(data_.begin(), data_.end(), os.begin(),
				[](auto&& a) { 
				return &a; 
				});
			std::sort(os.begin(), os.end(), pair_less);
			for (auto&& p : os)
			{
				f(&p->second);
			}
		}
	private:
		using container_type_ = std::unordered_map< CIName, T>;
		container_type_ data_;
	};
	/// @endcond

	class CINamePtr {
	public:
		/// @cond INTERNAL
		CINamePtr()
			: name_ptr_(nullptr)
		{}
		/// @endcond
		
		/// The name of the object
		const CIName& get_name() const	
		{
			assert(!!name_ptr_);
			return *name_ptr_;
		}
	private:
		void set_name_ptr(const CIName* p)
		{
			name_ptr_ = p;
		}

		const CIName* name_ptr_;

		template< typename T>
		friend class CINamedStorage;
	};

	// MANGLER

	/// @cond INTERNAL
	using CITypeMangle = std::string;

	using CIDecl = std::string;
	/// @endcond
	
	// TYPES

	class CKStructElement;

	using CKStructElementObs = const CKStructElement*;

	struct CKTypeRefPack {

		CKTypeRefPack()
			: type(nullptr), is_const(false)
		{}
		CKTypeRefPack(CKTypeObs t, bool is_c)
			: type(t), is_const(is_c)
		{}
		CKTypeObs type;		///< The type
		bool is_const;		///< Indicates the presence of the *const* flag
	};

	class CIAbstractType : CIImmovable {
	public:
		/// @cond INTERNAL
		virtual ~CIAbstractType() {}
		virtual std::size_t hash() const = 0;
		/// @endcond
		
		virtual CKIRTypeObs get_ir() const = 0;

		/// @name Determining the kind of the type
		/// @{
		
		virtual bool is_void() const { return false; }
		virtual bool is_bool() const { return false; }
		virtual bool is_char() const { return false; }
		virtual bool is_int() const { return false; }
		virtual bool is_enum() const { return false; }
		virtual bool is_array() const { return false; }
		virtual bool is_function() const { return false; }
		virtual bool is_pointer() const { return false; }
		virtual bool is_struct() const { return false; }
		/// @}

		/// @name Pointer properties
		/// @{
		virtual const CKTypeRefPack& get_pointer_points_to() const { assert(0); static CKTypeRefPack nullpack{ nullptr, false }; return nullpack; }
		/// @}

		/// @name Function properties
		/// @{
		virtual CKTypeObs get_function_return_type() const { assert(0); return nullptr; }
		/// @}

		/// @name Array properties
		/// @{
		virtual CKTypeObs get_array_element_type() const { assert(0); return nullptr; }
		virtual CKIRConstantIntObs get_array_size() const { assert(0); return nullptr; }
		/// @}

		/// @name Struct properties
		/// @{
		virtual CKStructElementObs find_struct_element(const CIName&) const { assert(0); return nullptr; }
		/// @}

		/// @cond INTERNAL
		virtual CITypeMangle mangle() const = 0;
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor = "") const = 0;
		/// @endcond
	};

	/// @cond INTERNAL
	inline bool operator==(const CKTypeRefPack& a, const CKTypeRefPack& b)
	{
		return compute_equal(a.type, b.type, a.is_const, b.is_const);
	}
	/// @endcond

	class CKVoidType : public CIAbstractType {
	public:
		/// @cond INTERNAL
		CKVoidType(CKIRContextRef Context)
			: irt_(CKGetVoidType(Context))
		{}
		virtual std::size_t hash() const { return 0; }
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return irt_; }
		virtual bool is_void() const { return true; }
		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return "V"; }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
	private:
		CKIRTypeObs irt_;
	};

	class CKBoolType : public CIAbstractType {
	public:
		/// @cond INTERNAL
		CKBoolType(CKIRContextRef Context)
			: irt_(CKGetInt1Type(Context))
		{}
		virtual std::size_t hash() const { return 1; }
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return irt_; }
		virtual bool is_bool() const { return true; }
		virtual bool is_signed() const { return false; }
		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return "B"; }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
	private:
		CKIRTypeObs irt_;
	};

	class CKCharType : public CIAbstractType {
	public:
		/// @cond INTERNAL
		CKCharType(CKIRContextRef Context)
			: irt_(CKGetInt8Type(Context))
		{}
		virtual std::size_t hash() const { return 8; }
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return irt_; }
		virtual bool is_char() const { return true; }
		virtual bool is_signed() const { return false; }
		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return "X"; }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
	private:
		CKIRTypeObs irt_;
	};

	class CKIntType : public CIAbstractType {
	public:
		/// @cond INTERNAL
		CKIntType(CKIRContextRef Context)
			: irt_(CKGetInt32Type(Context))
		{}
		virtual std::size_t hash() const { return 32; }
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return irt_; }
		virtual bool is_int() const { return true; }
		virtual bool is_signed() const { return true; }
		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return "I"; }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
	private:
		CKIRTypeObs irt_;
	};

	class CKPtrType : public CIAbstractType {
	public:
		/// @cond INTERNAL
		explicit CKPtrType(const CKTypeRefPack& points_to)
			: points_to_(points_to), irt_(CKGetPtrType(points_to.type->get_ir()))
		{}

		virtual std::size_t hash() const { return compute_hash(points_to_); }
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return irt_; }
		/// @cond INTERNAL
		bool operator==(const CKPtrType& b) const
		{
			return compute_equal(points_to_, b.points_to_);
		}
		/// @endcond

		virtual bool is_pointer() const { return true; }
		virtual const CKTypeRefPack& get_pointer_points_to() const { return points_to_; }

		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return (points_to_.is_const ? "pc" : "p") + points_to_.type->mangle(); }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
	private:
		CKTypeRefPack points_to_;
		CKIRTypeObs irt_;
	};

	class CKArrayType : public CIAbstractType {
	public:
		/// @cond INTERNAL
		CKArrayType(CKTypeObs element_type, CKIRConstantIntObs size)
			: element_type_(element_type), size_(size), irt_(CKGetArrayType(element_type->get_ir(), size))
		{}

		virtual std::size_t hash() const { return compute_hash(element_type_, size_->getValue()); }
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return irt_; }
		/// @cond INTERNAL
		bool operator==(const CKArrayType& b) const
		{
			return compute_equal(element_type_, b.element_type_, size_->getValue(), b.size_->getValue());
		}
		/// @endcond

		virtual bool is_array() const { return true; }
		virtual CKTypeObs get_array_element_type() const { return element_type_; }
		virtual CKIRConstantIntObs get_array_size() const { return size_; }

		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return "a" + std::to_string(size_->getValue().getZExtValue()) + element_type_->mangle(); }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
	private:
		CKTypeObs element_type_;
		CKIRConstantIntObs size_;
		CKIRTypeObs irt_;
	};

	class CKStructElement : public CINamePtr, CIImmovable {
	public:
		/// @cond INTERNAL
		explicit CKStructElement(const CKTypeRefPack& pack, unsigned int idx)
			: pack_(pack), idx_(idx)
		{}
		/// @endcond
		const CKTypeRefPack& get_type_pack() const { return pack_; }
		unsigned int get_idx() const { return idx_; }
	private:
		CKTypeRefPack pack_;
		unsigned int idx_;
	};

	using CKStructElementObs = const CKStructElement*;

	struct CKStructItem {
		CKStructItem(const CKTypeRefPack& p, CIName nm)
			: pack(p), name(std::move(nm))
		{}
		CKTypeRefPack pack;
		CIName name;
	};

	using CKStructItemArray = std::vector< CKStructItem>;

	class CKStructType : public CIAbstractType, public CINamePtr {
	public:
		/// @cond INTERNAL
		CKStructType(CKIRContextRef Context, const CIName& n)
			: defined_(false), irt_(CKCreateStructType(Context, n))
		{}
		/// @endcond
		void finalize(const CKStructItemArray& items);

		/// @cond INTERNAL
		virtual std::size_t hash() const { return std::hash<CIName>{}(get_name()); }
		virtual CKIRTypeObs get_ir() const { return irt_; }
		virtual bool is_struct() const { return true; }
		virtual bool is_defined() const { return defined_; }
		virtual CKStructElementObs find_struct_element(const CIName& n) const
		{
			assert(defined_);
			return elements_.find(n);
		}

		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return "S" + get_name() + '$'; }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		void dump(CIOStream& os, const std::string& indent) const;
		/// @endcond
	private:
		using element_storage_type = CINamedStorage< CKStructElement>;
		bool defined_;
		element_storage_type elements_;
		std::vector< CKStructElementObs> elements_ordered_;
		CKIRStructTypeObs irt_;
	};

	class CKConstant;

	using CKConstantConstObs = const CKConstant*;

	using CKConstantObsVector = std::vector<CKConstantConstObs>;

	class CKEnumType : public CIAbstractType, public CINamePtr {
	public:
		/// @cond INTERNAL
		CKEnumType(CKTypeObs base_type)
			: defined_(false), base_type_(base_type)
		{}
		/// @endcond
		void finalize(CKConstantObsVector items);

		/// @cond INTERNAL
		virtual std::size_t hash() const { return std::hash<CIName>{}(get_name()); }
		/// @endcond
		virtual bool is_enum() const { return true; }
		virtual bool is_defined() const { return defined_; }

		/// @cond INTERNAL
		virtual CITypeMangle mangle() const { return "E" + get_name() + '$'; }
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return base_type_->get_ir(); }
		/// @cond INTERNAL
		void dump(CIOStream& os, const std::string& indent) const;
		/// @endcond
	private:
		bool defined_;
		CKTypeObs base_type_;
		CKConstantObsVector elements_ordered_;
	};

	using CKTypeObsArray = std::vector<CKTypeObs>;

	class CKFunctionType : public CIAbstractType {
	public:
		/// @cond INTERNAL
		CKFunctionType(CKTypeObs ret_type, CKTypeObsArray arg_types, bool variadic = false);

		bool operator==(const CKFunctionType& b) const;

		virtual std::size_t hash() const;
		/// @endcond
		virtual CKIRTypeObs get_ir() const { return irt_; }
		virtual CKIRFunctionTypeObs get_function_ir() const { return irt_; }
		virtual bool is_function() const { return true; }
		virtual CKTypeObs get_function_return_type() const { return ret_type_; }
		virtual CKTypeObs get_function_arg_type(std::size_t ix) const { return arg_types_[ix]; }
		virtual std::size_t get_function_arg_count() const { return arg_types_.size(); }

		/// @cond INTERNAL
		virtual CITypeMangle mangle() const;
		virtual CIDecl declaration(bool is_const, const CIDecl& dtor) const;
		/// @endcond
	private:
		CKTypeObs ret_type_;
		CKTypeObsArray arg_types_;
		bool variadic_;
		CKIRFunctionTypeObs irt_;
	};

	using CKVoidTypeObs = const CKVoidType*;
	using CKBoolTypeObs = const CKBoolType*;
	using CKCharTypeObs = const CKCharType*;
	using CKIntTypeObs = const CKIntType*;
	using CKPtrTypeObs = const CKPtrType*;
	using CKArrayTypeObs = const CKArrayType*;
	using CKFunctionTypeObs = const CKFunctionType*;
	using CKStructTypeObs = CKStructType*;
	using CKEnumTypeObs = CKEnumType*;

	/// @cond INTERNAL
	class CKTypeTable : CIImmovable {
	public:
		CKTypeTable(CKIRContextRef Context)
			: voit_(Context), boot_(Context), chrt_(Context), intt_(Context)
		{}
		CKVoidTypeObs get_void_type() const { return &voit_; }
		CKBoolTypeObs get_bool_type() const { return &boot_; }
		CKCharTypeObs get_char_type() const { return &chrt_; }
		CKIntTypeObs get_int_type() const { return &intt_; }
		CKPtrTypeObs get_pointer_type(const CKTypeRefPack& pack) { return ptrts_.emplace(pack); }
		CKArrayTypeObs get_array_type(CKTypeObs element_type, CKIRConstantIntObs size) { return arrts_.emplace(element_type, size); }
		CKFunctionTypeObs get_function_type(CKTypeObs ret_type, CKTypeObsArray arg_types, bool variadic = false) { return fncts_.emplace(ret_type, std::move(arg_types), variadic); }
		void dump(CIOStream& os) const;
	private:
		CKVoidType voit_;
		CKBoolType boot_;
		CKCharType chrt_;
		CKIntType intt_;
		CIHashedStorage< CKPtrType> ptrts_;
		CIHashedStorage< CKArrayType> arrts_;
		CIHashedStorage< CKFunctionType> fncts_;
	};
	/// @endcond

	class CKAbstractNamed : public CINamePtr, CIImmovable {
	public:
		/// @cond INTERNAL
		virtual ~CKAbstractNamed() {}
		/// @endcond
		/// @name Determining the kind of the object 
		/// @{
		//virtual bool is_typedef() const { return false; }
		virtual bool is_var() const { return false; }
		virtual bool is_function() const { return false; }
		virtual bool is_constant() const { return false; }
		/// @}
		
		/// @name Object type
		/// @{
		virtual CKTypeObs get_type() const = 0;
		virtual bool is_const() const { return false; }
		/// @}

		/// @name Object representation in LLVM IR
		/// @{
		virtual CKIRValueObs get_ir() const { return nullptr; };
		virtual CKIRFunctionObs get_function_ir() const { return nullptr; };
		/// @}
	};

	using CKNamedObs = CKAbstractNamed*;

	class CKTypedef : public CINamePtr, CIImmovable {
	public:
		/// @cond INTERNAL
		CKTypedef(CKTypeRefPack type_pack)
			: type_pack_(type_pack)
		{}
		/// @endcond
		//virtual bool is_typedef() const { return true; }
		//CKTypeObs get_type() const { return type_pack_.type; }
		//bool is_const() const { return type_pack_.is_const; }
		const CKTypeRefPack& get_type_pack() const { return type_pack_; }
		/// @cond INTERNAL
		void dump(CIOStream& os) const;
		/// @endcond
	private:
		CKTypeRefPack type_pack_;
	};

	using CKTypedefConstObs = const CKTypedef*;

	class CKConstant : public CKAbstractNamed {
	public:
		/// @cond INTERNAL
		CKConstant(CKTypeObs type, CKIRConstantIntObs value)
			: type_pack_(type, true), value_(value)
		{}
		/// @endcond
		virtual bool is_constant() const { return true; }
		virtual CKTypeObs get_type() const { return type_pack_.type; }
		virtual bool is_const() const { return type_pack_.is_const; }
		//const CKTypeRefPack& get_type_pack() const { return type_pack_; }
		CKIRConstantIntObs get_constant_value() const { return value_; }
		virtual CKIRValueObs get_ir() const { return value_; }
		/// @cond INTERNAL
		std::string declaration() const;
		/// @endcond
	private:
		CKTypeRefPack type_pack_;
		CKIRConstantIntObs value_;
	};

	class CKVar : public CKAbstractNamed {
	public:
		/// @cond INTERNAL
		CKVar(CKTypeRefPack type_pack)
			: type_pack_(type_pack)
		{}
		/// @endcond
		virtual bool is_var() const { return true; }
		virtual CKTypeObs get_type() const { return type_pack_.type; }
		virtual bool is_const() const { return type_pack_.is_const; }
		//const CKTypeRefPack& get_type_pack() const { return type_pack_; }
		/// @cond INTERNAL
		void dump(CIOStream& os, const std::string& indent) const;
		/// @endcond
	private:
		CKTypeRefPack type_pack_;
	};

	using CKVarObs = CKVar*;

	class CKGlobalVar : public CKVar {
	public:
		/// @cond INTERNAL
		CKGlobalVar(CKTypeRefPack type_pack, CKIRConstantObs address)
			: CKVar(type_pack), address_(address)
		{}
		/// @endcond
		virtual CKIRValueObs get_ir() const { return address_; }
		CKIRConstantObs get_address() const { return address_; }
	private:
		CKIRConstantObs address_;
	};

	using CKGlobalVarObs = CKGlobalVar*;

	using CINameOpt = std::optional< CIName>;

	struct CKFunctionFormalPack {
		CKFunctionFormalPack(CINameOpt nm, bool c)
			: name(std::move(nm)), is_const(c)
		{}
		CINameOpt name;
		bool is_const;
	};

	class CKLocalTable;

	using CKLocalTableOwner = std::unique_ptr< CKLocalTable>;
	using CKLocalTableObs = CKLocalTable*;

	/// @cond INTERNAL
	class CKAbstractScope : CIImmovable {
	public:
		virtual ~CKAbstractScope() {}
		virtual CKTypedefConstObs find_typedef(const CIName& n) const = 0;
		virtual CKNamedObs find(const CIName& n) = 0;
		virtual CKLocalTableObs get_local();
	};

	using CKAbstractScopeObs = CKAbstractScope*;
	/// @endcond

	using CKFunctionFormalPackArray = std::vector< CKFunctionFormalPack>;

	class CKFunction : public CKAbstractNamed {
	public:
		/// @cond INTERNAL
		CKFunction(CKIRModuleObs M, CKFunctionTypeObs type, const CIName& irname)
			: type_(type), irf_(CKCreateFunction(type->get_function_ir(), irname, M))
		{}
		/// @endcond
		virtual bool is_function() const { return true; }
		bool is_defined() const { return !!loctab_; }
		virtual CKTypeObs get_type() const { return type_; }
		CKFunctionTypeObs get_function_type() const { return type_; }
		virtual CKIRValueObs get_ir() const { return irf_; }
		virtual CKIRFunctionObs get_function_ir() const { return irf_; }
		const CKFunctionFormalPack& get_formal_pack(std::size_t ix) const { return formal_packs_[ix]; }
		/// @cond INTERNAL
		CKLocalTableObs define(CKAbstractScopeObs parent, CKIRBuilderRef builder, CKFunctionFormalPackArray formal_packs);
		void dump(CIOStream& os) const;
		/// @endcond
	private:
		CKFunctionTypeObs type_;
		CKIRFunctionObs irf_;
		CKFunctionFormalPackArray formal_packs_;
		CKLocalTableOwner loctab_;
	};

	/// @cond INTERNAL
	class CKUniversalTable : public CKAbstractScope {
	public:
		CKStructTypeObs declare_struct_type(const CIName& n, CKIRContextRef Context)
		{
			return strts_.try_emplace(n, Context, n);
		}
		CKStructTypeObs find_struct_type(const CIName& n) { return strts_.find(n); }
		CKEnumTypeObs declare_enum_type(const CIName& n, CKTypeObs base_type)
		{
			return enmts_.try_emplace(n, base_type);
		}
		CKEnumTypeObs find_enum_type(const CIName& n) { return enmts_.find(n); }
		CKTypedefConstObs declare_typedef(const CIName& name, const CKTypeRefPack& type_pack);
		CKConstantConstObs declare_constant(const std::string& name, CKTypeObs type, CKIRConstantIntObs value);
		CKTypedefConstObs find_typedef_here(const CIName& n) const;
		CKNamedObs find_constant_here(const CIName& n);
		void dump_universal(CIOStream& os, const std::string& indent) const;
	private:
		CINamedStorage< CKStructType> strts_;
		CINamedStorage< CKEnumType> enmts_;
		CINamedStorage< CKTypedef> typedefs_;
		CINamedStorage< CKConstant> constants_;
	};
	/// @endcond

	using CKFunctionObs = CKFunction*;
	using CKFunctionConstObs = const CKFunction*;

	/// @cond INTERNAL
	class CKGlobalTable : public CKUniversalTable {
	public:
		CKGlobalTable()
		{}
		CKGlobalVarObs varDefine(CKIRModuleObs M, const std::string& name, const CKTypeRefPack& type_pack);
		CKGlobalVarObs declare_extern_variable(CKIRModuleObs M, const std::string& name, const CKTypeRefPack& type_pack);
		CKFunctionObs declare_function(const CIName& n, CKIRModuleObs M, CKFunctionTypeObs type);
		CKFunctionObs declare_function(const CIName& n, CKIRModuleObs M, CKFunctionTypeObs type, const std::string& irname);
		CKFunctionObs find_function(const CIName& n);
		CKFunctionConstObs find_function(const CIName& n) const;
		virtual CKTypedefConstObs find_typedef(const CIName& n) const;
		virtual CKNamedObs find(const CIName& n);

		void dump(CIOStream& os) const;
	private:
		CINamedStorage< CKFunction> fncs_;
		CINamedStorage< CKGlobalVar> vars_;
	};
	/// @endcond

	class CKLocalVar : public CKVar {
	public:
		/// @cond INTERNAL
		CKLocalVar(CKTypeRefPack type_pack, CKIRAllocaInstObs address, bool is_arg = false)
			: CKVar(type_pack), address_(address), is_arg_(is_arg)
		{}
		/// @endcond
		virtual CKIRValueObs get_ir() const { return address_; }
		CKIRAllocaInstObs get_address() const { return address_; }
		bool is_arg() const { return is_arg_; }
	private:
		CKIRAllocaInstObs address_;
		bool is_arg_;
	};

	using CKLocalVarObs = CKLocalVar*;

	/// @cond INTERNAL
	class CKLocalTable : public CKUniversalTable {
	public:
		CKLocalTable(CKAbstractScopeObs parent)
			: parent_scope_(parent)
		{}

		void varsFromArgs(CKIRBuilderRef builder, CKFunctionObs f, const CKFunctionFormalPackArray& formal_packs);

		CKLocalVarObs varDefine(CKIRBuilderRef builder, const std::string& name, const CKTypeRefPack& type_pack);

		virtual CKTypedefConstObs find_typedef(const CIName& n) const;
		virtual CKNamedObs find(const CIName& n);

		void dump(CIOStream& os, const std::string & indent) const;

		virtual CKLocalTableObs get_local();

		CKLocalTableObs create_block();
		CKLocalTableObs parent_block() const;

	private:
		CKAbstractScopeObs parent_scope_;
		CINamedStorage< CKLocalVar> vars_;
		std::vector< CKLocalTableOwner> block_scopes_;
	};
	/// @endcond

	// CONTEXT

	using CKTypeTableObs = CKTypeTable*;
	using CKGlobalTableObs = CKGlobalTable*;
	using CKGlobalTableConstObs = const CKGlobalTable*;

	/// @cond INTERNAL
	struct CKTables {
		CKTables(CKIREnvironmentObs irenv);

		CKIRModuleObs module() const
		{
			return module_;
		}

		CKTypeTableObs typetable()
		{
			return &typetable_;
		}

		CKGlobalTableObs globtable()
		{
			return &globtable_;
		}

		CKGlobalTableConstObs globtable() const
		{
			return &globtable_;
		}

		void dump_tables(std::ostream& os) const;

		void dump_ir_module(std::ostream& os) const;

		std::error_code write_bitcode_module(const std::string& fname) const;

		CKIRDataLayoutObs data_layout() const
		{
			return data_layout_;
		}

	private:
		void declare_library();

		CKIREnvironmentObs irenv_;
		CKIRModuleObs module_;
		CKIRDataLayoutObs data_layout_;

		CKTypeTable typetable_;
		CKGlobalTable globtable_;
	};
	/// @endcond

	using CKTablesObs = CKTables*;

	struct CKContext {
		/// @cond INTERNAL
		CKContext(CKTablesObs tab);
		/// @endcond

		/// @name Scope control
		/// @{
		void enter_function(CKFunctionObs f, CKFunctionFormalPackArray pack);
		void exit_function();
		void enter_block();
		void exit_block();
		/// @}
		
		/// @name Built-in types
		/// @{
		CKVoidTypeObs get_void_type() const { return typetable_->get_void_type(); }
		CKBoolTypeObs get_bool_type() const { return typetable_->get_bool_type(); }
		CKCharTypeObs get_char_type() const { return typetable_->get_char_type(); }
		CKIntTypeObs get_int_type() const { return typetable_->get_int_type(); }
		/// @}

		/// @name Creating unnamed types
		/// @{
		CKPtrTypeObs get_pointer_type(const CKTypeRefPack& pack) { return typetable_->get_pointer_type(pack); }
		CKArrayTypeObs get_array_type(CKTypeObs element_type, CKIRConstantIntObs size) { return typetable_->get_array_type(element_type, size); }
		CKFunctionTypeObs get_function_type(CKTypeObs ret_type, CKTypeObsArray arg_types, bool variadic = false) { return typetable_->get_function_type(ret_type, std::move(arg_types), variadic); }
		/// @}

		/// @name Named struct types
		/// @{
		CKStructTypeObs declare_struct_type(const CIName& n);
		CKStructTypeObs find_struct_type(const CIName& n);
		/// @}

		/// @name Named enum types
		/// @{
		CKEnumTypeObs declare_enum_type(const CIName& n);
		CKEnumTypeObs find_enum_type(const CIName& n);
		/// @}

		/// @name Variables
		/// @{
		CKVarObs define_var(const std::string& name, const CKTypeRefPack& type_pack);
		/// @}

		/// @name Typedefs
		/// @{
		CKTypedefConstObs define_typedef(const std::string& name, const CKTypeRefPack& type_pack);
		/// @}

		/// @name Enumeration constants
		/// @{
		CKConstantConstObs define_constant(const std::string& name, CKTypeObs type, CKIRConstantIntObs value);
		/// @}

		/// @name Functions
		/// @{
		CKFunctionObs declare_function(const CIName& n, CKFunctionTypeObs type)
		{
			return globtable_->declare_function(n, module_, type);
		}
		/// @}

		/// @name Finding named objects
		/// @{
		CKNamedObs find(const CIName& n);
		CKTypedefConstObs find_typedef(const CIName& n) const;
		/// @}

		/// @name Determination of typedefs in lexer
		/// @{
		bool is_typedef(const CIName& n) const;
		/// @}

		/// @name Representation of integer constants
		/// @{
		CKIRConstantIntObs get_int8_constant(std::int_fast32_t v)
		{
			return CKGetInt8Constant(builder_.getContext(), v);
		}

		CKIRConstantIntObs get_int32_constant(std::int_fast32_t v)
		{
			return CKGetInt32Constant(builder_.getContext(), v);
		}
		/// @}

		/// @name Active code builder
		/// @{
		CKIRBuilderObs builder()
		{
			return &builder_;
		}
		/// @}

		/// @name Type properties
		/// @{
		CKIRConstantIntObs get_type_size(CKTypeObs type) const
		{
			return CKGetInt32Constant(builder_.getContext(), CKGetTypeSize(data_layout_, type->get_ir()));
		}
		/// @}

		/// @cond INTERNAL
		CKIRDataLayoutObs data_layout() const
		{
			return data_layout_;
		}
		/// @endcond
	private:
		CKTypeTableObs typetable_;
		CKGlobalTableObs globtable_;
		CKIRModuleObs module_;
		CKLocalTableObs loctable_;

		CKIRBuilder builder_;
		CKIRDataLayoutObs data_layout_;
	};
}

inline std::size_t std::hash<cecko::CKTypeObs>::operator()(const cecko::CKTypeObs& a) const
{
	return cecko::compute_hash(a->hash());
}

inline std::size_t std::hash<cecko::CKTypeRefPack>::operator()(const cecko::CKTypeRefPack& a) const
{
	return cecko::compute_hash(a.type, a.is_const);
}


#endif
