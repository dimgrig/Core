#ifndef CUSTOMAXISTAG_H
#define CUSTOMAXISTAG_H

#include <QObject>
#include <qcustomplot/qcustomplot.h>

#include "core-plot-lib-global.h"

class CORE_PLOT_LIB_EXPORT CustomAxisTag : public QObject
{
  Q_OBJECT
public:
  explicit CustomAxisTag(QCPAxis *parentAxis);
  virtual ~CustomAxisTag();

  // setters:
  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);
  void setText(const QString &text);

  // getters:
  QPen pen() const { return mLabel->pen(); }
  QBrush brush() const { return mLabel->brush(); }
  QString text() const { return mLabel->text(); }

  // other methods:
  void updatePosition(double value);

protected:
  QCPAxis *mAxis;
  QPointer<QCPItemTracer> mDummyTracer;
  QPointer<QCPItemLine> mArrow;
  QPointer<QCPItemText> mLabel;
};

#endif // CUSTOMAXISTAG_H
