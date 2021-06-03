#include <atomic>
#include <catch2/catch.hpp>
#include <lib/mutex.h>
#include <thread>
#include <vector>

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
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    REQUIRE(th1);

    // Call one Writer
    th1 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Write r;
        r.lock();
        r.unlock();
        th1 = true;
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    REQUIRE(th1);

    // Call more readers - should work with previous
    for (size_t i = 0; i < 5; i++) {
        pool.emplace_back(std::thread([&] {
            fty::storage::Mutex::Read r;
            r.lock();
            result++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            r.unlock();
            result--;
        }));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    REQUIRE(result == 5);

    // Call Writer, should not work because we are in READ mode
    bool th2 = false;
    bool th5 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Write r;
        r.lock();
        th2 = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        r.unlock();
        th5 = true;
    }));

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    REQUIRE(!th2);
    REQUIRE(!th5);

    // Unlock all readers, will make OPEN mode
    std::this_thread::sleep_for(std::chrono::milliseconds(1600));
    REQUIRE(result == 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // WRiter can write and lock ascces to WRITE mode
    REQUIRE(th2);
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

    // unlock WRITE and this action will give access to another
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    REQUIRE(th5); // open writer
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    REQUIRE(th2); // reader
    REQUIRE(th1);
    REQUIRE(th3); // writer
    REQUIRE(th4);


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
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    REQUIRE(th1);
    REQUIRE(th2);


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
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    REQUIRE(th3);
    REQUIRE(th4);

    // Call one reader
    th1 = false;
    pool.emplace_back(std::thread([&] {
        fty::storage::Mutex::Read r;
        r.lock();
        r.unlock();
        th1 = true;
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    REQUIRE(th1);

    for (auto& th : pool) {
        th.join();
    }
}
