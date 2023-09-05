/*
 * File:   SpScLockFreeQueue.h
 * Author: Sander Jobing
 *
 * Created on July 29, 2017, 5:17 PM
 *
 * This class implements a Single Producer - Single Consumer lock-free and
 * wait-free queue. Only 1 thread can fill the queue, another thread can read
 * from the queue, but no more threads are allowed. This lock-free queue
 * is a fifo queue, the first element inserted is the first element which
 * comes out.
 *
 * Thanks to Timur Doumler, Juce
 * https://www.youtube.com/watch?v=qdrp6k4rcP4
 */

#ifndef SPSCLOCKFREEQUEUE_H
#define SPSCLOCKFREEQUEUE_H

#include <array>
#include <atomic>
#include <cassert>

template <typename T, size_t fixedSize>
class SpScLockFreeQueue
{
public:

  ///---------------------------------------------------------------------------
  /// @brief Constructor. Asserts when the underlying type is not lock free
  SpScLockFreeQueue()
  {
    std::atomic<size_t> test;
    assert(test.is_lock_free());
  }

  SpScLockFreeQueue(const SpScLockFreeQueue& src) = delete;

  virtual ~SpScLockFreeQueue()
  {
  }

  ///---------------------------------------------------------------------------
  /// @brief  Returns whether the queue is empty
  /// @return True when empty
  bool empty() const noexcept
  {
    bool isEmpty = false;
    const size_t readPosition = m_readPosition.load();
    const size_t writePosition = m_writePosition.load();

    if (readPosition == writePosition)
    {
      isEmpty = true;
    }

    return isEmpty;
  }

  ///---------------------------------------------------------------------------
  /// @brief  Pushes an element to the queue
  /// @param  element  The element to add
  /// @return True when the element was added, false when the queue is full
  bool push(const T& element)
  {
    const size_t oldWritePosition = m_writePosition.load();
    const size_t newWritePosition = getPositionAfter(oldWritePosition);
    const size_t readPosition = m_readPosition.load();

    if (newWritePosition == readPosition)
    {
      // The queue is full
      return false;
    }

    m_ringBuffer[oldWritePosition] = element;
    m_writePosition.store(newWritePosition);

    return true;
  }

  ///---------------------------------------------------------------------------
  /// @brief  Pops an element from the queue
  /// @param  element The returned element
  /// @return True when succeeded, false when the queue is empty
  bool pop(T& element)
  {
    if (empty())
    {
      // The queue is empty
      return false;
    }

    const size_t readPosition = m_readPosition.load();
    element = std::move(m_ringBuffer[readPosition]);
    m_readPosition.store(getPositionAfter(readPosition));

    return true;
  }

  ///---------------------------------------------------------------------------
  /// @brief Clears the content from the queue
  void clear() noexcept
  {
    const size_t readPosition = m_readPosition.load();
    const size_t writePosition = m_writePosition.load();

    if (readPosition != writePosition)
    {
      m_readPosition.store(writePosition);
    }
  }

  ///---------------------------------------------------------------------------
  /// @brief  Returns the maximum size of the queue
  /// @return The maximum number of elements the queue can hold
  constexpr size_t max_size() const noexcept
  {
    return RingBufferSize - 1;
  }

  ///---------------------------------------------------------------------------
  /// @brief  Returns the actual number of elements in the queue
  /// @return The actual size or 0 when empty
  size_t size() const noexcept
  {
    const size_t readPosition = m_readPosition.load();
    const size_t writePosition = m_writePosition.load();

    if (readPosition == writePosition)
    {
      return 0;
    }

    size_t size = 0;
    if (writePosition < readPosition)
    {
      size = RingBufferSize - readPosition + writePosition;
    }
    else
    {
      size = writePosition - readPosition;
    }

    return size;
  }

  static constexpr size_t getPositionAfter(size_t pos) noexcept
  {
    return ((pos + 1 == RingBufferSize) ? 0 : pos + 1);
  }

private:

  // A lock-free queue is basically a ring buffer.
  static constexpr size_t RingBufferSize = fixedSize + 1;
  std::array<T, RingBufferSize> m_ringBuffer;
  std::atomic<size_t> m_readPosition = {0};
  std::atomic<size_t> m_writePosition = {0};
};


#endif /* SPSCLOCKFREEQUEUE_H */
