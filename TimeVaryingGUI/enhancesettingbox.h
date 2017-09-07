#ifndef ENHANCESETTINGBOX_H
#define ENHANCESETTINGBOX_H

#include <QDoubleSpinBox>
#include <QWidget>

class EnhanceSettingBox: public QDoubleSpinBox {
    Q_OBJECT
public:
    EnhanceSettingBox(QWidget *parent=0) : QDoubleSpinBox(parent) {this->setEnabled(false);}

public slots:
    void disable();
    void enable(int id, double* list);
    void updateValue(double newVal);

signals:
    sendList(double* list);

private:
    int activeID = -1;
    double* enhanceList;
};

#endif // ENHANCESETTINGBOX_H
