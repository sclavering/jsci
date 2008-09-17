cd =ctoxml
vcbuild /useenv libctoxml.vcproj %config%
vcbuild /useenv libcpp.vcproj %config%
copy libcpp.h .. > NUL
copy libctoxml.h .. > NUL
copy %config%lib\libcpp.dll .. > NUL
copy %config%lib\libctoxml.dll .. > NUL
cd ..
copy ..\..\js\src\Debug\js32.lib . > NUL
copy ..\..\src\Debug\callc.lib . > NUL
