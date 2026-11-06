cmake_minimum_required(VERSION 3.20)
project(TeamOverclocked LANGUAGES CXX)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/llama.cpp)

# Enable Qt MOC (Meta-Object Compiler) processing automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 1. Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# 2. Find PostgreSQL C++ library (libpqxx) using PkgConfig
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

# 3. Executable definition
add_executable(PragmaOnce
        main.cpp
        SQLSecurityGuard.hpp
        MainWindow.hpp
        ConnectionDialog.hpp
        DatabaseManager.hpp
        DataExporter.hpp
        EducationalParser.hpp
        FocusRulerTextEdit.hpp
        LlamaEngine.hpp
        SQLSyntaxHighlighter.hpp
        ResilienceEngine.hpp
        ThemeManager.hpp
)

# 4. Target Include Directories
target_include_directories(PragmaOnce PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/external/llama.cpp/
        ${CMAKE_CURRENT_SOURCE_DIR}/external/llama.cpp/include
)

# 5. Link Libraries
target_link_libraries(PragmaOnce PRIVATE
        Qt6::Widgets
        Qt6::Network
        PkgConfig::PQXX
        llama
)
include(CPack)
set(CPACK_GENERATOR "TGZ")