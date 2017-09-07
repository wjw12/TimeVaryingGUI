#include "imageeditor.h"

// colorspace transform
double rgb2xyz[3][3] = {
    {0.5767309,  0.1855540,  0.1881852},
    {0.2973769,  0.6273491,  0.0752741},
    {0.0270343,  0.0706872,  0.9911085}
};

double xyz2rgb[3][3] = {
    {2.0413690, -0.5649464, -0.3446944},
    {-0.9692660,  1.8760108,  0.0415560},
    {0.0134474, -0.1183897,  1.0154096}
};

double* transformRgbaColor (double mat[3][3], double c[4]) {
    double* result = new double[4];
    for (int i=0; i<3; i++) {
        result[i] = 0.;
        for (int j=0; j<3; j++) result[i] += mat[i][j]*c[j];
    }
    result[3] = c[3]; //alpha
    return result;
}

ImageEditor::ImageEditor(QWidget *parent): QWidget(parent)
{
    QPixmap pm(32, 32);
    QPainter background_painter(&pm);

    // paint a checkerboard background
    background_painter.fillRect(0,0,16,16, Qt::lightGray);
    background_painter.fillRect(16,16,16,16, Qt::lightGray);
    background_painter.fillRect(0,16,16,16, Qt::darkGray);
    background_painter.fillRect(16,0,16,16, Qt::darkGray);
    background_painter.end();

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush(pm));
    setAutoFillBackground(true);
    setPalette(pal);

    img = QImage(256, 256, QImage::Format_RGBA8888);
    img.fill(0);

    initializeRectangles();
    // to show the image in widget, use paintEvent

    for (int i=0; i<10; i++) {
        edgeEnhanceList[3*i] = 0.;
        edgeEnhanceList[3*i+1] = 0.;
        edgeEnhanceList[3*i+2] = 1.;
    }
}

void ImageEditor::initializeRectangles() {
    int w = 256;
    int h = 256; // be aware of hard-coding!

    std::vector<QPointF> ps = std::vector<QPointF>(4);
    std::vector<QColor> cs = std::vector<QColor>(4);
    ps[0] = QPointF(0.18, 0.02); cs[0] = QColor(255,0,0,10);
    ps[1] = QPointF(0.18, 0.8); cs[1] = QColor(255,0,0,10);
    ps[2] = QPointF(0.125, 0.8); cs[2] = QColor(255,255,0,10);
    ps[3] = QPointF(0.125, 0.02); cs[3] = QColor(255,255,0,10);
    RectangleControl *r = new RectangleControl(ps, cs, w, h);
    rectangleList.push_back(*r);


    ps[0] = QPointF(0.53, 0.02); cs[0] = QColor(0,0,255,10);
    ps[1] = QPointF(0.53, 0.97); cs[1] = QColor(0,0,255,10);
    ps[2] = QPointF(0.23, 0.97); cs[2] = QColor(0,255,0,10);
    ps[3] = QPointF(0.23, 0.02); cs[3] = QColor(0,255,0,10);
    r = new RectangleControl(ps, cs, w, h);
    rectangleList.push_back(*r);


    ps[0] = QPointF(0.98, 0.02); cs[0] = QColor(50,255,50,10);
    ps[1] = QPointF(0.98, 0.97); cs[1] = QColor(50,255,50,10);
    ps[2] = QPointF(0.86, 0.97); cs[2] = QColor(0,100,255,10);
    ps[3] = QPointF(0.86, 0.02); cs[3] = QColor(0,100,255,10);
    r = new RectangleControl(ps, cs, w, h);
    rectangleList.push_back(*r);
}

void ImageEditor::paintEvent(QPaintEvent *e) {
    img.fill(0);
    QPainter imagePainter(&img);
    drawRectangles(imagePainter);
    QPainter p(this);
    if (improveOpacity) {
        int w = img.size().width();
        int h = img.size().height();
        QImage newimg(w, h, QImage::Format_RGBA8888);
        for (int i=0; i<w; i++)
            for (int j=0; j<h; j++) {
                QColor c = img.pixelColor(i, j);
                c.setAlpha(c.alpha()*2+50);
                newimg.setPixelColor(i, j, c);
            }
        p.drawImage(0, 0, newimg);
    }
    else p.drawImage(0, 0, img);

    if (drawing) {
        // if we are drawing a rectangle, draw it grey temporarily
        p.setBrush(QColor(128, 128, 128, 128));
        p.drawRect(QRect(startPoint, endPoint));
    }

    // emit signal to pass the image
    qDebug() << "Texture update emitted";
    emit imageUpdated(&img);
}

