@echo off
setlocal enabledelayedexpansion

REM 确保输出文件是空的
echo. > output.txt

set s="go_trace.txt"

for /l %%i in (0,1,1) do (
    for /l %%j in (0,1,1) do (
    
    REM 执行命令并将输出追加到文件中
    echo %%i >> output.txt
    try 32 4096 4 %%i %%j ./../traces/!s! >> my_output

    try 8 4096 4 %%i %%j ./../traces/!s! >> my_output
    try 16 4096 4 %%i %%j ./../traces/!s! >> my_output
    try 64 4096 4 %%i %%j ./../traces/!s! >> my_output
    try 128 4096 4 %%i %%j ./../traces/!s! >> my_output
    
    try 32 1024 4 %%i %%j ./../traces/!s! >> my_output
    try 32 2048 4 %%i %%j ./../traces/!s! >> my_output
    try 32 8192 4 %%i %%j ./../traces/!s! >> my_output
    try 32 16384 4 %%i %%j ./../traces/!s! >> my_output

    try 32 4096 1 %%i %%j ./../traces/!s! >> my_output
    try 32 4096 2 %%i %%j ./../traces/!s! >> my_output
    try 32 4096 8 %%i %%j ./../traces/!s! >> my_output
    try 32 4096 16 %%i %%j ./../traces/!s! >> my_output
)
)

echo ok
endlocal