/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "mdichild.h"

MdiChild::MdiChild()
{
    this->owner = nullptr;
    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
    isModified = false;
}

MdiChild::MdiChild(QObject *owner)
{
    this->owner = owner;
    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;
    isModified = false;
}

void MdiChild::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("image%1.jpg").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");

    connect(this, &MdiChild::imageChanged,
            this, &MdiChild::imageWasModified);
}

bool MdiChild::loadFromFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    image = reader.read();
    if (image.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    this->imageItem.setPixmap(QPixmap::fromImage(image));
    this->scene.addItem(&imageItem);
    this->setScene(&this->scene);
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    connect(this, &MdiChild::imageChanged,
            this, &MdiChild::imageWasModified);

    return true;
}

bool MdiChild::loadFromImage(const QImage &newImage)
{
    this->image = newImage;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    this->imageItem.setPixmap(QPixmap::fromImage(image));
    this->scene.addItem(&imageItem);
    this->setScene(&this->scene);
    QApplication::restoreOverrideCursor();

    connect(this, &MdiChild::imageChanged,
            this, &MdiChild::imageWasModified);

    return true;
}

bool MdiChild::save()
{
    if (isUntitled) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MdiChild::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                                                    curFile);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

bool MdiChild::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (!writer.write(image)) {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    return true;
}

QString MdiChild::userFriendlyCurrentFile()
{
    return strippedName(curFile);
}

void MdiChild::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
        if (!this->owner) // MdiChild is the owner
            emit ownerClosed();
    } else {
        event->ignore();
    }
}

void MdiChild::imageWasModified()
{
    setWindowModified(isModified);
}

bool MdiChild::maybeSave()
{
    if (!isModified)
        return true;
    const QMessageBox::StandardButton ret
            = QMessageBox::warning(this, tr("DIP"),
                                   tr("'%1' has been modified.\n"
                                      "Do you want to save your changes?")
                                   .arg(userFriendlyCurrentFile()),
                                   QMessageBox::Save | QMessageBox::Discard
                                   | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Discard:
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MdiChild::setCurrentFile(const QString &fileName)
{
    //curFile = QFileInfo(fileName).canonicalFilePath();
    curFile = fileName;
    isUntitled = false;
    isModified = false;
    setWindowModified(false);
    setWindowTitle(userFriendlyCurrentFile() + "[*]");
}

void MdiChild::setLabel(const QString &str)
{
    label = str;
}

QString MdiChild::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

bool MdiChild::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
    return true;
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

bool MdiChild::paste()
{
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (!newImage.isNull()) {
        setImage(newImage);
        //setWindowFilePath(QString());
        //const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
        //    .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        //statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
    return true;
}

void MdiChild::setImage(QImage newImage)
{
    image = newImage;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    this->scene.addPixmap(QPixmap::fromImage(image));
    this->setScene(&this->scene);
    QApplication::restoreOverrideCursor();
    isModified = true;
    emit imageChanged();
}

void MdiChild::zoomIn()
{
    this->scale(1.25, 1.25);
}

void MdiChild::zoomOut()
{
    this->scale(0.8, 0.8);
}

void MdiChild::normalSize()
{
    this->fitInView(this->rect(), Qt::KeepAspectRatio);
}

void MdiChild::fitToWindow()
{
    this->fitInView(this->sceneRect(), Qt::KeepAspectRatio);
}
