#include "templatedialog.h"
#include "previewarea.h"

TemplateDialog::TemplateDialog(QWidget *parent) : QDialog(parent) {
    setObjectName(QStringLiteral("TemplateDialog"));
    resize(449, 300);

    buttonBox = new QDialogButtonBox(this);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setGeometry(QRect(30, 240, 341, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName(QStringLiteral("tabWidget"));
    tabWidget->setGeometry(QRect(30, 30, 341, 191));

    for (int i=0; i<TABS; i++) tabs[i] = new PreviewArea(this);

    tabs[0]->loadFolder("./Assets/Templates/luminance");
    tabWidget->addTab(tabs[0], "Luminance");
    tabs[1]->loadFolder("./Assets/Templates/hue");
    tabWidget->addTab(tabs[1], "Hue");
    tabs[2]->loadFolder("./Assets/Templates/saturation");
    tabWidget->addTab(tabs[2], "Saturation");
    tabs[3]->loadFolder("./Assets/Templates/divergent");
    tabWidget->addTab(tabs[3], "Divergent");
    tabs[4]->loadFolder("./Assets/Templates/discreet");
    tabWidget->addTab(tabs[4], "Discreet");
    tabs[5]->loadFolder("./Assets/Templates/xtoon");
    tabWidget->addTab(tabs[5], "Xtoon");
    tabs[6]->loadFolder("./Assets/Templates/misc");
    tabWidget->addTab(tabs[6], "Miscellaneous");

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    tabWidget->setCurrentIndex(0);

    currPixmap = QPixmap();

    // tool button
    QPixmap leftPixmap("./Assets/Icons/left.png");
    QPixmap rightPixmap("./Assets/Icons/right.png");
    QIcon leftIcon(leftPixmap);
    QIcon rightIcon(rightPixmap);
    QToolButton *leftButton = new QToolButton(this);
    QToolButton *rightButton = new QToolButton(this);
    leftButton->setGeometry(QRect(400, 50, 20, 20));
    rightButton->setGeometry(QRect(400, 90, 20, 20));
    leftButton->setIcon(leftIcon);
    leftButton->setIconSize(QSize(20,20));
    rightButton->setIcon(rightIcon);
    rightButton->setIconSize(QSize(20,20));

    for (int i=0; i<TABS; i++) {
        connect(leftButton, SIGNAL(clicked(bool)), tabs[i], SLOT(rotateLeft()));
        connect(rightButton, SIGNAL(clicked(bool)), tabs[i], SLOT(rotateRight()));

        connect(tabs[i], SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(itemSelected(QListWidgetItem*)));
    }
    QMetaObject::connectSlotsByName(this);
}

TemplateDialog::~TemplateDialog()
{

}

void TemplateDialog::itemSelected(QListWidgetItem *pItem) {
    QIcon icon = pItem->icon();
    currPixmap = icon.pixmap(100,100);
}

QPixmap TemplateDialog::getPixmap() {
    return currPixmap;
}
