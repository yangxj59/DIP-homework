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

#include "mainwindow.h"
#include "mdichild.h"

MainWindow::MainWindow()
    : mdiArea(new QMdiArea)
{
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect(mdiArea, &QMdiArea::subWindowActivated,
            this, &MainWindow::updateMenus);

    createActions();
    createStatusBar();
    updateMenus();

    readSettings();

    setWindowTitle(tr("DIP"));
    setUnifiedTitleAndToolBarOnMac(true);

    this->installEventFilter(this);

    translator.load(":/languages/zh_CN.qm");
    qApp->installTranslator( &translator );
    this->retranslate();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}

void MainWindow::newFile()
{
    MdiChild *child = createMdiChild();
    child->newFile();
    child->show();
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void MainWindow::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    if (dialog.exec() == QDialog::Accepted)
    {
        QString fileName = dialog.selectedFiles().first();
        const bool succeeded = openFile(fileName);
        if (succeeded)
            statusBar()->showMessage(tr("Image loaded"), 2000);
    }
}

void MainWindow::initMdiSubWindow(MdiChild *child)
{
    child->show();
    fitToImage(child);
    fitToWindowAct->setChecked(true);
}

bool MainWindow::openFile(const QString &fileName)
{
    MdiChild *child = createMdiChild();
    const bool succeeded = child->loadFromFile(fileName);
    if (succeeded)
    {
        initMdiSubWindow(child);
        connect(child, &MdiChild::ownerClosed,
                this, &MainWindow::closeMdiChildView);
        connect(child, &MdiChild::imageChanged,
                this, &MainWindow::updateMdiChildView);
    }
    else
        child->close();
    MainWindow::prependToRecentFiles(fileName);
    return succeeded;
}

static inline QString recentFilesKey() { return QStringLiteral("recentFileList"); }
static inline QString fileKey() { return QStringLiteral("file"); }

static QStringList readRecentFiles(QSettings &settings)
{
    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(fileKey()).toString());
    }
    settings.endArray();
    return result;
}

static void writeRecentFiles(const QStringList &files, QSettings &settings)
{
    const int count = files.size();
    settings.beginWriteArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }
    settings.endArray();
}

bool MainWindow::hasRecentFiles()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
}

void MainWindow::prependToRecentFiles(const QString &fileName)
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
        writeRecentFiles(recentFiles, settings);

    setRecentFilesVisible(!recentFiles.isEmpty());
}

void MainWindow::setRecentFilesVisible(bool visible)
{
    recentFileSubMenuAct->setVisible(visible);
    recentFileSeparator->setVisible(visible);
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for ( ; i < count; ++i) {
        const QString fileName = QFileInfo(recentFiles.at(i)).fileName();
        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        recentFileActs[i]->setData(recentFiles.at(i));
        recentFileActs[i]->setVisible(true);
    }
    for ( ; i < MaxRecentFiles; ++i)
        recentFileActs[i]->setVisible(false);
}

void MainWindow::openRecentFile()
{
    if (const QAction *action = qobject_cast<const QAction *>(sender()))
    {
        QString fileName = action->data().toString();
        const bool succeeded = openFile(fileName);
        if (succeeded)
            statusBar()->showMessage(tr("Image loaded"), 2000);
    }
}

