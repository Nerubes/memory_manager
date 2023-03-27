#ifndef MEMORY_MANAGER_HEAD_H_2023_02_10
#define MEMORY_MANAGER_HEAD_H_2023_02_10

#include <iostream>
#include <cstring>
#include <unordered_set>

namespace lab618 {

template<class T>
class CMemoryManager {
 private:
  struct block {
    block(T *pdata) : pdata(pdata) {
      firstFreeIndex = 0;
      usedCount = 0;
    }
    // Массив данных блока
    T *pdata;
    // Адрес следующего блока
    block *pnext;
    // Первая свободная ячейка
    int firstFreeIndex;
    // Число заполненных ячеек
    int usedCount;
  };

 public:
  class CException {
   public:
    CException() {}
  };

 public:
  /**
    _default_block_size - количество элементов в блоке данных
    isDeleteElementsOnDestruct - уничтожать елементы в деструкторе менеджера или
    проверять на наличие неосвобожденных функцией deleteObject элементов.
  */
  CMemoryManager(int _default_block_size,
                 bool isDeleteElementsOnDestruct = false) {
    m_blkSize = _default_block_size;
    m_isDeleteElementsOnDestruct = isDeleteElementsOnDestruct;
    m_pBlocks = nullptr;
    m_pCurrentBlk = nullptr;
  }

  virtual ~CMemoryManager() {
    clear();
  }

  // Получить адрес нового элемента из менеджера
  T *newObject() {
    if (m_pCurrentBlk == nullptr) {
      block *new_block = newBlock();
      new_block->pnext = m_pBlocks;
      m_pBlocks = new_block;
      m_pCurrentBlk = new_block;
    }
    if (m_pCurrentBlk->usedCount == m_blkSize) {
      m_pCurrentBlk = m_pBlocks;
      bool flag = false;
      while (m_pCurrentBlk->usedCount == m_blkSize) {
        flag = true;
        m_pCurrentBlk = m_pCurrentBlk->pnext;
        break;
      }
      if (flag) {
        block *new_block = newBlock();
        new_block->pnext = m_pBlocks;
        m_pBlocks = new_block;
        m_pCurrentBlk = new_block;
      }
    }
    int free = m_pCurrentBlk->firstFreeIndex;
    int next_free = *reinterpret_cast<int*>(m_pCurrentBlk->pdata + m_pCurrentBlk->firstFreeIndex);
    memset(reinterpret_cast<void*>((m_pCurrentBlk->pdata + free)), 0, sizeof(T));
    m_pCurrentBlk->firstFreeIndex = next_free;
    ++m_pCurrentBlk->usedCount;
    T* obj = new ((m_pCurrentBlk->pdata + free)) T();
    return obj;
  }

  // Освободить элемент в менеджере
  bool deleteObject(T *p) {
    block *del_block = m_pBlocks;
    while (del_block != nullptr) {
      if ((del_block->pdata <= p) && (p <= (del_block->pdata + m_blkSize))) {
        int free = del_block->firstFreeIndex;
        while (free < m_blkSize) {
          if ((del_block->pdata + free) == p) {
            return false;
          }
          free = *reinterpret_cast<int*>(del_block->pdata + free);
        }
        m_pCurrentBlk = del_block;
        --m_pCurrentBlk->usedCount;
        p->~T();
        *(reinterpret_cast<int *>(p)) = del_block->firstFreeIndex;
        del_block->firstFreeIndex = p - del_block->pdata;
        return true;
      }
      del_block = del_block->pnext;
    }
    return false;
  }

  // Очистка данных, зависит от m_isDeleteElementsOnDestruct
  void clear() {
    if (m_isDeleteElementsOnDestruct) {
      while (m_pBlocks != nullptr) {
        std::unordered_set<int> free;
        int first = m_pBlocks->firstFreeIndex;
        while (first != m_blkSize) {
          free.insert(first);
          first = *reinterpret_cast<int *>(m_pBlocks->pdata + first);
        }
        for (int i = 0; i < m_blkSize; ++i) {
          if (free.find(i) == free.end()) {
            T *elem = m_pBlocks->pdata + i;
            elem->~T();
          }
        }
        block *curr = m_pBlocks->pnext;
        deleteBlock(m_pBlocks);
        m_pBlocks = curr;
      }
    } else {
      while (m_pBlocks != nullptr) {
        if (m_pBlocks->usedCount != 0) {
          throw CException();
        }
        block *curr = m_pBlocks->pnext;
        deleteBlock(m_pBlocks);
        m_pBlocks = curr;
      }
    }
  }

 private:
  // Создать новый блок данных. применяется в newObject
  block *newBlock() {
    T *data = reinterpret_cast<T *>(new char[sizeof(T) * m_blkSize]);
    auto new_block = new block(data);
    for (int i = 0; i < m_blkSize; ++i) {
      *(reinterpret_cast<int *>(data + i)) = i + 1;
    }
    return new_block;
  }

  // Освободить память блока данных. Применяется в clear
  void deleteBlock(block *p) {
    delete[] reinterpret_cast<char *>(p->pdata);
    delete p;
  }

  // Размер блока
  int m_blkSize;
  // Начало списка блоков
  block *m_pBlocks;
  // Текущий блок
  block *m_pCurrentBlk;
  // Удалять ли элементы при освобождении
  bool m_isDeleteElementsOnDestruct;
};
};  // namespace lab618

#endif  // #define MEMORY_MANAGER_HEAD_H_2023_02_10
