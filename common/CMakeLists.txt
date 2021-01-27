cmake_minimum_required(VERSION 3.13)

etn_target(static ${PROJECT_NAME}-common
    SOURCES
        common/message-bus.h
        common/message.h
        common/commands.h
        common/group.h
        common/logger.h

        message-bus.cpp
        message.cpp
        group.cpp
    USES
        fty-utils
        fty-pack
        fty_common_logging
        fty_common_messagebus
    PRIVATE
)