void MainWindow::save()
{
    MdiChild *child = activeMdiChild();
    if (child && child->save())
        statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::saveAs()
{
    MdiChild *child = activeMdiChild();
    if (child && child->saveAs()) {
        statusBar()->showMessage(tr("File saved"), 2000);
        MainWindow::prependToRecentFiles(child->currentFile());
    }
}

#ifndef QT_NO_CLIPBOARD
void MainWindow::copy()
{
    MdiChild *child = activeMdiChild();
    if (child) {
    //if (activeMdiChild())
        child->copy();
    }
}

void MainWindow::paste()
{
    MdiChild *child = activeMdiChild();
    if (child) {
        child->paste();
        fitToImage(child);
        connect(child, &MdiChild::ownerClosed,
                this, &MainWindow::closeMdiChildView);
        connect(child, &MdiChild::imageChanged,
                this, &MainWindow::updateMdiChildView);
    }
}
#endif

void MainWindow::about()
{
   QMessageBox::about(this, tr("About DIP"),
            tr("The <b>DIP</b> demonstrates Digital Image Processing using Qt."));
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = (activeMdiChild() != 0);
    saveAct->setEnabled(hasMdiChild);
    saveAsAct->setEnabled(hasMdiChild);
#ifndef QT_NO_CLIPBOARD
    pasteAct->setEnabled(hasMdiChild);
#endif
    closeAct->setEnabled(hasMdiChild);
    closeAllAct->setEnabled(hasMdiChild);
    viewModeAct->setEnabled(hasMdiChild);
    tileAct->setEnabled(hasMdiChild);
    cascadeAct->setEnabled(hasMdiChild);
    nextAct->setEnabled(hasMdiChild);
    previousAct->setEnabled(hasMdiChild);
    windowMenuSeparatorAct->setVisible(hasMdiChild);

#ifndef QT_NO_CLIPBOARD
    bool hasSelection = activeMdiChild();
    copyAct->setEnabled(hasSelection);
#endif

    zoomInAct->setEnabled(hasMdiChild);
    zoomOutAct->setEnabled(hasMdiChild);
    normalSizeAct->setEnabled(hasMdiChild);
    fitToWindowAct->setEnabled(hasMdiChild);
}

void MainWindow::updateWindowMenu()
{
    windowMenu->clear();
    windowMenu->addAction(closeAct);
    windowMenu->addAction(closeAllAct);
    windowMenu->addSeparator();
    windowMenu->addAction(viewModeAct);
    windowMenu->addAction(tileAct);
    windowMenu->addAction(cascadeAct);
    windowMenu->addSeparator();
    windowMenu->addAction(nextAct);
    windowMenu->addAction(previousAct);
    windowMenu->addAction(windowMenuSeparatorAct);

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    windowMenuSeparatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i) {
        QMdiSubWindow *mdiSubWindow = windows.at(i);
        MdiChild *child = qobject_cast<MdiChild *>(mdiSubWindow->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                               .arg(child->userFriendlyCurrentFile());
        } else {
            text = tr("%1 %2").arg(i + 1)
                              .arg(child->userFriendlyCurrentFile());
        }
        QAction *action = windowMenu->addAction(text, mdiSubWindow, [this, mdiSubWindow]() {
            mdiArea->setActiveSubWindow(mdiSubWindow);
        });
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
    }
}

MdiChild *MainWindow::createMdiChild()
{
    MdiChild *child = new MdiChild();
    mdiArea->addSubWindow(child);

#ifndef QT_NO_CLIPBOARD
    //connect(child, &QTextEdit::copyAvailable, copyAct, &QAction::setEnabled);
#endif

    return child;
}

void MainWindow::createActions()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileToolBar = addToolBar(tr("File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    saveAsAct = new QAction(saveAsIcon, tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();

    recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }

    recentFileSeparator = fileMenu->addSeparator();

    setRecentFilesVisible(MainWindow::hasRecentFiles());

    layoutAct = fileMenu->addAction(tr("Switch layout direction"), this, &MainWindow::switchLayoutDirection);

    fileMenu->addSeparator();

//! [0]
    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), qApp, &QApplication::closeAllWindows);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);
//! [0]

