call ..\..\..\setenv_gcc.cmd 4.6.2 C:\nxp\LPCXpresso_5.2.4_2122\lpcxpresso\tools
msbuild TinyCLR.proj /t:rebuild /p:flavor=release;memory=flash >msbuild_gcc.log