cmake_minimum_required(VERSION 3.16)

project(ExperimentTable VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ВАЖНО: Добавляем PrintSupport
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)

# ВАЖНО: Добавляем текущую директорию в include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    TableModel.h
    TableModel.cpp
    InstrumentsModel.h
    InstrumentsModel.cpp
    AbsoluteInstrument.h
    AbsoluteInstrument.cpp
    CombinedInstrument.h
    CombinedInstrument.cpp
    Variable.h
    Variable.cpp
    Experiment.h
    Experiment.cpp
    RelativeInstrument.h
    RelativeInstrument.cpp
    Instrument.h
    Instrument.cpp
    qcustomplot.h
    qcustomplot.cpp
    reportdialog.h
    reportdialog.cpp
    reportdialog.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ExperimentTable
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    # ВАЖНО: Явно указываем include директории для target
    target_include_directories(ExperimentTable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET ExperimentTable APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ExperimentTable SHARED
            ${PROJECT_SOURCES}
        )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ExperimentTable
            ${PROJECT_SOURCES}
        )
        # ВАЖНО: Для Qt5 тоже добавляем include директории
        target_include_directories(ExperimentTable PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endif()

# ВАЖНО: Добавляем PrintSupport к линковке
target_link_libraries(ExperimentTable PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ExperimentTable)
endif()

set_target_properties(ExperimentTable PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ExperimentTable
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ExperimentTable)
endif()
