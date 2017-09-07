#include "previewarea.h"

void PreviewArea::loadFolder(std::string folder) {
    QDir dir(QString::fromStdString(folder));
    QFileInfoList list = dir.entryInfoList();
    QIcon* pIcon;
    QListWidgetItem *pItem;
    for (int i = 0; i < list.size(); i++)
    {
        pIcon = new QIcon(list.at(i).filePath());
        pItem = new QListWidgetItem(*pIcon, QString());
        this->addItem(pItem);
    }
}


void PreviewArea::rotateLeft() {
    QListWidgetItem *pItem = this->currentItem();
    if (pItem) {
        QIcon icon = pItem->icon();
        QPixmap pixmap = icon.pixmap(100,100);
        QMatrix mat;
        mat.rotate(-90);
        pixmap = pixmap.transformed(mat);
        pItem->setIcon(QIcon(pixmap));
        emit this->itemClicked(pItem);
    }
}

void PreviewArea::rotateRight() {
    QListWidgetItem *pItem = this->currentItem();
    if (pItem) {
        QIcon icon = pItem->icon();
        QPixmap pixmap = icon.pixmap(100,100);
        QMatrix mat;
        mat.rotate(90);
        pixmap = pixmap.transformed(mat);
        pItem->setIcon(QIcon(pixmap));
        emit this->itemClicked(pItem);
    }
}
