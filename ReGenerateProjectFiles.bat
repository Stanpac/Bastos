@echo off

rem ## Batch File that Regenerate clean and regenerate the binaries and solution

rem ## Changes the current directory to the location of the batch file
cd %~dp0

rem ## Removes folder and there contents. The /s option removes all files and subdirectories, and the /q option does it quietly without prompting for confirmation.
rmdir /s /q ".idea", ".vs", "Binaries", "DerivedDataCache", "Intermediate", "Saved/Autosaves", "Saved/Collections", "Saved/Crashes", "Saved/Logs", "Saved/UnrealBuildTool", "Saved\Config\CrashReportClient"

rem ## Deletes all files with a ".txt" extension in the current directory. The /q option makes it quiet, and the /f option forces deleting read-only files.
del /q /f .vsconfig, MySolution.sln

rem ## Displays a message indicating that the folder has been deleted.
echo All files are deleted successfully.

rem ## Keeps the command prompt window open after execution, so you can see the result. You can remove this line if you don't want the window to stay open
pause
