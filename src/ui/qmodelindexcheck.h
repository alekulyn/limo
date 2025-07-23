#include <QModelIndex>

#ifndef QMODELINDEXCHECK_H
#define QMODELINDEXCHECK_H

bool sameRow (const QModelIndex& a, const QModelIndex& b);

template <typename T>
std::shared_ptr<T> qModelIndexToShared(const QModelIndex &index);

#endif // QMODELINDEXCHECK_H
