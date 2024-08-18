project(RenderModule)

#add_subdirectory("DirectX-Headers")

file(GLOB SOURCE_FILES "*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC "DLLEXPORT")
target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PUBLIC "./DirectX-Headers/include/directx")
target_include_directories(${PROJECT_NAME} PUBLIC "../util/DirectXMath/inc")
target_link_libraries(${PROJECT_NAME} PRIVATE "d3d12.lib" "dxgi.lib" "d3dcompiler.lib" "dxguid.lib")

add_custom_target(RenderShader)
add_dependencies(${PROJECT_NAME} RenderShader)
add_custom_command(
         TARGET RenderShader POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
         "${CMAKE_CURRENT_SOURCE_DIR}/shader"
         "${CMAKE_BINARY_DIR}/bin/shader"
         COMMENT "Copying shaders" VERBATIM
         )


add_custom_target(AgilityDX12)
add_dependencies(${PROJECT_NAME} AgilityDX12)
add_custom_command(
         TARGET AgilityDX12 POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
         "${CMAKE_CURRENT_SOURCE_DIR}/D3d12-Agility/bin"
         "${CMAKE_BINARY_DIR}/bin/D3D12"
         COMMENT "Copying agility sdk" VERBATIM
         )