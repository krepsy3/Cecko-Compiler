#ifndef ckir_hpp_
#define ckir_hpp_

#include "llvm/ADT/APInt.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/raw_os_ostream.h"

#include <iostream>

namespace cecko {
	// numbers
	using CKIRAPInt = llvm::APInt;
	// context
	using CKIRContextRef = llvm::LLVMContext&;
	// types
	using CKIRTypeObs = llvm::Type*;
	using CKIRStructTypeObs = llvm::StructType*;
	using CKIRFunctionTypeObs = llvm::FunctionType*;
	using CKIRTypeObsArray = std::vector<llvm::Type*>;
	using CKIRTypeObsArrayRef = llvm::ArrayRef<llvm::Type*>;
	// values
	using CKIRValueObs = llvm::Value*;
	using CKIRValueObsArray = std::vector<llvm::Value*>;
	using CKIRValueObsArrayRef = llvm::ArrayRef<llvm::Value*>;
	// constant values
	using CKIRConstantObs = llvm::Constant*;
	using CKIRConstantIntObs = llvm::ConstantInt*;
	// module
	using CKIRModuleObs = llvm::Module*;
	// function
	using CKIRFunctionObs = llvm::Function*;
	// basic block
	using CKIRBasicBlockObs = llvm::BasicBlock*;
	// instructions
	using CKIRAllocaInstObs = llvm::AllocaInst*;
	// builder
	using CKIRBuilder = llvm::IRBuilder<>;
	using CKIRBuilderRef = CKIRBuilder&;
	using CKIRBuilderObs = CKIRBuilder*;

	// string
	using CKIRName = llvm::Twine;

	inline std::size_t CKHashValue(const CKIRAPInt& Arg)
	{
		return llvm::hash_value(Arg);
	}

	inline CKIRTypeObs CKGetVoidType(CKIRContextRef Context)
	{
		return llvm::Type::getVoidTy(Context);
	}

	inline CKIRTypeObs CKGetInt1Type(CKIRContextRef Context)
	{
		return llvm::Type::getInt1Ty(Context);
	}

	inline CKIRTypeObs CKGetInt8Type(CKIRContextRef Context)
	{
		return llvm::Type::getInt8Ty(Context);
	}

	inline CKIRTypeObs CKGetInt32Type(CKIRContextRef Context)
	{
		return llvm::Type::getInt32Ty(Context);
	}

	inline CKIRTypeObs CKGetInt8PtrType(CKIRContextRef Context)
	{
		return llvm::Type::getInt8PtrTy(Context);
	}

	inline CKIRTypeObs CKGetArrayType(CKIRTypeObs element, CKIRConstantIntObs size)
	{
		return llvm::ArrayType::get(element, size->getValue().getSExtValue());
	}

	inline CKIRStructTypeObs CKCreateStructType(CKIRContextRef Context, const std::string& name)
	{
		return llvm::StructType::create(Context, name);
	}

	inline CKIRFunctionTypeObs CKGetFunctionType(CKIRTypeObs rettype, CKIRTypeObsArrayRef argtypes, bool variadic = false)
	{
		return llvm::FunctionType::get(rettype, argtypes, variadic);
	}

	inline CKIRConstantIntObs CKGetInt1Constant(CKIRContextRef Context, bool V)
	{
		return llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), V);
	}

	inline CKIRConstantIntObs CKGetInt32Constant(CKIRContextRef Context, std::int_fast32_t V)
	{
		return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), V);
	}

	inline CKIRFunctionObs CKCreateFunction(CKIRFunctionTypeObs FT, const std::string& name, CKIRModuleObs M)
	{
		return llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, M);
	}

	inline CKIRBasicBlockObs CKCreateBasicBlock(const std::string& name, CKIRFunctionObs F)
	{
		return llvm::BasicBlock::Create(F->getContext(), name, F);
	}

	inline CKIRConstantIntObs CKTryGetConstantInt(CKIRValueObs v)
	{
		return llvm::cast<llvm::ConstantInt>(v);
	}

	inline CKIRConstantObs CKCreateGlobalVariable(CKIRTypeObs irtp, const std::string& name, CKIRModuleObs M)
	{
		auto var = M->getOrInsertGlobal(name, irtp);
		auto gvar = llvm::cast<llvm::GlobalVariable>(var);
		if (irtp->isAggregateType())
			gvar->setInitializer(llvm::ConstantAggregateZero::get(irtp));
		else if (irtp->isPointerTy())
			gvar->setInitializer(llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(irtp)));
		else
			gvar->setInitializer(CKGetInt32Constant(M->getContext(), 0));
		return var;
	}

	class CKIREnvironment {
	public:
		CKIREnvironment()
		{
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();
			ckircontextptr_ = std::make_unique<llvm::LLVMContext>();
			ckirmoduleptr_ = std::make_unique<llvm::Module>("test", *ckircontextptr_);
			ckirmoduleobs_ = &*ckirmoduleptr_;
		}

		void dump_module(std::ostream& os, CKIRModuleObs module) const
		{
			llvm::raw_os_ostream raw_os(os);
			raw_os << *module;
		}

		std::error_code write_bitcode_module(const std::string& fname, CKIRModuleObs module) const
		{
			std::error_code oec;
			llvm::raw_fd_ostream ofile(fname, oec);
			if (!oec)
			{
				llvm::WriteBitcodeToFile(*module, ofile);
			}
			return oec;
		}

		int run_main(CKIRFunctionObs fnc, int argc, char** argv)
		{
			//llvm::raw_os_ostream raw_cerr(std::cerr);
			auto&& raw_cerr = llvm::errs();
			// Now we going to create JIT
			std::string errStr;
			auto EE =
				llvm::EngineBuilder(std::move(ckirmoduleptr_))
				.setErrorStr(&errStr)
				.create();

			if (!EE) {
				raw_cerr << "========== Failed to construct ExecutionEngine: " << errStr << "==========\n";
				return 1;
			}

			if (verifyModule(*ckirmoduleobs_)) {
				raw_cerr << "========== Error constructing function ==========\n";
				return 1;
			}

			raw_cerr << "========== starting main() ==========\n";

			std::vector<llvm::GenericValue> Args(2);
			Args[0].IntVal = llvm::APInt(32, argc);
			Args[1].PointerVal = argv;
			auto GV = EE->runFunction(fnc, Args);

			raw_cerr << "\n========== main() returned " << GV.IntVal << " ==========\n";
			return GV.IntVal.getSExtValue();
		}

		CKIRContextRef context()
		{
			return *ckircontextptr_;
		}

		CKIRModuleObs module()
		{
			return ckirmoduleobs_;
		}

	private:
		std::unique_ptr< llvm::LLVMContext> ckircontextptr_;
		std::unique_ptr< llvm::Module> ckirmoduleptr_;
		CKIRModuleObs ckirmoduleobs_;
	};

	using CKIREnvironmentObs = CKIREnvironment*;
};

#endif 
