#include "transform.h"
#include "imageprocess.h"

#ifndef PI
#define PI 3.1415926535
#endif
using namespace std;

int long upper_power_of_two(int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

void fftshift2D(complex<float> *src, int w, int h, complex<float> *dst)
{
    int i, j, ii, jj;
    for(i = 0; i < h; i++)
    {
        for(j = 0; j < 3 * w; j++)
        {
            if (i < h / 2)
                ii = i + h / 2;
            else
                ii = i - h / 2;

            if (j < w * 3 /2)
                jj = j + 3 * w / 2;
            else
                jj = j - 3 * w / 2;

            dst[i * w * 3 + j] = src[ii * w * 3 + jj];
        }
    }
}

void spectrum2QImage(complex<float> *s, int w, int h,  QImage &dst)
{
    int width = dst.width();
    int height = dst.height();
    int pixel_num = width*height;

    float *r = new float[width*height];
    float *g = new float[width*height];
    float *b = new float[width*height];
    float max_r, max_g, max_b;
    float min_r, min_g, min_b;
    dst = QImage(w, h, QImage::Format_RGB888);

    int i, j;
    for(j = (int)(h - height) / 2; j < (int)(h + height) / 2; j++)
    {
        for(i = (int)(w - width) / 2; i < (int)(w + width) / 2; i++)
        {
            // magnitude
            r[j*width+i] = log ( 1 + sqrt(1.0*s[w * 3 * j + 3*i].real() * s[w * 3 * j + 3*i].real() +
                         s[w * 3 * j + 3*i].imag() * s[w * 3 * j + 3*i].imag()) );
            g[j*width+i] = log ( 1+ sqrt(1.0*s[w * 3 * j + 3*i+1].real() * s[w * 3 * j + 3*i+1].real() +
                         s[w * 3 * j + 3*i+1].imag() * s[w * 3 * j + 3*i+1].imag()) );
            b[j*width+i] = log ( 1 + sqrt(1.0*s[w * 3 * j + 3*i+2].real() * s[w * 3 * j + 3*i+2].real() +
                         s[w * 3 * j + 3*i+2].imag() * s[w * 3 * j + 3*i+2].imag()) );
        }
    }
    max_r = r[0]; max_g = g[0]; max_b = b[0];
    min_r = r[0]; min_g = g[0]; min_b = b[0];
    for (i=1; i<pixel_num; i++)
    {
        max_r = r[i]>max_r ? r[i] : max_r;
        max_g = g[i]>max_g ? g[i] : max_g;
        max_b = b[i]>max_b ? b[i] : max_b;

        min_r = r[i]<min_r ? r[i] : min_r;
        min_g = g[i]<min_g ? g[i] : min_g;
        min_b = b[i]<min_b ? b[i] : min_b;
    }

    float temp_r, temp_g, temp_b;
    float s_r = 255/(max_r-min_r);
    float s_g = 255/(max_g-min_g);
    float s_b = 255/(max_b-min_b);
    for(j = 0; j < height; j++)
    {
#pragma omp parallel for
        for(i = 0; i < width; i++)
        {
            temp_r = (r[j*width+i] - min_r)*s_r;
            temp_g = (g[j*width+i] - min_g)*s_g;
            temp_b = (b[j*width+i] - min_b)*s_b;

            dst.setPixel(i, j, qRgb((int)temp_r, (int)temp_g, (int)temp_b));
        }
    }
    delete [] r; delete [] g; delete [] b;
}

void FFT1D(complex<float> *x, complex<float> *y, int r)
{
    int	i, j, k, p;
    complex<float> *weights, *x1, *x2, *temp;
    int N = 1 << r; // FFT length

    weights  = new complex<float>[N / 2];
    x1 = new complex<float>[N];
    x2 = new complex<float>[N];

    // weights
#pragma omp parallel for
    for(i = 0; i < N / 2; i++)
    {
        float angle = -i * PI * 2 / N;
        weights[i] = complex<float> (cos(angle), sin(angle));
    }

    memcpy(x1, x, sizeof(complex<float>) * N);

    // fft
    for(k = 0; k < r; k++)
    {
        for(j = 0; j < 1 << k; j++)
        {
            for(i = 0; i < 1<<(r - k -1); i++)
            {
                p = j * (1<<(r - k));
                x2[i + p] = x1[i + p] + x1[i + p + (int)(1<<(r - k -1))];
                x2[i + p + (int)(1<<(r - k -1))] = (x1[i + p] - x1[i + p + (int)(1<<(r - k -1))]) * weights[i * (1<<k)];
            }
        }
        temp  = x1;
        x1 = x2;
        x2 = temp;
    }

    // sort
    for(j = 0; j < N; j++)
    {
        p = 0;
        for(i = 0; i < r; i++)
        {
            if (j&(1<<i))
            {
                p+=1<<(r - i - 1);
            }
        }
        y[j] = x1[p];
    }

    delete weights;
    delete x1;
    delete x2;
}

void IFFT1D(complex<float> *y, complex<float> *x, int r)
{
    int	i;
    complex<float> *temp;
    int N = 1<<r; // FFT lengtjh

    temp = new complex<float>[N];
    memcpy(temp, y, sizeof(complex<float>) * N);

    // conjugation
#pragma omp parallel for
    for (i = 0; i < N; i++)
        temp[i] = complex<float> (temp[i].real(), -temp[i].imag());

    FFT1D(temp, x, r); // FFT

#pragma omp parallel for
    for (i = 0; i < N; i++)
        x[i] = complex<float> (x[i].real() / N, -x[i].imag() / N);

    delete temp;
}

bool FFT2D(complex<float> *x, int width, int height, complex<float> *y)
{
    int	i, j, k;
    int w = 1;
    int h = 1;
    int wp = 0;
    int hp = 0;

    // radix-2
    while (w < width)
    {
        w *= 2;
        wp++;
    }

    while (h < height)
    {
        h *= 2;
        hp++;
    }

    complex<float> *temp_x, *temp_y;
    temp_x = new complex<float>[h];
    temp_y = new complex<float>[h];

    // y fft
    for (i = 0; i < w * 3; i++)
    {
        for (j = 0; j < h; j++)
            temp_x[j] = x[j * w * 3 + i];

        FFT1D(temp_x, temp_y, hp);

        for (j = 0; j < h; j++)
            x[j * w * 3 + i] = temp_y[j];
    }

    delete [] temp_x;
    delete [] temp_y;

    temp_x = new complex<float>[w];
    temp_y = new complex<float>[w];

    // x fft
    for (i = 0; i < h; i++)
    {
        for (k = 0; k < 3; k++)
        {
            for (j = 0; j < w; j++)
                temp_x[j] = x[i * w * 3 + j * 3 + k];

            FFT1D(temp_x, temp_y, wp);

            for (j = 0; j < w; j++)
                y[i * w * 3 + j * 3 + k] = temp_y[j];
        }
    }

    delete [] temp_x;
    delete [] temp_y;

    return true;
}

bool IFFT2D2QImage(complex<float>* y, int width, int height, QImage &dst)
{
    int	i, j, k;
    int w = 1;
    int h = 1;
    int wp = 0;
    int hp = 0;

    // radix-2
    while(w < width)
    {
        w *= 2;
        wp++;
    }

    while(h < height)
    {
        h *= 2;
        hp++;
    }

    complex<float> *temp_x, *temp_y;
    temp_x = new complex<float>[w];
    temp_y = new complex<float>[w];

    // x fft
    for (i = 0; i < h; i++)
    {
        for (k = 0; k < 3; k++)
        {
            for (j = 0; j < w; j++)
                temp_y[j] = y[i * w * 3 + j * 3 + k];

            IFFT1D(temp_y, temp_x, wp);

            for (j = 0; j < w; j++)
                y[i * w * 3 + j * 3 + k] = temp_x[j];
        }
    }

    delete [] temp_x;
    delete [] temp_y;

    temp_x = new complex<float>[h];
    temp_y = new complex<float>[h];

    for (i = 0; i < w * 3; i++)
    {
        for (j = 0; j < h; j++)
            temp_y[j] = y[j * w * 3 + i];

        IFFT1D(temp_y, temp_x, hp);

        for (j = 0; j < h; j++)
            y[j * w * 3 + i] = temp_x[j];
    }

    delete [] temp_x;
    delete [] temp_y;

    dst = QImage(width, height, QImage::Format_RGB888);

    for(j = 0; j < h; j++)
    {
#pragma omp parallel for
        for(i = 0; i < w; i++)
        {
            if(j < height && i < width)
            {
                int r = (int)y[w * j * 3 + 3*i].real();
                int g = (int)y[w * j * 3 + 3*i+1].real();
                int b = (int)y[w * j * 3 + 3*i+2].real();
                if (r>255) r=255;
                if (g>255) g=255;
                if (b>255) b=255;
                dst.setPixel(i, j, qRgb(r,g,b));
            }
        }
    }
    return true;
}


QImage imageFFT2D(QImage src)
{
    int i, j;
    int width = src.width();
    int height = src.height();

    // radix-2
    int w = 1, h = 1;
    w = upper_power_of_two(width);
    h = upper_power_of_two(height);

    complex<float> *y, *x, *temp;
    y = new complex<float>[w * h * 3];
    x = new complex<float>[w * h * 3];
    temp =  new complex<float>[w * h * 3];
    memset(y, 0, w*h*3*sizeof(complex<float>));
    memset(x, 0, w*h*3*sizeof(complex<float>));
    memset(temp, 0, w*h*3*sizeof(complex<float>));

    for(j = 0; j < h; j++)
    {
        for(i = 0; i < w; i++)
        {
            if(j < height && i < width)
            {
                QRgb rgb = src.pixel(i,j);
                int r = qRed(rgb);
                int g = qGreen(rgb);
                int b = qBlue(rgb);

                x[w * j * 3 + 3*i] = complex<float>(r, 0.0f);
                x[w * j * 3 + 3*i+1] = complex<float>(g, 0.0f);
                x[w * j * 3 + 3*i+2] = complex<float>(b, 0.0f);
            }
        }
    }

    if (FFT2D(x, width, height, y) == false)
        return QImage();

    delete [] x;

    // fftshift
    fftshift2D(y, w, h, temp);

    QImage dst;
    spectrum2QImage(temp, w, h, dst);

    delete [] y;
    delete [] temp;

    return dst;
}

void imageFilterFFT2D1(QImage src, int r, int option, QImage &originalSpectrumImage,
                      QImage &filteredSpectrumImage, QImage &dstImage)
{
    int i, j;
    int width = src.width();
    int height = src.height();
    int pixel_num = width*height;

    // radix-2
    int w = 1, h = 1;
    w = upper_power_of_two(width);
    h = upper_power_of_two(height);

    complex<float> *y, *x, *temp;
    y = new complex<float>[w * h * 3];
    x = new complex<float>[w * h * 3];
    temp =  new complex<float>[w * h * 3];
    memset(y, 0, w*h*3*sizeof(complex<float>));
    memset(x, 0, w*h*3*sizeof(complex<float>));
    memset(temp, 0, w*h*3*sizeof(complex<float>));

    for(j = 0; j < h; j++)
    {
        for(i = 0; i < w; i++)
        {
            if(j < height && i < width)
            {
                QRgb rgb = src.pixel(i,j);
                int r = qRed(rgb);
                int g = qGreen(rgb);
                int b = qBlue(rgb);

                x[w * j * 3 + 3*i] = complex<float>(r, 0.0f);
                x[w * j * 3 + 3*i+1] = complex<float>(g, 0.0f);
                x[w * j * 3 + 3*i+2] = complex<float>(b, 0.0f);
            }
        }
    }

    if (FFT2D(x, width, height, y) == false)
        return ;

    /*
    // original spectrum
    originalSpectrumImage = src;
    spectrum2QImage(FD, w, h, originalSpectrumImage);
    */
    memset(x, 0, w*h*3*sizeof(complex<float>));

    // fftshift
    fftshift2D(y, w, h, temp);
    // original spectrum
    spectrum2QImage(temp, w, h, originalSpectrumImage);

    // filter
    for(i = 0; i < h; i++)
    {
        for(j = 0; j < 3 * w; j++)
        {
            int j1 = j/3;
            float r1 = sqrt( (i-h/2)*(i-h/2) + (j1-w/2)*(j1-w/2) );
            if (r1 > r && (option == 0) )
                temp[i * w * 3 + j] = 0;
            if (r1 < r && (option != 0) )
                temp[i * w * 3 + j] = 0;
        }
    }

    // filtered spectrum
    spectrum2QImage(temp, w, h, filteredSpectrumImage);

    // fftshift
    fftshift2D(temp, w, h, y);

    // spectrum to QImage
    IFFT2D2QImage(y, width, height, dstImage);

    delete [] x;
    delete [] y;
    delete [] temp;
}


void fftshift2D(fftwf_complex *src, int w, int h, fftwf_complex *dst)
{
    int i, j, ii, jj, c;
    int n = w*h;
    for (c = 0; c<3; c++)
    {
        for(i = 0; i < h; i++)
        {
            for(j = 0; j < w; j++)
            {
                if (i < h / 2)
                    ii = i + h / 2;
                else
                    ii = i - h / 2;

                if (j < w /2)
                    jj = j + w / 2;
                else
                    jj = j - w / 2;

                dst[c*n + i * w + j][0] = src[c*n + ii * w + jj][0];
                dst[c*n + i * w + j][1] = src[c*n + ii * w + jj][1];
            }
        }
    }
}

void fftw2d(float *x, int w, int h, fftwf_complex *y)
{
    fftwf_plan plan;
    int n = w*h;
    fftwf_complex *temp = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);
    for (int i=0; i<3*n; i++)
    {
        temp[i][0]= x[i];
        temp[i][1] = 0;
    }

    for (int i=0; i<3; i++)
    {
        plan = fftwf_plan_dft_2d(h, w, temp+i*n, y+i*n, FFTW_FORWARD, FFTW_ESTIMATE);
        fftwf_execute(plan);
        fftwf_destroy_plan(plan);
    }

    fftwf_free(temp);
}

