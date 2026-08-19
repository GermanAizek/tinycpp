# Tests for TinyCC

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# libtcc tests (host compiled binaries linking against libtcc)
add_executable(libtcc_test libtcc_test.c)
target_include_directories(libtcc_test PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
target_link_libraries(libtcc_test PRIVATE libtcc)

add_executable(libtcc_test_mt libtcc_test_mt.c)
target_include_directories(libtcc_test_mt PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
target_link_libraries(libtcc_test_mt PRIVATE libtcc Threads::Threads)

# CTest definitions
add_test(
    NAME test_hello_exe
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
        --test-type hello_exe
        --tcc $<TARGET_FILE:tcc>
        --src-dir ${CMAKE_SOURCE_DIR}
        --build-dir ${CMAKE_BINARY_DIR}
)

add_test(
    NAME test_hello_run
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
        --test-type hello_run
        --tcc $<TARGET_FILE:tcc>
        --src-dir ${CMAKE_SOURCE_DIR}
        --build-dir ${CMAKE_BINARY_DIR}
)

add_test(
    NAME test_libtcc
    COMMAND $<TARGET_FILE:libtcc_test> ${CMAKE_SOURCE_DIR}/tcc.c -B${CMAKE_BINARY_DIR} -I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR} -I${CMAKE_BINARY_DIR}
)

add_test(
    NAME test_libtcc_mt
    COMMAND $<TARGET_FILE:libtcc_test_mt> ${CMAKE_SOURCE_DIR}/tcc.c -B${CMAKE_BINARY_DIR} -I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR} -I${CMAKE_BINARY_DIR}
)
set_tests_properties(test_libtcc_mt PROPERTIES TIMEOUT 60)

add_test(
    NAME test_tcctest
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
        --test-type tcctest
        --tcc $<TARGET_FILE:tcc>
        --host-cc ${CMAKE_C_COMPILER}
        --src-dir ${CMAKE_SOURCE_DIR}
        --build-dir ${CMAKE_BINARY_DIR}
)

add_test(
    NAME test_vla
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
        --test-type vla_test
        --tcc $<TARGET_FILE:tcc>
        --src-dir ${CMAKE_SOURCE_DIR}
        --build-dir ${CMAKE_BINARY_DIR}
)

add_test(
    NAME test_asm_c_connect
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
        --test-type asm_c_connect
        --tcc $<TARGET_FILE:tcc>
        --src-dir ${CMAKE_SOURCE_DIR}
        --build-dir ${CMAKE_BINARY_DIR}
)

if(TCC_ENABLE_BCHECK)
    add_test(
        NAME test_btest
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
            --test-type btest
            --tcc $<TARGET_FILE:tcc>
            --src-dir ${CMAKE_SOURCE_DIR}
            --build-dir ${CMAKE_BINARY_DIR}
    )
endif()

add_test(
    NAME test_tests2
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
        --test-type tests2
        --tcc $<TARGET_FILE:tcc>
        --host-cc ${CMAKE_C_COMPILER}
        --src-dir ${CMAKE_SOURCE_DIR}
        --build-dir ${CMAKE_BINARY_DIR}
)
set_tests_properties(test_tests2 PROPERTIES TIMEOUT 120)

add_test(
    NAME test_pp
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
        --test-type pp
        --tcc $<TARGET_FILE:tcc>
        --src-dir ${CMAKE_SOURCE_DIR}
        --build-dir ${CMAKE_BINARY_DIR}
)

if(TCC_BUILD_CROSS_COMPILERS)
    add_test(
        NAME test_cross_compilers
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_test.py
            --test-type cross_test
            --src-dir ${CMAKE_SOURCE_DIR}
            --build-dir ${CMAKE_BINARY_DIR}
    )
endif()
