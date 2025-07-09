#include "deployerlistmodel.h"
#include "colors.h"
#include "core/deployerentry.hpp"
#include "core/treeitem.h"
#include "modlistmodel.h"
#include <QApplication>
#include <QBrush>
#include <ranges>

namespace str = std::ranges;

DeployerListModel::DeployerListModel(QObject* parent) : QAbstractItemModel(parent) {
}

QVariant DeployerListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(role == Qt::TextAlignmentRole && section == 2)
    return Qt::AlignLeft;
  if(role == Qt::DisplayRole)
  {
    if(orientation == Qt::Orientation::Vertical)
      return QString::number(section + 1);
    if(section == status_col)
      return QString("Status");
    if(section == name_col)
      return QString("Name");
    if(section == id_col)
    {
      if(!deployer_info_.ids_are_source_references)
        return QString("ID");
      else
        return QString("Source Mod");
    }
    if(section == tags_col)
      return QString("Tags");
  }
  return QVariant();
}

int DeployerListModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid() && deployer_info_.root == nullptr)
    return 0;

  const TreeItem<DeployerEntry> *parentItem = parent.isValid()
    ? static_cast<const TreeItem<DeployerEntry> *>(parent.internalPointer())
    : deployer_info_.root;

  return parentItem->childCount();
}

int DeployerListModel::columnCount(const QModelIndex& parent) const
{
  return 4;
}

QVariant DeployerListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  const int row = index.row();
  const int col = index.column();
  auto entry = static_cast<TreeItem<DeployerEntry> *>(index.internalPointer());
  auto data = entry->getData();
  if (role == Qt::CheckStateRole && index.column() == 1)
    return static_cast<int>( static_cast<DeployerModInfo *>(data)->enabled ? Qt::Checked : Qt::Unchecked );
  if(role == Qt::BackgroundRole)
  {
    if(col == status_col && !data->isSeparator)
      return QBrush(static_cast<DeployerModInfo *>(data)->enabled  ? colors::GREEN : colors::GRAY);
    return QBrush(colors::WHITE);
  }
  if(role == Qt::ForegroundRole)
  {
    if(col == status_col)
      return QBrush(QColor(255, 255, 255));
    if(!text_colors_.contains(static_cast<DeployerModInfo *>(data)->id))
      return QApplication::palette().text();
    return text_colors_.at(static_cast<DeployerModInfo *>(data)->id);
  }
  if(role == Qt::TextAlignmentRole && col == status_col)
    return Qt::AlignCenter;
  if(role == Qt::DisplayRole)
  {
    if(col == status_col)
    {
      if (!data->isSeparator)
        return QString(static_cast<DeployerModInfo *>(data)->enabled ? "Enabled" : "Disabled");
      return QString("");
    }
    if(col == name_col)
    {
      return data->name.c_str();
    }
    if(col == id_col)
    {
      if (!data->isSeparator) {
        const int id = static_cast<DeployerModInfo *>(data)->id;
        if(!deployer_info_.ids_are_source_references)
          return id;
        if(id == -1)
          return static_cast<DeployerModInfo *>(data)->sourceName.c_str();
        return std::format("{} [{}]", deployer_info_.source_mod_names_[row], id).c_str();
      }
      return QString("");
    }
    if(col == tags_col)
    {
      if(tags_.empty())
        return "";
      QStringList tags;
      for(const auto& tag : tags_.at(row))
        tags.append(tag.c_str());
      tags.sort(Qt::CaseInsensitive);
      return tags.join(", ");
    }
  }
  if(role == mod_status_role)
  {
    if (!data->isSeparator) {
      return static_cast<DeployerModInfo *>(data)->enabled;
    } else {
      return false;
    }
  }
  if(role == ModListModel::mod_id_role)
  {
    if(!deployer_info_.ids_are_source_references && !data->isSeparator)
      return static_cast<DeployerModInfo *>(data)->id;
    return row;
  }
  if(role == ModListModel::mod_name_role || role == Qt::EditRole)
    return data->name.c_str();
  if(role == mod_tags_role)
  {
    QStringList tags;
    for(const auto& tag : tags_.at(row))
      tags.append(tag.c_str());
    return tags;
  }
  if(role == ids_are_source_references_role)
    return deployer_info_.ids_are_source_references;
  if(role == source_mod_name_role)
  {
    if(deployer_info_.ids_are_source_references && !data->isSeparator)
      return static_cast<DeployerModInfo *>(data)->sourceName.c_str();
    else
      return data->name.c_str();
  }
  if(role == valid_mod_actions_role)
  {
    QVariant var;
    var.setValue<std::vector<int>>(deployer_info_.valid_mod_actions[row]);
    return var;
  }
  return QVariant();
}

