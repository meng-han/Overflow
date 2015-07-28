#-------------------------------------------------
#
# Project created by QtCreator 2014-10-22T11:10:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Overflow
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    videothread.cpp \
    WeightedMovingVarianceBGS.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    videothread.h \
    WeightedMovingVarianceBGS.h \
    IBGS.h \
    aboutdialog.h

FORMS    += mainwindow.ui \
    aboutdialog.ui
RC_FILE = overflowLogo.rc

INCLUDEPATH += C:\\OpenCV-2.4.9-build\\opencv\\build\\include
#------------------------------------------------------------
# LIBS for debug version
# Need to delete them when switching to Release version build
#------------------------------------------------------------
Debug {
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_calib3d249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_core249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_highgui249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_video249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_contrib249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_features2d249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_flann249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_gpu249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_imgproc249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_ml249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_objdetect249d.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Debug\\opencv_ts249d.lib
}

#------------------------------------------------------------
# LIBS for Release version
#------------------------------------------------------------

Release{
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_calib3d249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_core249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_highgui249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_video249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_contrib249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_features2d249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_flann249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_gpu249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_imgproc249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_ml249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_objdetect249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_ts249.lib
LIBS += C:\\OpenCV-2.4.9-build\\mybuild\\lib\\Release\\opencv_videostab249.lib
}

RESOURCES += \
    Resources.qrc
