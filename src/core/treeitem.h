// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef TREEITEM_H
#define TREEITEM_H

#include <vector>

template <typename T>
class TreeItem
{
public:
    explicit TreeItem(T *data, TreeItem *parent = nullptr);

    TreeItem *child(int number); int childCount() const; T *data() const;
    void emplace_back(TreeItem *data);
    void emplace_back(T *data);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    int row() const;
    bool setData(T *value);
    T *getData() const { return itemData; }
    std::vector<TreeItem<T> *> preOrderTraversal();
    void refresh() {
      traversal.clear();
      traversalItems.clear();
      preOrderTraversal();
      for (auto &item : traversal) {
        traversalItems.push_back(item->getData());
      }
    }
    
    std::vector<T *>::iterator begin() {
      if (dirty) refresh();
      return traversalItems.begin();
    }
    std::vector<T *>::iterator end() {
      if (dirty) refresh();
      return traversalItems.end();
    }
    int size() {
      return m_childItems.size();
    }
    bool empty() const { return m_childItems.empty(); }
    void erase(T *item) {
      if (dirty) preOrderTraversal();
      auto found = std::ranges::find_if(traversal.begin(), traversal.end(), [item](TreeItem<T> *e) { return e->getData() == item; });
    }
    TreeItem *operator[](int i) {
      if (dirty) refresh();
      return m_childItems[i];
    }

private:
    std::vector<TreeItem<T> *> m_childItems;
    T *itemData;
    TreeItem *m_parentItem;

    bool dirty = true;
    std::vector<TreeItem<T> *> traversal;
    std::vector<T *> traversalItems;
    int rows = -1;
};

#include "treeitem.t.hpp"
#endif // TREEITEM_H