void ImageEditor::drawRectangles(QPainter &p) {
    for (int i=0; i<rectangleList.size(); i++) {
        auto r = rectangleList[i];

        // get the geometry of destination rectangle
        QPoint topLeft = r.vertices[3];
        QPoint bottomRight = r.vertices[1];
        QRect destRect = QRect(topLeft.x(), topLeft.y(), \
                               bottomRight.x() - topLeft.x(), \
                               bottomRight.y() - topLeft.y());

        if (r.rectangleType == UserDefined) {
            // use a smaller image to perform bilinear interpolation in xyz colorspace
            QImage smallImg(10,10, QImage::Format_RGBA8888);
            double *c1,*c2,*c3,*c0,*c, t[4];
            c0 = new double[4];
            c1 = new double[4];
            c2 = new double[4];
            c3 = new double[4];
            r.colors[0].getRgbF(c0, c0+1, c0+2, c0+3);
            r.colors[1].getRgbF(c1, c1+1, c1+2, c1+3);
            r.colors[2].getRgbF(c2, c2+1, c2+2, c2+3);
            r.colors[3].getRgbF(c3, c3+1, c3+2, c3+3);
            c0 = transformRgbaColor(rgb2xyz, c0);
            c1 = transformRgbaColor(rgb2xyz, c1);
            c2 = transformRgbaColor(rgb2xyz, c2);
            c3 = transformRgbaColor(rgb2xyz, c3);

            // interpolate
            for (int i=0; i<10; i++)
                for (int j=0; j<10; j++) {
                    for (int k=0; k<4; k++) {
                        double x1 = (double) i / 9.;
                        double x2 = 1. - x1;
                        double y1 = (double) j / 9.;
                        double y2 = 1. - y1;
                        t[k] = (c3[k]*x2 + c0[k]*x1)*y2 + (c2[k]*x2 + c1[k]*x1)*y1;
                    }
                    c = transformRgbaColor(xyz2rgb, t);
                    QColor qc = QColor();
                    qc.setRgbF(qBound(0., c[0], 1.), qBound(0., c[1], 1.), qBound(0., c[2], 1.), qBound(0., c[3], 1.));
                    smallImg.setPixelColor(i, j, qc);
                }


            p.setOpacity(1.);
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);
            p.drawImage(destRect, smallImg);

            if (this->mode == Edit) {
                // in edit mode, we can adjust vertex color by clicking it

                // draw circles, we dont draw on image, but on the widget
                QPainter p1(this);

                // if a rect is chosen, draw a white boundary
                if (i == chooseRectID) {
                    p1.setPen(QPen(Qt::white, 2));
                    p1.drawLine(r.vertices[0], r.vertices[1]);
                    p1.drawLine(r.vertices[1], r.vertices[2]);
                    p1.drawLine(r.vertices[2], r.vertices[3]);
                    p1.drawLine(r.vertices[3], r.vertices[0]);
                }
                for (int i=0; i<4; i++) {
                    int x = r.vertices[i].x();
                    int y = r.vertices[i].y();
                    p1.setPen(QPen(QColor(80, 80, 80, 255), 1));
                    p1.setBrush(QColor(80, 80, 80, 255));
                    p1.drawEllipse(QPoint(x, y), 4, 4); // draw background grey circle

                    QColor c = r.colors[i];
                    c.setAlpha(255);
                    p1.setBrush(c);
                    p1.drawEllipse(QPoint(x, y), 3, 3);

                }

            }
        }
        else {
            // Template type rectangle
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);
            p.setOpacity((float) r.alpha / 255.); // set opacity
            p.drawPixmap(destRect, r.pixmap, QRect(0, 0, 100, 100));

            if (this->mode == Edit) {
                // in edit mode, we draw a white boundary if a rect is chosen
                QPainter p1(this);
                if (i == chooseRectID) {
                    p1.setPen(QPen(Qt::white, 2));
                    p1.drawLine(r.vertices[0], r.vertices[1]);
                    p1.drawLine(r.vertices[1], r.vertices[2]);
                    p1.drawLine(r.vertices[2], r.vertices[3]);
                    p1.drawLine(r.vertices[3], r.vertices[0]);
                }
                // draw black circles in all corners
                p1.setBrush(QColor(30, 30, 30, 255));
                for (int i=0; i<4; i++) {
                    int x = r.vertices[i].x();
                    int y = r.vertices[i].y();
                    p1.drawEllipse(QPoint(x, y), 3, 3);
                }
            }
        }

        edgeEnhanceList[3*i] = (float) topLeft.x() / 256.;
        edgeEnhanceList[3*i+1] = (float) bottomRight.x() / 256.;
        //edgeEnhanceList[3*i+2] = 1.;

    }
}

