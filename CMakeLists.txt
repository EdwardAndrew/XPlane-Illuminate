cmake_minimum_required(VERSION 2.8)
SET(PROJECT_NAME XPlane-Illuminate)
SET(CMAKE_CONFIGURATION_TYPES Release)
project(${PROJECT_NAME})
SET(XPlane_11_SDK_PATH "C:/XPlaneSDK" CACHE PATH "XPlane 11 SDK Path")
SET(CUESDK_PATH "C:/CUESDK" CACHE PATH "XPlane 11 SDK Path")
SET(NLOHMANN_JSON_PATH "C:/NLOHMANN_JSON_PATH" CACHE PATH "nlohmann json path")
SET(HEADER_FILES
	Condition.h
)
SET(SOURCES
	xplane.cpp
	Condition.cpp
)


include_directories("${XPlane_11_SDK_PATH}/CHeaders/XPLM")
include_directories("${XPlane_11_SDK_PATH}/CHeaders/Widgets")
include_directories("${XPlane_11_SDK_PATH}/CHeaders/Wrappers")
include_directories("${CUESDK_PATH}/include")
include_directories("${NLOHMANN_JSON_PATH}/single_include/nlohmann")
link_directories("${CUESDK_PATH}/lib/x64")
link_directories("${XPlane_11_SDK_PATH}/Libraries/Win")

add_library(${PROJECT_NAME} SHARED xplane.cpp ${HEADER_FILES} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".xpl")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "win")
set_target_properties(${PROJECT_NAME} PROPERTIES
 RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/plugins/${PROJECT_NAME}/64/"
)
target_link_libraries(${PROJECT_NAME} Opengl32)
target_link_libraries(${PROJECT_NAME} odbc32)
target_link_libraries(${PROJECT_NAME} CUESDK.x64_2013)
target_link_libraries(${PROJECT_NAME} odbccp32)
target_link_libraries(${PROJECT_NAME} XPLM_64)
target_link_libraries(${PROJECT_NAME} XPWidgets_64)

add_compile_definitions(IBM=1)
add_compile_definitions(WIN32)
add_compile_definitions(_WINDOWS)
add_compile_definitions(XPLM301)
add_compile_definitions(XPLM300)
add_compile_definitions(XPLM210)
add_compile_definitions(XPLM200)
