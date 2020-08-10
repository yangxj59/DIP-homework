// 形态学处理之膨胀
#include "dilate.h"
QImage calculatedilate(QImage &image)
{
    // obtain gray image
    int width = image.width();
    int height = image.height();

    QImage newImage = image;
    int kernel[7][7] = {
        { 0,0,0,1,0,0,0 },
        { 0,1,1,1,1,1,0 },
        { 0,1,1,1,1,1,0 },
        { 1,1,1,1,1,1,1 },
        { 0,1,1,1,1,1,0 },
        { 0,1,1,1,1,1,0 },
        { 0,0,0,1,0,0,0 } };
    int sizeKernel = 7;
    QColor color;
    QColor Rcolor;

    for (int y = sizeKernel / 2; y <height- sizeKernel / 2; y++)
    {
        for (int x = sizeKernel / 2; x < width- sizeKernel / 2; x++)
        {
            int kr = 0;
            int kg = 0;
            int kb = 0;
            Rcolor = QColor(image.pixel(x, y));
                        for (int j = -sizeKernel / 2; j <= sizeKernel / 2; j++)
                        {
                            for (int i = -sizeKernel / 2; i <= sizeKernel / 2; i++)
                            {
                                color = QColor(image.pixel(x + i, y + j));
                                while (color.red() > kr && kernel[sizeKernel / 2 + i][sizeKernel / 2 + j])
                                {
                                    kr = color.red();
                                }
                                while (color.green() > kg && kernel[sizeKernel / 2 + i][sizeKernel / 2 + j])
                                {
                                    kg = color.green();
                                }
                                while (color.blue() > kb && kernel[sizeKernel / 2 + i][sizeKernel / 2 + j])
                                {
                                    kb = color.blue();
                                }
                            }
                        }
                        newImage.setPixel(x, y, qRgb(kr, kg, kb));
                    }
                }
                return newImage;
            }
