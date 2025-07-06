#include "deployerlistview.h"
#include "deployerlistmodel.h"
#include "deployerlistproxymodel.h"
#include "modlistmodel.h"
#include "hotspot.h"
#include <QGuiApplication>

DeployerListView::DeployerListView(QWidget* parent) : ModListView(parent)
{
  setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void DeployerListView::mousePressEvent(QMouseEvent* event)
{
  const auto index = indexAt(event->pos());
  const int event_row = index.row();
  const int prev_row = selectionModel()->currentIndex().row();
  selectionModel()->clearSelection();
  const auto selection = QItemSelection(model()->index(event_row, 1, index.parent()),
                                        model()->index(event_row, model()->columnCount() - 1, index.parent()));
  selectionModel()->select(selection, QItemSelectionModel::Select);
  selectionModel()->setCurrentIndex(model()->index(event_row, 1, index.parent()),
                                    QItemSelectionModel::NoUpdate);
  updateMouseDownRow(event_row);  // BUG: Only works for flat data, not hierarchical
  updateRow(prev_row);
}

void DeployerListView::mouseMoveEvent(QMouseEvent* event)
{
  const int row = indexAt(event->pos()).row();
  if(QGuiApplication::mouseButtons().testFlag(Qt::LeftButton) && enable_drag_reorder_ &&
     mouse_down_row_ != -1)
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
      for (const int i : { -2, -1, 0, 1, 2 }) {
        updateRow(row + i);
      }
    }
  }
  updateMouseHoverRow(row);
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
    int target_row = 0;
    if (mouse_row_region == ROW_REGION.UPPER || mouse_row_region == ROW_REGION.HOTSPOT)
      target_row = event_row;
    else if (mouse_row_region == ROW_REGION.LOWER)
      target_row = event_row + 1;
    if(mouse_down_row_ < target_row && reorder)
      target_row--;
    target_row = std::min(target_row, model()->rowCount());
    if(target_row != mouse_down_row_ && rowIndexIsValid(target_row) &&
       rowIndexIsValid(mouse_down_row_))
    {
      const auto from_row = static_cast<DeployerListProxyModel*>(model())
                              ->mapToSource(model()->index(mouse_down_row_, 0))
                              .row();
      const auto to_row = static_cast<DeployerListProxyModel*>(model())
                            ->mapToSource(model()->index(target_row, 0))
                            .row();
      if (reorder)
        emit modMoved(from_row, to_row);
      else
        emit modCategorized(from_row, to_row);
      selectionModel()->setCurrentIndex(model()->index(target_row, 1),
                                        QItemSelectionModel::SelectCurrent);
      updateMouseDownRow(target_row);
    }
    else
    {
      updateRow(event_row);
      updateRow(event_row - 1);
      updateRow(event_row + 1);
    }
  }
  if(event_row != mouse_down_row_)
    updateMouseHoverRow(-1);
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

