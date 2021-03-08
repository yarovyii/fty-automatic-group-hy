#pragma once
#include <atomic>
#include <condition_variable>
#include <fty/expected.h>
#include <mutex>
#include <optional>

namespace fty::storage {

class Mutex
{
public:
    enum class AccessType
    {
        READ,
        WRITE
    };

    class Write
    {
        std::unique_ptr<Mutex> m_m;
        bool                   m_locked = false;

    public:
        Write();
        void lock();
        void unlock();
    };

    class Read
    {
        std::unique_ptr<Mutex> m_m;
        bool                   m_locked = false;

    public:
        Read();
        void lock();
        void unlock();
    };

private:
    static std::optional<AccessType> m_currentAccess;
    static std::atomic<uint16_t>     m_activeReaders;
    static std::mutex                m_mxCurrentAccess;
    static std::condition_variable   m_cvAccess;

    Expected<void> lock(AccessType access);
    Expected<void> unlock(AccessType access);
};

} // namespace fty::storage
