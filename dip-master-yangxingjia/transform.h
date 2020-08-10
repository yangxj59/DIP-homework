#ifndef TRANSFORM_H   // "if not define a.h"
#define TRANSFORM_H   // #define A_H  a.h
#include <complex>
#include <QImage>
#include "fftw3.h"
// define enum
enum ImageFilterType{
    IdealLowPass = 0,
    IdealHighPass,
    GaussainLowPass,
    ButterworthLowPass,
    ButterworthHighPass,
};

QImage imageFFT2D(QImage src);
void imageFilterFFT2D(QImage src, int r, int option, QImage &originalSpectrumImage,
                      QImage &filteredSpectrumImage, QImage &dstImage);
void imageFilterFFT2D(fftwf_complex *y, int w, int h, float *filter,
                      QImage &filteredSpectrumImage, QImage &dstImage);
void generateFilter(int w, int h, int r, ImageFilterType type, float *filter);
void fftw2d(float *x, int w, int h, fftwf_complex *y);
void fftshift2D(fftwf_complex *src, int w, int h, fftwf_complex *dst);
void calcImageSpectrum(QImage src, QImage &dst);
void spectrum2QImage(fftwf_complex *s, int width, int height, QImage &dst);


#endif // TRANSFORM_H   #endif
