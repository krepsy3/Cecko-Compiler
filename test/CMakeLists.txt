function(MAKE_TEST ANUM NAME)
	set(TEST_NAME_RUN "${SOL_PREFIX}A${ANUM}T${NAME}R")
	set(TEST_NAME_JUDGE "${SOL_PREFIX}A${ANUM}T${NAME}J")
	set(TEST_EXE "${SOL_PREFIX}cecko${ANUM}")
	set(TEST_MESSAGE "cecko${ANUM}")
	set(TEST_INPUT "${NAME}.c")
	set(TEST_OUTPUT "${NAME}.${TEST_EXE}.gold")
	set(TEST_COVERAGE "${NAME}.${TEST_EXE}.cov")
	set(TEST_LINE_COVERAGE "${NAME}.${TEST_EXE}.cov.c")
	set(TEST_ASSEMBLY "${NAME}.${TEST_EXE}.ll")
	set(TEST_GOLD "${NAME}.cecko${ANUM}.gold")

	set(FNAME_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${TEST_OUTPUT}")
	set(FNAME_COVERAGE "${CMAKE_CURRENT_BINARY_DIR}/${TEST_COVERAGE}")
	set(FNAME_LINE_COVERAGE "${CMAKE_CURRENT_BINARY_DIR}/${TEST_LINE_COVERAGE}")
	set(FNAME_ASSEMBLY "${CMAKE_CURRENT_BINARY_DIR}/${TEST_ASSEMBLY}")

	add_test(NAME "${TEST_NAME_RUN}" COMMAND "${TEST_EXE}" 
		"-a" "${FNAME_ASSEMBLY}" 
		"-c" "${FNAME_COVERAGE}" 
		"-d" "${FNAME_LINE_COVERAGE}" 
		"-z" "${FNAME_OUTPUT}" 
		"${TEST_INPUT}" 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	#set_tests_properties("${TEST_NAME_RUN}" PROPERTIES PASS_REGULAR_EXPRESSION ".*${TEST_MESSAGE} done.*")
	set_tests_properties("${TEST_NAME_RUN}" PROPERTIES FIXTURES_SETUP "${TEST_OUTPUT}")
	set_tests_properties("${TEST_NAME_RUN}" PROPERTIES TIMEOUT "5")

	add_test(NAME "${TEST_NAME_JUDGE}" COMMAND "${CMAKE_COMMAND}" "-E" "compare_files" "--ignore-eol" "${FNAME_OUTPUT}" "${TEST_GOLD}" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set_tests_properties("${TEST_NAME_JUDGE}" PROPERTIES FIXTURES_REQUIRED "${TEST_OUTPUT}")
	set_tests_properties("${TEST_NAME_JUDGE}" PROPERTIES TIMEOUT "5")
endfunction()

MAKE_TEST("1" "test1")
MAKE_TEST("1" "testla-chr-n")
MAKE_TEST("1" "testla-cmt1-n")
MAKE_TEST("1" "testla-cmt2-n")
MAKE_TEST("1" "testla-num-n")
MAKE_TEST("1" "testla-str-n")

MAKE_TEST("2" "test1")
MAKE_TEST("2" "test2typedef")
MAKE_TEST("2" "testsxa-arr")
MAKE_TEST("2" "testsxa-enum")
MAKE_TEST("2" "testsxa-expr")
MAKE_TEST("2" "testsxa-stmt")
MAKE_TEST("2" "testsxa-struct")

MAKE_TEST("3" "test1")
MAKE_TEST("3" "test2typedef")

MAKE_TEST("4" "test1")
MAKE_TEST("4" "test2typedef")

MAKE_TEST("5" "test1")
MAKE_TEST("5" "test2typedef")
