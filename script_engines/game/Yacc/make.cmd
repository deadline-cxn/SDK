@cls
@REM
@REM This bach file actually creates the SCRIPT.CPP
@REM by running the YAY application (YACC clone)
@REM It then removes all those annoying UNIX directives
@REM (such as the "#line xxx" entries which messes up
@REM debugging and finally makes the file READ-ONLY
@REM
@SET YAY_INST=D:\projects\game\yacc\instinfo
@attrib script.cpp -r
@del script.cpp
yay script.y Parser=script.cpp | more
@umedit script.cpp $DELETE(#line,ALL,NOCASE)
@attrib script.cpp +r
@pause
