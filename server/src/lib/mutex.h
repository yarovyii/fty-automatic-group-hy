#pragma once
#include <fty/expected.h>
#include <optional>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace fty::storage {

class Mutex
{
public:
    Mutex();
    ~Mutex();

    enum class AccessType{
        READ,
        WRITE
    };

    class WRITE {
        Mutex * m;
    public:
        WRITE();
        ~WRITE();
        void lock();
        void unlock();
    };

    class READ {
        Mutex * m;
    public:
        READ();
        ~READ();
        void lock();
        void unlock();
    };

private:

    static std::optional<AccessType>    m_currentAccess;
    static std::atomic<uint16_t>        m_activeReaders;
    static std::mutex                   m_mx_currentAccess;
    static std::condition_variable      m_cv_access;

    Expected<void>  lock(AccessType access);
    Expected<void>  unlock(AccessType access);
};

} // namespace fty::Storage