void ImageEditor::edgeEnhanceModified(double* list) {
    if (chooseRectID >= 0) {
        for(int i=0; i<30; i++) edgeEnhanceList[i] = list[i];
    }
    else {
        qDebug() << "Something is wrong";
    }
}

void ImageEditor::overallOpacityChanged(int alpha) {
    //
    if (chooseRectID >= 0) {
        if (rectangleList[chooseRectID].rectangleType == UserDefined) {
            for (int i=0; i<4; i++)
                rectangleList[chooseRectID].colors[i].setAlpha(alpha);
        }
        else {
            rectangleList[chooseRectID].alpha = alpha;
        }
    }
    update();
}

void ImageEditor::mousePressEvent(QMouseEvent *e) {
    QPoint hit = e->pos();
    if (e->button() == Qt::LeftButton) {
        if (mode == Edit) {
            // if hit a vertex of any rectangle
            for (auto &r:rectangleList) {
                if (r.rectangleType == Template) continue;
                // we can change the vertex of user-defined rectangle
                for (int i=0; i<4; i++) {
                    int x = r.vertices[i].x();
                    int y = r.vertices[i].y();
                    QPoint v = QPoint(x, y) - hit;
                    if (v.manhattanLength() <= 4) {
                        // we hit a vertex
                        // change its color
                        QColor c = QColorDialog::getColor(r.colors[i], this, "Change Color", QColorDialog::ShowAlphaChannel);
                        if (c.isValid()) r.setVertexColor(i, c); // set the color of i-th vertex
                        break;
                    }
                }
            }

            // if hit a boundary, we can scale the rectangle
            for (int i=0; i<rectangleList.size(); i++) {
                int corner = 5;
                auto r = rectangleList[i];
                // up or down edge
                int x1 = r.vertices[3].x();
                int x2 = r.vertices[0].x();
                int y1 = r.vertices[0].y();
                int y2 = r.vertices[1].y();
                if (hit.y() == y1) {
                    if(x1+corner < hit.x() && hit.x() < x2-corner) {
                        scaling = true;
                        activeRectID = i;
                        activeEdge = Up;
                        break;
                        // do the scaling in mouseMoveEvent
                    }
                }

                // decide which edge
                if (hit.y() == y2) {
                    if(x1+corner < hit.x() && hit.x() < x2-corner) {
                        scaling = true;
                        activeRectID = i;
                        activeEdge = Down;
                        break;
                    }
                }

                if (hit.x() == x1) {
                    if(y1+corner < hit.y() && hit.y() < y2-corner) {
                        scaling = true;
                        activeRectID = i;
                        activeEdge = Left;
                        break;
                    }
                }

                if (hit.x() == x2) {
                    if(y1+corner < hit.y() && hit.y() < y2-corner) {
                        scaling = true;
                        activeRectID = i;
                        activeEdge = Right;
                        break;
                    }
                }
            }
        }
        else if (mode == Draw) {
            // draw a rectangle
            drawing = true;
            startPoint = hit;
        }
    }
    // right button, popup a window to select predefined colour gradients
    else {
        if (chooseRectID >= 0) {
            // if within the rectangle:
            auto &r = rectangleList[chooseRectID];
            int xmin = r.vertices[3].x();
            int ymin = r.vertices[3].y();
            int xmax = r.vertices[1].x();
            int ymax = r.vertices[1].y();
            if (hit.x() > xmin && hit.x() < xmax && hit.y() > ymin && hit.y() < ymax) {
                TemplateDialog t;
                t.setModal(true);
                if(t.exec()) {
                    r.pixmap = t.getPixmap();
                    if (!r.pixmap.isNull() && r.rectangleType == UserDefined) {
                        r.rectangleType = Template;
                        r.alpha = (r.colors[0].alpha()+r.colors[1].alpha()
                                +r.colors[2].alpha()+r.colors[3].alpha())/4;
                    }
                }
            }

        }
    }
    update();
}

