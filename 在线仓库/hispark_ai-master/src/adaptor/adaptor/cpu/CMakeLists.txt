set(COMPONENT_NAME "ai_adaptor_cpu")

if(EXISTS "${ROOT_DIR}/middleware/utils/ai_mcu/lib")
set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/env.c
    ${CMAKE_CURRENT_SOURCE_DIR}/context.c
    ${CMAKE_CURRENT_SOURCE_DIR}/model.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tensor.c
)

set(PUBLIC_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${ROOT_DIR}/middleware/utils/ai_mcu/adaptor/cpu/include"
    "${ROOT_DIR}/include/middleware/utils"
)

set(LIBS
    "${ROOT_DIR}/middleware/utils/ai_mcu/lib/libmicro_runtime.a"
    "${ROOT_DIR}/middleware/utils/ai_mcu/lib/libnet.a"
)
endif()

# use this when you want to add ccflags like -include xxx
set(COMPONENT_PUBLIC_CCFLAGS
)

set(COMPONENT_CCFLAGS
)

set(WHOLE_LINK
    true
)

set(MAIN_COMPONENT
    false
)

build_component()
install_sdk(${CMAKE_CURRENT_SOURCE_DIR} "*.h")