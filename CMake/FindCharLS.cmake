find_path(CHARLS_CAP_INCLUDE_DIR CharLS/charls.h /usr/include /usr/local/include)
find_library(CHARLS_CAP_LIBRARY NAMES CharLS PATHS /usr/lib /usr/local/lib)
if(CHARLS_CAP_LIBRARY AND CHARLS_CAP_INCLUDE_DIR)
  set(CHARLS_LIBRARIES    ${CHARLS_CAP_LIBRARY})
  set(CHARLS_INCLUDE_DIRS ${CHARLS_CAP_INCLUDE_DIR})
  set(CHARLS_FOUND "YES")
  set(CHARLS_LOW FALSE)
else()
  find_path(CHARLS_INCLUDE_DIR charls/charls.h /usr/include /usr/local/include)
  find_library(CHARLS_LIBRARY NAMES charls PATHS /usr/lib /usr/local/lib)
  if(CHARLS_LIBRARY AND CHARLS_INCLUDE_DIR)
    set(CHARLS_LIBRARIES    ${CHARLS_LIBRARY})
    set(CHARLS_INCLUDE_DIRS ${CHARLS_INCLUDE_DIR})
    set(CHARLS_FOUND "YES")
	set(CHARLS_LOW TRUE)
  else()
    set(CHARLS_FOUND "NO")
  endif()
endif()

if(CHARLS_FOUND)
  message(STATUS "Found CharLS library: ${CHARLS_LIBRARIES}, headers: ${CHARLS_INCLUDE_DIRS}")
else()
  message(FATAL_ERROR "CharLS not found")
endif()

mark_as_advanced(CHARLS_CAP_LIBRARY CHARLS_CAP_INCLUDE_DIR CHARLS_LIBRARY CHARLS_INCLUDE_DIR)

