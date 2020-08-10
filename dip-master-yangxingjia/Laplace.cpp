// Laplace 算子边缘检测：mode=1 为正边缘；mode=0为负边缘
#include "Laplace.h"
QImage calculateLaplace(QImage &image)
{
    int width = image.width();
    int height = image.height();
    int scale = 1;
    int mode = 0;

    QImage newImage = image;
    QColor color0;
    int templt[3][3] = {
        {1,1,1},
        {1,-8,1},
        {1,1,1}
    };
    int templtsize = 3;
    for(int y = templtsize/2; y < height - templtsize/2; y++)
    {
        for(int x = templtsize/2; x < width  - templtsize/2;x++)
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
           r*= scale;
           g*=scale;
           b*=scale;
           r = qBound(-255,r,255);
           g = qBound(-255,g,255);
           b = qBound(-255,b,255);
           switch (mode) {
           case 0:
               if(r<0) r = 0;
               break;
           case 1:
               if(r>0) r = 0;
               r = -r;
               break;
           case 2:
               if(r<0) r = -r;
               break;
           case 3:
               if(r<0) r = 128 + r/2;
               break;
           default:
               break;
           }
           switch (mode) {
           case 0:
               if(g<0) g = 0;
               break;
           case 1:
               if(g>0) g = 0;
               g = -g;
               break;
           case 2:
               if(g<0) g = -g;
               break;
           case 3:
               if(g<0) g = 128 + g/2;
               break;
           default:
               break;
           }
           switch (mode) {
           case 0:
               if(b<0) b = 0;
               break;
           case 1:
               if(b>0) b = 0;
               b = -b;
               break;
           case 2:
               if(b<0) b = -b;
               break;
           case 3:
               if(b<0) b = 128 + b/2;
               break;
           default:
               break;
           }
            newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }
    return newImage;
}
