project(msg_queue)

include_directories(
    include
)

file(GLOB HEAD_FILES ./*.h)

add_executable(receiver tests/receiver.cpp ${HEAD_FILES})

target_link_libraries(receiver ipc)

add_executable(sender tests/sender.cpp ${HEAD_FILES})

target_link_libraries(sender ipc)
