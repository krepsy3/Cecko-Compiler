@page faq FAQ

### I assume at this stage we are constructing a syntax tree? At least part of it.

No. There is no need to build a syntax tree. Bison will perform a bottom-up post-order traversal through a virtual tree and it will execute a fragment of your C++ code when exiting every subtree.

Since your editing environment does not recognize `.y` files as C++ sources, it is a good idea to keep the fragments minimal, like 
\code{.cpp}
{ $$ = rule_something( ctx, $1, $3, @2); }
\endcode
and implement the `rule_something` functions in `casem.*`.

So the first part is that you need to understand how the `$$`, `$i` and `@i` work, including setting their types. A short description is in the slides, details are in the bison manual.

### I assume that @n will return loc_t?

Yes, `@i` is of type `loc_t`. `loc_t` is just an `int`. It is defined in the framework and told bison via some `%%define` declaration in `.y`

### But if we don't build the tree itself, then are we also supposed to later type check and translate in these fragments? I assumed we are recording this tree and then in a separate run we are type checking it and then in a separate run we are translating it.

No. There is no separate run. Directly when the source is parsed, bison calls your C++ fragments and you call framework functions which create some internal representation of the types and variables in the ctx. There will be no physical tree in our code. We are building a single-pass compiler (which generates the LLVM IR).

With some dirty tricks, you can implement the compiler inside these C++ fragments. For Assignment 3, the only really dirty tricks are associated with the IDF/TYPEIDF distinction in the lexer which you already did in Asgn 2, and the related warnings (in the slides) on the effect of look-ahead in bison.

### Is it better to start writing from the top symbol, for example from declaration?

Yes, that might be a good approach. It is a good idea to rearrange the rules just under `declaration` so that you have a nice place where one `init-declarator` meets with `declaration-specifiers`. Then, you will write a C++ fragment into this place to generate the variable/function/typedef.

Of course, your code will need some inputs from the underlying syntactic parts - this will tell you what data you need from the parts, i.e. the types for `%type` (you will need to define some of them in `casem.hpp`, the framework types are not sufficient).

To start with Assignment 3, you will probably need to create your own test input, starting from the simplest things like 
\code{.cpp}
int x;
\endcode
For this case, your job is essentially to call these framework functions:
\code{.cpp}
loc_t loc = /* actual line number where the token "x" is located */;
CKTypeSafeObs tp = ctx->get_int_type();
ctx->define_var("x", CKTypeRefPack(tp, false), loc);
\endcode

Of course, you can't do it in a single code fragment - you have to split it into several pieces, capable of handling much more complex cases.

The `ctx->get_int_type()` call will neatly fit into the rule `type-specifier: int` (it will however be slightly different in your grammar due to grouped tokens).

The `ctx->define_var()` call must be located at a place where both the `declaration-specifiers` (which tell you the base type) and `init-declarator` (which contains the variable name and additional type constructions) are available. For this, it is a good idea to regroup the grammar slightly (as shown in the slides) to let the two pieces of information meet already when one `init-declarator` is encountered (i.e. not above the `init-declarator-list_opt`).

