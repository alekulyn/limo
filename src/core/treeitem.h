// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TREEITEM_H
#define TREEITEM_H

#include <json/value.h>
#include <vector>

template <typename T>
class TreeItem
{
public:
    explicit TreeItem(T *data, TreeItem *parent = nullptr);

    TreeItem *child(int number);
    int childCount() const;
    TreeItem *parent();
    void setParent(TreeItem<T> *parent);
    int row() const;
    bool setData(T *value);
    T *getData() const { return itemData; }
    std::vector<TreeItem<T> *> getTraversal() {
      if (dirty) refresh();
      return traversal;
    }
    std::vector<T *> getTraversalItems() {
      if (dirty) refresh();
      return traversalItems;
    }
    void refresh() {
      traversal.clear();
      traversalItems.clear();
      preOrderTraversal();
      for (auto &item : traversal) {
        traversalItems.push_back(item->getData());
      }
      dirty = false;
    }
    
    void emplace_back(TreeItem *data);
    void emplace_back(T *data);
    void insert(int position, TreeItem<T> *item);
    void remove(TreeItem<T> *item);
    void markNull(TreeItem<T> *item);
    std::vector<T *>::iterator begin() {
      return traversalItems.begin();
    }
    std::vector<T *>::iterator end() {
      if (dirty) refresh();
      return traversalItems.end();
    }
    int size() const {
      return m_childItems.size();
    }
    bool empty() const { return m_childItems.empty(); }
    void erase(T *item);
    TreeItem *operator[](int i) {
      return m_childItems[i];
    }
    Json::Value toJson() const;
    TreeItem<T> *back() {
      if (m_childItems.empty()) return nullptr;
      return m_childItems.back();
    }
    void rotate(int from, int to);

private:
    std::vector<TreeItem<T> *> m_childItems;
    T *itemData;
    TreeItem<T> *m_parentItem;

    bool dirty = true;
    std::vector<TreeItem<T> *> traversal;
    std::vector<T *> traversalItems;
    int rows = -1;

    std::vector<TreeItem<T> *> preOrderTraversal();
};

#include "treeitem.t.hpp"
#endif // TREEITEM_H
