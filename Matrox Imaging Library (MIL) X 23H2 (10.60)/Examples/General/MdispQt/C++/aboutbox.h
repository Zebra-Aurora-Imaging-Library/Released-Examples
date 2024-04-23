﻿//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved

#ifndef ABOUTBOX_H
#define ABOUTBOX_H
#include <mil.h>

#include <QDialog>

namespace Ui {
    class AboutBox;
}

class AboutBox : public QDialog {
    Q_OBJECT
public:
    AboutBox(QWidget *parent = 0);
    ~AboutBox();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AboutBox *ui;
};

#endif // ABOUTBOX_H