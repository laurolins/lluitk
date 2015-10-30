set(CMAKE_INCLUDE_CURRENT_DIR on)
include_directories(../src)
include_directories(../submodules/llsg/submodules/d3cpp/src)
include_directories(../submodules/llsg/src)

add_executable (example_list example_list.cc)
target_link_libraries(example_list PUBLIC lluitk_core ${GLFW_LIBRARIES} ${FREEIMAGE_LIBRARIES})

add_executable (example_textedit example_textedit.cc)
target_link_libraries(example_textedit PUBLIC lluitk_core ${GLFW_LIBRARIES} ${FREEIMAGE_LIBRARIES})