The most complex part of Assignment 3 is dealing with the declarators. They are structured in exactly the opposite way than you need. So, for this part, you will need a temporary structure which will somehow hold the declarators (i.e. pointer/array/function constructs) together with the identifier placed inside. Above the declarators (when you finally meet the information on the base type from the `declaration-specifiers`, you will have to execute the correct framework functions (in the correct order) to create the pointer/array/function types from the base type.

Example:
`const int * p[10]` is syntactically grouped as `const int (* (p[10]))`. The meaning is "p is an array of pointers to const int". Therefore, you have to call this:
\code{.cpp}
CKTypeSafeObs tp1 = ctx->get_int_type();
bool is_const1 = true;
CKTypeSafeObs tp2 = ctx->get_pointer_type(CKTypeRefPack(tp1, is_const1));
bool is_const2 = false; // the pointer is not const (but points to const)
CKTypeSafeObs tp3 = ctx->get_array_type(tp2,ctx->get_int32_constant(10));
bool is_const3 = is_const2; // const element implies const array
ctx->define_var("x", CKTypeRefPack(tp3, is_const3), loc);
\endcode
Your job is to arrange a temporary structure which will allow you to execute the sequence of `get_pointer_type` and `get_array_type` in the right moment (when you have the `tp1` at hand).
In addition, if the last type (`tp3`) were a function, you have to call `declare_function` instead of `define_var`. And if there were a `typedef` keyword, you call `define_typedef` instead.

### Jaké chyby máme ohlašovat, pokud declaration-specifiers obsahuje nekompatibilní nebo zduplikované specifiery (resp. qualifiery)? Něco jako const const int i; nebo bool int i;.

Ohlaste `errors::INVALID_SPECIFIERS` v techto pripadech: `FILE int`, `char int`, `const x;`, `typedef T;`, `typedef int f() { /.../ }`, `int f(typedef int x)`. 

Pripad `const const int`  nebo `typedef const int T; const T` nepovazujeme v Cecku za chybu.

### Je sémanticky správně deklarace int func(bool b)[];? Gramatika ji umožňuje a mělo by jít o deklaraci funkce func  s návratovou hodnotou int[] . Nikde jsem ale nenašel, že je něco takového možné.

Neni to mozne, funkce nesmeji vracet pole (take proto je v C++ std::array). 

Funkce `get_array_type`  a  `get_function_type` vraceji `nullptr`, pokud by tim vznikl nelegalni typ. Ten `nullptr` tedy musite otestovat a pripadne ohlasit `errors::INVALID_ARRAY_TYPE`  resp. `errors::INVALID_FUNCTION_TYPE` . 

Funkce `declare_function` akceptuje `CKTypeObs` a sama si otestuje, jestli to je typ funkce, a pokud ne, tak sama ohlasi chybu (`INVALID_FUNCTION_TYPE`), cimz je osetren pripad `int f {/.../}`.

### Může být návratová hodnota funkce const ? Protože pokud ano, není kam příznak const uložit.

To `const` u funkce je syntakticky i semanticky povoleno, ale ignoruje se (protoze volani funkce v C neni L-value a nejde tudiz nijak modifikovat jeho hodnotu). Proto framework ten bit ani neuklada.

### Je povolené definovat struct,  enum  nebo typedef uvnitř parametrů funkce?

Definice `struct`/`enum` mohou byt kdekoliv, syntakticky je to povoleno vsude, semanticky je to v C snad zakazano uvnitr cast a `sizeof` expression, to ale nereste. Naopak, `typedef` v deklaraci parametru byt nesmi (ackoliv gramatika to nejspis povoluje, coz je dusledek archaismu jako `register`, ktere syntakticky jsou ve stejne kategorii jako `typedef`). Ten `typedef` muzete vyresit budto vyhozenim z gramatiky (tj. jako syntax error), nebo semanticky jako `errors::INVALID_SPECIFIERS`.

### What does "CKTypeRefPack" represent?

Example:

\code{.cpp}
typedef int T1;
typedef const int T2;
typedef const T1 T3[10];
typedef const T2 T4[10];
typedef T2 T5[10];
\endcode
In this case, the three typedefs T3, T4, T5 are by definition identical, i.e. it does not matter where exactly the const was added (and the double const in T4 is silently merged).
A related case is in the definition of a function type:
\code{.cpp}
typedef int TF1(int, int);
typedef int TF2(const int, const int);
\endcode
These two typedefs are by definition identical (the const flag on a parameter is simply ignored because it does not affect the caller).
Therefore, it is easier to handle the type descriptors independently of the const flag. The only case where the const flag is a part of a type descriptor is a pointer, i.e. `const X*` is different from `X*`.
A typedef stores a pair of a type descriptor and a const flag, which is exactly the contents of the structure `CKTypeRefPack`. And you may also use the structure in your code, for instance when representing a type specifier list like `T2` or `const int`.
