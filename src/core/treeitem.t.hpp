// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#ifndef TREEITEM_T_H
#define TREEITEM_T_H

#include "core/deployerentry.hpp"
#include "treeitem.h"
#include <utility>

template <typename T>
TreeItem<T>::TreeItem(T *data, TreeItem *parent)
    : itemData(std::move(data)), m_parentItem(parent)
{
  m_childItems = std::vector<TreeItem *>();
}

template <typename T>
TreeItem<T> *TreeItem<T>::child(int row)
{
  return row >= 0 && row < childCount() ? m_childItems.at(row) : nullptr;
}

template <typename T>
int TreeItem<T>::childCount() const
{
  return int(m_childItems.size());
}

template <typename T>
int TreeItem<T>::row() const
{
  if (!m_parentItem)
    return 0;
  const auto it = std::ranges::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                       [this](const TreeItem *treeItem) {
                                       return treeItem == this;
                                       });

  if (it != m_parentItem->m_childItems.cend())
    return std::distance(m_parentItem->m_childItems.cbegin(), it);
  return -1;
}

template <typename T>
TreeItem<T> *TreeItem<T>::parent()
{
    return m_parentItem;
}

template <typename T>
bool TreeItem<T>::setData(T *value)
{
    itemData = value;
    dirty = true;
    return true;
}

template <typename T>
std::vector<TreeItem<T> *> TreeItem<T>::preOrderTraversal()
{
    if (!dirty) {
        return traversal;
    }
    std::vector<TreeItem<T> *> result = std::vector<TreeItem<T> *>();
    if (itemData)
        result.emplace_back(this);
    for (const auto &child : m_childItems)
    {
        auto child_result = child->preOrderTraversal();
        result.insert(result.end(), child_result.begin(), child_result.end());
    }
    traversal = result;
    dirty = false;
    return result;
}

template <typename T>
void TreeItem<T>::emplace_back(TreeItem *data)
{
  m_childItems.emplace_back(data);
  dirty = true;
}

template <typename T>
void TreeItem<T>::emplace_back(T *data)
{
  m_childItems.emplace_back(new TreeItem<T>(data, this));
  dirty = true;
}

template <typename T>
Json::Value TreeItem<T>::toJson() const {
  Json::Value json;
  if (itemData) {
    json = itemData->toJson();
  }
  for (const auto &child : m_childItems) {
    json["children"].append(child->toJson());
  }
  return json;
}

template <typename T>
void TreeItem<T>::rotate(int from, int to) {
  if(to < from)
  {
    std::rotate(m_childItems.begin() + to,
                m_childItems.begin() + from,
                m_childItems.begin() + from + 1);
  }
  else
  {
    std::rotate(m_childItems.begin() + from,
                m_childItems.begin() + from + 1,
                m_childItems.begin() + to + 1);
  }
  dirty = true;
}

template <typename T>
void TreeItem<T>::recategorize(int from, int to) {
  if (from == to || from < 0 || to < 0 || from >= m_childItems.size() || to >= m_childItems.size())
    return;
  if (dirty) preOrderTraversal();
  auto from_item = traversal[from+1];
  auto to_item = traversal[to+1];
  from_item->m_parentItem->remove(from_item);
  to_item->emplace_back(from_item);
  dirty = true;
}

template <typename T>
void TreeItem<T>::insert(int position, TreeItem<T> *item) {
  m_childItems.insert(m_childItems.begin() + position, item);
  dirty = true;
}

template <typename T>
void TreeItem<T>::remove(TreeItem<T> *item) {
  auto it = std::ranges::find(m_childItems, item);
  if (it != m_childItems.end()) {
    m_childItems.erase(it);
    dirty = true;
  }
}

template <typename T>
void TreeItem<T>::markNull(TreeItem<T> *item) {
  if (dirty) refresh();
  auto it = std::ranges::find(m_childItems, item);
  if (it != m_childItems.end()) {
    *it = nullptr;
    dirty = true;
  }
}

template <typename T>
void TreeItem<T>::setParent(TreeItem<T> *parent) {
  m_parentItem = parent;
  dirty = true;
}

// TODO: Finish
template <typename T>
void TreeItem<T>::erase(T *item) {
  if (dirty) preOrderTraversal();
  auto found = std::ranges::find_if(traversal.begin(), traversal.end(), [item](TreeItem<T> *e) { return e->getData() == item; });
}

template class TreeItem<DeployerEntry>;

#endif
