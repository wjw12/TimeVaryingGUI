#ifndef OPACITYSETTINGBOX_H
#define OPACITYSETTINGBOX_H

#include <QSpinBox>

class OpacitySettingBox: public QSpinBox {
    Q_OBJECT
public:
    OpacitySettingBox(QWidget *parent=0) : QSpinBox(parent) {this->setEnabled(false);}

public slots:
    void enable(int alpha);
    void disable();
};


#endif // OPACITYSETTINGBOX_H
