call ..\..\..\setenv_mdk.cmd 4.71 C:\Keil\ARM
msbuild TinyCLR.proj /t:rebuild /p:flavor=release;memory=flash >msbuild.log