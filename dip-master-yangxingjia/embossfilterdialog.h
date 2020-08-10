#ifndef EMBOSSFILTERDIALOG_H
#define EMBOSSFILTERDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QImage>
#include <QComboBox>
#include "imageprocess.h"
#include "padding.h"
#include "floatslider.h"
#include <QApplication>
#include <QDesktopWidget>

enum EmbossFilterType {
    Emboss1,
    Emboss2,
    Emboss3,
    Emboss4,
    Emboss5,
    Emboss6,
    Emboss7,
    Emboss8,
};


QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSlider;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QBoxLayout;
class QHBoxLayout;
class QVBoxLayout;
class QComboBox;
class QApplication;
class QDesktopWidget;
QT_END_NAMESPACE

class EmbossFilterDialog : public QDialog
{
    Q_OBJECT
public:
    EmbossFilterDialog(QImage inputImage);
    ~EmbossFilterDialog()
    {
        if (rgb)
            delete [] rgb;
        if (rgbPadded)
            delete [] rgbPadded;
        if (rgbFilteredX)
            delete [] rgbFilteredX;
        if (filterKernelX)
            delete [] filterKernelX;
    }
    QImage getImage() {return dstImage;}
    //void showEvent(QShowEvent *event);
private:
    void iniUI();
    QImage imageFilter(int inputFilterType);
    QImage imageLOGFilter(float sigma);
    QImage emboss1Image;
    QImage emboss2Image;
    QImage emboss3Image;
    QImage emboss4Image;
    QImage emboss5Image;
    QImage emboss6Image;
    QImage emboss7Image;
    QImage emboss8Image;

    QImage paddedImage;
    QImage dstImage;

    uchar *rgb = nullptr;
    uchar *rgbPadded = nullptr;
    uchar *rgbFilteredX = nullptr;
    uchar *rgbFilteredY = nullptr;
    float *filterKernelX = nullptr;
    float *filterKernelY = nullptr;

    QLabel *imageLabel[8];
    QLabel *imageTitleLabel[8];

    QLabel *emboss1ImageLabel;
    QLabel *emboss2ImageLabel;
    QLabel *emboss3ImageLabel;
    QLabel *emboss4ImageLabel;
    QLabel *emboss5ImageLabel;
    QLabel *emboss6ImageLabel;
    QLabel *emboss7ImageLabel;
    QLabel *emboss8ImageLabel;

    QLabel *paddedImageLabel;
    QLabel *filteredSpectrumImageLabel;
    QLabel *dstImageLabel;

    int halfKernelSize = 3;
    float sigma;
    float maxSigma;
    FloatSlider *sigmaSlider;
    QLineEdit *sigmaEdit;
    QLabel *sigmaLabel;

    int borderType = 0;
    QLabel *borderTypeLabel;
    QComboBox *borderTypeComboBox;

    int filterType = 0;
    QLabel *filterTypeLabel;
    QComboBox *filterTypeComboBox;

    QPushButton     *btnOK;
    QPushButton     *btnCancel;
    QPushButton     *btnClose;

private slots:
    void setImage(QImage image, QLabel *label);
};

#endif // TDFILTERDIALOG_H
