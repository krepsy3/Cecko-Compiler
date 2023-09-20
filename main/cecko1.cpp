/*

cecko1.cpp

main program

*/

#include "ckmain.hpp"

#include "ckdumper.hpp"

int main(int argc, char **argv)
{
	cecko::main_state_code ms;

	auto rv1 = ms.setup(argc, argv);
	if (!rv1)
		return 0;

	auto rv2 = ms.parse< cecko::parser>();
	if (!rv2)
		return 0;

	ms.out() << "========== cecko1 done ==========" << std::endl;

	return 0;
}
