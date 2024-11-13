project(GameModule)

file(GLOB SOURCE_FILES "*.cpp")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC "DLLEXPORT")
target_include_directories(${PROJECT_NAME} PUBLIC "./include")
target_include_directories(${PROJECT_NAME} PUBLIC "../util/dxtk12/include")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_link_directories(${PROJECT_NAME} PUBLIC "../util/dxtk12/native/lib/x64/Debug/")
else()
	target_link_directories(${PROJECT_NAME} PUBLIC "../util/dxtk12/native/lib/x64/Release/")
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC "DirectXTK12.lib")


target_link_libraries(${PROJECT_NAME} PRIVATE RenderModule)
target_include_directories(${PROJECT_NAME} PUBLIC "../render/include")