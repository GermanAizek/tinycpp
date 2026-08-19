cmake_minimum_required(VERSION 3.15)

# Read version from VERSION file if available
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" TCC_VERSION_RAW)
    string(STRIP "${TCC_VERSION_RAW}" TCC_VERSION)
else()
    set(TCC_VERSION "0.9.28rc")
endif()

# Extract numeric version for CMake project
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" TCC_PROJECT_VERSION "${TCC_VERSION}")
if(NOT TCC_PROJECT_VERSION)
    set(TCC_PROJECT_VERSION "0.9.28")
endif()

project(tinycpp
    VERSION ${TCC_PROJECT_VERSION}
    LANGUAGES C
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Build options
option(BUILD_SHARED_LIBS "Build libtcc as a shared library" OFF)
option(TCC_BUILD_CROSS_COMPILERS "Build cross compilers" ON)
option(TCC_BUILD_LIBTCC1 "Build libtcc1.a runtime library using tcc" ON)
option(TCC_BUILD_TESTS "Build and enable tests" ON)
option(TCC_BUILD_DOCS "Build documentation if tools are available" ON)
option(TCC_ENABLE_BACKTRACE "Enable backtrace support" ON)
option(TCC_ENABLE_BCHECK "Enable bounds checking support" ON)

# Path options
set(CONFIG_SYSROOT "" CACHE STRING "Target root prefix (CONFIG_SYSROOT)")
set(CONFIG_TCCDIR "" CACHE STRING "TCC directory for includes and libs (CONFIG_TCCDIR)")
set(CONFIG_TCC_SYSINCLUDEPATHS "" CACHE STRING "Custom sysincludepaths (CONFIG_TCC_SYSINCLUDEPATHS)")
set(CONFIG_TCC_LIBPATHS "" CACHE STRING "Custom libpaths (CONFIG_TCC_LIBPATHS)")
set(CONFIG_TCC_CRTPREFIX "" CACHE STRING "Custom crtprefix (CONFIG_TCC_CRTPREFIX)")
set(CONFIG_TCC_ELFINTERP "" CACHE STRING "Custom elf interpreter (CONFIG_TCC_ELFINTERP)")
set(CONFIG_TRIPLET "" CACHE STRING "Target architecture triplet (CONFIG_TRIPLET)")
set(CONFIG_OS_RELEASE "" CACHE STRING "Target OS release (CONFIG_OS_RELEASE)")

# Compiler warning flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wdeclaration-after-statement -Wno-unused-result)
endif()

# Git hash extraction for version info
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --date=short "--pretty=format:%cd ${GIT_BRANCH}@%h"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH_STR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} diff --quiet
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_DIFF_RES
        ERROR_QUIET
    )
    if(GIT_DIFF_RES EQUAL 1)
        set(TCC_GITHASH "${GIT_HASH_STR}*")
    else()
        set(TCC_GITHASH "${GIT_HASH_STR}")
    endif()
endif()

