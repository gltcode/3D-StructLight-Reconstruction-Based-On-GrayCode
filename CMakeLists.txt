cmake_minimum_required(VERSION 2.8)

PROJECT(Camera)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Widgets) 
find_package(Qt5Core)
find_package(Qt5Gui)
FILE(GLOB UI_FILES "./*.ui")
qt5_wrap_ui(WRAP_FILES ${UI_FILES})
source_group("Ui" FILES ${UI_FILES} ${WRAP_FILES} )


#opencv
FIND_PACKAGE(OpenCV  REQUIRED)


#vtk
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
link_directories(${VTK_LIBRARY_DIRS})

include_directories( ${OPENCV_INCLUDE_DIR} 
	./Camera
        ./
		${PROJECT_BINARY_DIR}
)
# Set (SRC_FILES main.cpp Camera.cpp mainwindow.cpp)
# Set (HEAD_FILES Camera.h mainwindow.h)
FILE(GLOB SRC_FILES "./*.cpp")
FILE(GLOB HEAD_FILES "./*.h")
ADD_EXECUTABLE(Camera ${SRC_FILES} ${HEAD_FILES} ${RCC_FILES} ${WRAP_FILES})


TARGET_LINK_LIBRARIES( Camera
	${OpenCV_LIBS} ${VTK_LIBRARIES} Qt5::Widgets Qt5::Core Qt5::Gui
)


