include_directories(${CMAKE_SOURCE_DIR})

add_executable(lighting
	lighting/main.cpp)
target_link_libraries(lighting thrender SDL2 assimp boost_system boost_chrono)

add_executable(split_buffers
	split_buffers/main.cpp)
target_link_libraries(split_buffers thrender SDL2 assimp boost_system boost_chrono)