void spectrum2QImage(fftwf_complex *s, int width, int height, QImage &dst)
{
    int pixel_num = width*height;

    float *r = new float[width*height];
    float *g = new float[width*height];
    float *b = new float[width*height];
    float max_r, max_g, max_b;
    float min_r, min_g, min_b;

    dst = QImage(width, height, QImage::Format_RGB888);

    int i, j;
    for(j = 0; j < height; j++)
    {
        for(i = 0; i < width; i++)
        {
            // magnitude
            r[j*width+i] = log ( 1 + sqrt(1.0*s[j*width+i][0] * s[j*width+i][0] +
                         s[j*width+i][1] * s[j*width+i][1]) );
            g[j*width+i] = log ( 1+ sqrt(1.0*s[pixel_num+j*width+i][0] * s[pixel_num+j*width+i][0] +
                         s[pixel_num+j*width+i][1] * s[pixel_num+j*width+i][1]) );
            b[j*width+i] = log ( 1 + sqrt(1.0*s[2*pixel_num+j*width+i][0] * s[2*pixel_num+j*width+i][0] +
                         s[2*pixel_num+j*width+i][1] * s[2*pixel_num+j*width+i][1]) );
        }
    }
    max_r = r[0]; max_g = g[0]; max_b = b[0];
    min_r = r[0]; min_g = g[0]; min_b = b[0];
    for (i=1; i<pixel_num; i++)
    {
        max_r = r[i]>max_r ? r[i] : max_r;
        max_g = g[i]>max_g ? g[i] : max_g;
        max_b = b[i]>max_b ? b[i] : max_b;

        min_r = r[i]<min_r ? r[i] : min_r;
        min_g = g[i]<min_g ? g[i] : min_g;
        min_b = b[i]<min_b ? b[i] : min_b;
    }

    float temp_r, temp_g, temp_b;
    float s_r = 255/(max_r-min_r);
    float s_g = 255/(max_g-min_g);
    float s_b = 255/(max_b-min_b);
    for(j = 0; j < height; j++)
    {
        for(i = 0; i < width; i++)
        {
            temp_r = (r[j*width+i] - min_r)*s_r;
            temp_g = (g[j*width+i] - min_g)*s_g;
            temp_b = (b[j*width+i] - min_b)*s_b;

            dst.setPixel(i, j, qRgb((int)temp_r, (int)temp_g, (int)temp_b));
        }
    }
    delete [] r; delete [] g; delete [] b;
}