#ifndef QT_NO_CLIPBOARD
    editMenu = menuBar()->addMenu(tr("&Edit"));
    editToolBar = addToolBar(tr("Edit"));

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.png"));
    copyAct = new QAction(copyIcon, tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    connect(copyAct, &QAction::triggered, this, &MainWindow::copy);
    editMenu->addAction(copyAct);
    editToolBar->addAction(copyAct);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(":/images/paste.png"));
    pasteAct = new QAction(pasteIcon, tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    connect(pasteAct, &QAction::triggered, this, &MainWindow::paste);
    editMenu->addAction(pasteAct);
    editToolBar->addAction(pasteAct);
#endif

    windowMenu = menuBar()->addMenu(tr("&Window"));
    connect(windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);

    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, &QAction::triggered,
            mdiArea, &QMdiArea::closeActiveSubWindow);

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, &QAction::triggered, mdiArea, &QMdiArea::closeAllSubWindows);

    viewModeAct = new QAction(tr("&Switch View Mode"), this);
    viewModeAct->setStatusTip(tr("Switch View Mode"));
    connect(viewModeAct, &QAction::triggered, this, &MainWindow::switchViewMode);

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, &QAction::triggered, mdiArea, &QMdiArea::tileSubWindows);

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, &QAction::triggered, mdiArea, &QMdiArea::cascadeSubWindows);

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, &QAction::triggered, mdiArea, &QMdiArea::activateNextSubWindow);

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));
    connect(previousAct, &QAction::triggered, mdiArea, &QMdiArea::activatePreviousSubWindow);

    windowMenuSeparatorAct = new QAction(this);
    windowMenuSeparatorAct->setSeparator(true);

    updateWindowMenu();

    menuBar()->addSeparator();

    viewMenu = menuBar()->addMenu(tr("&View"));
    //connect(windowMenu, &QMenu::aboutToShow, this, &MainWindow::updateWindowMenu);

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &MainWindow::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &MainWindow::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &MainWindow::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &MainWindow::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    viewMenu->addSeparator();
    /*
    grayViewAct = viewMenu->addAction(tr("Gray Image"), this, &MainWindow::grayView);
    histViewMenu = viewMenu->addMenu(tr("Histogram"));
    histViewYAct = histViewMenu->addAction(tr("Y"), this, &MainWindow::histogramYView);
    histViewRAct = histViewMenu->addAction(tr("R"), this, &MainWindow::histogramRView);
    histViewGAct = histViewMenu->addAction(tr("G"), this, &MainWindow::histogramGView);
    histViewBAct = histViewMenu->addAction(tr("B"), this, &MainWindow::histogramBView);
    */
    grayViewAct = viewMenu->addAction(tr("Gray Image"), this, &MainWindow::createMdiChildView); // add functions to menu
    spectrumViewAct = viewMenu->addAction(tr("Spectrum"), this, &MainWindow::createMdiChildView);//Spectrum spectrumViewAct
    transformViewAct = viewMenu->addAction(tr("Transform"), this, &MainWindow::createMdiChildView);//Spectrum spectrumViewAct
    histViewMenu = viewMenu->addMenu(tr("Histogram"));
    histViewYAct = histViewMenu->addAction(tr("Y"), this, &MainWindow::createMdiChildView);
    histViewRAct = histViewMenu->addAction(tr("R"), this, &MainWindow::createMdiChildView);
    histViewGAct = histViewMenu->addAction(tr("G"), this, &MainWindow::createMdiChildView);
    histViewBAct = histViewMenu->addAction(tr("B"), this, &MainWindow::createMdiChildView);

    viewMenu->addSeparator();
    negativeViewMenu = viewMenu->addMenu(tr("Negative"));
    /*
    negativeViewAllAct = negativeViewMenu->addAction(tr("All"), this, &MainWindow::negativeAllView);
    negativeViewRAct = negativeViewMenu->addAction(tr("R"), this, &MainWindow::negativeRView);
    negativeViewGAct = negativeViewMenu->addAction(tr("G"), this, &MainWindow::negativeGView);
    negativeViewBAct = negativeViewMenu->addAction(tr("B"), this, &MainWindow::negativeBView);
    */
    negativeViewAllAct = negativeViewMenu->addAction(tr("All"), this, &MainWindow::createMdiChildView);
    negativeViewRAct = negativeViewMenu->addAction(tr("R"), this, &MainWindow::createMdiChildView);
    negativeViewGAct = negativeViewMenu->addAction(tr("G"), this, &MainWindow::createMdiChildView);
    negativeViewBAct = negativeViewMenu->addAction(tr("B"), this, &MainWindow::createMdiChildView);

    viewMenu->addSeparator();
    pseudoColorViewMenu = viewMenu->addMenu(tr("PseudoColor"));
    /*
    pseudoColorJetViewAct = pseudoColorViewMenu->addAction(tr("Jet"), this, &MainWindow::pseudoColorJetView);
    pseudoColorParulaViewAct = pseudoColorViewMenu->addAction(tr("Parula"), this, &MainWindow::pseudoColorParulaView);
    pseudoColorHotViewAct = pseudoColorViewMenu->addAction(tr("Hot"), this, &MainWindow::pseudoColorHotView);
    */

    pseudoColorJetViewAct = pseudoColorViewMenu->addAction(tr("Jet"), this, &MainWindow::createMdiChildView);
    pseudoColorParulaViewAct = pseudoColorViewMenu->addAction(tr("Parula"), this, &MainWindow::createMdiChildView);
    pseudoColorHotViewAct = pseudoColorViewMenu->addAction(tr("Hot"), this, &MainWindow::createMdiChildView);
    //updateWindowMenu();

    menuBar()->addSeparator();
    enhancementMenu = menuBar()->addMenu(tr("&Enhancement"));
    convertGrayImageAct = enhancementMenu->addAction(tr("Gray Image"), this, &MainWindow::convertGrayImage);
    equalizeHistogramAct = enhancementMenu->addAction(tr("Hist Equalization"), this, &MainWindow::equalizeHistogram);
    adaptiveContrastEnhancementAct = enhancementMenu->addAction(tr("Adaptive Contrast Equalization"), this, &MainWindow::adaptiveContrastEnhancement);

    menuBar()->addSeparator();
    filterMenu = menuBar()->addMenu(tr("&Filter"));
    timeDomainAct = filterMenu->addAction(tr("Space Domain"), this, &MainWindow::spaceDomainFiltering);
    frequencyDomainAct = filterMenu->addAction(tr("Frequency Domain"), this, &MainWindow::frequencyDomainFiltering);
    embossFilterAct = filterMenu->addAction(tr("Emboss Filter"), this, &MainWindow::embossFiltering);

    menuBar()->addSeparator();
    morphologyMenu = menuBar()->addMenu(tr("&Morphology"));
    corrosionAct = morphologyMenu->addAction(tr("Corrosion"), this, &MainWindow::createMdiChildView);
    dilateAct = morphologyMenu->addAction(tr("Dilate"), this, &MainWindow::createMdiChildView);
    openingAct = morphologyMenu->addAction(tr("Opening"), this, &MainWindow::createMdiChildView);
    closingAct = morphologyMenu->addAction(tr("Closing"), this, &MainWindow::createMdiChildView);
    refinementAct = morphologyMenu->addAction(tr("Refinement"), this, &MainWindow::createMdiChildView);

    menuBar()->addSeparator();
    sidetestMenu = menuBar()->addMenu(tr("&Sidetest"));
    laplaceAct = sidetestMenu->addAction(tr("Laplace"), this, &MainWindow::createMdiChildView);
    gaussAct = sidetestMenu->addAction(tr("Gauss"), this, &MainWindow::createMdiChildView);
    sketchAct = sidetestMenu->addAction(tr("Sketch"), this, &MainWindow::createMdiChildView);
    sobelAct = sidetestMenu->addAction(tr("Sobel"), this, &MainWindow::createMdiChildView);
    robertAct = sidetestMenu->addAction(tr("Robert"), this, &MainWindow::createMdiChildView);


    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));

    languageMenu = helpMenu->addMenu(tr("&Language"));
    // Chinese
    zhCNAct = languageMenu->addAction(tr("&Chinese"));
    languageMenu->addAction(zhCNAct);
    connect(zhCNAct, &QAction::triggered, this, &MainWindow::switchLanguage);

    // English
    enUSAct = languageMenu->addAction(tr("&English"));
    languageMenu->addAction(enUSAct);
    connect(enUSAct, &QAction::triggered, this, &MainWindow::switchLanguage);

    aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

