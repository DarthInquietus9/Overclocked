cmake_minimum_required(VERSION 3.20)
project(TeamOverclocked LANGUAGES CXX)

# Require C++20 standard (modern llama.cpp demands it)
set(CMAKE_CXX_STANDARD 20)
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

# 6. Installation Rule (Crucial so CPack actually grabs the binary!)
install(TARGETS PragmaOnce
    RUNTIME DESTINATION bin
)

# 7. CPack Packaging Configuration
set(CPACK_PACKAGE_NAME "PragmaOnce")
set(CPACK_PACKAGE_VERSION "0.1.1")
set(CPACK_PACKAGE_VENDOR "TeamOverclocked")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;/.github/;.*~")

if(WIN32)
    set(CPACK_GENERATOR "WIX") # Generates Windows MSI installer
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ") # Generates macOS tarball
else()
    set(CPACK_GENERATOR "TGZ") # Generates Linux tarball
endif()

include(CPack)