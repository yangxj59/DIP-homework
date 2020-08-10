QT += widgets
requires(qtConfig(filedialog))
qtHaveModule(printsupport): QT += printsupport
QMAKE_CXXFLAGS+= -openmp

HEADERS       = mainwindow.h \
                Gauss.h \
                Laplace.h \
                Refinement.h \
                Robert.h \
                Sketch.h \
                Sobel.h \
                acedialog.h \
                closing.h \
                corrosion.h \
                dilate.h \
                opening.h \
                embossfilterdialog.h \
                fdfilterdialog.h \
                fftw3.h \
                floatslider.h \
                imageprocess.h \
                mdichild.h \
                padding.h \
                sdfilterdialog.h \
                transform.h
SOURCES       = main.cpp \
                Gauss.cpp \
                Laplace.cpp \
                Refinement.cpp \
                Robert.cpp \
                Sketch.cpp \
                Sobel.cpp \
                acedialog.cpp \
                closing.cpp \
                corrosion.cpp \
                dilate.cpp \
                opening.cpp \
                embossfilterdialog.cpp \
                fdfilterdialog.cpp \
                imagepocess.cpp \
                mainwindow.cpp \
                mdichild.cpp \
                padding.cpp \
                sdfilterdialog.cpp \
                transform.cpp
RESOURCES     = \
    dip.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/mdi
INSTALLS += target

win32: LIBS += -L$$PWD/./ -llibfftw3-3 -llibfftw3f-3 -llibfftw3l-3

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

TRANSLATIONS += $$PWD/languages/zh_CN.ts \
               $$PWD/languages/en_US.ts
