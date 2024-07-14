cmake_minimum_required(VERSION 3.14)

project(aufgaben C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


set(VCPKG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-linux/include")
set(VCPKG_LIB_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-linux/lib")



add_executable(aufgabe01 aufgabe01.c)
target_include_directories(aufgabe01 PRIVATE ${VCPKG_INCLUDE_DIR})
target_link_directories(aufgabe01 PRIVATE ${VCPKG_LIB_DIR})
target_link_libraries(aufgabe01 fftw3 m)  

add_executable(aufgabe01_kiss aufgabe01_kiss.c)
target_include_directories(aufgabe01_kiss PRIVATE ${VCPKG_INCLUDE_DIR})
target_link_directories(aufgabe01_kiss PRIVATE ${VCPKG_LIB_DIR})
target_link_libraries(aufgabe01_kiss kissfft-float m)  



add_executable(aufgabe02 aufgabe02.c)
target_link_libraries(aufgabe02 m)  


add_executable(aufgabe03 aufgabe03.c)
target_include_directories(aufgabe03 PRIVATE ${VCPKG_INCLUDE_DIR})
target_link_directories(aufgabe03 PRIVATE ${VCPKG_LIB_DIR})
target_link_libraries(aufgabe03 fftw3 fftw3_threads m pthread)


add_executable(aufgabe03_kiss aufgabe03_kiss.c)
target_include_directories(aufgabe03_kiss PRIVATE ${VCPKG_INCLUDE_DIR})
target_link_directories(aufgabe03_kiss PRIVATE ${VCPKG_LIB_DIR})
target_link_libraries(aufgabe03_kiss kissfft-float m pthread)  




add_executable(aufgabe04 aufgabe04.c)
target_include_directories(aufgabe04 PRIVATE ${VCPKG_INCLUDE_DIR})
target_link_directories(aufgabe04 PRIVATE ${VCPKG_LIB_DIR})
target_link_libraries(aufgabe04 m OpenCL)
