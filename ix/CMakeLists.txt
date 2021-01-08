set(HEADERS
        ${HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_Manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_Internal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_IndexHandle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_FindSpeRec.h
        PARENT_SCOPE
        )

set(SOURCE
        ${SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_Manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_IndexHandle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IX_FindSpeRec.cpp
        PARENT_SCOPE
        )