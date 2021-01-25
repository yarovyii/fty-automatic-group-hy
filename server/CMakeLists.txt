cmake_minimum_required(VERSION 3.13)

########################################################################################################################

etn_target(static ${PROJECT_NAME}-static
    SOURCES
        src/lib/db.h
        src/lib/db.cpp
        src/lib/config.h
        src/lib/config.cpp
        src/lib/group.h
        src/lib/group.cpp
    USES
        fty-utils
        fty-pack
)

########################################################################################################################

etn_target(exe ${PROJECT_NAME}
    SOURCES
        src/main.cpp
    USES
        ${PROJECT_NAME}-static
)

########################################################################################################################

etn_test(${PROJECT_NAME}-test
    SOURCES
        test/main.cpp
        test/db.cpp
    CONFIGS
        test/conf/agroup.conf
        test/conf/logger.conf

    INCLUDE_DIRS src
    USES
        ${PROJECT_NAME}-static
)

########################################################################################################################
