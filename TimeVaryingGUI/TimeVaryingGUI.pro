#-------------------------------------------------
#
# Project created by QtCreator 2017-08-10T09:36:01
#
#-------------------------------------------------

QT       += core gui
QT       += widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TimeVaryingGUI
TEMPLATE = app

INCLUDEPATH += ..

LIBS   += -lopengl32
LIBS   += -lglu32
LIBS   += -lglut32

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    imageeditor.cpp \
    mainopenglwidget.cpp \
    opengl/Camera.cpp \
    opengl/GPURaycaster.cpp \
    opengl/OpenGLRenderer.cpp \
    opengl/Shader.cpp \
    opengl/ShaderManager.cpp \
    opengl/VolumeDataset.cpp \
    opengl/VoxelReader.cpp \
    enhancesettingbox.cpp \
    opacitysettingbox.cpp \
    templatedialog.cpp \
    previewarea.cpp

HEADERS += \
        mainwindow.h \
    imageeditor.h \
    mainopenglwidget.h \
    opengl/Camera.h \
    opengl/GLM.h \
    opengl/GPURaycaster.h \
    opengl/OpenGLRenderer.h \
    opengl/Shader.h \
    opengl/ShaderManager.h \
    opengl/VolumeDataset.h \
    opengl/VoxelReader.h \
    enhancesettingbox.h \
    opacitysettingbox.h \
    templatedialog.h \
    previewarea.h

FORMS += \
        mainwindow.ui

DISTFILES += \
    CMakeLists.txt \
    ../build-TimeVaryingGUI/TransFuncVertShader.txt \
    ../build-TimeVaryingGUI/FrameVarianceShader.txt \
    ../build-TimeVaryingGUI/NV_Shader.txt \
    ../build-TimeVaryingGUI/GradientShader.txt

RESOURCES += \
    resources.qrc
