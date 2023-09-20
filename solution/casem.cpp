#include "casem.hpp"

namespace casem {
//#pragma region Lexer
    void shift_enregister(int enregistered, int *numregister, int maxlength, int &length) {
        for (int i = maxlength - 1; i > 0; i--) {
            numregister[i] = numregister[i - 1];
        }

        numregister[0] = enregistered;
        length++;
    }

    int char_to_int(char c) {
        return (c >= 'A') ? (c >= 'a') ? (c - 'a' + 10) : (c - 'A' + 10) : (c - '0');
    }

    bool number_literal_outrange(char *text, int length, bool hexa) {
        int start = 0;
        while (text[start] == '0') {
            start++;
        }

        length -= start;

        if (hexa) {
            return length > 8;
        }

        else {
            if (length < 10) {
                return false;
            }

            if (length > 10) {
                return true;
            }

            for (int i = start; i < 10; i++) {
                if ("2147483647"[i] < text[i]) {
                    return true;
                }

                else if ("2147483647"[i] > text[i]) {
                    return false;
                }
            }

            return false;
        }
    }

    int parse_number_literal(char *text, int length, bool hexa) {
        int result = 0;
        int magnitude = hexa ? 16 : 10;
        int start = 0;

        while (text[start] == '0') {
            start++;
        }

        if (hexa && ((length - start) > 8)) {
            start = (length - 8);
        }

        for (int i = start; i < length; i++) {
            result *= magnitude;
            result += char_to_int(text[i]);
        }

        return result;
    }

