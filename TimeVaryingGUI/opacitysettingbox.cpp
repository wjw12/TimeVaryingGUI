#include "opacitysettingbox.h"

void OpacitySettingBox::enable(int alpha) {
    this->setEnabled(true);
    this->setValue(alpha);
}

void OpacitySettingBox::disable() {
    this->setEnabled(false);
}
