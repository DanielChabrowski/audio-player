if(SANITIZE_THREAD AND SANITIZE_ADDRESS)
    message(
        FATAL_ERROR
        "AddressSanitizer is not compatible with ThreadSanitizer."
    )
endif()

if(SANITIZE_ADDRESS)
    message(STATUS "AddressSanitizer enabled")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined")
    add_compile_options("-fno-sanitize=signed-integer-overflow")
endif()

if(SANITIZE_THREAD)
    message(STATUS "ThreadSanitizer enabled")
    set(SANITIZER_FLAGS "-fsanitize=thread")
endif()

if(SANITIZE_THREAD OR SANITIZE_ADDRESS)
    add_compile_options(${SANITIZER_FLAGS})
    add_compile_options("-fno-sanitize-recover=all")
    add_compile_options("-fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS} -fuse-ld=gold"
    )
endif()
