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
TreeItem<T>::TreeItem(std::shared_ptr<T> data, std::shared_ptr<TreeItem<T>> parent)
    : itemData(std::move(data)), m_parentItem(parent)
{
  m_childItems = std::vector<std::shared_ptr<TreeItem>>();
}

template <typename T>
std::shared_ptr<TreeItem<T>> TreeItem<T>::child(int row)
{
  return this->operator[](row);
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
  const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                       [this](const auto& treeItem) {
                                       return treeItem.get() == this;
                                       });

  if (it != m_parentItem->m_childItems.cend())
    return std::distance(m_parentItem->m_childItems.cbegin(), it);
  return -1;
}

template <typename T>
std::shared_ptr<TreeItem<T>> TreeItem<T>::parent()
{
    return m_parentItem;
}

template <typename T>
bool TreeItem<T>::setData(std::unique_ptr<T> value)
{
    itemData = std::move(value);
    dirty = true;
    return true;
}

template <typename T>
std::vector<std::weak_ptr<TreeItem<T>>> TreeItem<T>::preOrderTraversal()
{
    if (!dirty) {
        return traversal_cache;
    }
    std::vector<std::weak_ptr<TreeItem<T>>> result = std::vector<std::weak_ptr<TreeItem<T>>>();
    if (itemData)
        result.emplace_back(std::weak_ptr<TreeItem<T>>(this->shared_from_this()));
    for (const auto &child : m_childItems)
    {
        auto child_result = child->preOrderTraversal();
        result.insert(result.end(), child_result.begin(), child_result.end());
    }
    traversal_cache = result;
    dirty = false;
    return result;
}

template <typename T>
void TreeItem<T>::emplace_back(std::shared_ptr<TreeItem<T>> data)
{
  m_childItems.emplace_back(std::move(data));
  dirty = true;
}

template <typename T>
void TreeItem<T>::emplace_back(std::shared_ptr<T> data)
{
  m_childItems.emplace_back(make_shared<TreeItem<T>>(std::move(data), this->shared_from_this()));
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
void TreeItem<T>::swapChild(int from, int to) {
  iter_swap(m_childItems.begin() + from, m_childItems.begin() + to);
  dirty = true;
}

template <typename T>
void TreeItem<T>::insert(int position, std::shared_ptr<TreeItem> item) {
  m_childItems.insert(m_childItems.begin() + position, std::move(item));
  dirty = true;
}

template <typename T>
void TreeItem<T>::remove(std::weak_ptr<TreeItem<T>> child) {
  auto childPtr = child.lock();
  if (!childPtr) {
    auto it = std::find_if(m_childItems.begin(), m_childItems.end(),
                          [](const auto& ptr) { return ptr.get() == nullptr; });
    if (it != m_childItems.end()) {
      m_childItems.erase(it);
    }
    return;
  }

  auto it = std::find_if(m_childItems.begin(), m_childItems.end(),
                         [childPtr](const auto& ptr) { return ptr == childPtr; });
  if (it != m_childItems.end()) {
    // 2. Extract the doomed child
    auto doomedChild = std::move(*it);

    // 3. Steal its children (they're now orphaned)
    auto orphans = std::move(doomedChild->m_childItems);

    // 4. Remove the child (will be destroyed here)
    m_childItems.erase(it);

    // 5. Adopt orphans at same position
    m_childItems.insert(it,
                        std::make_move_iterator(orphans.begin()),
                        std::make_move_iterator(orphans.end()));

    // 6. Update parent pointers
    for (auto& adopted : orphans) {
      adopted->setParent(this->shared_from_this());
    }
    dirty = true;
  }
}

template <typename T>
std::shared_ptr<TreeItem<T>> TreeItem<T>::markNull(TreeItem<T> *item) {
  if (dirty) refresh();
  auto it = std::find_if(m_childItems.begin(), m_childItems.end(),
                         [item](const auto& ptr) { return ptr.get() == item; });
  if (it != m_childItems.end()) {
    dirty = true;
    return std::move(*it);
  } else {
    return nullptr;
  }
}

template <typename T>
void TreeItem<T>::setParent(std::shared_ptr<TreeItem<T>> parent) {
  m_parentItem = parent;
  dirty = true;
}

template <typename T>
void TreeItem<T>::erase(std::weak_ptr<T> item) {
  if (dirty) preOrderTraversal();
  auto found = std::ranges::find_if(
    traversal_cache.begin(),
    traversal_cache.end(),
    [item](const std::weak_ptr<TreeItem<T>> e) { return e.lock()->getData() == item.lock(); });
  (*found).lock()->parent()->remove(*found);
}

template <typename T>
void TreeItem<T>::refresh() {
  traversal_cache.clear();
  traversalItems_cache.clear();
  preOrderTraversal();
  for (auto &item : traversal_cache) {
    traversalItems_cache.push_back(item.lock()->getData());
  }
  dirty = false;
}

template class TreeItem<DeployerEntry>;

#endif