MdiChild *MainWindow::activeMdiChild() const
{
    if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
    {
        QWidget *w  = activeSubWindow->widget();
        MdiChild *child = qobject_cast<MdiChild *>(w);
        if (child->owner != nullptr)
            return qobject_cast<MdiChild *>(child->owner);
        else
            return child;
    }
    return 0;
}


void MainWindow::switchLayoutDirection()
{
    if (layoutDirection() == Qt::LeftToRight)
        QGuiApplication::setLayoutDirection(Qt::RightToLeft);
    else
        QGuiApplication::setLayoutDirection(Qt::LeftToRight);
}

void MainWindow::switchViewMode()
{
    if (mdiArea->viewMode() == QMdiArea::SubWindowView)
        mdiArea->setViewMode(QMdiArea::TabbedView);
    else
        mdiArea->setViewMode(QMdiArea::QMdiArea::SubWindowView);
}

void MainWindow::zoomIn()
{
    if (activeMdiChild())
    {
        if (fitToWindowAct->isChecked())
            fitToWindowAct->setChecked(false);
        activeMdiChild()->zoomIn();
        //zoomInAct->setEnabled(activeMdiChild()->getScaleFactor() < 3.0);
    }
}

void MainWindow::zoomOut()
{
    if (activeMdiChild())
    {
        if (fitToWindowAct->isChecked())
            fitToWindowAct->setChecked(false);
        activeMdiChild()->zoomOut();
    }
}

