#include <atomic>
#include <catch2/catch.hpp>
#include <lib/mutex.h>
#include <thread>

TEST_CASE("Lock/Unlock")
{
    bool                th1    = false;
    std::atomic<size_t> result = 0;

    // Call one reader
    std::thread *th;
    th = new std::thread([&]{
        fty::storage::Mutex::READ r;
        r.lock();
        th1 = true;
    });

    // Call more readers - should work with previous
    for (size_t i = 0; i < 5; i++) {
        th = new std::thread([&] {
            fty::storage::Mutex::READ r;
            r.lock();
            result++;
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    REQUIRE(result == 5);

    // Call Writer, should not work because we are in READ mode
    bool        th2 = false;
    th = new std::thread([&] {
        fty::storage::Mutex::WRITE r;
        r.lock();
        th2 = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    REQUIRE(!th2);

    // Unlock all readers, will make OPEN mode
    for (size_t i = 0; i < 6; i++) {
        th = new std::thread([&] {
            fty::storage::Mutex::READ r;
            r.unlock();
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    // WRiter can write and lock ascces to WRITE mode
    REQUIRE(th2);

    th2 = false;
    th1 = false;
    // add Reader, cannot read because WRITE mode
    th = new std::thread([&] {
        fty::storage::Mutex::WRITE r;
        r.lock();
        th2 = true;
        r.unlock();
        th1 = true;
    });
    REQUIRE(!th2);
    REQUIRE(!th1);

    // Call another writer| cannot write
    bool th3 = false;
    bool th4 = false;
    th = new std::thread([&] {
        fty::storage::Mutex::WRITE r;
        r.lock();   //must be locked here
        th3 = true;
        r.unlock();
        th4 = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    REQUIRE(!th3);

    //unlock WRITE and this action will give access to another
    fty::storage::Mutex::WRITE r;
    r.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    REQUIRE(th3);
    REQUIRE(th4);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    //open for Reader who waited
    REQUIRE(th2);
    REQUIRE(th2);

    th->join();
}
