#ifndef FLOATSLIDER_H
#define FLOATSLIDER_H

#include <QSlider>

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

class FloatSlider : public QSlider
{
    Q_OBJECT

public:
    FloatSlider(Qt::Orientation ori = Qt::Horizontal, QWidget *parent = nullptr)
        : QSlider(ori, parent)
    {
        prec = 100;
        this->setOrientation(ori);
        connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(notifyValueChanged(int)));
    }
    void setFloatValue(float value)
    {
        this->setValue(value*prec);
    }
    void setFloatRange(float min, float max)
    {
        this->setRange(min*prec, max*prec);
    }
    void setFloatStep(float step)
    {
        this->setSingleStep(step*prec);
    }

private:
    int prec;

signals:
    void floatValueChanged(float value);

public slots:
    void notifyValueChanged(int value)
    {
        float floatValue = value*1.0 / prec;
        emit floatValueChanged(floatValue);
    }
};

#endif // FLOATSLIDER_H