void MainWindow::normalSize()
{
    if (activeMdiChild())
    {
        if (fitToWindowAct->isChecked())
            fitToWindowAct->setChecked(false);
        activeMdiChild()->normalSize();
    }
}

void MainWindow::fitToWindow()
{
    if (activeMdiChild())
    {
        bool fitToWindow = fitToWindowAct->isChecked();
        if (fitToWindow)
            activeMdiChild()->fitToWindow();
    }
}

void MainWindow::fitToImage(MdiChild *child)
{
    // calculate additional window size
    QMdiSubWindow *w = mdiArea->currentSubWindow();
    QStyle *wStyle = w->style();
    QStyleOptionTitleBar so;
    so.titleBarState = 1;
    so.titleBarFlags = Qt::Window;
    int dh = wStyle->pixelMetric(QStyle::PM_TitleBarHeight, &so, w);
    int dw = wStyle->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth , &so, w);

    // resize subwindow size to image size
    QSize sz = child->image.size();
    mdiArea->currentSubWindow()->resize(sz + QSize(dw+10, dh+10));
}

QString MainWindow::joinFileName(const QString fileName, const QString str)
{
    QFileInfo fi(fileName);
    QString base = fi.baseName();
    QString ext = fi.completeSuffix();
    QString path = fi.absoluteFilePath();

    QString newFileName = base + "-" + str + "." + ext;
    newFileName = QDir(path).filePath(newFileName);
    return newFileName;
}
/*
MdiChild *MainWindow::createMdiViewChild(const QString viewString)
{
    MdiChild * owner = activeMdiChild();
    if (owner)
    {
        QString owerFileName = owner->currentFile();
        QString fileName = joinFileName(owerFileName, viewString);
        if (QMdiSubWindow *existing = findMdiViewChild(owner, fileName)) {
            mdiArea->setActiveSubWindow(existing);
            return 0;
        }
        MdiChild *viewChild = new MdiChild(owner);
        viewChild->setCurrentFile(fileName);
        mdiArea->addSubWindow(viewChild);

        return viewChild;
    }
    return 0;
}
*/

QString MainWindow::getMdiChildViewLabel(QObject *sender)
{
    QString label;
    if (sender == grayViewAct) {
        label = "gray";
    }

    if (sender == spectrumViewAct) {
        label = "spectrum";
    }

    if (sender == histViewYAct) {
        label = "histY";
    }
    if (sender == histViewRAct) {
        label = "histR";
    }
    if (sender == histViewGAct) {
        label = "histG";
    }
    if (sender == histViewBAct) {
        label = "histB";
    }

    if (sender == negativeViewAllAct) {
        label = "negativeAll";
    }
    if (sender == negativeViewRAct) {
        label = "negativeR";
    }
    if (sender == negativeViewGAct) {
        label = "negativeG";
    }
    if (sender == negativeViewBAct) {
        label = "negativeB";
    }

    if (sender == pseudoColorJetViewAct) {
        label = "pseudoColorJet";
    }
    if (sender == pseudoColorParulaViewAct) {
        label = "pseudoColorParula";
    }
    if (sender == pseudoColorHotViewAct) {
        label = "pseudoColorHot";
    }

    if (sender == corrosionAct) {
        label = "corrosion";
    }
    if (sender == dilateAct) {
        label = "dilate";
    }
    if (sender == openingAct) {
        label = "opening";
    }
    if (sender == closingAct) {
        label = "closing";
    }
    if (sender == refinementAct) {
        label = "refinement";
    }

    if (sender == laplaceAct) {
        label = "laplace";
    }
    if (sender == gaussAct) {
        label = "gauss";
    }
    if (sender == sketchAct) {
        label = "sketch";
    }
    if (sender == sobelAct) {
        label = "sobel";
    }
    if (sender == robertAct) {
        label = "robert";
    }
    return label;
}

