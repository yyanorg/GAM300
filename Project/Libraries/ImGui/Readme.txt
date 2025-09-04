Project > Properties > C/C++ > General > Additional Include Directories: $(SolutionDir)Libraries\ImGui
Project > Properties > Linker > General > Additional Library Directories: $(SolutionDir)Libraries\ImGui
Project > Properties > Linker > Input > Additional Dependencies: imgui.lib

cl /c /EHsc /I. imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp
cl /c /EHsc /I. /I"..\glfw\include" imgui_impl_opengl3.cpp imgui_impl_glfw.cpp
cl /c /EHsc /I. GraphEditor.cpp ImCurveEdit.cpp ImGradient.cpp ImGuizmo.cpp ImSequencer.cpp
lib /out:imgui.lib /machine:x64 imgui.obj imgui_draw.obj imgui_tables.obj imgui_widgets.obj imgui_impl_opengl3.obj imgui_impl_glfw.obj GraphEditor.obj ImCurveEdit.obj ImGradient.obj ImGuizmo.obj ImSequencer.obj
del imgui.obj imgui_draw.obj imgui_tables.obj imgui_widgets.obj imgui_impl_opengl3.obj imgui_impl_glfw.obj GraphEditor.obj ImCurveEdit.obj ImGradient.obj ImGuizmo.obj ImSequencer.obj

