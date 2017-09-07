#include "mainopenglwidget.h"

void MainOpenGLWidget::initializeGL() {
    // Set up the rendering context, load shaders and other resources, etc.:
    qDebug() << "InitializeGL called";
    QOpenGLContext *c = context();
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &MainOpenGLWidget::cleanup);


    initializeOpenGLFunctions();

    // load volume data file
    QString file = QFileDialog::getOpenFileName(this,
        tr("Load Volume Dataset"), "../Samples", tr("Volume Data Files (*.mhd)"));
    QString fileName = QFileInfo(file).fileName();
    QDir dir = QFileInfo(file).absoluteDir();

    m_width = size().width();
    m_height = size().height();

    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // initialize camera
    m_camera.Init(m_width, m_height);
    // initialize volume data
    m_volumedataset.Init(dir.absolutePath().toStdString(), fileName.toStdString());
    // initialize shaders
    ShaderManager::Init();

    oneStep = true;

    if (m_volumedataset.volumeType == TimeVarying) {
        startTimeStep = m_volumedataset.timesteps / 2;
        currentTimeStep = startTimeStep;
        shaderType = FrameVarianceShader; // default shader
        emit setTimeVarying(true);
    }
    else {
        startTimeStep = 1;
        currentTimeStep = 1;
        shaderType = NV_Shader; // default for time-static data
        emit setTimeVarying(false);
    }

    glGenTextures(1, &TF_textureUnit);
    glBindTexture(GL_TEXTURE_2D, TF_textureUnit);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // initialize renderer
    qDebug() << "to init renderer";
    m_openglrenderer = new OpenGLRenderer(m_width, m_height, m_volumedataset, m_camera, TF_textureUnit);
    qDebug() << "new renderer done";
    m_openglrenderer->UpdateTexture(currentTimeStep, m_volumedataset);

    if (m_volumedataset.volumeType == TimeVarying) {
        // set a timer for animation
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
        timer->start(50);
    }

    { // initial intensity scale=1
        horizontalScale = 1;
        int uniformLoc;
        unsigned int shaderProgramID = ShaderManager::UseShader(shaderType);
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        uniformLoc = f->glGetUniformLocation(shaderProgramID, "intensityScale");
        f->glUniform1f(uniformLoc, horizontalScale);
    }

    initialized = true;
    qDebug() << "OpenGL initialization done";

}

void MainOpenGLWidget::reloadData() {
    initialized = false;

    // load volume data file
    QString file = QFileDialog::getOpenFileName(this,
        tr("Load Volume Dataset"), "../Samples", tr("Volume Data Files (*.mhd)"));
    QString fileName = QFileInfo(file).fileName();
    QDir dir = QFileInfo(file).absoluteDir();

    // initialize volume data
    m_volumedataset.Clean();
    m_volumedataset.Init(dir.absolutePath().toStdString(), fileName.toStdString());

    // initialize shaders
    ShaderManager::Init();

    oneStep = true;
    if (m_volumedataset.volumeType == TimeVarying) {
        startTimeStep = m_volumedataset.timesteps / 2;
        currentTimeStep = startTimeStep;
        emit setTimeVarying(true);
    }
    else {
        startTimeStep = 1;
        currentTimeStep = 1;
        emit setTimeVarying(false);
    }

    glGenTextures(1, &TF_textureUnit);
    glBindTexture(GL_TEXTURE_2D, TF_textureUnit);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    // initialize renderer
    delete m_openglrenderer;
    m_openglrenderer = new OpenGLRenderer(m_width, m_height, m_volumedataset, m_camera, TF_textureUnit);
    m_openglrenderer->UpdateTexture(currentTimeStep, m_volumedataset);

    if (m_volumedataset.volumeType == TimeVarying) {
        // set a timer for animation
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
        timer->start(100);
    }

    { // initial intensity scale=1
        horizontalScale = 1;
        int uniformLoc;
        unsigned int shaderProgramID = ShaderManager::UseShader(shaderType);
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
        uniformLoc = f->glGetUniformLocation(shaderProgramID, "intensityScale");
        f->glUniform1f(uniformLoc, horizontalScale);
    }

    initialized = true;
}

void MainOpenGLWidget::changeStartFrame(int frame) {
    qDebug() << "changeStartFrame";
    if (initialized) {
        startTimeStep = frame;
        currentTimeStep = frame;
        m_openglrenderer->UpdateTexture(currentTimeStep, m_volumedataset);
        makeCurrent();
        paintGL();
        doneCurrent();
    }
}

void MainOpenGLWidget::changeOneStepMode() {
    qDebug() << "changeOneStepMode";
    if (initialized && !oneStep) {
        oneStep = true;
        currentTimeStep = startTimeStep;
        m_openglrenderer->UpdateTexture(currentTimeStep, m_volumedataset);
        makeCurrent();
        paintGL();
        doneCurrent();
    }
}

void MainOpenGLWidget::changeAnimationMode() {
    if (initialized && oneStep) {
        oneStep = false;
        currentTimeStep = startTimeStep;
        m_openglrenderer->UpdateTexture(currentTimeStep, m_volumedataset);
        makeCurrent();
        paintGL();
        doneCurrent();
    }
}

void MainOpenGLWidget::textureUpdated(QImage *img) {
    if (initialized) {
        makeCurrent();
        glBindTexture(GL_TEXTURE_2D, TF_textureUnit);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, img->bits());
        paintGL();
        doneCurrent();
    }
}


