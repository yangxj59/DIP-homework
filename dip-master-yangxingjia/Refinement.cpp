// Refinement of morphology
#include "Refinement.h"
QImage calculateRefinement(QImage &image)
{
    int width = image.width();
    int height = image.height();


    QImage newImage = image;
    int count;
    int finish = 0;
    int nb[5][5];
    int r = 0;
    int b = 0;
    int g = 0;
    QColor color;
    /*二值化*/

    for (int y = 0; y<height; y++)
    {
        for (int x = 0; x<width; x++)
        {
            color = QColor(image.pixel(x, y));


            color = QColor(image.pixel(x, y));
            if (color.red()< 100)
            {
                r = 0;
            }
            else
            {
                r = 255;
            }
            if (color.green()< 100)
            {
                g = 0;
            }
            else
            {
                g = 255;
            }
            if (color.blue() < 100)
            {
                b = 0;
            }
            else
            {
                b = 255;
            }
            newImage.setPixel(x, y, qRgb(r, g, b));
        }
    }
    while (!finish)
    {
        finish = 1;
        for (int y = 2; y< image.height() - 2; y++)
        {
            for (int x = 2; x< image.width() - 2; x++)
            {
                color = QColor(newImage.pixel(x, y));
                if (color.red() == 0)
                {
                    continue;
                }
                int  t1 = 0;
                int  t2 = 0;

                for (int j = 0; j < 5; j++)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        color = QColor(newImage.pixel(x + i - 2, y + j - 2));
                        if (color.red() == 255)
                        {
                            nb[j][i] = 1;
                        }
                        else
                        {
                            nb[j][i] = 0;
                        }
                    }
                }
                /*条件1*/
                count = nb[1][1] + nb[1][2] + nb[1][3] + nb[2][1] + nb[2][3] + nb[3][1] + nb[3][2] + nb[3][3];
                if (count >= 2 && count <= 6)
                {
                    t1 = 1;
                }
                else
                {
                    continue;
                }
                /*条件2*/
                count = 0;
                if (nb[1][2] == 0 && nb[1][1] == 1)
                    count++;
                if (nb[1][1] == 0 && nb[2][1] == 1)
                    count++;
                if (nb[2][1] == 0 && nb[3][1] == 1)
                    count++;
                if (nb[3][1] == 0 && nb[3][2] == 1)
                    count++;
                if (nb[3][2] == 0 && nb[3][3] == 1)
                    count++;
                if (nb[3][3] == 0 && nb[2][3] == 1)
                    count++;
                if (nb[2][3] == 0 && nb[1][3] == 1)
                    count++;
                if (nb[1][3] == 0 && nb[1][2] == 1)
                    count++;
                if (count == 1)
                {
                    t2 = 1;
                }
                else
                {
                    continue;
                }
                if (t1*t2 == 1)
                {
                    newImage.setPixel(x, y, qRgb(0, 0, 0));
                    finish = 0;
                }
            }
        }
        finish = 1;
    }

    return newImage;
}
