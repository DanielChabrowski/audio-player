add_compile_options(
    -Werror
    -Wall
    -Wextra
    -Wpedantic
    -Wredundant-decls
    -Wnon-virtual-dtor
    -Wnull-dereference
    -Wzero-as-null-pointer-constant

    $<$<CXX_COMPILER_ID:GNU>:-Wuseless-cast>
    $<$<CXX_COMPILER_ID:GNU>:-Wsuggest-override>
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-branches>
    $<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>
)