QImage MainWindow::getMdiChildViewImage(QImage ownerImage, QString label)
{
    QImage image;

    if (label == "gray") {
        image = ownerImage.convertToFormat(QImage::Format_Grayscale8);
    }

    if (label == "spectrum") {
        calcImageSpectrum(ownerImage, image);
    }

    if (label == "histY") {
        image = calculateHistogram(ownerImage, ImageChannel::Y);
    }
    if (label == "histR") {
        image = calculateHistogram(ownerImage, ImageChannel::R);
    }
    if (label == "histG") {
        image = calculateHistogram(ownerImage, ImageChannel::G);
    }
    if (label == "histB") {
        image = calculateHistogram(ownerImage, ImageChannel::B);
    }

    if (label == "negativeAll") {
        image = calculateNegative(ownerImage, ImageChannel::Y);
    }
    if (label == "negativeR") {
        image = calculateNegative(ownerImage, ImageChannel::R);
    }
    if (label == "negativeG") {
        image = calculateNegative(ownerImage, ImageChannel::G);
    }
    if (label == "negativeB") {
        image = calculateNegative(ownerImage, ImageChannel::B);
    }

    if (label == "pseudoColorJet") {
        image = convertToPseudoColor(ownerImage, ColorMap::Jet);
    }
    if (label == "pseudoColorParula") {
        image = convertToPseudoColor(ownerImage, ColorMap::Parula);
    }
    if (label == "pseudoColorHot") {
        image = convertToPseudoColor(ownerImage, ColorMap::Hot);
    }

    if (label == "corrosion") {
        image = calculatecorrosin(ownerImage);
    }
    if (label == "dilate") {
        image = calculatedilate(ownerImage);
    }
    if (label == "opening") {
        image = calculateopening(ownerImage);
    }
    if (label == "closing") {
        image = calculateclosing(ownerImage);
    }
    if (label == "refinement") {
        image = calculateRefinement(ownerImage);
    }

    if (label == "laplace") {
        image = calculateLaplace(ownerImage);
    }
    if (label == "gauss") {
        image = calculateGauss(ownerImage);
    }
    if (label == "sketch") {
        image = calculateSketch(ownerImage);
    }
    if (label == "sobel") {
        image = calculateSobel(ownerImage);
    }
    if (label == "robert") {
        image = calculateRobert(ownerImage);
    }
    return image;
}

void MainWindow::createMdiChildView()
{
    MdiChild *owner = activeMdiChild();
    if (owner)
    {
        QString owerFileName = owner->currentFile();
        QString label = getMdiChildViewLabel(sender());
        QString fileName = joinFileName(owerFileName, label);
        if (QMdiSubWindow *existing = findMdiViewChild(owner, fileName)) {
            mdiArea->setActiveSubWindow(existing);
            return;
        }

        QImage ownerImage = owner->image;
        QImage image = getMdiChildViewImage(ownerImage, label);
        MdiChild *viewChild = new MdiChild(owner);

        viewChild->setCurrentFile(fileName);
        viewChild->loadFromImage(image);
        viewChild->setLabel(label);

        mdiArea->addSubWindow(viewChild);
        initMdiSubWindow((MdiChild*)viewChild);
    }
}

QMdiSubWindow *MainWindow::findMdiViewChild(MdiChild *owner, const QString &fileName) const
{
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        if ( (mdiChild->currentFile() == fileName) && (owner == mdiChild->owner) )
            return window;
    }
    return 0;
}

