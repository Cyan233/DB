set(HEADERS
        ${HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/struct.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileHandle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RecordManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileScan.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Compare.cpp
        PARENT_SCOPE
        )

set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileHandle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RecordManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RM_FileScan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Compare.cpp
        PARENT_SCOPE
        )