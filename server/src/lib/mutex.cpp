#include "mutex.h"
#include "config.h"
#include <iostream>
#include <mutex>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fty::storage {

std::optional<fty::storage::Mutex::AccessType> Mutex::m_currentAccess;
std::atomic<uint16_t>                          Mutex::m_activeReaders;
std::mutex                                     Mutex::m_mxCurrentAccess;
std::condition_variable                        Mutex::m_cvAccess;

Mutex::Write::Write()
    : m_m(new Mutex)
{
}

void Mutex::Write::lock()
{
    if (!m_locked) {
        m_locked = true;
        m_m->lock(AccessType::WRITE);
    }
}

void Mutex::Write::unlock()
{
    if (m_locked) {
        m_locked = false;
        m_m->unlock(AccessType::WRITE);
    }
}

Mutex::Read::Read()
    : m_m(new Mutex)
{
}

void Mutex::Read::lock()
{
    if (!m_locked) {
        m_locked = true;
        m_m->lock(AccessType::READ);
    }
}

void Mutex::Read::unlock()
{
    if (m_locked) {
        m_locked = false;
        m_m->unlock(AccessType::READ);
    }
}

Expected<void> Mutex::lock(AccessType access)
{
    try {
        std::unique_lock<std::mutex> locker(m_mxCurrentAccess);
        m_cvAccess.wait(locker, [&]() {
            return (
                (m_currentAccess == AccessType::READ && access == AccessType::READ) || !m_currentAccess.has_value());
        });

        if (access == AccessType::READ) {
            m_currentAccess = access;
            locker.unlock();
            m_cvAccess.notify_all();
            m_activeReaders++;
        } else if (access == AccessType::WRITE) {
            m_currentAccess = access;

            std::string path = fty::Config::instance().dbpath;

            if (m_fd = open(path.c_str(), O_WRONLY | O_CREAT, 0600); m_fd == -1) {
                return unexpected(strerror(errno));
            }

            if (flock(m_fd, LOCK_EX | LOCK_NB) != 0) {
                return unexpected(strerror(errno));
            }
        }
    } catch (const std::system_error& e) {
        return unexpected(e.what());
    }
    return {};
}

Expected<void> Mutex::unlock(AccessType access)
{
    try {
        std::unique_lock<std::mutex> locker(m_mxCurrentAccess);
        if (access == AccessType::READ) {
            m_activeReaders--;
        } else if (access == AccessType::WRITE) {
            if (flock(m_fd, LOCK_UN) != 0) {
                return unexpected(strerror(errno));
            }
            if (close(m_fd) != 0) {
                return unexpected(strerror(errno));
            }
        }
        if (access == AccessType::WRITE || m_activeReaders == 0) {
            m_currentAccess.reset();
        }

        locker.unlock();
        m_cvAccess.notify_all();
    } catch (const std::system_error& e) {
        return unexpected(e.what());
    }

    return {};
}
} // namespace fty::storage
