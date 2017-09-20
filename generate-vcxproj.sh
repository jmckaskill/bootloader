#!/bin/sh

source toolchain/generate-vcxproj.sh

write_command "_GENERATE PROJECTS" {2750320C-BC9E-49FB-95FC-98EAE56402CB} '"c:\Program Files\git\usr\bin\sh.exe"' --login '$(SolutionDir)\generate-vcxproj.sh'

write_exe httpboot httpboot-am335x {20B3434C-7FA3-4585-B923-3939F4B8CC04} .
write_exe httpboot httpboot-unit_test {836C3A4F-E1F6-4BFA-863C-34FB3FA82F6F} .
write_exe bootp-deploy-win32 bootp-deploy-win32 {E127B728-760A-4F60-B3FF-8F499968A95D} .

write_solution httpboot.sln
