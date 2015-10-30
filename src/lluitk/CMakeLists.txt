set(CMAKE_INCLUDE_CURRENT_DIR on)
include_directories(../../submodules/llsg/src)
include_directories(../../submodules/llsg/submodules/d3cpp/src)

add_library (lluitk_core STATIC
app.cc
canvas.cc
event.cc
grid.cc
os.cc
simple_widget.cc
style.cc
textedit.cc
widget.cc)

target_link_libraries(lluitk_core llsg_core ${GLFW_LIBRARIES} ${FREEIMAGE_LIBRARIES})