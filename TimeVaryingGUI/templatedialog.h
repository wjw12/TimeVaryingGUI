#ifndef TEMPLATEDIALOG_H
#define TEMPLATEDIALOG_H

#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>
#include <QToolButton>
#include <QRect>
#include <QPixmap>
#include <QListWidgetItem>
#include "previewarea.h"

#define TABS 7

class TemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TemplateDialog(QWidget *parent = 0);
    ~TemplateDialog();

    QDialogButtonBox *buttonBox;
    QTabWidget *tabWidget;
    PreviewArea *tabs[TABS];

    QPixmap currPixmap;
    QPixmap getPixmap();

public slots:
    void itemSelected(QListWidgetItem *pItem);
};

#endif // TEMPLATEDIALOG_H