void DeployerListModel::setDeployerInfo(const DeployerInfo& info)
{
  emit layoutAboutToBeChanged();
  tags_.clear();
  // if(info.manual_tags.size() == 0)
  // {
  //   for(const auto& tag : info.auto_tags)
  //     tags_.push_back(tag);
  // }
  // else
  // {
  //   for(const auto& [man_tags, auto_tags] : str::zip_view(info.manual_tags, info.auto_tags))
  //   {
  //     std::vector<std::string> all_tags = man_tags;
  //     all_tags.insert(all_tags.end(), auto_tags.begin(), auto_tags.end());
  //     tags_.push_back(all_tags);
  //   }
  // }
  deployer_info_ = info;
  for(int group = 0; group < info.conflict_groups.size(); group++)
  {
    for(int mod_id : info.conflict_groups[group])
    {
      QBrush color(group % 2 == 0 ? colors::LIGHT_BLUE : colors::ORANGE);
      if(group == info.conflict_groups.size() - 1)
        color = QApplication::palette().text();
      text_colors_[mod_id] = color;
    }
  }
  emit layoutChanged();
}

bool DeployerListModel::hasSeparateDirs() const
{
  return deployer_info_.separate_profile_dirs;
}

bool DeployerListModel::hasIgnoredFiles() const
{
  return deployer_info_.has_ignored_files;
}

bool DeployerListModel::usesUnsafeSorting() const
{
  return deployer_info_.uses_unsafe_sorting;
}

QModelIndex DeployerListModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return {};

  TreeItem<DeployerEntry> *parentItem = parent.isValid()
    ? static_cast<TreeItem<DeployerEntry> *>(parent.internalPointer())
    : deployer_info_.root;

  if (auto *childItem = parentItem->child(row))
    return createIndex(row, column, childItem);
  return {};
}

QModelIndex DeployerListModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return {};

  auto *childItem = static_cast<TreeItem<DeployerEntry> *>(index.internalPointer());
  TreeItem<DeployerEntry> *parentItem = childItem->parent();

  return parentItem != deployer_info_.root && parentItem != nullptr
  ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

bool DeployerListModel::hasChildren(const QModelIndex &parent = QModelIndex()) const
{
  if (deployer_info_.supports_expandable) {
    if (parent.isValid()) {
      auto item = static_cast<TreeItem<DeployerModInfo> *>(parent.internalPointer());
      if (item != nullptr && item->getData()->isSeparator) {
        return true;
      }
      return false;
    } else {
      return true;
    }
  } else {
    if (!parent.isValid()) {
      return true;
    } else {
      return false;
    }
  }
}

bool DeployerListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    auto item = static_cast<TreeItem<DeployerEntry> *>(index.internalPointer())->getData();
    item->name = value.toString().toStdString();
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return value.toString() == item->name.c_str();
}

TreeItem<DeployerEntry> *DeployerListModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
      return deployer_info_.root->child(index.row());
    }
    return deployer_info_.root;
}

void DeployerListModel::addSeparator()
{
  emit layoutAboutToBeChanged();
  if (deployer_info_.supports_expandable) {
    this->deployer_info_.root->emplace_back(
      new DeployerEntry (true, "Separator")
    );
  }
  emit layoutChanged();
}

Qt::ItemFlags DeployerListModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  auto flags = QAbstractItemModel::flags(index);
  if (static_cast<TreeItem<DeployerEntry> *>(index.internalPointer())->getData()->isSeparator == false)
    flags |= Qt::ItemIsUserCheckable;
  else
    flags |= Qt::ItemIsEditable;
  return flags;
}
