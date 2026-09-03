#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace fincon
{
    template <typename T>
    class MessageQueue final
    {
    public:
        explicit MessageQueue(std::size_t capacity)
            : capacity_(capacity)
        {
            if (capacity == 0)
                throw std::invalid_argument("Message queue capacity must be positive.");
        }

        MessageQueue(const MessageQueue&) = delete;
        MessageQueue& operator=(const MessageQueue&) = delete;

        bool push(T message)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            notFull_.wait(lock, [this]
            {
                return shutdown_ || messages_.size() < capacity_;
            });

            if (shutdown_)
                return false;

            messages_.push_back(std::move(message));
            notEmpty_.notify_one();
            return true;
        }

        std::optional<T> pop()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            notEmpty_.wait(lock, [this]
            {
                return shutdown_ || !messages_.empty();
            });

            if (messages_.empty())
                return std::nullopt;

            T message = std::move(messages_.front());
            messages_.pop_front();
            notFull_.notify_one();
            return message;
        }

        bool tryPush(T message)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (shutdown_ || messages_.size() >= capacity_)
                return false;

            messages_.push_back(std::move(message));
            notEmpty_.notify_one();
            return true;
        }

        std::optional<T> tryPop()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (messages_.empty())
                return std::nullopt;

            T message = std::move(messages_.front());
            messages_.pop_front();
            notFull_.notify_one();
            return message;
        }

        void shutdown()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
            notEmpty_.notify_all();
            notFull_.notify_all();
        }

        std::size_t size() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return messages_.size();
        }

        bool isShutdown() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return shutdown_;
        }

    private:
        const std::size_t capacity_;
        mutable std::mutex mutex_;
        std::condition_variable notEmpty_;
        std::condition_variable notFull_;
        std::deque<T> messages_;
        bool shutdown_ = false;
    };
}