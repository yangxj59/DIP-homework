// sidetest:Gauss
#include "Gauss.h"
QImage calculateGauss(QImage &image)
{
    int width = image.width();
    int height = image.height();
    int scale = 1;

    QImage newImage = image;
    QColor color0;
    int templt[5][5] = {
        {0,0,-1,0,0},
        {0,-1,-2,-1,0},
        {-1,-2,16,-2,-1},
        {0,-1,-2,-1,0},
        {0,0,-1,0,0}
    };

    int templtsize = 5;
    for(int y = templtsize/2; y < height - templtsize/2; y++)
    {
        for(int x = templtsize/2; x < width - templtsize/2;x++)
        {
            int r =0;
            int g = 0;
            int b = 0;
            for(int j = -templtsize/2; j <= templtsize/2;j++)
            {
                for(int i = -templtsize/2; i <= templtsize/2;i++)
                {
                       color0 = QColor(image.pixel(x + i,y + j));
                       r += color0.red() * templt[i + templtsize/2][j + templtsize/2];
                       g += color0.green() * templt[i + templtsize/2][j + templtsize/2];
                       b += color0.blue() * templt[i + templtsize/2][j + templtsize/2];
                }
            }
            r*=scale;
            g*=scale;
            b*=scale;
            r = qBound(0,r,255);
            g = qBound(0,g,255);
            b = qBound(0,b,255);
            newImage.setPixel(x,y,qRgb(r,g,b));
    }
}
    return newImage;
}