# Target OS detection
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(TCC_TARGET_PE 1)
    set(TARGETOS_WIN32 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TCC_TARGET_MACHO 1)
    set(TARGETOS_Darwin 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TARGETOS_Linux 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(TARGETOS_FreeBSD 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    set(TARGETOS_NetBSD 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    set(TARGETOS_OpenBSD 1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(TARGETOS_ANDROID 1)
endif()

# Target CPU architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
    set(TCC_TARGET_X86_64 1)
    set(TCC_ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i.86|x86|X86")
    set(TCC_TARGET_I386 1)
    set(TCC_ARCH "i386")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
    set(TCC_TARGET_ARM64 1)
    set(TCC_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
    set(TCC_TARGET_ARM 1)
    set(TCC_ARM_EABI 1)
    set(TCC_ARM_VFP 1)
    set(TCC_ARM_HARDFLOAT 1)
    set(TCC_ARCH "arm")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
    set(TCC_TARGET_RISCV64 1)
    set(TCC_ARCH "riscv64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "c67")
    set(TCC_TARGET_C67 1)
    set(TCC_ARCH "c67")
else()
    message(WARNING "Unknown processor ${CMAKE_SYSTEM_PROCESSOR}, defaulting to x86_64")
    set(TCC_TARGET_X86_64 1)
    set(TCC_ARCH "x86_64")
endif()

# Host Compiler vendor detection
if(CMAKE_C_COMPILER_VERSION MATCHES "^([0-9]+)\\.([0-9]+)")
    set(GCC_MAJOR ${CMAKE_MATCH_1})
    set(GCC_MINOR ${CMAKE_MATCH_2})
else()
    set(GCC_MAJOR 0)
    set(GCC_MINOR 0)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CC_NAME "CC_gcc")
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CC_NAME "CC_clang")
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(CC_NAME "CC_msvc")
    set(GCC_MAJOR 0)
    set(GCC_MINOR 0)
else()
    set(CC_NAME "CC_unknown")
    set(GCC_MAJOR 0)
    set(GCC_MINOR 0)
endif()

# Default triplet and dynamic linker paths on Linux
if(TCC_TARGET_X86_64)
    set(DEFAULT_TRIPLET "x86_64-linux-gnu")
    set(DEFAULT_ELFINTERP "/lib64/ld-linux-x86-64.so.2")
elseif(TCC_TARGET_I386)
    set(DEFAULT_TRIPLET "i386-linux-gnu")
    set(DEFAULT_ELFINTERP "/lib/ld-linux.so.2")
elseif(TCC_TARGET_ARM64)
    set(DEFAULT_TRIPLET "aarch64-linux-gnu")
    set(DEFAULT_ELFINTERP "/lib/ld-linux-aarch64.so.1")
elseif(TCC_TARGET_ARM)
    set(DEFAULT_TRIPLET "arm-linux-gnueabihf")
    set(DEFAULT_ELFINTERP "/lib/ld-linux-armhf.so.3")
elseif(TCC_TARGET_RISCV64)
    set(DEFAULT_TRIPLET "riscv64-linux-gnu")
    set(DEFAULT_ELFINTERP "/lib/ld-linux-riscv64-lp64d.so.1")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CONFIG_TCC_MUSL)
        set(DEFAULT_ELFINTERP "/lib/ld-musl-${TCC_ARCH}.so.1")
    endif()

    if(NOT CONFIG_TRIPLET)
        execute_process(
            COMMAND ${CMAKE_C_COMPILER} -print-multiarch
            OUTPUT_VARIABLE GCC_MULTIARCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(GCC_MULTIARCH AND EXISTS "/usr/lib/${GCC_MULTIARCH}/crti.o")
            set(CONFIG_TRIPLET "${GCC_MULTIARCH}")
        elseif(DEFAULT_TRIPLET AND EXISTS "/usr/lib/${DEFAULT_TRIPLET}/crti.o")
            set(CONFIG_TRIPLET "${DEFAULT_TRIPLET}")
        else()
            set(CONFIG_TRIPLET "${DEFAULT_TRIPLET}")
        endif()
    endif()

    if(NOT CONFIG_TCC_SYSINCLUDEPATHS)
        set(CONFIG_TCC_SYSINCLUDEPATHS "{B}/include:{B}/include/${CONFIG_TRIPLET}:/usr/local/include:/usr/local/include/${CONFIG_TRIPLET}:/usr/include/${CONFIG_TRIPLET}:/usr/include")
    endif()
    if(NOT CONFIG_TCC_LIBPATHS)
        set(CONFIG_TCC_LIBPATHS "{B}:/usr/local/lib/${CONFIG_TRIPLET}:/usr/local/lib64:/usr/local/lib:/usr/lib/${CONFIG_TRIPLET}:/usr/lib64:/usr/lib:/lib/${CONFIG_TRIPLET}:/lib64:/lib")
    endif()
    if(NOT CONFIG_TCC_CRTPREFIX)
        set(CONFIG_TCC_CRTPREFIX "{B}:/usr/lib/${CONFIG_TRIPLET}:/usr/lib64:/usr/lib")
    endif()
    if(NOT CONFIG_TCC_ELFINTERP)
        set(CONFIG_TCC_ELFINTERP "${DEFAULT_ELFINTERP}")
    endif()
    if(NOT CONFIG_OS_RELEASE)
        set(CONFIG_OS_RELEASE "linux")
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE OSX_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(OSX_SDK_PATH)
        set(CONFIG_TCC_SYSINCLUDEPATHS "{B}/include:${OSX_SDK_PATH}/usr/include")
    else()
        set(CONFIG_TCC_SYSINCLUDEPATHS "{B}/include:/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include:/usr/include")
    endif()
    set(CONFIG_TCC_LIBPATHS "{B}:/usr/lib:/usr/local/lib")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CONFIG_TCC_SYSINCLUDEPATHS "{B}/include:{B}/include/winapi")
    set(CONFIG_TCC_LIBPATHS "{B}/lib")
endif()

if(NOT CONFIG_TCCDIR)
    if(WIN32)
        set(CONFIG_TCCDIR "${CMAKE_INSTALL_PREFIX}")
    else()
        set(CONFIG_TCCDIR "${CMAKE_INSTALL_FULL_LIBDIR}/tcc")
    endif()
endif()

# Features configuration
set(CONFIG_TCC_PREDEFS 1)
if(TCC_ENABLE_BACKTRACE)
    set(CONFIG_TCC_BACKTRACE 1)
else()
    set(CONFIG_TCC_BACKTRACE 0)
endif()
if(TCC_ENABLE_BCHECK)
    set(CONFIG_TCC_BCHECK 1)
else()
    set(CONFIG_TCC_BCHECK 0)
endif()
set(CONFIG_TCC_SEMLOCK 1)

# Generate config.h and config.texi
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    @ONLY
)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.texi" "@set VERSION ${TCC_VERSION}\n")

# Copy internal headers and tcclib.h to build include directory for local execution
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/include")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/tcclib.h" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/include")
if(WIN32)
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/win32/include/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/include")
endif()

# Helper tool c2str to generate tccdefs_.h
add_executable(c2str conftest.c)
target_compile_definitions(c2str PRIVATE C2STR=1)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tccdefs_.h"
    COMMAND c2str "${CMAKE_CURRENT_SOURCE_DIR}/include/tccdefs.h" "${CMAKE_CURRENT_BINARY_DIR}/tccdefs_.h"
    DEPENDS c2str "${CMAKE_CURRENT_SOURCE_DIR}/include/tccdefs.h"
    COMMENT "Generating tccdefs_.h from include/tccdefs.h"
)
add_custom_target(tccdefs_h DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/tccdefs_.h")

# libtcc source files
set(LIBTCC_CORE_SRCS
    libtcc.c
    tccpp.c
    tccgen.c
    tccdbg.c
    tccelf.c
    tccasm.c
    tccrun.c
)

if(TCC_TARGET_X86_64)
    set(LIBTCC_ARCH_SRCS x86_64-gen.c x86_64-link.c i386-asm.c)
elseif(TCC_TARGET_I386)
    set(LIBTCC_ARCH_SRCS i386-gen.c i386-link.c i386-asm.c)
elseif(TCC_TARGET_ARM)
    set(LIBTCC_ARCH_SRCS arm-gen.c arm-link.c arm-asm.c)
elseif(TCC_TARGET_ARM64)
    set(LIBTCC_ARCH_SRCS arm64-gen.c arm64-link.c arm64-asm.c)
elseif(TCC_TARGET_RISCV64)
    set(LIBTCC_ARCH_SRCS riscv64-gen.c riscv64-link.c riscv64-asm.c)
elseif(TCC_TARGET_C67)
    set(LIBTCC_ARCH_SRCS c67-gen.c c67-link.c tcccoff.c)
endif()

if(TCC_TARGET_PE)
    list(APPEND LIBTCC_ARCH_SRCS tccpe.c)
elseif(TCC_TARGET_MACHO)
    list(APPEND LIBTCC_ARCH_SRCS tccmacho.c)
endif()

add_library(libtcc ${LIBTCC_CORE_SRCS} ${LIBTCC_ARCH_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/tccdefs_.h")
add_dependencies(libtcc tccdefs_h)
target_compile_definitions(libtcc PRIVATE ONE_SOURCE=0)
if(TCC_GITHASH)
    target_compile_definitions(libtcc PRIVATE "TCC_GITHASH=\"${TCC_GITHASH}\"")
endif()
if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_definitions(libtcc PRIVATE LIBTCC_AS_DLL)
endif()
target_include_directories(libtcc
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(libtcc PROPERTIES
    OUTPUT_NAME "tcc"
    SOVERSION 1
    VERSION ${PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE ON
)

find_package(Threads REQUIRED)
target_link_libraries(libtcc PUBLIC Threads::Threads)
if(UNIX)
    target_link_libraries(libtcc PUBLIC ${CMAKE_DL_LIBS} m)
endif()

# Native tcc CLI executable
add_executable(tcc tcc.c)
add_dependencies(tcc tccdefs_h)
target_compile_definitions(tcc PRIVATE ONE_SOURCE=0)
if(TCC_GITHASH)
    target_compile_definitions(tcc PRIVATE "TCC_GITHASH=\"${TCC_GITHASH}\"")
endif()
target_include_directories(tcc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(tcc PRIVATE libtcc)

# Cross compilers
if(TCC_BUILD_CROSS_COMPILERS)
    set(ALL_CROSS_TARGETS "")

    function(add_cross_compiler TGT_NAME)
        set(TGT_DEFS ${ARGN})
        set(CROSS_EXE_NAME "${TGT_NAME}-tcc")
        add_executable(${CROSS_EXE_NAME} tcc.c)
        add_dependencies(${CROSS_EXE_NAME} tccdefs_h)
        target_compile_definitions(${CROSS_EXE_NAME} PRIVATE ONE_SOURCE=1 ${TGT_DEFS})
        if(TCC_GITHASH)
            target_compile_definitions(${CROSS_EXE_NAME} PRIVATE "TCC_GITHASH=\"${TCC_GITHASH}\"")
        endif()
        target_include_directories(${CROSS_EXE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
        target_link_libraries(${CROSS_EXE_NAME} PRIVATE Threads::Threads)
        if(UNIX)
            target_link_libraries(${CROSS_EXE_NAME} PRIVATE ${CMAKE_DL_LIBS} m)
        endif()
        install(TARGETS ${CROSS_EXE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
        set(ALL_CROSS_TARGETS ${ALL_CROSS_TARGETS} ${CROSS_EXE_NAME} PARENT_SCOPE)
    endfunction()

    add_cross_compiler(i386 TCC_TARGET_I386)
    add_cross_compiler(x86_64 TCC_TARGET_X86_64)
    add_cross_compiler(i386-win32 TCC_TARGET_I386 TCC_TARGET_PE)
    add_cross_compiler(x86_64-win32 TCC_TARGET_X86_64 TCC_TARGET_PE)
    add_cross_compiler(x86_64-osx TCC_TARGET_X86_64 TCC_TARGET_MACHO)
    add_cross_compiler(arm TCC_TARGET_ARM TCC_ARM_VFP TCC_ARM_EABI TCC_ARM_HARDFLOAT)
    add_cross_compiler(arm-wince TCC_TARGET_ARM TCC_ARM_VFP TCC_ARM_EABI TCC_ARM_HARDFLOAT TCC_TARGET_PE)
    add_cross_compiler(arm64 TCC_TARGET_ARM64)
    add_cross_compiler(arm64-osx TCC_TARGET_ARM64 TCC_TARGET_MACHO)
    add_cross_compiler(arm64-win32 TCC_TARGET_ARM64 TCC_TARGET_PE)
    add_cross_compiler(riscv64 TCC_TARGET_RISCV64)
    add_cross_compiler(c67 TCC_TARGET_C67)

    add_custom_target(cross DEPENDS ${ALL_CROSS_TARGETS})
endif()

# Build libtcc1.a runtime library and extra runtime objects with tcc
if(TCC_BUILD_LIBTCC1)
    set(LIBTCC1_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/stdatomic.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/atomic.S
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/builtin.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/alloca.S
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/alloca-bt.S
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/tcov.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/dsohandle.c
    )
    if(TCC_TARGET_X86_64)
        list(APPEND LIBTCC1_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/libtcc1.c
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/va_list.c
        )
    elseif(TCC_TARGET_I386)
        list(APPEND LIBTCC1_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/libtcc1.c
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/pic86.S
        )
    elseif(TCC_TARGET_ARM)
        list(APPEND LIBTCC1_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/libtcc1.c
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/armeabi.c
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/armflush.c
        )
    elseif(TCC_TARGET_ARM64)
        list(APPEND LIBTCC1_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/lib-arm64.c
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/armflush.c
        )
    elseif(TCC_TARGET_RISCV64)
        list(APPEND LIBTCC1_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/lib-arm64.c
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/armflush.c
        )
    endif()

    if(TCC_TARGET_PE)
        list(APPEND LIBTCC1_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/chkstk.S
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/crt1.c
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/crt1w.c
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/wincrt1.c
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/wincrt1w.c
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/dllcrt1.c
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/dllmain.c
            ${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/winex.c
        )
    endif()

    set(LIBTCC1_OBJS "")
    foreach(SRC IN LISTS LIBTCC1_SRCS)
        get_filename_component(SRC_NAME ${SRC} NAME_WE)
        set(OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SRC_NAME}.o")
        add_custom_command(
            OUTPUT "${OBJ_FILE}"
            COMMAND $<TARGET_FILE:tcc> -c "${SRC}" -o "${OBJ_FILE}" -B"${CMAKE_CURRENT_BINARY_DIR}" -I"${CMAKE_CURRENT_SOURCE_DIR}/include" -I"${CMAKE_CURRENT_SOURCE_DIR}" -I"${CMAKE_CURRENT_BINARY_DIR}"
            DEPENDS tcc "${SRC}"
            COMMENT "Compiling runtime object ${SRC_NAME}.o with tcc"
        )
        list(APPEND LIBTCC1_OBJS "${OBJ_FILE}")
    endforeach()

    set(EXTRA_RUNTIME_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/runmain.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/bt-exe.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/bt-log.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/bcheck.c
    )
    if(TCC_TARGET_PE)
        list(APPEND EXTRA_RUNTIME_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/lib/bt-dll.c)
    endif()
    set(EXTRA_RUNTIME_OBJS "")
    foreach(SRC IN LISTS EXTRA_RUNTIME_SRCS)
        get_filename_component(SRC_NAME ${SRC} NAME_WE)
        set(OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SRC_NAME}.o")
        set(EXTRA_FLAGS "")
        if(SRC_NAME STREQUAL "bcheck")
            set(EXTRA_FLAGS "-bt")
        endif()
        add_custom_command(
            OUTPUT "${OBJ_FILE}"
            COMMAND $<TARGET_FILE:tcc> -c "${SRC}" -o "${OBJ_FILE}" ${EXTRA_FLAGS} -B"${CMAKE_CURRENT_BINARY_DIR}" -I"${CMAKE_CURRENT_SOURCE_DIR}/include" -I"${CMAKE_CURRENT_SOURCE_DIR}" -I"${CMAKE_CURRENT_BINARY_DIR}"
            DEPENDS tcc "${SRC}"
            COMMENT "Compiling extra runtime object ${SRC_NAME}.o with tcc"
        )
        list(APPEND EXTRA_RUNTIME_OBJS "${OBJ_FILE}")
    endforeach()

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libtcc1.a"
        COMMAND $<TARGET_FILE:tcc> -ar rcs "${CMAKE_CURRENT_BINARY_DIR}/libtcc1.a" ${LIBTCC1_OBJS}
        DEPENDS tcc ${LIBTCC1_OBJS}
        COMMENT "Creating libtcc1.a"
    )
    add_custom_target(libtcc1 ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/libtcc1.a" ${EXTRA_RUNTIME_OBJS})
endif()

# Documentation
if(TCC_BUILD_DOCS)
    find_program(MAKEINFO_PROG makeinfo)
    find_program(POD2MAN_PROG pod2man)
    find_package(Perl)

    set(DOC_TARGETS "")
    if(MAKEINFO_PROG)
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.html"
            COMMAND ${MAKEINFO_PROG} -I "${CMAKE_CURRENT_BINARY_DIR}" --no-split --html --number-sections -o "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.html" "${CMAKE_CURRENT_SOURCE_DIR}/tcc-doc.texi"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tcc-doc.texi" "${CMAKE_CURRENT_BINARY_DIR}/config.texi"
            COMMENT "Building tcc-doc.html"
        )
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.info"
            COMMAND ${MAKEINFO_PROG} -I "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/tcc-doc.texi" -o "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.info" || true
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tcc-doc.texi" "${CMAKE_CURRENT_BINARY_DIR}/config.texi"
            COMMENT "Building tcc-doc.info"
        )
        list(APPEND DOC_TARGETS "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.html" "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.info")
    endif()

    if(PERL_FOUND AND POD2MAN_PROG)
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tcc.pod"
            COMMAND ${PERL_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/texi2pod.pl" "${CMAKE_CURRENT_SOURCE_DIR}/tcc-doc.texi" "${CMAKE_CURRENT_BINARY_DIR}/tcc.pod"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/texi2pod.pl" "${CMAKE_CURRENT_SOURCE_DIR}/tcc-doc.texi"
            COMMENT "Generating tcc.pod"
        )
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tcc.1"
            COMMAND ${POD2MAN_PROG} --section=1 --center="Tiny C Compiler" --release="${TCC_VERSION}" "${CMAKE_CURRENT_BINARY_DIR}/tcc.pod" "${CMAKE_CURRENT_BINARY_DIR}/tcc.1"
            DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/tcc.pod"
            COMMENT "Generating tcc.1 man page"
        )
        list(APPEND DOC_TARGETS "${CMAKE_CURRENT_BINARY_DIR}/tcc.1")
    endif()

    if(DOC_TARGETS)
        add_custom_target(doc ALL DEPENDS ${DOC_TARGETS})
    endif()
endif()

# Tests
if(TCC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
set(TCC_INSTALL_TCCDIR "${CMAKE_INSTALL_LIBDIR}/tcc")
if(WIN32)
    set(TCC_INSTALL_TCCDIR ".")
endif()

install(TARGETS tcc libtcc
    EXPORT TinyCCTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES libtcc.h tcclib.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${TCC_INSTALL_TCCDIR}/include FILES_MATCHING PATTERN "*.h")
install(FILES tcclib.h DESTINATION ${TCC_INSTALL_TCCDIR}/include)

if(TCC_BUILD_LIBTCC1)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libtcc1.a" ${EXTRA_RUNTIME_OBJS} DESTINATION ${TCC_INSTALL_TCCDIR})
endif()

if(WIN32)
    install(DIRECTORY win32/include/ DESTINATION ${TCC_INSTALL_TCCDIR}/include)
    install(DIRECTORY win32/examples/ DESTINATION ${TCC_INSTALL_TCCDIR}/examples)
    install(FILES win32/lib/*.def DESTINATION ${TCC_INSTALL_TCCDIR}/lib)
endif()

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tcc.1")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tcc.1" DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 OPTIONAL)
endif()
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.html")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.html" DESTINATION ${CMAKE_INSTALL_DOCDIR} OPTIONAL)
endif()
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.info")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tcc-doc.info" DESTINATION ${CMAKE_INSTALL_INFODIR} OPTIONAL)
endif()

# Package Config (pkg-config and CMake)
set(PKG_CONFIG_LIBS "")
if(UNIX)
    set(PKG_CONFIG_LIBS "-lm -ldl -lpthread")
endif()
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/libtcc.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/libtcc.pc"
    @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libtcc.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

install(EXPORT TinyCCTargets
    FILE TinyCCTargets.cmake
    NAMESPACE TinyCC::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TinyCC"
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/TinyCCConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/TinyCCConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TinyCC"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/TinyCCConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/TinyCCConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/TinyCCConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TinyCC"
)