void MainWindow::updateMdiChildView()
{
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        MdiChild *owner = ((MdiChild*)mdiChild->owner);
        if (owner == sender()) {
            QString label = mdiChild->label;
            QImage ownerImage = owner->image;
            QImage image = getMdiChildViewImage(ownerImage, label);
            mdiChild->setImage(image);
        }
    }
}

void MainWindow::closeMdiChildView()
{
    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
        MdiChild *owner = ((MdiChild*)mdiChild->owner);
        if (owner == sender())
            mdiArea->removeSubWindow(window);
    }
}

void MainWindow::convertGrayImage()
{
    MdiChild * owner = activeMdiChild();
    if (owner) {
        QImage image = owner->image;
        QImage newImage = image.convertToFormat(QImage::Format_Grayscale8);
        owner->setImage(newImage);
    }
}

void MainWindow::equalizeHistogram()
{
    MdiChild * owner = activeMdiChild();
    if (owner) {
        QImage image = owner->image;
        QImage newImage = equalizeHistogramProc(image);
        owner->setImage(newImage);
    }
}

void MainWindow::adaptiveContrastEnhancement()
{
    MdiChild * owner = activeMdiChild();
    if (owner) {
        QImage image = owner->image;

        ACEDialog *d = new ACEDialog(image);
        //Qt::WindowFlags flags = d->windowFlags();
        //d->setWindowFlags(flags | Qt::MSWindowsFixedSizeDialogHint);
        int ret = d->exec () ; // modal dialog
        if (ret == QDialog::Accepted)
        {
            QImage newImage = d->getImage();
            owner->setImage(newImage);
        }
        else if (ret == QDialog::Rejected)
        {
            ;
        }

        delete d;
    }
}

void MainWindow::spaceDomainFiltering()
{
    MdiChild * owner = activeMdiChild();
    if (owner) {
        QImage image = owner->image;

        SDFilterDialog *d = new SDFilterDialog(image);
        int ret = d->exec () ; // modal dialog
        if (ret == QDialog::Accepted)
        {
            QImage newImage = d->getImage();
            owner->setImage(newImage);
        }
        else if (ret == QDialog::Rejected)
        {
            ;
        }

        delete d;
    }
}
void MainWindow::frequencyDomainFiltering()
{
    MdiChild * owner = activeMdiChild();
    if (owner) {
        QImage image = owner->image;

        FDFilterDialog *d = new FDFilterDialog(image);
        d->setWindowTitle(tr("Frequency Domain Filtering"));
        int ret = d->exec () ; // modal dialog
        if (ret == QDialog::Accepted)
        {
            QImage newImage = d->getImage();
            owner->setImage(newImage);
        }

        delete d;
    }
}

void MainWindow::embossFiltering()
{
    MdiChild * owner = activeMdiChild();
    if (owner) {
        QImage image = owner->image;

        EmbossFilterDialog *d = new EmbossFilterDialog(image);
        d->setWindowTitle(tr("Emboss Filtering"));
        int ret = d->exec () ; // modal dialog
        if (ret == QDialog::Accepted)
        {
            QImage newImage = d->getImage();
            owner->setImage(newImage);
        }

        delete d;
    }
}

void MainWindow::switchLanguage()
{
    QString languageFile;
    if (this->sender() == zhCNAct) {
        languageFile = ":/languages/zh_CN.qm";
    } else if (this->sender() == enUSAct) {
        languageFile = ":/languages/en_US.qm";
    }
    translator.load(languageFile);
    qApp->installTranslator( &translator );
    this->retranslate();
}

