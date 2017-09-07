#ifndef PREVIEWAREA_H
#define PREVIEWAREA_H

#include <QWidget>
#include <QListWidget>
#include <QFileInfoList>
#include <QIcon>
#include <QDir>
#include <QMatrix>

class PreviewArea : public QListWidget {
    Q_OBJECT

public:
    PreviewArea(QWidget *parent=0) : QListWidget(parent) {
        setViewMode(QListWidget::IconMode);
    }
    void loadFolder(std::string folder);


public slots:
    void rotateLeft();
    void rotateRight();


};

#endif // PREVIEWAREA_H
