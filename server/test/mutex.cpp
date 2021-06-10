#include <atomic>
#include <catch2/catch.hpp>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <lib/mutex.h>
#include <thread>
#include <vector>

std::condition_variable cvGlob;

bool check(const bool& th, std::condition_variable& cv = cvGlob)
{
    using namespace std::chrono_literals;
    std::mutex                   mx;
    std::unique_lock<std::mutex> lk(mx);

    return cv.wait_for(lk, std::chrono::seconds(5), [&] {
        return th;
    });
}

TEST_CASE("Lock/Unlock storage Mutex", "[.]")
{
    bool                     th1    = false;
    std::atomic<size_t>      result = 0;
    std::vector<std::thread> pool;

    // Call one reader
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Read r;
        r.lock();
        r.unlock();
        th1 = true;
        cvGlob.notify_all();
    }));

    REQUIRE(check(th1));

    // Call one Writer
    th1 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Write r;
        r.lock();
        r.unlock();
        th1 = true;
        cvGlob.notify_all();
    }));

    REQUIRE(check(th1));

    bool                    startAgain = false;
    std::condition_variable cvStartTmp;
    // Call more readers - should work with previous
    for (size_t i = 0; i < 5; i++) {
        th1 = false;
        pool.emplace_back(std::thread([&] {
            fty::storage::Mutex::Read r;
            r.lock();
            result++;
            th1 = true;
            cvGlob.notify_all();
            check(startAgain);
            r.unlock();
            result--;
            cvStartTmp.notify_all();
        }));
        REQUIRE(check(th1));
    }
    REQUIRE(result == 5);

    // Call Writer, should not work because we are in READ mode
    bool th2   = false;
    bool th5   = false;
    bool thtmp = false;

    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Write r;
        r.lock();
        th2 = true;
        cvGlob.notify_all();
        check(thtmp);
        r.unlock();
        th5 = true;
        cvGlob.notify_all();
    }));

    REQUIRE(!th2);
    REQUIRE(!th5);

    // Unlock all readers, will make OPEN mode
    startAgain = true;
    cvGlob.notify_all();
    {
        auto tmpLambda = [&]() {
            using namespace std::chrono_literals;
            std::mutex                   mx;
            std::unique_lock<std::mutex> lk(mx);

            return cvStartTmp.wait_for(lk, std::chrono::seconds(5), [&] {
                return result == 0;
            });
        };
        REQUIRE(tmpLambda());
        REQUIRE(result == 0);
    }

    // WRiter can write and lock ascces to WRITE mode
    REQUIRE(check(th2));
    REQUIRE(!th5);

    // add Reader, cannot read because WRITE mode
    th2 = false;
    th1 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Write r;
        r.lock();
        th2 = true;
        r.unlock();
        th1 = true;
    }));
    REQUIRE(!th2);
    REQUIRE(!th1);

    // Call another writer| cannot write
    bool th3 = false;
    bool th4 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Write r;
        r.lock(); // must be locked here
        th3 = true;
        r.unlock();
        th4 = true;
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(!th3);

    thtmp = true;

    // unlock WRITE and this action will give access to another
    REQUIRE(check(th5)); // open writer
    REQUIRE(check(th2)); // reader
    REQUIRE(check(th1));
    REQUIRE(check(th3)); // writer
    REQUIRE(check(th4));


    th1 = false;
    th2 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Read m;
        m.unlock();
        m.unlock();
        m.unlock();
        m.lock();
        th1 = true;
        m.lock();
        m.unlock();
        th2 = true;
    }));
    REQUIRE(check(th1));
    REQUIRE(check(th2));

    th3 = false;
    th4 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Write m;
        m.lock();
        m.lock();
        th3 = true;
        m.unlock();
        th4 = true;
    }));
    REQUIRE(check(th3));
    REQUIRE(check(th4));

    // Call one reader
    th1 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Read r;
        r.lock();
        r.unlock();
        th1 = true;
    }));
    REQUIRE(check(th1));

    for (auto& th : pool) {
        th.join();
    }
}
