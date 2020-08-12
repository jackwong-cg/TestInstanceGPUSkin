@echo off

set CurrentPath=%~dp0
set EnginePath=E:\UEGit\UnrealEngine

rem 以下参数根据自己实际路径进行修改

set ProjectName=TestInstanceGPUSkin
set ProjectDir=%CurrentPath%..
set UATPath="%EnginePath%\Engine\Build\BatchFiles\RunUAT.bat"
set AutomationScriptPath="%CurrentPath%AllBuild.xml"

echo ProjectName=%ProjectName%
echo ProjectDir=%ProjectDir%
echo UATPath=%UATPath%
echo AutomationScriptPath=%AutomationScriptPath%
echo ===================================

rem ends

if %Target%=="" set Target="Build Editor Win64"
if "%IterativeCooking%"=="" set IterativeCooking=true
if "%BuildConfiguration%"=="" set BuildConfiguration=Development
if "%WithClean%"=="" set WithClean=false

call %UATPath% BuildGraph -Script=%AutomationScriptPath% -Target=%Target% -set:ProjectDir=%ProjectDir% -set:ProjectName=%ProjectName% -set:IterativeCooking=%IterativeCooking% -set:BuildConfiguration=%BuildConfiguration% -set:WithClean=%WithClean%

pause