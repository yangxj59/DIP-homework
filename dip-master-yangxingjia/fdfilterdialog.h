#ifndef FDFILTERDIALOG_H
#define FDFILTERDIALOG_H

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
#include "transform.h"
#include "floatslider.h"

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
QT_END_NAMESPACE

class FDFilterDialog : public QDialog
{
    Q_OBJECT
public:
    FDFilterDialog(QImage inputImage);
    ~FDFilterDialog()
    {
        if (rgb)
            delete [] rgb;
        if (filter)
            delete [] filter;
        if (shiftedSpectrum)
            fftwf_free(shiftedSpectrum);
        if (spectrum)
            fftwf_free(spectrum);
    }
    QImage getImage() {return dstImage;}
private:
    void iniUI();
    QImage srcImage;
    QImage spectrumImage;
    QImage filteredSpectrumImage;
    QImage dstImage;

    float *rgb = nullptr;
    float *filter = nullptr;
    fftwf_complex *shiftedSpectrum = nullptr;
    fftwf_complex *spectrum = nullptr;
    QLabel *srcImageLabel;
    QLabel *spectrumImageLabel;
    QLabel *filteredSpectrumImageLabel;
    QLabel *dstImageLabel;

    int filterType = 0;
    int filterSize = 3;
    int maxFilterSize;
    QSlider *filterSizeSlider;
    QLineEdit *filterSizeEdit;
    QLabel *filterSizeLabel;
    QLabel *filterTypeLabel;
    QComboBox *filterTypeComboBox;

    QPushButton     *btnOK;
    QPushButton     *btnCancel;
    QPushButton     *btnClose;

private slots:
    void setImage(QImage image, QLabel *label);
    void updateDstImage(int value);
};

#endif // FDFILTERDIALOG_H
