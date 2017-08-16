cmake_minimum_required(VERSION 3.9)

project("networkmanager" LANGUAGES CXX)

add_definitions(-DQT_NO_KEYWORDS)

#add_definitions(-DQT_NO_INFO_OUTPUT)
#add_definitions(-DQT_NO_DEBUG_OUTPUT)
#add_definitions(-DQT_NO_WARNING_OUTPUT)

execute_process(COMMAND git describe --always --tags --dirty --abbrev=40
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE PROJECT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(-DUNICODE -DPROJECT_VERSION="${PROJECT_VERSION}")

add_compile_options(-W)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options(-Weverything)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wsuggest-override)
else()
    message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} currently is not supported")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG=1 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -fno-inline -DDEBUG=1 -march=x86-64 -mtune=generic")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -D_GLIBCXX_DEBUG -D_LIBCPP_DEBUG2=1")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -D_GLIBCXX_DEBUG")
endif()
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -DNDEBUG=1 -march=native")
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -gline-tables-only")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Og -ggdb")
endif()
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os -DNDEBUG=1 -march=native")

add_definitions(-DORGANIZATION_NAME="Example Inc.")
add_definitions(-DORGANIZATION_DOMAIN="example.org")
add_definitions(-DPACKAGE_NAME="org.example")

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_RESTRICTED_CAST_FROM_ASCII)
#add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)
#add_definitions(-DQT_NO_PROCESS_COMBINED_ARGUMENT_START)
add_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

add_definitions(-DQT_MESSAGELOGCONTEXT)

# set OPENSSL_ROOT_DIR
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick DBus LinguistTools)

# apt-get install libdbus-1-dev libglib2.0-dev libnm-dev network-manager-dev
find_package(PkgConfig REQUIRED)
pkg_search_module(NM REQUIRED NetworkManager)
pkg_search_module(DBUS REQUIRED dbus-1)
pkg_search_module(GLIB REQUIRED glib-2.0)

set(HEADERS)
list(APPEND HEADERS "networkmanager.hpp")
list(APPEND HEADERS "networkmanagerinterface.hpp")

set(SOURCES)
list(APPEND SOURCES "main.cpp")
list(APPEND SOURCES "networkmanager.cpp")
list(APPEND SOURCES "networkmanagerinterface.cpp")

qt5_add_resources(RESOURCES "${PROJECT_NAME}.qrc")

qt5_create_translation(QM_FILES
    ${PROJECT_SOURCE_DIR} "${PROJECT_NAME}.ru_RU.ts"
    OPTIONS -I ${PROJECT_SOURCE_DIR} -source-language en_US -locations relative)

add_executable(${PROJECT_NAME} ${OS_BUNDLE} ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES} ${QM_FILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE -DPROJECT_NAME="${PROJECT_NAME}")

target_link_libraries(${PROJECT_NAME} PRIVATE ${NM_LIBRARIES} ${DBUS_LIBRARIES} ${GLIB_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${NM_INCLUDE_DIRS} ${DBUS_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto ${CMAKE_DL_LIBS})

qt5_use_modules(${PROJECT_NAME} LINK_PRIVATE Core Gui Qml Quick DBus LinguistTools)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
    )

