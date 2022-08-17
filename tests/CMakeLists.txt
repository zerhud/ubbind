cmake_minimum_required(VERSION 3.1)
project(uvbind_tests LANGUAGES CXX VERSION 0.0.0)
set(CMAKE_CXX_STANDARD 20)

set(test_list tcp udp)

enable_testing()
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
foreach(name ${test_list})
	add_executable(${name} src/${name}.cpp)
	target_link_libraries(${name} PRIVATE uvbind ${Boost_LIBRARIES})
	add_test(${name}_test ${name})
endforeach()

include(CTest)
