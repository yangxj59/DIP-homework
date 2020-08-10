// Laplace 算子实现素描
#include "Sketch.h"
QImage calculateSketch(QImage &image)
{
    int width = image.width();
    int height = image.height();
    int scale = 1;
    int k = 20;
    int ka = 20;

    QImage newImage = image;
    QImage newImage1 = image;
    QColor color0;
    int templt[3][3] = {
        {1,1,1},
        {1,-8,1},
        {1,1,1}
    };
    int templt1[3][3] = {
        {1,1,-1},
        {1,0,-1},
        {1,-1,-1}
    };
    int templt2[3][3] = {
        {1,1,1},
        {-1,0,1},
        {-1,-1,-1}
    };
    int templt3[3][3] = {
        {1,1,1},
        {1,4,1},
        {1,1,1}
    };
    int templtsize = 3;
    for(int y = templtsize/2; y < height - templtsize/2; y++)
    {
        for(int x = templtsize/2; x < width - templtsize/2;x++)
        {
            int r =0;
            int g = 0;
            int b = 0;
            int r1 =0;
            int g1= 0;
            int b1 = 0;
            int r2 =0;
            int g2 = 0;
            int b2 = 0;
            int r3=0;
            int g3 = 0;
            int b3 = 0;
            for(int j = -templtsize/2; j <= templtsize/2;j++)
            {
                for(int i = -templtsize/2; i <= templtsize/2;i++)
                {
                       color0 = QColor(image.pixel(x + i,y + j));
                       r += color0.red() * templt[i + templtsize/2][j + templtsize/2];
                       g += color0.green() * templt[i + templtsize/2][j + templtsize/2];
                       b += color0.blue() * templt[i + templtsize/2][j + templtsize/2];

                       r1+= color0.red() * templt1[i + templtsize/2][j + templtsize/2];
                       g1 += color0.green() * templt1[i + templtsize/2][j + templtsize/2];
                       b1 += color0.blue() * templt1[i + templtsize/2][j + templtsize/2];

                       r2 += color0.red() * templt2[i + templtsize/2][j + templtsize/2];
                       g2 += color0.green() * templt2[i + templtsize/2][j + templtsize/2];
                       b2 += color0.blue() * templt2[i + templtsize/2][j + templtsize/2];
                }
            }
            r1 = abs(r1);
            g1 = abs(g1);
            b1 = abs(b1);

            r2 = abs(r2);
            g2 = abs(g2);
            b2 = abs(b2);

            r3 = r1>r2?r1:r2;
            g3 = g1>g2?g1:g2;
            b3 = b1>b2?b1:b2;

            if(r3<k) r = 0;
            else
            {
                 r*= scale;
                 if(r > 255) r = 255;
                 else if(r < ka) r = 0;
            }
            if(g3<k) g = 0;
            else
            {
                 g*= scale;
                 if(g > 255) g = 255;
                 else if(g < ka) g = 0;
            }
            if(b3<k) b = 0;
            else
            {
                 b*= scale;
                 if(b > 255) b = 255;
                 else if(b < ka) b = 0;
            }

            r = 255 -r;
            g = 255 - g;
            b = 255 - b;

            newImage1.setPixel(x,y,qRgb(r,g,b));
    }
}
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
                       color0 = QColor(newImage1.pixel(x + i,y + j));
                       r += color0.red() * templt3[i + templtsize/2][j + templtsize];
                       g += color0.green() * templt3[i + templtsize/2][j + templtsize];
                       b += color0.blue() * templt3[i + templtsize/2][j + templtsize];
                }
            }
            r = r/12;
            g = g/12;
            b = b/12;
          newImage.setPixel(x,y,qRgb(r,g,b));
        }
    }
    //delete newImage1;
    return newImage;
}
