add_subdirectory( DalitzAnalysis_D0KsKK )
add_subdirectory( DalitzFit )
add_subdirectory( SimFit )
#add_subdirectory( SliceFit )
if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_subdirectory( PythonDalitzFit )
endif()