void MainWindow::retranslate()
{
    fileMenu->setTitle(tr("&File"));
    //fileToolBar->setToolTip(tr("file"));
    newAct->setText(tr("&New"));
    newAct->setStatusTip(tr("Create a new file"));

    openAct->setText(tr("&Open..."));
    openAct->setStatusTip(tr("Open an existing file"));

    saveAct->setText(tr("&Save"));
    saveAct->setStatusTip(tr("Save the document to disk"));

    saveAsAct->setText(tr("Save &As..."));
    saveAsAct->setStatusTip(tr("Save the document under a new name"));

    recentMenu->setTitle(tr("Recent..."));

    layoutAct->setText(tr("Switch layout direction"));

    exitAct->setText(tr("E&xit"));
    exitAct->setStatusTip(tr("Exit the application"));
    editMenu->setTitle(tr("&Edit"));
    //editToolBar = addToolBar(tr("Edit"));

    copyAct->setText(tr("&Copy"));
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));

    pasteAct->setText(tr("&Paste"));
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));

    windowMenu->setTitle(tr("&Window"));

    closeAct->setText(tr("Cl&ose"));
    closeAct->setStatusTip(tr("Close the active window"));

    closeAllAct->setText(tr("Close &All"));
    closeAllAct->setStatusTip(tr("Close all the windows"));

    viewModeAct->setText(tr("&Switch View Mode"));
    viewModeAct->setStatusTip(tr("Switch View Mode"));

    tileAct->setText(tr("&Tile"));
    tileAct->setStatusTip(tr("Tile the windows"));

    cascadeAct->setText(tr("&Cascade"));
    cascadeAct->setStatusTip(tr("Cascade the windows"));

    nextAct->setText(tr("Ne&xt"));
    nextAct->setStatusTip(tr("Move the focus to the next window"));

    previousAct->setText(tr("Pre&vious"));
    previousAct->setStatusTip(tr("Move the focus to the previous "
                                 "window"));

    viewMenu->setTitle(tr("&View"));
    zoomInAct->setText(tr("Zoom &In (25%)"));
    zoomOutAct->setText(tr("Zoom &Out (25%)"));
    normalSizeAct->setText(tr("&Normal Size"));
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    fitToWindowAct->setText(tr("&Fit to Window"));
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    grayViewAct->setText(tr("Gray Image"));
    spectrumViewAct->setText(tr("Spectrum"));
    transformViewAct->setText(tr("Transform"));  // what menu show

    histViewMenu->setTitle(tr("Histogram"));
    histViewYAct->setText(tr("Y"));
    histViewRAct->setText(tr("R"));
    histViewGAct->setText(tr("G"));
    histViewBAct->setText(tr("B"));

    negativeViewMenu->setTitle(tr("Negative"));
    negativeViewAllAct->setText(tr("All"));
    negativeViewRAct->setText(tr("R"));
    negativeViewGAct->setText(tr("G"));
    negativeViewBAct->setText(tr("B"));

    pseudoColorViewMenu->setTitle(tr("PseudoColor"));
    pseudoColorJetViewAct->setText(tr("Jet"));
    pseudoColorParulaViewAct->setText(tr("Parula"));
    pseudoColorHotViewAct->setText(tr("Hot"));

    enhancementMenu->setTitle(tr("&Enhancement"));
    convertGrayImageAct->setText(tr("Gray Image"));
    equalizeHistogramAct->setText(tr("Hist Equalization"));
    adaptiveContrastEnhancementAct->setText(tr("Adaptive Contrast Equalization"));

    filterMenu->setTitle(tr("&Filter"));
    timeDomainAct->setText(tr("Space Domain"));
    frequencyDomainAct->setText(tr("Frequency Domain"));
    embossFilterAct->setText(tr("Emboss Filter"));

    morphologyMenu->setTitle(tr("&Morphology"));
    corrosionAct->setText(tr("Corrosion"));
    dilateAct->setText(tr("Dilate"));
    openingAct->setText(tr("Opening"));
    closingAct->setText(tr("Closing"));
    refinementAct->setText(tr("Refinement"));

    sidetestMenu->setTitle(tr("&Sidetest"));
    laplaceAct->setText(tr("Laplace"));
    gaussAct->setText(tr("Gauss"));
    sketchAct->setText(tr("Sketch"));
    sobelAct->setText(tr("Sobel"));
    robertAct->setText(tr("Robert"));

    helpMenu->setTitle(tr("&Help"));
    languageMenu->setTitle(tr("&Language"));
    zhCNAct->setText(tr("&Chinese"));
    enUSAct->setText(tr("&English"));

    aboutAct->setText(tr("&About"));
    aboutAct->setStatusTip(tr("Show the application's About box"));

    aboutQtAct->setText(tr("About &Qt"));
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
}
