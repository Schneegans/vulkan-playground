@echo off

set SCRIPT_DIR=%~dp0
set CURRENT_DIR=%cd%

cd "%SCRIPT_DIR%"
git submodule update --init --recursive

echo ""
echo "-------------------------------------------------------------------------"
echo "------------------------- Building GLFW ---------------------------------"
echo "-------------------------------------------------------------------------"
echo ""
call third-party\make_glfw.bat Release
@echo off
call third-party\make_glfw.bat Debug
@echo off

echo ""
echo "-------------------------------------------------------------------------"
echo "---------------------- Copying STB Headers ------------------------------"
echo "-------------------------------------------------------------------------"
echo ""
call third-party\make_stb.bat Release
@echo off
call third-party\make_stb.bat Debug
@echo off

echo ""
echo "-------------------------------------------------------------------------"
echo "------------------------- Installing GLM --------------------------------"
echo "-------------------------------------------------------------------------"
echo ""
call third-party\make_glm.bat Release
@echo off
call third-party\make_glm.bat Debug
@echo off

echo ""
echo "-------------------------------------------------------------------------"
echo "---------------------- Building SPIRV-Cross -----------------------------"
echo "-------------------------------------------------------------------------"
echo ""
call third-party\make_spirv_cross.bat Release
@echo off
call third-party\make_spirv_cross.bat Debug
@echo off

cd "%CURRENT_DIR%"

@echo on
