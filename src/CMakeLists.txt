project(Binrary)

#modules
add_subdirectory("launch")
add_subdirectory("render")
add_subdirectory("game")

file(GLOB SOURCE_FILES "*.cpp")
add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
#target_link_options(${PROJECT_NAME} PRIVATE /NATVIS:${CMAKE_CURRENT_SOURCE_DIR}/STL.natvis)

#define
target_compile_definitions(${PROJECT_NAME} PRIVATE "DLLIMPORT")
#add dll library
target_link_libraries(${PROJECT_NAME} PUBLIC LaunchModule)
target_link_libraries(${PROJECT_NAME} PUBLIC RenderModule)
target_link_libraries(${PROJECT_NAME} PUBLIC GameModule)