    bool is_numeral_char(char c, bool hexa) {
        if (hexa) {
            return (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
        }

        else {
            return ((c >= '0') && (c <= '9'));
        }
    }
//#pragma endregion

//#pragma region Declarations

    cecko::CKTypeObs get_ETYPE_type(cecko::context *ctx, const cecko::gt_etype &etype) {
        switch (etype) {
            case cecko::gt_etype::INT:
                return ctx->get_int_type();
            case cecko::gt_etype::CHAR:
                return ctx->get_char_type();
            case cecko::gt_etype::BOOL:
                return ctx->get_bool_type();
            default:
                return ctx->get_void_type();
        }
    }

    cecko::CKTypeObs get_TYPEIDF_type(cecko::context *ctx, const cecko::CIName &type) {
        auto typedf = ctx->find_typedef(type);
        return typedf->get_type_pack().type;
    }

    decl_specificator create_type_specifier(cecko::CKTypeObs type, cecko::loc_t location) {
        decl_specificator result;
        result.is_type = true;
        result.type = type;
        result.location = location;
        return result;
    }

    decl_specificator create_const_qualifier(cecko::loc_t location) {
        decl_specificator result;
        result.is_const = true;
        result.location = location;
        return result;
    }

    decl_specificator create_typedef_specifier(cecko::loc_t location) {
        decl_specificator result;
        result.is_typedef = true;
        result.location = location;
        return result;
    }

    decl_enumerator create_enumerator(cecko::context *ctx, const cecko::CIName &identifier, cecko::loc_t location) {
        decl_enumerator result;
        result.identifier = identifier;
        result.identifier_loc = location;
        return result;
    }

    decl_enumerator create_enumerator(cecko::context *ctx, const cecko::CIName &identifier, cecko::loc_t location, expression_part &operand) {
        decl_enumerator result;
        result.identifier = identifier;
        result.identifier_loc = location;
        result.has_value = true;

        if (!has_implicit_conversion(operand.type_descriptor, ctx->get_int_type())) {
            ctx->message(cecko::errors::NOT_NUMBER, location);
            result.value = nullptr;
        }

        else {
            auto initializer = generate_implicit_conversion(ctx, operand, ctx->get_int_type());
            result.value = cecko::CKTryGetConstantInt(initializer.value_representation);
        }

        if (result.value == nullptr) {
            ctx->message(cecko::errors::SYNTAX, location, "enum value initializer must be a constant value");
        }

        return result;
    }

    decl_specificator create_enum_definition(cecko::context *ctx, const cecko::CIName &identifier, cecko::loc_t location, const decl_enumerators &items) {
        decl_specificator result;
        result.is_type = true;
        auto type = ctx->define_enum_type_open(identifier, location);
        result.type = type;

        int value = 0;
        cecko::CKConstantObsVector constants;
        for (auto &&item : items) {
            auto IRvalue = ctx->get_int32_constant(value);
            value++;

            if (item.has_value) {
                IRvalue = item.value;
                if (IRvalue == nullptr) {
                    continue;
                }
                value = (int)(IRvalue->getSExtValue() + 1);
            }

            auto itemConstant = ctx->define_constant(item.identifier, IRvalue, item.identifier_loc);
            constants.push_back(itemConstant);
        }

        ctx->define_enum_type_close(type, constants);
        return result;
    }

    cecko::CKTypeRefPack transform_decl_specs(cecko::context *ctx, const decl_specificators &specs) {
        bool is_const = false, is_type = false;
        cecko::CKTypeObs type = nullptr;

        for (auto &&spec : specs) {
            if (spec.is_const) {
                is_const = true;
            }
            if (spec.is_type) {
                if (is_type == true) {
                    ctx->message(cecko::errors::INVALID_SPECIFIERS, spec.location);
                }
                is_type = true;
                type = spec.type;
            }
        }

        cecko::CKTypeRefPack result(type, is_const);
        return result;
    }

    decl_pointer create_decl_pointer() {
        decl_pointer p;
        return p;
    }

    decl_pointer create_decl_const_pointer() {
        decl_pointer p;
        p.is_const = true;
        return p;
    }

    cecko::CKTypeRefPack transform_type_pointerize(cecko::context *ctx, const cecko::CKTypeRefPack &base, const decl_pointer_sequence &pointer) {
        cecko::CKTypeRefPack result = base;
        for (auto &&ptr : pointer) {
            auto type = ctx->get_pointer_type(result);
            cecko::CKTypeRefPack newresult(type, ptr.is_const);
            result = newresult;
        }
        return result;
    }

    void append_pointer_to_declarator(decl_declarator_parts &parts, const decl_pointer_sequence &pointer) {
        parts.back().pointer = pointer;
    }

    decl_declarator_part create_idf_declarator(const cecko::CIName &name, cecko::loc_t location) {
        decl_declarator_part result;
        result.is_identifier = true;
        result.identifier = name;
        result.identifier_loc = location;
        return result;
    }

    decl_declarator_part create_array_declarator(cecko::context *ctx, expression_part &operand, cecko::loc_t location) {
        decl_declarator_part result;
        result.is_array = true;
        result.array_size = nullptr;
        if (has_implicit_conversion(operand.type_descriptor, ctx->get_int_type())) {
            auto initializer = generate_implicit_conversion(ctx, operand, ctx->get_int_type());
            result.array_size = cecko::CKTryGetConstantInt(initializer.value_representation);
        }

        if (result.array_size == nullptr) {
            ctx->message(cecko::errors::SYNTAX, location, "array size initializer must be a constant value");
        }

        return result;
    }

    decl_declarator_part create_function_declarator(const singular_declarations &parameters) {
        decl_declarator_part result;
        result.is_function = true;
        result.function_params = parameters;
        return result;
    }

    decl_declarator_part create_pointer_declarator(const decl_pointer_sequence &pointer) {
        decl_declarator_part result;
        result.pointer = pointer;
        return result;
    }

    singular_declaration merge_specificators_declarator(const decl_specificators &specificators, const decl_declarator_parts &declarator) {
        singular_declaration result;
        result.specs = specificators;
        result.declarator = declarator;
        return result;
    }

    transformed_singular_declaration transform_declaration(cecko::context *ctx, const cecko::CKTypeRefPack &base, const decl_declarator_parts &declarator) {
        transformed_singular_declaration result;
        result.type = base;

        for (auto decl = declarator.rbegin(); decl < declarator.rend(); ++decl) {
            if (!decl->pointer.empty()) {
                result.type = transform_type_pointerize(ctx, result.type, decl->pointer);
            }

            if (decl->is_identifier) {
                result.has_identifier = true;
                result.identifier = decl->identifier;
                result.identifier_loc = decl->identifier_loc;
            }
            else if (decl->is_function) {
                result.function_data = transform_function_decl(ctx, result.type.type, *decl);
                result.type = cecko::CKTypeRefPack(result.function_data.type, false);
            }
            else if (decl->is_array) {
                auto newtype = ctx->get_array_type(result.type.type, decl->array_size);
                result.type = cecko::CKTypeRefPack(newtype, false);
            }
        }

        return result;
    }

    function_header_data transform_function_decl(cecko::context *ctx, cecko::CKTypeObs return_type, const decl_declarator_part &decl) {
        function_header_data result;

        bool formal = true;
        int voids = 0;

        for (auto &&param : decl.function_params) {
            auto param_specs = transform_decl_specs(ctx, param.specs);
            auto param_transformed = transform_declaration(ctx, param_specs, param.declarator);

            if (param_transformed.type.type->is_void()) {
                ++voids;
                if (param_transformed.has_identifier) {
                    ctx->message(cecko::errors::SYNTAX, param_transformed.identifier_loc, "unexpected identifier next to 'void' in function argument list");
                }
            }
            else {
                result.function_params.push_back(param_transformed.type.type);
            }

            if (formal) {
                if (param_transformed.has_identifier) {
                    cecko::CKFunctionFormalPack formal_pack(param_transformed.identifier, false, param_transformed.identifier_loc);
                    result.formal_pack_array.push_back(formal_pack);
                }
                else {
                    formal = false;
                }
            }
        }

        if ((voids > 0) && (result.function_params.size() != 0)) {
            ctx->message(cecko::errors::SYNTAX, decl.identifier_loc, "unexpected 'void' in function argument list");
        }

        if (voids > 1) {
            ctx->message(cecko::errors::SYNTAX, decl.identifier_loc, "multiple 'void's in function argument list");
        }

        cecko::CKFunctionTypeSafeObs type = ctx->get_function_type(return_type, result.function_params, false);
        result.type = type;
        return result;
    }

    plural_declaration create_declaration(const decl_specificators &specs) {
        plural_declaration result;
        result.specs = specs;
        return result;
    }

    plural_declaration create_declaration(const decl_specificators &specs, const decl_declarator_parts &declarator) {
        plural_declaration result;
        result.specs = specs;
        result.declarators.push_back(declarator);
        return result;
    }

    void append_declaration(cecko::context *ctx, plural_declaration &decl, cecko::loc_t commaLocation, const decl_declarator_parts &declarator) {
        //production: core -> core comma declarator -> specifiers comma declarator; comma before first declarator
        if (decl.declarators.size() == 0) {
            ctx->message(cecko::errors::SYNTAX, commaLocation, "Expected identifier or '(' before ','");
        }

        decl.declarators.push_back(declarator);
    }

    plural_declaration create_declaration(const decl_specificators &specs, const decl_declarators &declarators) {
        plural_declaration result;
        result.specs = specs;
        result.declarators = declarators;
        return result;
    }

    bool _is_typedef(cecko::context *ctx, const decl_specificators &specs) {
        size_t count = 0;

        for (auto &&spec : specs) {
            if (spec.is_typedef) {
                ++count;
                if (count > 1) {
                    ctx->message(cecko::errors::INVALID_SPECIFIERS, spec.location);
                    return false;
                }
            }
        }

        return count > 0;
    }

    void resolve_declarations(cecko::context *ctx, const plural_declaration &decl) {
        cecko::CKTypeRefPack base = transform_decl_specs(ctx, decl.specs);

        for (auto &&declarator : decl.declarators) {
            auto declaration = transform_declaration(ctx, base, declarator);

            if (_is_typedef(ctx, decl.specs)) {
                ctx->define_typedef(declaration.identifier, declaration.type, declaration.identifier_loc);
                continue;
            }

            auto type = declaration.type.type;
            if (type->is_function()) {
                ctx->declare_function(declaration.identifier, declaration.type.type, declaration.identifier_loc);
                continue;
            }

            if (type->is_bool() || type->is_char() || type->is_int() || type->is_pointer() || type->is_struct() || type->is_array() || type->is_enum()) {
                ctx->define_var(declaration.identifier, declaration.type, declaration.identifier_loc);
                continue;
            }

            ctx->message(cecko::errors::INVALID_SPECIFIERS, declaration.identifier_loc);
        }
    }

    void resolve_function_definition(cecko::context *ctx, const decl_specificators &specs, const decl_declarator_parts &declarator) {
        cecko::CKTypeRefPack base = transform_decl_specs(ctx, specs);
        auto declaration = transform_declaration(ctx, base, declarator);
        cecko::CKFunctionSafeObs fc_type;

        if (declaration.type.type->is_function()) {
            fc_type = ctx->declare_function(declaration.identifier, declaration.type.type, declaration.identifier_loc);
        }

        if ((declaration.function_data.formal_pack_array.size() <= 0) && (declaration.function_data.function_params.size() > 0)) {
            ctx->message(cecko::errors::SYNTAX, declaration.identifier_loc, "function argument name missing");
        }

        auto type = ctx->declare_function(declaration.identifier, declaration.type.type, declaration.identifier_loc);
        ctx->enter_function(type, declaration.function_data.formal_pack_array, declaration.identifier_loc);
    }

    decl_specificator create_struct_definition(cecko::context *ctx, const decl_struct_type_carrier &type, const plural_declarations &members) {
        cecko::CKStructItemArray items;
        for (auto &&member : members) {
            cecko::CKTypeRefPack base = transform_decl_specs(ctx, member.specs);
            for (auto &&declarator : member.declarators) {
                auto declaration = transform_declaration(ctx, base, declarator);
                cecko::CKStructItem item(declaration.type, declaration.identifier, declaration.identifier_loc);
                items.push_back(item);
            }
        }

        ctx->define_struct_type_close(type.type, items);
        return create_type_specifier(type.type, type.struct_identifier_loc);
    }
//#pragma endregion

//#pragma region Expressions
#define RETURN_EMPTY expression_part EXPR_expression_part_result_variable; return EXPR_expression_part_result_variable
#define EXPR_CHECK(EXPR) \
    if (EXPR.mode == expression_part_mode::empty) { \
        RETURN_EMPTY; \
    } \
    else if (EXPR.type_descriptor->is_void()) { \
        ctx->message(cecko::errors::VOIDEXPR, location); \
        RETURN_EMPTY; \
    }
#define EXPR_CHECK2(EXPR1, EXPR2) \
    if ((EXPR1.mode == expression_part_mode::empty) || (EXPR2.mode == expression_part_mode::empty)) { \
        RETURN_EMPTY; \
    } \
    else if (EXPR1.type_descriptor->is_void() || EXPR2.type_descriptor->is_void()) { \
        ctx->message(cecko::errors::VOIDEXPR, location); \
        RETURN_EMPTY; \
    }

    bool has_implicit_conversion(cecko::CKTypeObs from, cecko::CKTypeSafeObs to) {
        if (to->is_enum()) return from->is_enum();
        if (to->is_struct()) return from->is_struct();
        if (to->is_pointer()) return from->is_pointer() || from->is_array();
        if (to->is_int() || to->is_char()) return from->is_bool() || from->is_char() || from->is_int();
        if (to->is_bool()) return from->is_bool() || from->is_char() || from->is_int() || from->is_pointer();
        return false;
    }

    bool is_comparable(cecko::CKTypeObs from, cecko::CKTypeSafeObs to) {
        if (to->is_enum()) return from->is_enum();
        if (to->is_pointer()) return from->is_pointer();
        if (to->is_int() || to->is_char() || to->is_bool()) return from->is_int() || from->is_char() || from->is_bool();
        return false;
    }

    //Relies on has_implicit_conversion behaviour!
    expression_part generate_implicit_conversion(cecko::context *ctx,expression_part &from, cecko::CKTypeSafeObs to) {
        if (!has_implicit_conversion(from.type_descriptor, to)) {
            RETURN_EMPTY;
        }

        if (to->is_pointer()) {
            if (from.type_descriptor->is_array()) {
                auto target = to->get_pointer_points_to();
                if (target.type != from.type_descriptor->get_array_element_type()) {
                    RETURN_EMPTY;
                }

                auto twine = from.twine_base + "_convertedToPtr";
                auto CIBGEPtype = from.type_descriptor->get_ir();

                auto IRvalue = ctx->builder()->CreateConstInBoundsGEP2_32(CIBGEPtype, from.value_representation, 0, 0, twine);
                expression_part result(IRvalue, to, twine);
                return result;
            }

            if (to->get_pointer_points_to().type != from.type_descriptor->get_pointer_points_to().type) {
                RETURN_EMPTY;
            }
        }

        expression_part step = from;

        if (from.mode == expression_part_mode::lvalue) {
            step = generate_lvalue_load(ctx, from);
        }

        if ((step.type_descriptor->is_bool() && to->is_bool()) ||
            (step.type_descriptor->is_char() && to->is_char()) ||
            (step.type_descriptor->is_int()  && to->is_int() ) ||
            (step.type_descriptor->is_pointer() && to->is_pointer())) {
            return step;
        }

        if (to->is_int()) {
            auto twine = step.twine_base + "_ZExted";
            auto IRvalue = ctx->builder()->CreateZExt(step.value_representation, to->get_ir(), twine);
            expression_part result(IRvalue, to, twine);
            return result;
        }

        if (to->is_char()) {
            auto twine = step.type_descriptor->is_bool() ?
                step.twine_base + "_ZExted" :
                step.twine_base + "_trunced";
            auto IRvalue = step.type_descriptor->is_bool() ?
                ctx->builder()->CreateZExt(step.value_representation, to->get_ir(), twine) :
                ctx->builder()->CreateTrunc(step.value_representation, to->get_ir(), twine);
            expression_part result(IRvalue, to, twine);
            return result;
        }

        //to->is_bool() remains
        if (step.type_descriptor->is_pointer()) {
            auto twine = step.twine_base + "_notNulled";
            auto IRValue = ctx->builder()->CreateIsNotNull(step.value_representation, twine);
            expression_part result(IRValue, to, twine);
            return result;
        }
        
        auto zero = step.type_descriptor->is_int() ? ctx->get_int32_constant(0) : ctx->get_int8_constant(0);
        auto twine = step.twine_base + "_nequedZero";
        auto IRvalue = ctx->builder()->CreateICmpNE(step.value_representation, zero, twine);
        expression_part result(IRvalue, to, twine);
        return result;
    }

    expression_part generate_lvalue_load(cecko::context *ctx, expression_part &expr) {
        if (expr.mode != expression_part_mode::lvalue) {
            RETURN_EMPTY;
        }

        auto twine = expr.twine_base + "_loaded";
        auto IRvalue = ctx->builder()->CreateLoad(expr.type_descriptor->get_ir(), expr.value_representation, twine);
        expression_part result(IRvalue, expr.type_descriptor, twine);
        return result;
    }

    expression_part generate_assign_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        
        if ((left_operand.mode != expression_part_mode::lvalue) || left_operand.is_const) {
            ctx->message(cecko::errors::SYNTAX, location, "expression must be a non-const lvalue");
            return right_operand;
        }

        expression_part storable = generate_implicit_conversion(ctx, right_operand, left_operand.type_descriptor);
        if (storable.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            return right_operand;
        }

        ctx->builder()->CreateStore(storable.value_representation, left_operand.value_representation);
        return right_operand;
    }