void ImageEditor::mouseReleaseEvent(QMouseEvent *e) {
    if (mode == Edit) {
        scaling = false;
        activeRectID = -1;
        setCursor(QCursor(Qt::ArrowCursor));
    }
    else if (mode == Draw) {
        if(drawing) {
            drawing = false;
            endPoint = e->pos();
            QPoint d = endPoint - startPoint;
            if (d.manhattanLength() > 10) {
                // create a new rectangle
                std::vector<QPoint> v = std::vector<QPoint>(4);
                std::vector<QColor> c = std::vector<QColor>(4);
                v[0] = QPoint(endPoint.x(), startPoint.y());
                v[1] = QPoint(endPoint.x(), endPoint.y());
                v[2] = QPoint(startPoint.x(), endPoint.y());
                v[3] = QPoint(startPoint.x(), startPoint.y());
                for (int i=0; i<4; i++) c[i] = QColor(128, 128, 128, 10);
                RectangleControl *r = new RectangleControl(v, c);
                r->rectangleType = UserDefined;
                rectangleList.push_back(*r);
            }
            update(); // paint the img to screen
        }
    }
    //QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void ImageEditor::mouseMoveEvent(QMouseEvent *e) {
    QPoint pos = e->pos();

    // clamp the value, so that rectangles won't exceed the boundary
    pos.setX(qBound(0, pos.x(), this->width()-1));
    pos.setY(qBound(0, pos.y(), this->height()-1));

    if (mode == Edit) {
        if (scaling && activeRectID >= 0) {
            // we want to scale the rectangle
            auto &r = rectangleList[activeRectID];
            switch(activeEdge) {
            case Up:
                r.vertices[0].setY(pos.y());
                r.vertices[3].setY(pos.y());
                break;
            case Down:
                r.vertices[1].setY(pos.y());
                r.vertices[2].setY(pos.y());
                break;
            case Left:
                r.vertices[2].setX(pos.x());
                r.vertices[3].setX(pos.x());
                break;
            case Right:
                r.vertices[0].setX(pos.x());
                r.vertices[1].setX(pos.x());
                break;
            }

            update();
        }

        // if hover a boundary of any rectangle
        bool hover = false;
        int corner = 3; // pixels away from the corner
        for (auto &r:rectangleList) {
            // up or down edge
            int x1 = r.vertices[3].x();
            int x2 = r.vertices[0].x();
            int y1 = r.vertices[0].y();
            int y2 = r.vertices[1].y();
            if (pos.y() == y1 || pos.y() == y2 ) {
                if(x1+corner < pos.x() && pos.x() < x2-corner) {
                    setCursor(QCursor(Qt::SizeVerCursor));
                    hover = true;
                    break;
                }
            }

            if (pos.x() == x1 || pos.x() == x2) {
                if(y1+corner < pos.y() && pos.y() < y2-corner) {
                    setCursor(QCursor(Qt::SizeHorCursor));
                    hover = true;
                    break;
                }
            }
        }
        if (!hover) setCursor(QCursor(Qt::ArrowCursor));
    }
    else if (mode == Draw) {
        setCursor(QCursor(Qt::CrossCursor));
        endPoint = pos;
        update();
    }
}

