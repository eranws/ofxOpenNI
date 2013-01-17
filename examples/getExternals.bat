::echo off

echo copying OpenNI2 %OPENNI2_REDIST%
xcopy /E /Y /D "%OPENNI2_REDIST%*.*" ".\bin\"

echo copying NiTE2 from %NITE2_REDIST%
xcopy /E /Y /D "%NITE2_REDIST%*.*" ".\bin\"

pause