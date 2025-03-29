@ECHO OFF

gcc ^
	-Wall ^
	-Wextra ^
	-Wpedantic ^
	-Wno-unused-parameter ^
	-fno-strict-aliasing ^
	-DCAR_DEFAULT ^
	-Iinclude ^
	main.c ^
	source/*.c ^
	-o main
