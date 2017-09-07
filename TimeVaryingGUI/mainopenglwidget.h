#ifndef MAINOPENGLWIDGET_H
#define MAINOPENGLWIDGET_H

#define GL_GLEXT_PROTOTYPES 1

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QFileDialog>
#include <QString>
#include <QDir>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QPoint>
#include <QImage>
#include <QTimer>
#include <QDebug>


#include "opengl/Camera.h"
#include "opengl/OpenGLRenderer.h"
#include "opengl/ShaderManager.h"
#include <time.h>
#include <iostream>


class MainOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    MainOpenGLWidget(QWidget *parent=0) : QOpenGLWidget(parent)  {}
    ~MainOpenGLWidget(){}

    int startTimeStep = 0;
    int currentTimeStep = 0;
    bool oneStep = true;

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);

signals:
    void setTimeVarying(bool isTimeVarying);

public slots:
    void textureUpdated(QImage* img);
    void changeStartFrame(int frame);
    void changeOneStepMode();
    void changeAnimationMode();
    void updateAnimation();
    void setEdgeEnhance(double *enhanceList);
    void reloadData();
    void useFrameVarianceShader();
    void useNV_Shader();
    void useGradientShader();
    void setIntensityScale(double scale);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    Camera m_camera;
    OpenGLRenderer *m_openglrenderer;
    VolumeDataset m_volumedataset;

    ShaderType shaderType;

    void cleanup();
    QTimer *timer;
    clock_t oldTime;
    int m_width;
    int m_height;
    QOpenGLTexture *m_texture;
    unsigned int TF_textureUnit; // for transfer function
    bool initialized = false;

    bool mousePressed;
    QPoint mouseStartPoint;

    double horizontalScale; // scale for horizontal axis

};

#endif // MAINOPENGLWIDGET_H