bool IFFT2D2QImage(fftwf_complex *y, int w, int h, QImage &dst)
{
    int	i, j;
    int n = w*h;
    fftwf_plan plan;
    fftwf_complex *x = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);

    for (i=0; i<3; i++)
    {
        plan = fftwf_plan_dft_2d(h, w, y+i*n, x+i*n, FFTW_BACKWARD, FFTW_ESTIMATE);
        fftwf_execute(plan);
        fftwf_destroy_plan(plan);
    }

    dst = QImage(w, h, QImage::Format_RGB888);
    for(j = 0; j < h; j++)
    {
        for(i = 0; i < w; i++)
        {
            int r = (int)(x[j*w+i][0]/n);
            int g = (int)(x[j*w+i+n][0]/n);
            int b = (int)(x[j*w+i+2*n][0]/n);
            if (r>255) r=255;
            if (g>255) g=255;
            if (b>255) b=255;
            dst.setPixel(i, j, qRgb(r,g,b));
        }
    }
    fftwf_free(x);
    return true;
}

void imageFilterFFT2D(QImage src, int r, int option, QImage &originalSpectrumImage,
                      QImage &filteredSpectrumImage, QImage &dstImage)
{
    int i, j;
    int w = src.width();
    int h = src.height();
    int n = w*h;

    float *channels = new float[n*3];
    float *rr = channels;
    float *gg = channels+n;
    float *bb = channels+2*n;
    splitImageChannel(src, rr, gg, bb);

    fftwf_complex *y, *temp;
    y = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);
    temp = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);

    // original spectrum
    fftw2d(channels, w, h, y);

    // fftshift
    fftshift2D(y, w, h, temp);

    // original spectrum
    spectrum2QImage(temp, w, h, originalSpectrumImage);

    // filter
    for (int c = 0; c<3; c++)
    {
        for(i = 0; i < h; i++)
        {
            for(j = 0; j < w; j++)
            {
                float r1 = sqrt( i*i + j*j );
                if (r1 > r && (option == 0) )
                {
                    temp[c*n + i * w + j][0] = 0;
                    temp[c*n + i * w + j][1] = 0;
                }
                if (r1 < r && (option != 0) )
                {
                    temp[c*n + i * w + j][0] = 0;
                    temp[c*n + i * w + j][1] = 0;
                }
            }
        }
    }

    // filtered spectrum
    spectrum2QImage(temp, w, h, filteredSpectrumImage);

    // fftshift
    fftshift2D(temp, w, h, y);

    // spectrum to QImage
    IFFT2D2QImage(y, w, h, dstImage);

    fftwf_free(y);
    fftwf_free(temp);
}

