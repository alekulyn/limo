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
TreeItem<T> *TreeItem<T>::child(int number)
{
    return (number >= 0 && number < childCount())
        ? m_childItems.at(number) : nullptr;
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
T *TreeItem<T>::data() const
{
    return itemData;
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

template class TreeItem<DeployerEntry>;

#endif
