set(PUBLIC_HEADER_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${ROOT_DIR}/include/middleware/utils"
    "${ROOT_DIR}/middleware/utils"
)

set(SOURCES_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ai_audio_main.c
)

set(LIBS
    PARENT_SCOPE
)
set(SOURCES "${SOURCES_LIST}" PARENT_SCOPE)
set(PUBLIC_HEADER "${PUBLIC_HEADER_LIST}" PARENT_SCOPE)
