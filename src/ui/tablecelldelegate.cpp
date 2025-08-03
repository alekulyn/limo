#include "tablecelldelegate.h"
#include "modlistmodel.h"
#include "qmodelindexutils.h"
#include <QApplication>
#include <QPainter>

TableCellDelegate::TableCellDelegate(QSortFilterProxyModel* proxy, QObject* parent) :
  QStyledItemDelegate{ parent }, proxy_model_(proxy),
  parent_view_(static_cast<ModListView*>(parent))
{}

void TableCellDelegate::paint(QPainter* painter,
                              const QStyleOptionViewItem& option,
                              const QModelIndex& view_index) const
{
  auto model_index = proxy_model_ == nullptr ? view_index : proxy_model_->mapToSource(view_index);
  QStyleOption cell;
  QRect rect = option.rect;
  cell.rect = rect;
  const bool is_even_row = view_index.row() % 2 == 0;
  const auto mouse_row = parent_view_->getHoverRow();
  const bool row_is_selected =
    parent_view_->selectionModel()->rowIntersectsSelection(view_index.row(), view_index.parent());

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, view_index);
  if(parent_view_->isInDragDrop() &&
    sameRow(mouse_row, view_index) &&
    parent_view_->getMouseRegion() != parent_view_->ROW_REGION.UPPER &&
    parent_view_->getMouseRegion() != parent_view_->ROW_REGION.LOWER)
  {
    opt.backgroundBrush = option.palette.brush(
      parent_view_->hasFocus() ? QPalette::Active : QPalette::Inactive,
      QPalette::Highlight);
  }
  opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

  auto icon_var = model_index.data(ModListModel::icon_role);
  if(!icon_var.isNull())
  {
    auto icon = icon_var.value<QIcon>();
    const int icon_width = 16;
    const int icon_height = 16;
    const QPixmap icon_map = icon.pixmap(icon_width, icon_height);
    const auto center = rect.center();
    painter->drawPixmap(center.x() - icon_width / 2 + 1, center.y() - icon_height / 2, icon_map);
  }
  if(parent_view_->isInDragDrop())
  {
    if (parent_view_->getMouseRegion() != parent_view_->ROW_REGION.HOTSPOT) {
      bool MOUSE_IN_UPPER = parent_view_->getMouseRegion() == parent_view_->ROW_REGION.UPPER;
      bool MOUSE_IN_LOWER = parent_view_->getMouseRegion() == parent_view_->ROW_REGION.LOWER;
      if((MOUSE_IN_UPPER && sameRow(mouse_row, view_index)) ||
            (MOUSE_IN_LOWER && sameRow(parent_view_->indexBelow(mouse_row), view_index)))
        painter->drawLine(rect.topLeft(), rect.topRight());
      if((MOUSE_IN_LOWER && sameRow(mouse_row, view_index)) ||
            (MOUSE_IN_UPPER && sameRow(parent_view_->indexAbove(mouse_row), view_index)))
        painter->drawLine(rect.bottomLeft(), rect.bottomRight());
    }
  }
  if(!parent_view_->selectionModel()->rowIntersectsSelection(view_index.row(), view_index.parent()) &&
     parent_view_->selectionModel()->currentIndex().row() == view_index.row())
  {
    QStyleOptionFocusRect indicator;
    indicator.rect = option.rect;
    QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &indicator, painter);
  }
}
