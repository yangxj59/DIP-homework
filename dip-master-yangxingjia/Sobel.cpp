// sidetest:Sobel
#include "Sobel.h"
QImage calculateSobel(QImage &image)
{
    int width = image.width();
    int height = image.height();
    int scale = 1;
    int type = 1;         // use which formula  type=0 or 1

    QImage newImage = image;

    QColor color0;
    QColor color1;
    QColor color2;
    QColor color3;
    QColor color4;
    QColor color5;
    QColor color6;
    QColor color7;
    QColor color8;
    int  r = 0;
    int g = 0;
    int b = 0;
    int rgb = 0;
    int r1 = 0;
    int g1 = 0;
    int b1 = 0;
    int rgb1 = 0;
    int a = 0;
    for( int y = 1; y < height - 1; y++)
    {
        for(int x = 1; x < width - 1; x++)
        {
            color0 =   QColor ( image.pixel(x,y));

            color1=   QColor ( image.pixel(x-1,y-1));
            color2 =   QColor ( image.pixel(x,y-1));
            color3 =   QColor ( image.pixel(x+1,y));

            color4 =   QColor ( image.pixel(x-1,y));
            color5 =   QColor ( image.pixel(x+1,y));

            color6 =   QColor ( image.pixel(x-1,y+1));
            color7=   QColor ( image.pixel(x,y+1));
            color8 =   QColor ( image.pixel(x+1,y+1));
            r = abs(color1.red() + color2.red() * 2 + color3.red() - color6.red() - color7.red() * 2 - color8.red());
            g = abs(color1.green() + color2.green() * 2 + color3.green() - color6.green() - color7.green() * 2 - color8.green());
            b = abs(color1.blue() + color2.blue() * 2 + color3.blue() - color6.blue() - color7.blue() * 2 - color8.blue());
            rgb = r + g + b;

            r1 = abs(color1.red() + color4.red() * 2 + color6.red() - color3.red() - color5.red() * 2 - color8.red());
            g1= abs(color1.green() + color4.green() * 2 + color6.green() - color3.green() - color5.green() * 2 - color8.green());
            b1 = abs(color1.blue() + color4.blue() * 2 + color6.blue() - color3.blue() - color5.blue() * 2 - color8.blue());
            rgb1 = r1 + g1 + b1;
           if(type == 0)
           {
               if (rgb > rgb1)
                   a = rgb;
               else
                   a = rgb1;
           }
           else if(type == 1)
           {
            a = (rgb + rgb1)/2;
           }
            a = a * scale;
            a = a>255?255:a;

            newImage.setPixel(x,y,qRgb(a,a,a));
        }
    }
    return newImage;
}
