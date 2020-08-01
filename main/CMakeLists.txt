
MAKE_TARGET_DUMP("A1" "cecko1")
MAKE_TARGET("A3" "cecko3")
MAKE_TARGET("A4" "cecko4")
MAKE_TARGET("A6" "cecko6")

target_sources(${TARGET_A1} PUBLIC "cecko1.cpp")
target_sources(${TARGET_A3} PUBLIC "cecko3.cpp")
target_sources(${TARGET_A4} PUBLIC "cecko4.cpp")
target_sources(${TARGET_A6} PUBLIC "cecko6.cpp")
