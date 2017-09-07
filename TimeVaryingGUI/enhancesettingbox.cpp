#include "enhancesettingbox.h"

void EnhanceSettingBox::enable(int id, double *list) {
    if (!this->isEnabled())
        this->setEnabled(true);
    activeID = id;
    enhanceList = list;
    this->setValue(list[activeID*3+2]);
}


void EnhanceSettingBox::updateValue(double newVal) {
    if (activeID >= 0) {
        enhanceList[activeID*3+2] = newVal;
        emit sendList(enhanceList);
    }
}

void EnhanceSettingBox:: disable() {
     activeID = -1;
     if (this->isEnabled())
        this->setDisabled(true);
}
