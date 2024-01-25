@echo off
setlocal

REM Get current date
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set "dt=%%I"
set "year=%dt:~0,4%"
set "month=%dt:~4,2%"
set "day=%dt:~6,2%"
set "hour=%dt:~8,2%"
set "minute=%dt:~10,2%"
set "second=%dt:~12,2%"

REM Map numeric month to three-letter abbreviation
if %month% equ 01 set "month_abbr=Jan"
if %month% equ 02 set "month_abbr=Feb"
if %month% equ 03 set "month_abbr=Mar"
if %month% equ 04 set "month_abbr=Apr"
if %month% equ 05 set "month_abbr=May"
if %month% equ 06 set "month_abbr=Jun"
if %month% equ 07 set "month_abbr=Jul"
if %month% equ 08 set "month_abbr=Aug"
if %month% equ 09 set "month_abbr=Sep"
if %month% equ 10 set "month_abbr=Oct"
if %month% equ 11 set "month_abbr=Nov"
if %month% equ 12 set "month_abbr=Dec"

set "build_date=%day% %month_abbr% %year%"
set "build_time=%hour%:%minute%:%second%"

REM Update BuildDateTime.h with time and date
(
	echo #ifndef ZENPP_BUILDDATETIME_H
	echo #define ZENPP_BUILDDATETIME_H
	echo.
	echo #ifdef _MSC_VER
	echo #pragma once
	echo #endif
	echo.
	echo #define BUILD_DATE L"%build_date%"
	echo #define BUILD_TIME L"%build_time%"
	echo.
	echo #endif
) > BuildDateTime.h