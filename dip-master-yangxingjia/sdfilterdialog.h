#ifndef SDFILTERDIALOG_H
#define SDFILTERDIALOG_H

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

enum FilterType {
    Sobel,
    Roberts,
    Prewitt,
    Laplacian4,
    Laplacian8,
    LOG,
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

class SDFilterDialog : public QDialog
{
    Q_OBJECT
public:
    SDFilterDialog(QImage inputImage);
    ~SDFilterDialog()
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
    QImage srcImage;
    QImage robertsImage;
    QImage prewittImage;
    QImage sobelImage;
    QImage laplacian4Image;
    QImage laplacian8Image;
    QImage LOGImage;

    QImage paddedImage;
    QImage filteredSpectrumImage;
    QImage dstImage;

    uchar *rgb = nullptr;
    uchar *rgbPadded = nullptr;
    uchar *rgbFilteredX = nullptr;
    uchar *rgbFilteredY = nullptr;
    float *filterKernelX = nullptr;
    float *filterKernelY = nullptr;

    QLabel *srcLabel;
    QLabel *robertsLabel;
    QLabel *prewittLabel;
    QLabel *sobelLabel;
    QLabel *laplacian4Label;
    QLabel *laplacian8Label;
    QLabel *LOGLabel;

    QLabel *srcImageLabel;
    QLabel *robertsImageLabel;
    QLabel *prewittImageLabel;
    QLabel *sobelImageLabel;
    QLabel *laplacian4ImageLabel;
    QLabel *laplacian8ImageLabel;
    QLabel *LOGImageLabel;

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
    void updateLOGImage(float value);
};

#endif // TDFILTERDIALOG_H
