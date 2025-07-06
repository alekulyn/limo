#include "qmodelindexcheck.h"

bool sameRow (const QModelIndex& a, const QModelIndex& b)
{
  return
    a.row() == b.row() &&
    a.parent() == b.parent() &&
    a.model() == b.model() &&
    a.internalPointer() == b.internalPointer();
}
