//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
#include "aboutbox.h"
#include "ui_aboutbox.h"
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

AboutBox::AboutBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutBox)
{
    ui->setupUi(this);
}

AboutBox::~AboutBox()
{
    delete ui;
}

void AboutBox::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

#if (M_MIL_USE_LINUX && !STATIC_QT5) || (M_MIL_USE_WINDOWS && STATIC_QT5)
#include "moc_aboutbox.cpp"
#endif
