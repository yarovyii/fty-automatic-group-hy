#include "mutex.h"
#include "config.h"
#include "storage.h"
#include <iostream>
#include <mutex>

namespace fty::storage {

void Mutex::WRITE::lock()
{
    m.lock(AccessType::WRITE);
}

void Mutex::WRITE::unlock()
{
    m.unlock(AccessType::WRITE);
}

void Mutex::READ::lock()
{
    m.lock(AccessType::READ);
}

void Mutex::READ::unlock()
{
    m.unlock(AccessType::READ);
}

Expected<void> Mutex::lock(AccessType access)
{
    std::unique_lock<std::mutex> locker(m_mx_currentAccess);
    if (!m_currentAccess.has_value()) {
        m_currentAccess = access;
    } else if (access == AccessType::READ && m_currentAccess != AccessType::READ) {
        locker.unlock();

        std::mutex                   mx_tmp;
        std::unique_lock<std::mutex> tmpLocker(mx_tmp);

        m_cv_access.wait(tmpLocker, [&]() {
            locker.lock();
            if (m_currentAccess == AccessType::READ || !m_currentAccess.has_value()) {
                return true;
            }
            locker.unlock();
            return false;
        });

        locker.unlock();
        m_cv_access.notify_all();
    } 
    else if (access == AccessType::WRITE) {
        if (m_currentAccess == AccessType::WRITE);
        else {
            locker.unlock();

            std::mutex                   mx_tmp;
            std::unique_lock<std::mutex> tmpLocker(mx_tmp);

            m_cv_access.wait(tmpLocker, [&]() {
                locker.lock();
                if (!m_currentAccess.has_value()) {
                    return true;
                }
                locker.unlock();
                return false;
            });
        }
        // flock close for WRITE
    }

    if (access == AccessType::READ) {
        m_activeReaders++;
    }
    m_currentAccess = access;
    return {};
}

Expected<void> Mutex::unlock(AccessType access)
{
    std::unique_lock<std::mutex> locker(m_mx_currentAccess);
    if (access == AccessType::READ) 
        m_activeReaders--;
    else if(access == AccessType::WRITE){
        // flock open
    }
    if(access == AccessType::WRITE || m_activeReaders == 0)
        m_currentAccess.reset();

    locker.unlock();
    m_cv_access.notify_all();   //or notify_one - not sure about it
}
} // namespace fty::storage
