if(SANITIZE_THREAD AND SANITIZE_ADDRESS)
    message(
        FATAL_ERROR
        "AddressSanitizer is not compatible with ThreadSanitizer."
    )
endif()

if(SANITIZE_ADDRESS)
    message(STATUS "AddressSanitizer enabled")
    set(SANITIZER_FLAGS "-fsanitize=address,undefined")
endif()

if(SANITIZE_THREAD)
    message(STATUS "ThreadSanitizer enabled")
    set(SANITIZER_FLAGS "-fsanitize=thread")
endif()

if(SANITIZE_THREAD OR SANITIZE_ADDRESS)
    add_compile_options(
        ${SANITIZER_FLAGS}
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer
    )

    add_link_options(${SANITIZER_FLAGS})
endif()
