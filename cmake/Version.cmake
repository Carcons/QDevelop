set(QDevelop_VERSION_MAJOR 2)
set(QDevelop_VERSION_MINOR 0)
set(QDevelop_VERSION_PATCH 0)
set(QDevelop_VERSION
  "${QDevelop_VERSION_MAJOR}.${QDevelop_VERSION_MINOR}.${QDevelop_VERSION_PATCH}")
add_definitions( -DQDEVELOP_MAJOR=${QDevelop_VERSION_MAJOR} )
add_definitions( -DQDEVELOP_MINOR=${QDevelop_VERSION_MINOR} )
add_definitions( -DQDEVELOP_PATCH=${QDevelop_VERSION_PATCH} )