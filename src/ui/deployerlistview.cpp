#include "deployerlistview.h"
#include "deployerlistmodel.h"
#include "deployerlistproxymodel.h"
#include "modlistmodel.h"
#include "hotspot.h"
#include "qmodelindexcheck.h"
#include <QGuiApplication>

DeployerListView::DeployerListView(QWidget* parent) : ModListView(parent)
{
  setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void DeployerListView::mousePressEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const auto prev_index = selectionModel()->currentIndex();
  selectionModel()->clearSelection();
  const auto selection = QItemSelection(model()->index(event_row, 1, index.parent()),
                                        model()->index(event_row, model()->columnCount() - 1, index.parent()));
  selectionModel()->select(selection, QItemSelectionModel::Select);
  selectionModel()->setCurrentIndex(model()->index(event_row, 1, index.parent()),
                                    QItemSelectionModel::NoUpdate);
  updateMouseDownRow(index);
  updateRow(prev_index);
}

void DeployerListView::mouseMoveEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int row = index.row();
  if(QGuiApplication::mouseButtons().testFlag(Qt::LeftButton) && enable_drag_reorder_ &&
     mouse_down_.isValid())
  {
    setCursor(Qt::ClosedHandCursor);
    is_in_drag_drop_ = true;
  }
  int mouse_row_region_curr = ROW_REGION.NONE;
  auto rect = visualRect(indexAt(event->pos()));

  auto rowHeight = rect.bottom() - rect.top();
  if(inHotspot(rect, event->pos()))
    mouse_row_region_curr = ROW_REGION.HOTSPOT;
  else if(event->pos().y() < rect.top() + rowHeight / 2)
    mouse_row_region_curr = ROW_REGION.UPPER;
  else if(event->pos().y() > rect.top() + rowHeight / 2)
    mouse_row_region_curr = ROW_REGION.LOWER;

  if(mouse_row_region_curr != mouse_row_region)
  {
    mouse_row_region = mouse_row_region_curr;
    if(is_in_drag_drop_)
    {
      for (const auto i : {
        indexAbove(indexAbove(index)),
        indexAbove(index),
        index,
        indexBelow(index),
        indexBelow(indexBelow(index)) }) {
        updateRow(i);
      }
    }
  }
  updateMouseHoverRow(index);
}

void DeployerListView::mouseReleaseEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int event_col = index.column();
  auto rect = visualRect(index);

  bool reorder = true;
  if (inHotspot(rect, event->pos()))
  {
    reorder = false;
  }

  const bool was_in_drag_drop_ = is_in_drag_drop_;
  is_in_drag_drop_ = false;
  setCursor(Qt::ArrowCursor);
  if(enable_drag_reorder_ && event_row > -1 && was_in_drag_drop_)
  {
    QModelIndex target = index;;
    // If separator expanded and mouse is in lower region, make separator's first child
    // If separator is not expanded and mouse is in lower region, make separator's next sibling
    if(target != mouse_down_ && target.isValid() && mouse_down_.isValid())
    {
      auto target_idx = static_cast<DeployerListProxyModel*>(model())->mapToSource(index);
      auto mouse_down_idx = static_cast<DeployerListProxyModel*>(model())->mapToSource(mouse_down_);
      auto target = static_cast<TreeItem<DeployerEntry> *>(target_idx.internalPointer());
      auto mouse_down = static_cast<TreeItem<DeployerEntry> *>(mouse_down_idx.internalPointer());
      if (reorder)
      {
        rowsAboutToBeRemoved(index.parent(), index.row(), index.row());
        // make null and then remove after
        mouse_down->parent()->markNull(mouse_down);
        if (mouse_row_region == ROW_REGION.UPPER)
          target->parent()->insert(target_idx.row(), mouse_down);
        else if (mouse_row_region == ROW_REGION.LOWER)
          target->parent()->insert(target_idx.row()+1, mouse_down);
        mouse_down->parent()->remove(nullptr);
        mouse_down->setParent(target->parent());
        emit modMoved();
      }
      else if (mouse_row_region == ROW_REGION.HOTSPOT && target->getData()->isSeparator)
      {
        rowsAboutToBeRemoved(index.parent(), index.row(), index.row());
        // target is separator/category
        target->emplace_back(mouse_down);
        mouse_down->parent()->remove(mouse_down);
        mouse_down->setParent(target);
        emit modMoved();
      }
      // selectionModel()->setCurrentIndex(model()->index(target.row(), 1, target.parent()),
      //                                   QItemSelectionModel::SelectCurrent);
      updateMouseDownRow(target_idx);
    }
    else
    {
      updateRow(index);
      updateRow(indexAbove(index));
      updateRow(indexBelow(index));
    }
  }
  if(!sameRow(index, mouse_down_))
    updateMouseHoverRow(QModelIndex());
  else if(event_col == DeployerListModel::status_col && event_row > -1 &&
          event_row < model()->rowCount() && enable_buttons_ && !was_in_drag_drop_)
  {
    auto mod_status_data = model()->data(index, Qt::DisplayRole).toString();
    if (mod_status_data == "")
      isExpanded(index) ? collapse(index) : expand(index);
    else
      emit modStatusChanged(model()->data(index, ModListModel::mod_id_role).toInt(),
                            !model()->data(index, DeployerListModel::mod_status_role).toBool());
  }
   QTreeView::mouseReleaseEvent(event);
}

bool DeployerListView::enableDragReorder() const
{
  return enable_drag_reorder_;
}

void DeployerListView::setEnableDragReorder(bool enabled)
{
  enable_drag_reorder_ = enabled;
}

