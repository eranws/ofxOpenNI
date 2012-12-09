::echo off

echo create directories
FOR %%G IN (lib, examples\bin, include, examples\bin\Data) DO (if not exist %%G md %%G)

echo copying OpenNI2 %OPENNI2_REDIST%
xcopy /E /Y /D "%OPENNI2_REDIST%*.*" ".\examples\bin\"
::xcopy /E /Y /D "%OPENNI2_INCLUDE%*.*" ".\include"
::xcopy /E /Y /D "%OPENNI2_LIB%*.*" ".\lib"


echo copying NiTE2 from %NITE2_REDIST%
xcopy /E /Y /D "%NITE2_REDIST%*.*" ".\examples\bin\"
::xcopy /E /Y /D "%NITE2_INCLUDE%*.*" ".\include"
::xcopy /E /Y /D "%NITE2_LIB%*.*" ".\lib"

echo copy local Data folder
::copy Data bin\Data

pause