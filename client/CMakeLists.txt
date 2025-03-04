cmake_minimum_required(VERSION 3.16)

project(WeChat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_PREFIX_PATH E:/Qt/Qt/6.8.1/mingw_64)

set(app_icon_resource "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")

# 设定源文件路径
set(TargetConfig "${CMAKE_SOURCE_DIR}/config.ini")

# 设定目标路径
set(OutputDir "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        httpmgr.cpp
        httpmgr.h
        logindialog.h logindialog.cpp logindialog.ui
        rc.qrc
        registerdialog.h registerdialog.cpp registerdialog.ui
        global.h global.cpp
        singleton.h
        timerbtn.h timerbtn.cpp
        clickvisible.h clickvisible.cpp
        clicklabel.h clicklabel.cpp
        resetdialog.h resetdialog.cpp resetdialog.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(WeChat
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${app_icon_resource}
        tcpmgr.h tcpmgr.cpp
        usermgr.h usermgr.cpp
        chatdialog.h chatdialog.cpp chatdialog.ui
        clickedbtn.h clickedbtn.cpp
        customizeedit.h customizeedit.cpp
        chatuserlist.h chatuserlist.cpp
        chatuserwid.h chatuserwid.cpp chatuserwid.ui
        listitembase.h listitembase.cpp
        loadingdlg.h loadingdlg.cpp loadingdlg.ui
        chatpage.h chatpage.cpp chatpage.ui
        chatview.h chatview.cpp
        chatitembase.h chatitembase.cpp
        bubbleframe.h bubbleframe.cpp
        textbubble.h textbubble.cpp
        picturebubble.h picturebubble.cpp
        messagetextedit.h messagetextedit.cpp
        statewidget.h statewidget.cpp
        searchlist.h searchlist.cpp
        userdata.h userdata.cpp
        adduseritem.h adduseritem.cpp adduseritem.ui
        findsuccessdlg.h findsuccessdlg.cpp findsuccessdlg.ui
        applyfriend.h applyfriend.cpp applyfriend.ui
        clickedoncelabel.h clickedoncelabel.cpp
        friendlabel.h friendlabel.cpp friendlabel.ui
        contactuserlist.h contactuserlist.cpp
        conuseritem.h conuseritem.cpp conuseritem.ui
        grouptipitem.h grouptipitem.cpp grouptipitem.ui
        applyfriendpage.h applyfriendpage.cpp applyfriendpage.ui
        applyfrienditem.h applyfrienditem.cpp applyfrienditem.ui
        applyfriendlist.h applyfriendlist.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET WeChat APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(WeChat SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(WeChat
            ${PROJECT_SOURCES}
            ${app_icon_resource}
        )
    endif()
endif()

target_link_libraries(WeChat PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.WeChat)
endif()
set_target_properties(WeChat PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS WeChat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 添加构建后命令
add_custom_command(
    TARGET WeChat
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TargetConfig}" "${OutputDir}/config.ini"
    COMMENT "Copying config.ini to output directory"
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(WeChat)
endif()
