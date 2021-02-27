cmake_minimum_required(VERSION 3.0.0)
project(game_project CXX C)


find_package(SDL2 REQUIRED)
add_library(engine SHARED   src/engine.cpp
                            include/engine.hpp
                            src/figure_struct.cpp
                            include/figure_struct.hpp
                            src/shader.cpp
                            include/shader.hpp
                            src/glad.c
                            include/glad/glad.h
                            include/KHR/khrplatform.h
                            )
                            
target_include_directories(engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(engine PUBLIC cxx_std_17)

target_link_libraries(engine PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(engine PRIVATE GL)


add_executable(game src/game.cpp)
target_compile_features(game PUBLIC cxx_std_17)
target_link_libraries(game PRIVATE engine)

file(COPY res/vertexes.txt DESTINATION ./res/)
file(COPY shader/test.vert DESTINATION ./shader/)
file(COPY shader/test.frag DESTINATION ./shader/)
file(COPY shader/test2.vert DESTINATION ./shader/)
file(COPY shader/test2.frag DESTINATION ./shader/)

# Install
install(TARGETS game
        RUNTIME DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin
        LIBRARY DESTINATION ${CMAKE_CURRENT_LIST_DIR}/lib
        ARCHIVE DESTINATION ${CMAKE_CURRENT_LIST_DIR}/lib)
install(FILES  resource/Star.ppm  DESTINATION ${CMAKE_CURRENT_LIST_DIR}/bin)