void ImageEditor::mouseDoubleClickEvent(QMouseEvent *e) {
    QPoint pos = e->pos();
    bool found = false;
    int chosenID = chooseRectID; // previously chosen ID
    // find if it is in a rectangle
    int start = (chooseRectID + 1) % rectangleList.size();
    for (int j=0; j<rectangleList.size(); j++) {
        int i = (start + j) % rectangleList.size();
        auto r = rectangleList[i];
        int xmin = r.vertices[3].x();
        int ymin = r.vertices[3].y();
        int xmax = r.vertices[1].x();
        int ymax = r.vertices[1].y();
        if (pos.x() > xmin && pos.x() < xmax && pos.y() > ymin && pos.y() < ymax) {
            chooseRectID = i;
            found = true;
            if (r.rectangleType == UserDefined)
                emit sendOpacity((r.colors[0].alpha()+r.colors[1].alpha()
                        +r.colors[2].alpha()+r.colors[3].alpha())/4);
            else
                emit sendOpacity(r.alpha);
            emit sendList(chooseRectID, edgeEnhanceList);
            break;
        }
    }

    if (found) {
        if (chooseRectID == chosenID) {
            chooseRectID = -1; // unchoose it
            emit disableSetting();
        }
    }
    else {
        // not found
        chooseRectID = -1;
        emit disableSetting();
    }
    update();
}

void ImageEditor::keyPressEvent(QKeyEvent *e) {
    auto k = e->key();
    // delete a rectangle
    if (k == Qt::Key_Escape || k == Qt::Key_Delete) {
        // delete a rectangle
        if (chooseRectID >= 0) {
            rectangleList.erase(rectangleList.begin() + chooseRectID);
            for (int i=chooseRectID*3; i<27; i++) {
                edgeEnhanceList[i] = edgeEnhanceList[i+3];
            }
            chooseRectID = -1;
        }
    }
    update();
}

void ImageEditor::saveConfig() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Transfer Function"), "../transferfuncs", tr("Transfer Function Files (*.tf)"));
    QFile f( filename );
    f.open( QIODevice::WriteOnly );
    QDataStream output(&f);
    RectangleType t;
    // store data in f
    output << rectangleList.size();
    for (int i=0; i<rectangleList.size(); i++) {
        t = rectangleList[i].rectangleType;
        output << (int) t;
        if (t == (int) UserDefined) {
            for (int j=0; j<4; j++) {
                output << rectangleList[i].vertices[j];
                output << rectangleList[i].colors[j];
            }
        }
        else {
            for (int j=0; j<4; j++) output << rectangleList[i].vertices[j];
            output << rectangleList[i].pixmap;
            output << rectangleList[i].alpha;
        }
    }
    f.close();
}

void ImageEditor::loadConfig() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Transfer Function"), "../transferfuncs", tr("Transfer Function Files (*.tf)"));
    if (!filename.isNull()) {
        QFile f( filename );
        f.open( QIODevice::ReadOnly );
        QDataStream input(&f);
        // read data in f, and restart everything

        img = QImage(256, 256, QImage::Format_RGBA8888);
        img.fill(0);
        for (int i=0; i<10; i++) {
            edgeEnhanceList[3*i] = 0.;
            edgeEnhanceList[3*i+1] = 0.;
            edgeEnhanceList[3*i+2] = 1.;
        }

        int s; // size
        input >> s;
        rectangleList = std::vector<RectangleControl>();

        std::vector<QPoint> ps = std::vector<QPoint>(4);
        std::vector<QColor> cs = std::vector<QColor>(4);
        RectangleControl *r;
        int t;

        // input the data in the same order as we output them
        for (int i=0; i<s; i++) {
            input >> t; // type
            if (t == (int) UserDefined) {
                for (int j=0; j<4; j++) {
                    input >> ps[j];
                    input >> cs[j];
                }
                r = new RectangleControl(ps,cs);
                r->rectangleType = UserDefined;
                rectangleList.push_back(*r);
            }
            else {
                // pixmap template
                for (int j=0; j<4; j++) input >> ps[j];
                r = new RectangleControl(ps,cs);
                input >> r->pixmap;
                input >> r->alpha;
                r->rectangleType = Template;
                rectangleList.push_back(*r);
            }
        }
        f.close();

        if (chooseRectID >= 0) {
            chooseRectID = -1;
            emit disableSetting();
        }

        update();
    }
}