    expression_part generate_assign_arith_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::gt_cass operator_type, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        
        expression_part result = right_operand;

        if ((left_operand.mode != expression_part_mode::lvalue) || left_operand.is_const) {
            ctx->message(cecko::errors::SYNTAX, location, "expression must be a non-const lvalue");
            return result;
        }

        if (left_operand.type_descriptor->is_pointer()) {
            if (right_operand.type_descriptor->is_pointer()) {
                ctx->message(cecko::errors::INCOMPATIBLE, location);
                return result;
            }

            auto ptr_result = generate_pointer_add(ctx, left_operand, right_operand, get_arith_op(operator_type), location);
            if (ptr_result.mode == expression_part_mode::empty) {
                return result;
            }

            result = ptr_result;
        }

        else {
            auto num_result = generate_number_arith(ctx, left_operand, right_operand, get_arith_op(operator_type), location);
            if (num_result.mode == expression_part_mode::empty) {
                return result;
            }

            result = num_result;
        }

        expression_part storable = generate_implicit_conversion(ctx, result, left_operand.type_descriptor);
        if (storable.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            return right_operand;
        }

        ctx->builder()->CreateStore(storable.value_representation, left_operand.value_representation);
        return storable;
    }

    expression_part generate_or_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::loc_t location) {
        RETURN_EMPTY;
    }

    expression_part generate_and_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::loc_t location) {
        RETURN_EMPTY;
    }

    expression_part generate_relational_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, rel_op operator_type, cecko::loc_t location) { 
        if (!is_comparable(left_operand.type_descriptor, right_operand.type_descriptor)) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }

        expression_part left = left_operand, right = right_operand;
        bool get_signed_icmp = !left.type_descriptor->is_pointer();

        if (left.type_descriptor->is_int() || left.type_descriptor->is_char() || left.type_descriptor->is_bool()) {
            left  = generate_implicit_conversion(ctx, left,  ctx->get_int_type());
            right = generate_implicit_conversion(ctx, right, ctx->get_int_type());
        }

        else {
            if (left.mode == expression_part_mode::lvalue) {
                left = generate_lvalue_load(ctx, left);
            }

            if (right.mode == expression_part_mode::lvalue) {
                right = generate_lvalue_load(ctx, right);
            }
        }

        cecko::CKIRValueObs IRvalue = nullptr;
        auto twine = "(" + left.twine_base + ")_(" + right.twine_base;
        switch (operator_type) {
            case rel_op::EQ:
                twine += ")__equed";
                IRvalue = ctx->builder()->CreateICmpEQ(left.value_representation, right.value_representation, twine);
                break;
            case rel_op::NEQ:
                twine += ")__notequed";
                IRvalue = ctx->builder()->CreateICmpNE(left.value_representation, right.value_representation, twine);
                break;
            case rel_op::LT:
                twine += ")__lessed";
                IRvalue = get_signed_icmp ?
                    ctx->builder()->CreateICmpSLT(left.value_representation, right.value_representation, twine) :
                    ctx->builder()->CreateICmpULT(left.value_representation, right.value_representation, twine);
                break;
            case rel_op::LEQ:
                twine += ")__lessequed";
                IRvalue = get_signed_icmp ?
                    ctx->builder()->CreateICmpSLE(left.value_representation, right.value_representation, twine) :
                    ctx->builder()->CreateICmpULE(left.value_representation, right.value_representation, twine);
                break;
            case rel_op::GEQ:
                twine += ")__greatequed";
                IRvalue = get_signed_icmp ?
                    ctx->builder()->CreateICmpSGE(left.value_representation, right.value_representation, twine) :
                    ctx->builder()->CreateICmpUGE(left.value_representation, right.value_representation, twine);
                break;
            case rel_op::GT:
                twine += ")__greated";
                IRvalue = get_signed_icmp ?
                    ctx->builder()->CreateICmpSGT(left.value_representation, right.value_representation, twine) :
                    ctx->builder()->CreateICmpUGT(left.value_representation, right.value_representation, twine);
                break;
        }

        expression_part result(IRvalue, ctx->get_bool_type(), twine);
        return result;
    }

    expression_part generate_eq_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::gt_cmpe operator_type, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        return generate_relational_expression(ctx, left_operand, right_operand, get_rel_op(operator_type), location);
    }

    expression_part generate_compare_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::gt_cmpo operator_type, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        return generate_relational_expression(ctx, left_operand, right_operand, get_rel_op(operator_type), location);
    }

    expression_part generate_number_arith(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, arith_op operator_type, cecko::loc_t location) {
        auto left  = generate_implicit_conversion(ctx,  left_operand, ctx->get_int_type());
        auto right = generate_implicit_conversion(ctx, right_operand, ctx->get_int_type());
        if ((left.mode == expression_part_mode::empty) || (right.mode == expression_part_mode::empty)) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }

        auto twine = "(" + left.twine_base + ")_(" + right.twine_base;
        cecko::CKIRValueObs IRvalue = nullptr;

        switch (operator_type) {
            case arith_op::add:
                twine += ")__added";
                IRvalue = ctx->builder()->CreateAdd(left.value_representation, right.value_representation, twine);
                break;
            case arith_op::sub:
                twine += ")__subbed";
                IRvalue = ctx->builder()->CreateSub(left.value_representation, right.value_representation, twine);
                break;
            case arith_op::mul:
                twine += ")__multed";
                IRvalue = ctx->builder()->CreateMul(left.value_representation, right.value_representation, twine);
                break;
            case arith_op::div:
                twine += ")__divved";
                IRvalue = ctx->builder()->CreateSDiv(left.value_representation, right.value_representation, twine);
                break;
            case arith_op::mod:
                twine += ")__modued";
                IRvalue = ctx->builder()->CreateSRem(left.value_representation, right.value_representation, twine);
                break;
        }

        expression_part result(IRvalue, ctx->get_int_type(), twine);
        return result;
    }

    //call if you are sure pointer arith is due. If none of the operators is a ptr, reports error!
    expression_part generate_pointer_add(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, arith_op operator_type, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        int intype = 
            (left_operand.type_descriptor->is_pointer() ? 1 : 0) +
            (right_operand.type_descriptor->is_pointer() ? 1 : 0);

        //no operand is pointer
        if (intype == 0) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }

        auto left = left_operand;
        if (left.mode == expression_part_mode::lvalue) {
            left = generate_lvalue_load(ctx, left);
        }

        auto right = right_operand;
        if (right.mode == expression_part_mode::lvalue) {
            right = generate_lvalue_load(ctx, right);
        }

        //both operands are pointer
        if (intype == 2) {
            if ((operator_type != arith_op::sub) || (left.type_descriptor->get_pointer_points_to() != right.type_descriptor->get_pointer_points_to())) {
                ctx->message(cecko::errors::INCOMPATIBLE, location);
                RETURN_EMPTY;
            }

            auto PTRtype = left.type_descriptor->get_pointer_points_to().type->get_ir();
            auto twine = "(" + left.twine_base + ")_(" + right.twine_base + ")__ptrDiffed";
            auto i64diff = ctx->builder()->CreatePtrDiff(PTRtype, left.value_representation, right.value_representation, twine);
            twine += "_trunced";
            auto IRvalue = ctx->builder()->CreateTrunc(i64diff, ctx->get_int_type()->get_ir(), twine);
            expression_part result(IRvalue, ctx->get_int_type(), twine);
            return result;
        }

        //a single operand is a pointer
        if ((operator_type != arith_op::add) && (operator_type != arith_op::sub)) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }
        
        auto ptr = left;
        auto num = right;

        if (right.type_descriptor->is_pointer()) {
            ptr = right;
            num = left;
        }

        num = generate_implicit_conversion(ctx, num, ctx->get_int_type());
        if (num.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }

        if (operator_type == arith_op::sub) {
            auto twine = num.twine_base + "_negged";
            auto IRValue = ctx->builder()->CreateNeg(num.value_representation, twine);
            num = expression_part(IRValue, num.type_descriptor, twine);
        }

        auto twine = ptr.twine_base + "_incedBy_" + num.twine_base;
        auto IRvalue = ctx->builder()->CreateGEP(ptr.type_descriptor->get_pointer_points_to().type->get_ir(), ptr.value_representation, num.value_representation, twine);
        expression_part result(IRvalue, ptr.type_descriptor, twine);
        return result;
    }

    expression_part generate_add_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::gt_addop operator_type, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        if (left_operand.type_descriptor->is_pointer() || right_operand.type_descriptor->is_pointer()) {
            return generate_pointer_add(ctx, left_operand, right_operand, get_arith_op(operator_type), location);
        }

        return generate_number_arith(ctx, left_operand, right_operand, get_arith_op(operator_type), location);
    }

    expression_part generate_mult_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        return generate_number_arith(ctx, left_operand, right_operand, arith_op::mul, location);
    }

    expression_part generate_div_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::gt_divop operator_type, cecko::loc_t location) {
        EXPR_CHECK2(left_operand, right_operand)
        return generate_number_arith(ctx, left_operand, right_operand, get_arith_op(operator_type), location);
    }

    expression_part generate_incdec_expression(cecko::context *ctx, expression_part &operand, cecko::gt_incdec operator_type, cecko::loc_t location, bool postfix) {
        EXPR_CHECK(operand)
        if ((operand.mode != expression_part_mode::lvalue) || operand.is_const) {
            ctx->message(cecko::errors::SYNTAX, location, "expression must be a non-const lvalue");
            RETURN_EMPTY;
        }
        
        expression_part one(ctx->get_int32_constant(1), ctx->get_int_type());
        expression_part base = generate_lvalue_load(ctx, operand);
        expression_part arith_inst = operand.type_descriptor->is_pointer() ?
            generate_pointer_add (ctx, base, one, get_arith_op(operator_type), location) :
            generate_number_arith(ctx, base, one, get_arith_op(operator_type), location);

        EXPR_CHECK(arith_inst)
        
        expression_part storable = generate_implicit_conversion(ctx, arith_inst, base.type_descriptor);
        if (storable.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }
        ctx->builder()->CreateStore(storable.value_representation, operand.value_representation);
        return postfix ? base : storable;
    }

    expression_part generate_ref_expression(cecko::context *ctx, expression_part &operand, cecko::loc_t location) {
        EXPR_CHECK(operand)
        if (operand.mode != expression_part_mode::lvalue) {
            ctx->message(cecko::errors::SYNTAX, location, "expression must be an lvalue");
            RETURN_EMPTY;
        }

        auto result = operand;
        result.mode = expression_part_mode::rvalue;
        cecko::CKTypeRefPack TDescPack(result.type_descriptor, result.is_const);
        result.is_const = false;
        result.type_descriptor = ctx->get_pointer_type(TDescPack);
        result.twine_base += "_reffed";
        return result;
    }

    expression_part generate_deref_expression(cecko::context *ctx, expression_part &operand, cecko::loc_t location) {
        EXPR_CHECK(operand)
        if (!operand.type_descriptor->is_pointer()) {
            ctx->message(cecko::errors::NOT_POINTER, location);
            RETURN_EMPTY;
        }

        expression_part result = operand;
        if (result.mode == expression_part_mode::lvalue) {
            result = generate_lvalue_load(ctx, result);
        }

        auto target = result.type_descriptor->get_pointer_points_to();
        result.type_descriptor = target.type;
        result.is_const = target.is_const;
        result.twine_base += "_dereffed";
        result.mode = expression_part_mode::lvalue;
        return result;
    }

    //if unary plus, doesn't promote char (i8) to int (i32); if unary minus, doesn't demote char operand back to char
    expression_part generate_sign_expression(cecko::context *ctx, expression_part &operand, cecko::gt_addop operator_type, cecko::loc_t location) {
        EXPR_CHECK(operand)
        if (!has_implicit_conversion(operand.type_descriptor, ctx->get_int_type())) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }

        if (operand.type_descriptor->is_char() && (operator_type == cecko::gt_addop::ADD)) {
            if (operand.mode == expression_part_mode::rvalue) {
                return operand;
            }

            return generate_lvalue_load(ctx, operand);
        }

        expression_part target = generate_implicit_conversion(ctx, operand, ctx->get_int_type());
        if (operator_type == cecko::gt_addop::SUB) {
            auto twine = target.twine_base + "_negged";
            auto IRvalue = ctx->builder()->CreateNeg(target.value_representation, twine);
            target.value_representation = IRvalue;
            target.twine_base = twine;
        }

        return target;
    }

    expression_part generate_not_expression(cecko::context *ctx, expression_part &operand, cecko::loc_t location) {
        EXPR_CHECK(operand)
        auto target = generate_implicit_conversion(ctx, operand, ctx->get_bool_type());
        if (target.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            RETURN_EMPTY;
        }

        auto twine = target.twine_base + "_opposite";
        auto IRvalue = ctx->builder()->CreateNot(target.value_representation, twine);
        expression_part result(IRvalue, target.type_descriptor, twine);
        return result;
    }

    expression_part generate_sizeof_expression(cecko::context *ctx, singular_declaration &declaration, cecko::loc_t location) {
        RETURN_EMPTY;
    }

    expression_part generate_arrow_expression(cecko::context *ctx, expression_part &operand, cecko::CIName &identifier, cecko::loc_t location) {
        RETURN_EMPTY;
    }

    expression_part generate_access_expression(cecko::context *ctx, expression_part &operand, cecko::CIName &identifier, cecko::loc_t location) {
        RETURN_EMPTY;
    }

    expression_part generate_call_expression(cecko::context *ctx, expression_part &operand, expression_parts &args, cecko::loc_t location) {
        EXPR_CHECK(operand)

        auto desc = operand.type_descriptor;
        if (!desc->is_function() && (!desc->is_pointer() || !desc->get_pointer_points_to().type->is_function())) {
            ctx->message(cecko::errors::SYNTAX, location, "expected a function");
            RETURN_EMPTY;
        }

        auto funcIR = operand.value_representation;
        if (desc->is_pointer()) {
            desc = desc->get_pointer_points_to().type;

            if (operand.mode == expression_part_mode::lvalue) {
                auto twine = operand.twine_base + "_FuncPtrLoaded";
                funcIR = ctx->builder()->CreateLoad(desc->get_ir(), funcIR, twine);
            }
        }

        auto argc = desc->get_function_arg_count();
        if ((argc != args.size()) && ((argc > args.size()) || !desc->is_function_variadic())) {
            ctx->message(cecko::errors::BAD_NUMBER_OF_ARGUMENTS, location);
            RETURN_EMPTY;
        }

        cecko::CKIRValueObsArray argarr;

        for (size_t i = 0; i < argc; ++i) {
            auto storable = generate_implicit_conversion(ctx, args.at(i), desc->get_function_arg_type(i));
            if (storable.mode == expression_part_mode::empty) {
                ctx->message(cecko::errors::INCOMPATIBLE, location);
                RETURN_EMPTY;
            }

            argarr.push_back(storable.value_representation);
        }

        for (size_t i = argc; i < args.size(); ++i) {
            auto arg = args.at(i);
            if (has_implicit_conversion(arg.type_descriptor, ctx->get_int_type())) {
                auto argexpr = generate_implicit_conversion(ctx, arg, ctx->get_int_type());
                argarr.push_back(argexpr.value_representation);
            }

            else if (arg.type_descriptor->is_pointer()) {
                auto argexpr = arg;
                if (argexpr.mode == expression_part_mode::lvalue) {
                    argexpr = generate_lvalue_load(ctx, argexpr);
                }
                argarr.push_back(argexpr.value_representation);
            }

            else {
                ctx->message(cecko::errors::NOT_NUMBER_OR_POINTER, location);
                RETURN_EMPTY;
            }
        }

        cecko::CKIRValueObsArrayRef arg_pack(argarr);
        auto twine = operand.twine_base + "_called";
        auto IRtype = desc->get_function_return_type();

        llvm::CallInst *IRvalue;
        if (IRtype->is_void()) {
            IRvalue = ctx->builder()->CreateCall(static_cast<cecko::CKIRFunctionObs>(funcIR), arg_pack);
        }
        else {
            IRvalue = ctx->builder()->CreateCall(static_cast<cecko::CKIRFunctionObs>(funcIR), arg_pack, twine);
        }

        expression_part result(IRvalue, IRtype, twine);
        return result;
    }

    expression_part generate_index_expression(cecko::context *ctx, expression_part &left_operand, expression_part &right_operand, cecko::loc_t location) {
        RETURN_EMPTY;
    }

    expression_part generate_identifier_expression(cecko::context *ctx, cecko::CIName &identifier, cecko::loc_t location) {
        auto desc = ctx->find(identifier);
        if (!desc) {
            ctx->message(cecko::errors::UNDEF_IDF, location, identifier);
            RETURN_EMPTY;
        }

        expression_part result(desc);
        return result;
    }

    expression_part generate_intlit_expression(cecko::context *ctx, int value, cecko::loc_t location) {
        auto IRvalue = ctx->get_int32_constant(value);
        auto IRtype = ctx->get_int_type();
        cecko::CIName twine = "int_" + std::to_string(value);
        expression_part result(IRvalue, IRtype, twine);
        return result;
    }

    expression_part generate_strlit_expression(cecko::context *ctx, cecko::CIName &value, cecko::loc_t location) {
        cecko::CIName twine = "strlit_" + ((value.length() > 5) ? value.substr(0, 3) + "..." : value);
        auto IRvalue = ctx->builder()->CreateGlobalString(value, twine);
        auto IRtype = ctx->get_array_type(ctx->get_char_type(), ctx->get_int32_constant(value.length() + 1));
        expression_part result(IRvalue, IRtype, twine);
        result.mode = expression_part_mode::lvalue;
        result.is_const = true;
        return result;
    }

