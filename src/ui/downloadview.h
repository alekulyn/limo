#pragma once

#include <QTreeView>
#include <QMouseEvent>
#include <QFileSystemModel>
#include <QDebug>

class DownloadView : public QTreeView
{
  Q_OBJECT
public:
  explicit DownloadView(QWidget* parent) : QTreeView(parent)
  {
  }
signals:
  /*!
   * \brief Signals files have been dropped into this widget.
   * \param path Paths to the dropped files.
   */
  void modAdded(QList<QUrl> path);

protected:
  void mouseDoubleClickEvent(QMouseEvent* event) override {
    auto index = indexAt(event->pos());
    if (index.isValid()) {
      auto filepath = static_cast<QFileSystemModel *>(model())->filePath(index);
      qDebug() << "Double clicked on file:" << filepath;
      emit modAdded(QList<QUrl>{QUrl::fromLocalFile(filepath)});
    }
    QTreeView::mouseDoubleClickEvent(event);
  }
};
