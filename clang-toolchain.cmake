set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_AR      "llvm-ar")
set(CMAKE_LINKER  "llvm-ld")
set(CMAKE_NM      "llvm-nm")
set(CMAKE_OBJDUMP "llvm-objdump")
set(CMAKE_RANLIB  "llvm-ranlib")


set(CMAKE_C_FLAGS_INIT                "-march=native -fstack-protector-strong -Wall -Wextra -Wno-unused-parameter -Wno-missing-braces -std=c11")
set(CMAKE_C_FLAGS_DEBUG_INIT          "-g")
set(CMAKE_C_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE_INIT        "-flto=thin -O3 -D_FORTIFY_SOURCE=2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-flto=thin -O3 -D_FORTIFY_SOURCE=2 -g")

set(CMAKE_CXX_FLAGS_INIT                "-march=native -fstack-protector-strong -Wall -Wextra -Wno-unused-parameter -Wno-missing-braces -std=c++20")
set(CMAKE_CXX_FLAGS_DEBUG_INIT          "-g")
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT        "-flto=thin -O3 -D_FORTIFY_SOURCE=2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-flto=thin -O3 -D_FORTIFY_SOURCE=2 -g")

set(CMAKE_LD_FLAGS_INIT    "-fuse-ld=lld -rtlib=compiler-rt -unwindlib=libunwind -Wl,--as-needed -Wl,-z,relro,-z,now")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_LD_FLAGS_INIT})
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_LD_FLAGS_INIT})
set(CMAKE_MODULE_LINKER_FLAGS ${CMAKE_LD_FLAGS_INIT})
