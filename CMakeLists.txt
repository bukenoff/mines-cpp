# Set the minimum required CMake version.
cmake_minimum_required(VERSION 3.10)

# Define your project.
project(mines LANGUAGES CXX)

# Find the ncurses package provided by vcpkg.
# find_package(ncurses CONFIG REQUIRED)
find_package(Curses REQUIRED)

# Add your executable from the source file.
add_executable(${PROJECT_NAME} main.cpp)

# Link your executable to the ncurses library.
# The 'ncurses::ncurses' target is created by find_package.
# target_link_libraries(${PROJECT_NAME}
#     PRIVATE
#         ncurses::ncurses
# )

target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
