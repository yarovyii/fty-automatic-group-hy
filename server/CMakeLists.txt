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

        src/lib/jobs/create.h
        src/lib/jobs/create.cpp
        src/lib/jobs/update.h
        src/lib/jobs/update.cpp
    INCLUDE_DIRS
        src
    USES
        ${PROJECT_NAME}-common
        pthread
    PRIVATE
)

########################################################################################################################

etn_target(exe ${PROJECT_NAME}
    CONFIGS
        conf/agroup.conf.in
        conf/logger.conf.in
    SOURCES
        src/main.cpp
    USES
        ${PROJECT_NAME}-static
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

########################################################################################################################

etn_test(${PROJECT_NAME}-test
    SOURCES
        test/main.cpp
        test/db.cpp
        test/request.cpp
    CONFIGS
        test/conf/agroup.conf
        test/conf/logger.conf

    INCLUDE_DIRS src
    USES
        ${PROJECT_NAME}-static
)

########################################################################################################################
