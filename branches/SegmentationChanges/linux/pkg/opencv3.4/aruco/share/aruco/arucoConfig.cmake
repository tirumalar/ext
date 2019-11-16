# ===================================================================================
#  aruco CMake configuration file
#
#             ** File generated automatically, do not modify **
#
#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    FIND_PACKAGE(aruco REQUIRED )
#    TARGET_LINK_LIBRARIES(MY_TARGET_NAME )
#
#    This file will define the following variables:
#      - aruco_LIBS          : The list of libraries to links against.
#      - aruco_LIB_DIR       : The directory where lib files are. Calling LINK_DIRECTORIES
#                                with this path is NOT needed.
#      - aruco_VERSION       : The  version of this PROJECT_NAME build. Example: "1.2.0"
#      - aruco_VERSION_MAJOR : Major version part of VERSION. Example: "1"
#      - aruco_VERSION_MINOR : Minor version part of VERSION. Example: "2"
#      - aruco_VERSION_PATCH : Patch version part of VERSION. Example: "0"
#
# ===================================================================================
INCLUDE_DIRECTORIES("/home/eyelock/extworkspace/arucolibs/include")
INCLUDE_DIRECTORIES("/home/eyelock/extworkspace/arucolibs/include/aruco")
SET(aruco_INCLUDE_DIRS "/home/eyelock/extworkspace/arucolibs/include")

LINK_DIRECTORIES("/home/eyelock/extworkspace/arucolibs/lib")
SET(aruco_LIB_DIR "/home/eyelock/extworkspace/arucolibs/lib")

SET(aruco_LIBS opencv_superres;opencv_photo;opencv_video;opencv_videostab;opencv_highgui;opencv_calib3d;opencv_features2d;opencv_dnn;opencv_shape;opencv_objdetect;opencv_imgcodecs;opencv_ml;opencv_stitching;opencv_videoio;opencv_flann;opencv_core;opencv_imgproc aruco) 

SET(aruco_FOUND YES)
SET(aruco_FOUND "YES")
SET(aruco_VERSION        3.0.10)
SET(aruco_VERSION_MAJOR  3)
SET(aruco_VERSION_MINOR  0)
SET(aruco_VERSION_PATCH  10)
