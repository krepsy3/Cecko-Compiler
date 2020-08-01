
add_test(NAME "A1T1" COMMAND "cecko1" "test1.c" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties("A1T1" PROPERTIES PASS_REGULAR_EXPRESSION ".*cecko1 done.*")

add_test(NAME "A3T1" COMMAND "cecko3" "test1.c" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties("A3T1" PROPERTIES PASS_REGULAR_EXPRESSION ".*cecko3 done.*")

add_test(NAME "A4T1" COMMAND "cecko4" "test1.c" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties("A4T1" PROPERTIES PASS_REGULAR_EXPRESSION ".*cecko4 done.*")

add_test(NAME "A6T1" COMMAND "cecko6" "test1.c" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties("A6T1" PROPERTIES PASS_REGULAR_EXPRESSION ".*cecko6 done.*")