void generateFilter(int w, int h, int r, ImageFilterType type, float *filter)
{
    int i, j, c;
    int pixel_num = w*h;
    float area = r*r;
    int n = 2;

    memset(filter, 0, 3*pixel_num*sizeof(float));
    for (c = 0; c<3; c++)
    {
        for(i = 0; i < h; i++)
        {
            for(j = 0; j < w; j++)
            {
                float area1 = (i-h/2)*(i-h/2) + (j-w/2)*(j-w/2);
                switch ((int)type) {
                case ImageFilterType::IdealLowPass:
                    if (area1 <= area )// low-pass filter
                        filter[c*pixel_num + i * w + j] = 1;
                    break;
                case ImageFilterType::IdealHighPass:
                    if (area1 > area ) // high-pass filter
                        filter[c*pixel_num + i * w + j] = 1;
                    break;
                case ImageFilterType::GaussainLowPass:
                    filter[c*pixel_num + i * w + j] = exp(-area1 / (2*r*r));
                    break;
                case ImageFilterType::ButterworthLowPass:
                    filter[c*pixel_num + i * w + j] = 1 / (1 + pow(sqrt(area1)/r,  2*n));
                    break;
                case ImageFilterType::ButterworthHighPass:
                    filter[c*pixel_num + i * w + j] = 1 / (1 + pow(r/sqrt(area1),  2*n));
                    break;
                }
            }
        }
    }
}

