cmake_minimum_required(VERSION 3.13)

########################################################################################################################

etn_target(shared ${PROJECT_NAME}-rest
    SOURCES
        src/group-rest.h
        src/read.h
        src/read.cpp
        src/create.h
        src/create.cpp
        src/edit.h
        src/edit.cpp
        src/remove.h
        src/remove.cpp
        src/list.h
        src/list.cpp
        src/resolve.h
        src/resolve.cpp
    USES
        fty-cmake-rest
        cxxtools
        tntnet
        fty_common_rest
        fty_common
        ${PROJECT_NAME}-common
        fty-asset-libng
        fty_common_messagebus
    TARGET_DESTINATION /usr/lib/bios
)

########################################################################################################################

# rest endpoints for tntnet
etn_configure_file(
    conf/54_agroups.xml.in

    TARGET      ${PROJECT_NAME}
    DESTINATION /etc/tntnet/bios.d
)

########################################################################################################################