//#pragma endregion

//#pragma region Control Flow
    void finalize_function_exit(cecko::context *ctx, cecko::loc_t location) {
        //generate implicit void return
        if (ctx->builder()->GetInsertBlock() != nullptr) {
            if (ctx->current_function_return_type()->is_void()) {
                ctx->builder()->CreateRetVoid();
            }
            else {
                ctx->message(cecko::errors::SYNTAX, location, "missing final return statement");
            }
            ctx->builder()->ClearInsertionPoint();
        }
        
        ctx->exit_function();
    }

    void generate_explicit_return(cecko::context *ctx, expression_part &value, cecko::loc_t location) {
        if (value.mode == expression_part_mode::empty) {
            ctx->builder()->CreateRetVoid();
        } 
        else {
            expression_part storable = generate_implicit_conversion(ctx, value, ctx->current_function_return_type());
            if (storable.mode == expression_part_mode::empty) {
                ctx->message(cecko::errors::INCOMPATIBLE, location);
            }
            else {
                ctx->builder()->CreateRet(storable.value_representation);
            }
        }

        ctx->builder()->ClearInsertionPoint();
    }

    condition_statement_info parse_condition_if(cecko::context *ctx, expression_part &operand, cecko::loc_t location) {
        if (operand.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            condition_statement_info empty;
            return empty;
        }

        auto cond_expr = generate_implicit_conversion(ctx, operand, ctx->get_bool_type());
        if (cond_expr.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, location);
            condition_statement_info empty;
            return empty;
        }

        auto tbl = ctx->create_basic_block("trueblock-" + std::to_string(location));
        auto fbl = ctx->create_basic_block("falseblock-" + std::to_string(location));
        ctx->builder()->CreateCondBr(cond_expr.value_representation, tbl, fbl);

        condition_statement_info result(tbl, fbl, location);
        ctx->builder()->SetInsertPoint(tbl);
        return result;
    }

    void parse_condition_else(cecko::context *ctx, condition_statement_info &info) {
        info.endblock = nullptr;
        if (ctx->builder()->GetInsertBlock() != nullptr) {
            auto ebl = ctx->create_basic_block("finalblock-" + std::to_string(info.if_location));
            info.endblock = ebl;
            ctx->builder()->CreateBr(ebl);
        }

        ctx->builder()->SetInsertPoint(info.falseblock);
    }

    void parse_condition_finalize(cecko::context *ctx, condition_statement_info &info) {
        if (ctx->builder()->GetInsertBlock() != nullptr) {
            if (info.endblock == nullptr) {
                auto ebl = ctx->create_basic_block("finalblock-" + std::to_string(info.if_location));
                info.endblock = ebl;
            }
            ctx->builder()->CreateBr(info.endblock);
        }

        if (info.endblock != nullptr) {
            ctx->builder()->SetInsertPoint(info.endblock);
        }
    }


    loop_statement_info parse_loop_while(cecko::context *ctx, cecko::loc_t location) {
        auto cbl = ctx->create_basic_block("condblock-" + std::to_string(location));
        ctx->builder()->CreateBr(cbl);
        ctx->builder()->SetInsertPoint(cbl);

        loop_statement_info result(location);
        result.condblock = cbl;
        return result;
    }

    void parse_loop_cond(cecko::context *ctx, loop_statement_info &info, expression_part &operand) {
        if (operand.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, info.while_location);
            return;
        }

        auto cond_expr = generate_implicit_conversion(ctx, operand, ctx->get_bool_type());
        if (cond_expr.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, info.while_location);
            return;
        }

        auto ebl = ctx->create_basic_block("finalblock-" + std::to_string(info.while_location));
        auto lbl = ctx->create_basic_block("loopblock-" + std::to_string(info.while_location));
        ctx->builder()->CreateCondBr(cond_expr.value_representation, lbl, ebl);
        ctx->builder()->SetInsertPoint(lbl);

        info.endblock = ebl;
        info.loopblock = lbl;
    }

    void parse_loop_finalize(cecko::context *ctx, loop_statement_info &info) {
        if (ctx->builder()->GetInsertBlock() != nullptr) {
            ctx->builder()->CreateBr(info.condblock);
        }

        ctx->builder()->SetInsertPoint(info.endblock);
    }


    doloop_statement_info parse_doloop_do(cecko::context *ctx, cecko::loc_t location) {
        auto lbl = ctx->create_basic_block("loopblock-" + std::to_string(location));
        ctx->builder()->CreateBr(lbl);
        ctx->builder()->SetInsertPoint(lbl);

        doloop_statement_info result;
        result.do_location = location;
        result.loopblock = lbl;
        return result;
    }

    void parse_doloop_while(cecko::context *ctx, doloop_statement_info &info, cecko::loc_t location) {
        info.while_location = location;
        auto ebl = ctx->create_basic_block("finalblock-" + std::to_string(location));
        
        if (ctx->builder()->GetInsertBlock() == nullptr) {
            ctx->builder()->SetInsertPoint(ebl);
        }
        else {
            info.endblock = ebl;
        }
    }

    void parse_doloop_finalize(cecko::context *ctx, doloop_statement_info &info, expression_part &operand) {
        if (operand.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, info.while_location);
            return;
        }

        auto cond_expr = generate_implicit_conversion(ctx, operand, ctx->get_bool_type());
        if (cond_expr.mode == expression_part_mode::empty) {
            ctx->message(cecko::errors::INCOMPATIBLE, info.while_location);
            return;
        }

        if (info.endblock != nullptr) {
            ctx->builder()->CreateCondBr(cond_expr.value_representation, info.loopblock, info.endblock);
            ctx->builder()->SetInsertPoint(info.endblock);
        }
        else {
            ctx->builder()->CreateBr(info.loopblock);
            ctx->builder()->ClearInsertionPoint();
        }
    }

    iteration_statement_info parse_iteration_init(cecko::context *ctx, cecko::loc_t location) {
        iteration_statement_info result(location);
        auto cbl = ctx->create_basic_block("condblock-" + std::to_string(location));
        result.condblock = cbl;
        ctx->builder()->CreateBr(cbl);
        ctx->builder()->SetInsertPoint(cbl);

        return result;
    }

    void parse_iteration_cond(cecko::context *ctx, iteration_statement_info &info, expression_part &operand) {
        cecko::CKIRValueObs IRvalue = nullptr;
        if (operand.mode != expression_part_mode::empty) {
            auto cond_expr = generate_implicit_conversion(ctx, operand, ctx->get_bool_type());
            if (cond_expr.mode == expression_part_mode::empty) {
                ctx->message(cecko::errors::INCOMPATIBLE, info.for_location);
                return;
            }

            IRvalue = cond_expr.value_representation;
        }
        
        auto ebl = ctx->create_basic_block("finalblock-" + std::to_string(info.for_location));
        auto ibl = ctx->create_basic_block("incblock-" + std::to_string(info.for_location));
        auto lbl = ctx->create_basic_block("loopblock-" + std::to_string(info.for_location));
        info.endblock = ebl;
        info.incblock = ibl;
        info.loopblock = lbl;

        if (IRvalue != nullptr) {
            ctx->builder()->CreateCondBr(IRvalue, lbl, ebl);
        }
        else {
            ctx->builder()->CreateBr(lbl);
        }

        ctx->builder()->SetInsertPoint(ibl);
    }
    
    void parse_iteration_inc(cecko::context *ctx, iteration_statement_info &info) {
        ctx->builder()->CreateBr(info.condblock);
        ctx->builder()->SetInsertPoint(info.loopblock);
    }
    
    void parse_iteration_finalize(cecko::context *ctx, iteration_statement_info &info) {
        if (ctx->builder()->GetInsertBlock() != nullptr) {
            ctx->builder()->CreateBr(info.incblock);
        }

        ctx->builder()->SetInsertPoint(info.endblock);
    }
//#pragma endregion

}