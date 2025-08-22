cmake_minimum_required(VERSION 3.16)

project(thread_cpp_qt VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)

# set(CMAKE_FIND_DEBUG_MODE 1)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        OpenGLWidget.cpp
        OpenGLWidget.hpp
)

add_subdirectory(EGL_Component)
include_directories( ${PROJECT_SOURCE_DIR}/EGL_Component )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(thread_cpp_qt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

else()
    if(ANDROID)
        add_library(thread_cpp_qt SHARED
            ${PROJECT_SOURCES}
        )

    else()
        add_executable(thread_cpp_qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(thread_cpp_qt PRIVATE 
                    Qt${QT_VERSION_MAJOR}::Widgets
                    Qt6::Core
                    Qt6::OpenGLWidgets
                    EGL_Component
                    )


if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.thread_cpp_qt)
endif()
set_target_properties(thread_cpp_qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS thread_cpp_qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(thread_cpp_qt)
endif()