void MainOpenGLWidget::updateAnimation() {
    if (initialized && !oneStep) { // animation mode
        if (m_volumedataset.timesteps > 1) {
            clock_t currentTime = clock();
            float time = (currentTime - oldTime) / (float) CLOCKS_PER_SEC;

            if (time > m_volumedataset.timePerFrame)
            {
                if (currentTimeStep < m_volumedataset.timesteps - 1)
                    currentTimeStep++;
                else
                    currentTimeStep = startTimeStep;

                oldTime = currentTime;

                float a = clock();
                m_openglrenderer->UpdateTexture(currentTimeStep, m_volumedataset);
                float b = clock();
                qDebug() << "Update time = " << (b - a) / (float) CLOCKS_PER_SEC << '\n';
            }
        }
    }

    makeCurrent();
    paintGL();
    doneCurrent();
}

void MainOpenGLWidget::setEdgeEnhance(double* enhanceList) {
    makeCurrent();
    GLfloat *temp = new GLfloat[30];

    for (int i=0; i<30; i++) temp[i] = enhanceList[i];

    unsigned int shaderProgramID = ShaderManager::UseShader(shaderType);

    int uniformLoc;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    uniformLoc = f->glGetUniformLocation(shaderProgramID, "enhanceList");
    f->glUniform1fv(uniformLoc, 30, temp);

    delete[] temp;

    paintGL();
    doneCurrent();
}

void MainOpenGLWidget::setIntensityScale(double scale) {
    horizontalScale = scale;
    makeCurrent();
    int uniformLoc;
    unsigned int shaderProgramID = ShaderManager::UseShader(shaderType);
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    uniformLoc = f->glGetUniformLocation(shaderProgramID, "intensityScale");
    f->glUniform1f(uniformLoc, scale);

    paintGL();
    doneCurrent();
}

void MainOpenGLWidget::paintGL() {
    // Draw the scene:
    //qDebug() << "paintGL called";
    m_camera.Update();
    m_openglrenderer->Draw(m_volumedataset, m_camera, shaderType);
    update();
}

void MainOpenGLWidget::resizeGL(int w, int h) {
    qDebug() << "resizeGL called";
    m_camera.Update();
}

void MainOpenGLWidget::cleanup() {
    qDebug() << "cleanup called";
    makeCurrent();
    delete m_texture;
    m_texture = 0;


    doneCurrent();
}

void MainOpenGLWidget::useFrameVarianceShader() {
     shaderType = FrameVarianceShader;
     makeCurrent();
     int uniformLoc;
     unsigned int shaderProgramID = ShaderManager::UseShader(shaderType);
     QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
     uniformLoc = f->glGetUniformLocation(shaderProgramID, "intensityScale");
     f->glUniform1f(uniformLoc, horizontalScale);
     paintGL();
     doneCurrent();
}

void MainOpenGLWidget::useNV_Shader() {
     shaderType = NV_Shader;
     makeCurrent();
     int uniformLoc;
     unsigned int shaderProgramID = ShaderManager::UseShader(shaderType);
     QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
     uniformLoc = f->glGetUniformLocation(shaderProgramID, "intensityScale");
     f->glUniform1f(uniformLoc, horizontalScale);
     paintGL();
     doneCurrent();
}

void MainOpenGLWidget::useGradientShader() {
     shaderType = GradientShader;
     makeCurrent();
     int uniformLoc;
     unsigned int shaderProgramID = ShaderManager::UseShader(shaderType);
     QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
     uniformLoc = f->glGetUniformLocation(shaderProgramID, "intensityScale");
     f->glUniform1f(uniformLoc, horizontalScale);
     paintGL();
     doneCurrent();
}

void MainOpenGLWidget::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::RightButton && oneStep) {
        qDebug() << "Next frame updated";
        makeCurrent();
        if (currentTimeStep < m_volumedataset.timesteps - 1)
            currentTimeStep++;
        else
            currentTimeStep = startTimeStep;
        m_openglrenderer->UpdateTexture(currentTimeStep, m_volumedataset);
        paintGL();
        doneCurrent();
    }
    if (e->button() == Qt::LeftButton) {
        mousePressed = true;
        mouseStartPoint = e->pos();
    }

}

void MainOpenGLWidget::mouseMoveEvent(QMouseEvent *e) {
    if (mousePressed) {
        if (mouseStartPoint.x() >= 0) {
            m_camera.Rotate((float)(e->x() - mouseStartPoint.x())*0.05f);
            mouseStartPoint.setX(e->x());
        }
        if (mouseStartPoint.y() >= 0) {
            m_camera.Translate(glm::vec3(0.0f, -(e->y() - mouseStartPoint.y()) * 0.05f, 0.0f));
            mouseStartPoint.setY(e->y());
        }
        makeCurrent();
        paintGL();
        doneCurrent();
    }
}

void MainOpenGLWidget::mouseReleaseEvent(QMouseEvent *e) {
    mousePressed = false;
}

void MainOpenGLWidget::keyPressEvent(QKeyEvent *e) {
    bool valid = true;
    switch (e->key()) {
    case Qt::Key_Up:
        m_camera.position.z -= 0.5f;
        break;
    case Qt::Key_Down:
        m_camera.position.z += 0.5f;
        break;
    case Qt::Key_Left:
        m_camera.position.x -= 0.5f;
        break;
    case Qt::Key_Right:
        m_camera.position.x += 0.5f;
        break;
    default:
        valid = false;
        break;
    }
    if (valid) {
        qDebug() << "Key pressed";
        makeCurrent();
        paintGL();
        doneCurrent();
    }
}

void MainOpenGLWidget::wheelEvent(QWheelEvent *e) {
    m_camera.Zoom(-e->delta()*0.001f);
    makeCurrent();
    paintGL();
    doneCurrent();
}