void calcImageSpectrum(QImage src, QImage &dst)
{
    int w = src.width();
    int h = src.height();
    int n = w*h;

    float *channels = new float[n*3];
    float *rr = channels;
    float *gg = channels+n;
    float *bb = channels+2*n;
    splitImageChannel(src, rr, gg, bb);

    fftwf_complex *y, *temp;
    y = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);
    temp = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);

    // original spectrum
    fftw2d(channels, w, h, y);

    // fftshift
    fftshift2D(y, w, h, temp);

    // original spectrum
    spectrum2QImage(temp, w, h, dst);

    fftwf_free(y);
    fftwf_free(temp);
    delete [] channels;
}

void imageFilterFFT2D(fftwf_complex *y, int w, int h, float *filter,
                      QImage &filteredSpectrumImage, QImage &dstImage)
{
    int i;
    int n = w*h;

    fftwf_complex *yy = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);
    memcpy(yy, y, 3*n*sizeof(fftwf_complex));

    // filter
    for (i = 0; i<3*n; i++)
    {
        yy[i][0] *= filter[i];
        yy[i][1] *= filter[i];
    }

    // filtered spectrum
    spectrum2QImage(yy, w, h, filteredSpectrumImage);

    // fftshift
    fftwf_complex *temp = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * 3*n);
    fftshift2D(yy, w, h, temp);

    // spectrum to QImage
    IFFT2D2QImage(temp, w, h, dstImage);

    fftwf_free(yy);
    fftwf_free(temp);
}
