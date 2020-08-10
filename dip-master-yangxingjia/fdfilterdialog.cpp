#include "fdfilterdialog.h"

FDFilterDialog::FDFilterDialog(QImage inputImage)
{
    srcImage = inputImage;

    int image_width = srcImage.width();
    int image_height = srcImage.height();
    int pixel_num = image_width*image_height;

    rgb = new float[3*pixel_num];
    filter = new float[3*pixel_num];
    filterType = 0;
    filterSize = 3;
    maxFilterSize = std::min(image_width, image_height) / 2;
    spectrum = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*pixel_num);
    shiftedSpectrum = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*pixel_num);

    // obtain image channels
    splitImageChannel(srcImage, rgb, rgb+pixel_num, rgb+2*pixel_num);

    // original spectrum
    fftw2d(rgb, image_width, image_height, spectrum);

    // fftshift
    fftshift2D(spectrum, image_width, image_height, shiftedSpectrum);

    // spectrum QImage
    spectrum2QImage(shiftedSpectrum, image_width, image_height, spectrumImage);

    // generate filter
    generateFilter(image_width, image_height, filterSize, (ImageFilterType)filterType, filter);

    // filtering
    imageFilterFFT2D(shiftedSpectrum, image_width, image_height, filter,
                          filteredSpectrumImage, dstImage);

    iniUI();

    srcImageLabel->setPixmap(QPixmap::fromImage(srcImage));
    spectrumImageLabel->setPixmap(QPixmap::fromImage(spectrumImage));
    filteredSpectrumImageLabel->setPixmap(QPixmap::fromImage(filteredSpectrumImage));
    dstImageLabel->setPixmap(QPixmap::fromImage(dstImage));
}

void FDFilterDialog::iniUI()
{
    // four image labels
    srcImageLabel = new QLabel();
    srcImageLabel->setAlignment(Qt::AlignCenter);
    spectrumImageLabel = new QLabel();
    spectrumImageLabel->setAlignment(Qt::AlignCenter);
    filteredSpectrumImageLabel = new QLabel();
    filteredSpectrumImageLabel->setAlignment(Qt::AlignCenter);
    dstImageLabel = new QLabel();
    dstImageLabel->setAlignment(Qt::AlignCenter);

    // filter type
    filterTypeLabel = new QLabel(tr("Filter Type"));
    filterTypeLabel->setAlignment(Qt::AlignRight);
    filterTypeComboBox = new QComboBox;
    filterTypeComboBox->addItem(tr("ideal low pass"));
    filterTypeComboBox->addItem(tr("ideal high pass"));
    filterTypeComboBox->addItem(tr("Gaussian low pass"));
    filterTypeComboBox->addItem(tr("Butterworth low pass"));
    filterTypeComboBox->addItem(tr("Butterworth high pass"));

    // filter size
    filterSizeLabel = new QLabel(tr("Filter Size"));
    filterSizeSlider = new FloatSlider(Qt::Horizontal);
    filterSizeEdit = new QLineEdit("3");

    filterSizeLabel->setMinimumWidth(30);
    filterSizeLabel->setAlignment(Qt::AlignRight);
    filterSizeEdit->setMinimumWidth(16);
    filterSizeSlider->setRange(3, maxFilterSize);
    filterSizeSlider->setValue(filterSize);
    filterSizeEdit->setText(QString("%1").arg(filterSize));

    // signal slot
    connect(filterSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDstImage(int)));
    connect(filterTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDstImage(int)));

    // three buttons
    btnOK = new QPushButton(tr("OK"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnClose = new QPushButton(tr("Exit"));

    connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(srcImageLabel);
    layout1->addWidget(spectrumImageLabel);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(filteredSpectrumImageLabel);
    layout2->addWidget(dstImageLabel);

    QHBoxLayout *layout3 = new QHBoxLayout;

    layout3->addWidget(filterTypeLabel, 3);
    layout3->addWidget(filterTypeComboBox, 5);
    layout3->addStretch();
    layout3->addWidget(filterSizeLabel, 3);
    layout3->addWidget(filterSizeSlider, 5);
    layout3->addWidget(filterSizeEdit, 3);
    layout3->addStretch();

    QHBoxLayout *layout4=new QHBoxLayout;
    layout4->addStretch();
    layout4->addWidget(btnOK);
    layout4->addWidget(btnCancel);
    layout4->addStretch();
    layout4->addWidget(btnClose);

    // main layout
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addLayout(layout1, 8);
    mainlayout->addLayout(layout2, 1);
    mainlayout->addLayout(layout3, 1);
    mainlayout->addLayout(layout4, 1);
    setLayout(mainlayout);
}

void FDFilterDialog::setImage(QImage image, QLabel *label)
{
    QPixmap pix;
    pix.fromImage(image);
    label->setPixmap(pix);
}

void FDFilterDialog::updateDstImage(int value)
{
    if (QObject::sender() == filterSizeSlider)
    {
        filterSize = value;
        filterSizeEdit->setText(QString("%1").arg((int)value));
    }
    else if (QObject::sender() == filterTypeComboBox)
    {
        filterType = value;
    }

    // generate filter
    generateFilter(srcImage.width(), srcImage.height(), filterSize, (ImageFilterType)filterType, filter);

    imageFilterFFT2D(shiftedSpectrum, srcImage.width(), srcImage.height(), filter,
                          filteredSpectrumImage, dstImage);
    filteredSpectrumImageLabel->setPixmap(QPixmap::fromImage(filteredSpectrumImage));
    dstImageLabel->setPixmap(QPixmap::fromImage(dstImage));
}
