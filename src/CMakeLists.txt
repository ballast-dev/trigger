# ▗   ▘        
# ▜▘▛▘▌▛▌▛▌█▌▛▘
# ▐▖▌ ▌▙▌▙▌▙▖▌ 
#      ▄▌▄▌    
project(trigger)

set(CMAKE_C_STANDARD 99) # maximum support!
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX /usr)

add_library(trigger SHARED trigger.c)

set_target_properties(trigger PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    PUBLIC_HEADER trigger.h
)

add_executable(trigger_example example.c)
target_link_libraries(trigger_example trigger)

install(TARGETS trigger
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

install(TARGETS trigger_example
    RUNTIME DESTINATION bin
)

target_include_directories(trigger PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(trigger_example PRIVATE trigger
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(trigger_test example.c)
target_link_libraries(trigger_test trigger)