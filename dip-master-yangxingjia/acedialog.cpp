#include "acedialog.h"

ACEDialog::ACEDialog(QImage inputImage)
{
    srcImage = inputImage;

    int image_width = srcImage.width();
    int image_height = srcImage.height();
    int ori_pixel_num = image_width*image_height;

    int pixel_num = (image_width+2*maxFilterSize)*(image_height+2*maxFilterSize);

    // obtain image channels
    rgb = new float[3*ori_pixel_num];
    rgb_pad = new float[3*pixel_num];
    rgb_ii = new float[3*pixel_num];
    rgb_ii_power = new float[3*pixel_num];
    splitImageChannel(srcImage, rgb, rgb+ori_pixel_num, rgb+2*ori_pixel_num);
    paddingZeros(rgb, rgb+ori_pixel_num, rgb+2*ori_pixel_num, image_width, image_height,
                 maxFilterSize, maxFilterSize, rgb_pad, rgb_pad+pixel_num, rgb_pad+2*pixel_num);
    for (int i=0; i<3; i++)
    {
        //calculate_integral_image(rgb+i*pixel_num, image_width, image_height, rgb_ii+i*pixel_num);
        //calculate_integral_image_power(rgb+i*pixel_num, image_width, image_height, rgb_ii_power+i*pixel_num);
        calculate_integral_image(rgb_pad+i*pixel_num, image_width+2*maxFilterSize,
                                 image_height+2*maxFilterSize, rgb_ii+i*pixel_num);
        calculate_integral_image_power(rgb_pad+i*pixel_num, image_width+2*maxFilterSize,
                                       image_height+2*maxFilterSize, rgb_ii_power+i*pixel_num);
    }
    adaptiveContrastEnhancement(srcImage, rgb_pad, rgb_ii, rgb_ii_power, maxFilterSize,
                                filterSize,  gainCoef, maxCG, dstImage);

    iniUI();

    srcImageLabel->setPixmap(QPixmap::fromImage(srcImage));
    dstImageLabel->setPixmap(QPixmap::fromImage(dstImage));

}

void ACEDialog::iniUI()
{
    // two image labels
    srcImageLabel = new QLabel();
    srcImageLabel->setAlignment(Qt::AlignCenter);
    dstImageLabel = new QLabel();
    dstImageLabel->setAlignment(Qt::AlignCenter);

    // filter size
    filterSizeLabel = new QLabel(tr("Filter Size"));
    filterSizeSlider = new FloatSlider(Qt::Horizontal);
    filterSizeEdit = new QLineEdit("0");

    filterSizeLabel->setMinimumWidth(30);
    filterSizeLabel->setAlignment(Qt::AlignRight);
    filterSizeEdit->setMinimumWidth(16);
    filterSizeSlider->setFloatRange(1, maxFilterSize);
    filterSizeSlider->setFloatStep(2);
    filterSizeSlider->setFloatValue(filterSize);
    filterSizeEdit->setText(QString("%1").arg(filterSize));

    // constrast gain upper limit
    maxCGLabel = new QLabel(tr("MAX CG"), this);
    maxCGLabel->setAlignment(Qt::AlignRight);
    maxCGSlider = new FloatSlider(Qt::Horizontal, this);
    maxCGEdit = new QLineEdit("0", this);
    maxCGLabel->setMinimumWidth(30);
    maxCGEdit->setMinimumWidth(16);
    maxCGSlider->setFloatRange(1, 10);
    maxCGSlider->setFloatStep(0.01f);
    maxCGSlider->setFloatValue(maxCG);
    maxCGEdit->setText(QString::number(maxCG, 'f', 2));

    // gain coef
    gainCoefLabel = new QLabel(tr("Gain Coef"), this);
    gainCoefLabel->setAlignment(Qt::AlignRight);
    gainCoefSlider = new FloatSlider(Qt::Horizontal, this);
    gainCoefEdit = new QLineEdit("0", this);
    gainCoefLabel->setMinimumWidth(30);
    gainCoefEdit->setMinimumWidth(16);
    gainCoefSlider->setFloatRange(1, 500);
    gainCoefSlider->setFloatStep(0.01f);
    gainCoefSlider->setFloatValue(gainCoef);
    gainCoefEdit->setText(QString::number(gainCoef, 'f', 2));

    // signal slot
    connect(filterSizeSlider, SIGNAL(floatValueChanged(float)), this, SLOT(updateDstImage(float)));
    connect(maxCGSlider, SIGNAL(floatValueChanged(float)), this, SLOT(updateDstImage(float)));
    connect(gainCoefSlider, SIGNAL(floatValueChanged(float)), this, SLOT(updateDstImage(float)));

    filterSizeSlider->setFloatValue(filterSize);
    gainCoefSlider->setFloatValue(gainCoef);
    maxCGSlider->setFloatValue(maxCG);

    // three buttons
    btnOK = new QPushButton(tr("OK"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnClose = new QPushButton(tr("Exit"));

    connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(srcImageLabel);
    layout1->addWidget(dstImageLabel);

    QHBoxLayout *layout2 = new QHBoxLayout;

    layout2->addWidget(filterSizeLabel, 3);
    layout2->addWidget(filterSizeSlider, 5);
    layout2->addWidget(filterSizeEdit, 2);
    layout2->addStretch();

    layout2->addWidget(maxCGLabel, 3);
    layout2->addWidget(maxCGSlider, 5);
    layout2->addWidget(maxCGEdit, 2);
    layout2->addStretch();

    layout2->addWidget(gainCoefLabel,3);
    layout2->addWidget(gainCoefSlider,5);
    layout2->addWidget(gainCoefEdit,2);

    QHBoxLayout *layout3=new QHBoxLayout;
    layout3->addStretch();
    layout3->addWidget(btnOK);
    layout3->addWidget(btnCancel);
    layout3->addStretch();
    layout3->addWidget(btnClose);


    // main layout
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addLayout(layout1, 8);
    mainlayout->addLayout(layout2, 1);
    mainlayout->addLayout(layout3, 1);
    setLayout(mainlayout);
}

void ACEDialog::setImage(QImage image, QLabel *label)
{
    QPixmap pix;
    pix.fromImage(image);
    //pix = pix.scaled(label->size(), Qt::KeepAspectRatio);
    label->setPixmap(pix);
}

void ACEDialog::updateDstImage(float value)
{
    if (QObject::sender() == filterSizeSlider)
    {
        filterSize = (int)value;
        filterSizeEdit->setText(QString("%1").arg((int)value));
    }
    else if (QObject::sender() == gainCoefSlider)
    {
        gainCoef = value;
        gainCoefEdit->setText(QString("%1").arg(value));
    }
    else if (QObject::sender() == maxCGSlider)
    {
        maxCG = value;
        maxCGEdit->setText(QString("%1").arg(value));
    }

    adaptiveContrastEnhancement(srcImage, rgb_pad, rgb_ii, rgb_ii_power, maxFilterSize,
                                filterSize,  gainCoef, maxCG, dstImage);
    dstImageLabel->setPixmap(QPixmap::fromImage(dstImage));
}
