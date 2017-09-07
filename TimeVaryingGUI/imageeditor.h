#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H
#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QPoint>
#include <QPaintEvent>
#include <QSize>
#include <QRect>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>
#include <QCursor>
#include <QApplication>
#include "templatedialog.h"

#include <vector>
#include <cmath>

#include <QtDebug>

enum RectangleType {UserDefined, Template};

class RectangleControl{
public:
    RectangleControl() = default;
    RectangleControl(std::vector<QPointF> v, std::vector<QColor> c, int canvaWidth, int canvaHeight) {
        vertices = std::vector<QPoint>(4);
        colors = std::vector<QColor>(4);
        for (int i=0; i<4; i++) {
            vertices[i].setX(canvaWidth * v[i].x());
            vertices[i].setY(canvaHeight * v[i].y());
        }
        colors = c;
        rectangleType = UserDefined;
    }
    RectangleControl(std::vector<QPoint> v, std::vector<QColor> c) {
        vertices = v;
        colors = c;
        rectangleType = UserDefined;
    }

    ~RectangleControl() {}

    void setVertexColor(int index, QColor c) {colors[index] = c;}
    void setVertexPosition(int index, int x, int y);

    std::vector<QPoint> vertices;
    std::vector<QColor> colors;

    QPixmap pixmap;
    RectangleType rectangleType;
    int alpha; // overall alpha, only for Template type

};

enum EditModes{ Draw, Edit};

enum Directions{Up, Down, Left, Right};

class ImageEditor: public QWidget {
    Q_OBJECT
public:
    ImageEditor(QWidget *parent = 0);

    QImage img; // this is the final image

    void paintEvent(QPaintEvent *e) override;
    void drawRectangles(QPainter& p);
    EditModes mode = Edit;

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void keyPressEvent(QKeyEvent *e);



public slots:
    void changeDrawMode() {mode = Draw; update();}
    void changeEditMode() {mode = Edit; update();}
    void reset() {
        for (int i=rectangleList.size()-1; i>=0; i--) {
            rectangleList.pop_back();
        }
        initializeRectangles();
        update();
    }
    void edgeEnhanceModified(double* list);
    void overallOpacityChanged(int alpha);
    void saveConfig();
    void loadConfig();
    void improveEditorOpacity(int on) {improveOpacity = on; update();}

signals:
  void imageUpdated(QImage* TF_image);
  void sendList(int id, double* list);
  void sendOpacity(int alpha);
  void disableSetting();
  void rectangleSelected();

private:
    void initializeRectangles(void);
    std::vector<RectangleControl> rectangleList;
    bool scaling = false;
    int activeRectID = -1; // for dragging and scaling
    Directions activeEdge;
    QPoint startPoint, endPoint;
    bool drawing = false;
    bool improveOpacity = false;

    int chooseRectID = -1; // for choosing, deleting, changing

    double edgeEnhanceList[30];
};


#endif // IMAGEEDITOR_H
