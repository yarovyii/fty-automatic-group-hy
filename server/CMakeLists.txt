cmake_minimum_required(VERSION 3.13)

########################################################################################################################

etn_target(static ${PROJECT_NAME}-static
    SOURCES
        src/lib/storage.h
        src/lib/storage.cpp
        src/lib/config.h
        src/lib/config.cpp
        src/lib/daemon.h
        src/lib/daemon.cpp
        src/lib/server.h
        src/lib/server.cpp
        src/lib/task.h
        src/lib/mutex.h
        src/lib/mutex.cpp

        src/lib/jobs/create.h
        src/lib/jobs/create.cpp
        src/lib/jobs/update.h
        src/lib/jobs/update.cpp
        src/lib/jobs/remove.h
        src/lib/jobs/remove.cpp
        src/lib/jobs/list.h
        src/lib/jobs/list.cpp
        src/lib/jobs/read.h
        src/lib/jobs/read.cpp
        src/lib/jobs/srr.h
        src/lib/jobs/srr.cpp
        src/lib/jobs/resolve.h
        src/lib/jobs/resolve.cpp
        src/lib/jobs/srr/utils.h
        src/lib/jobs/srr/utils.cpp
        src/lib/jobs/srr/save.h
        src/lib/jobs/srr/save.cpp
        src/lib/jobs/srr/restore.h
        src/lib/jobs/srr/restore.cpp
        src/lib/jobs/srr/reset.h
        src/lib/jobs/srr/reset.cpp
    INCLUDE_DIRS
        src
    USES
        ${PROJECT_NAME}-common
        pthread
        fty-asset-libng
        fty_common_socket
        fty_common_dto
        fty_common_db
        fty_common
        protobuf
        stdc++fs
        tntdb
        cxxtools
    PRIVATE
)

########################################################################################################################

etn_target(exe ${PROJECT_NAME}
    SOURCES
        src/main.cpp
    USES
        ${PROJECT_NAME}-static
        fty_common_db
)

etn_configure_file(
    conf/agroup.conf.in

    TARGET      ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}
)

etn_configure_file(
    conf/logger.conf.in

    TARGET      ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}
)

etn_configure_file(
    ${PROJECT_NAME}.service.in

    TARGET      ${PROJECT_NAME}
    DESTINATION /usr/lib/systemd/system/
)

########################################################################################################################

etn_test_target(${PROJECT_NAME}-static
    SOURCES
        test/main.cpp
        test/storage.cpp
        test/list.cpp
        test/create.cpp
        test/read.cpp
        test/resolve.cpp
        test/mutex.cpp

        test/db.cpp
        test/db.h
    INCLUDE_DIRS
        src
    CONFIGS
        test/conf/agroup.conf
        test/conf/logger.conf
    USES
        tntdb
        mysqld
        cxxtools
        fty-asset-test-db
    SUBDIR
        test
)

########################################################################################################################
