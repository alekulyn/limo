#pragma once

#include <QTreeView>
#include <QMouseEvent>
#include <QFileSystemModel>
#include <QMessageBox>
#include <QMenu>

class DownloadView : public QTreeView
{
  Q_OBJECT
public:
  explicit DownloadView(QWidget* parent) : QTreeView(parent) {
    setSortingEnabled(true);
  }

  void setModel(QAbstractItemModel* model) override {
    QTreeView::setModel(model);
    if (model) {
      setColumnWidth(0, 400);
    }
  }

signals:
  /*!
   * \brief Signals a download file has been clicked and should be added as a mod.
   * \param path Paths to the dropped files.
   */
  void modAdded(QList<QUrl> path);

protected:
  void mouseDoubleClickEvent(QMouseEvent* event) override {
    auto index = indexAt(event->pos());
    if (index.isValid()) {
      auto filepath = static_cast<QFileSystemModel *>(model())->filePath(index);
      emit modAdded(QList<QUrl>{QUrl::fromLocalFile(filepath)});
    }
    QTreeView::mouseDoubleClickEvent(event);
  }

  void contextMenuEvent(QContextMenuEvent* event) override {
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) return;

    QMenu menu(this);
    QAction* deleteAction = menu.addAction("Delete");
    connect(deleteAction, &QAction::triggered, this, [this, index]() {
      QString filepath = static_cast<QFileSystemModel *>(model())->filePath(index);
      QFile file(filepath);
      if (file.remove()) {
        // TODO: Emit a signal or update the model to reflect the deletion. Also show a disable-able confirmation message.
      } else {
        QMessageBox::warning(this, "Error", "Failed to delete the file.");
      }
    });

    menu.exec(event->globalPos());
  }
};
