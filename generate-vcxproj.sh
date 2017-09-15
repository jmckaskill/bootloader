#!/bin/sh

source toolchain/generate-vcxproj.sh

write_command "_GENERATE PROJECTS" {2750320C-BC9E-49FB-95FC-98EAE56402CB} '"c:\Program Files\git\usr\bin\sh.exe"' --login '$(SolutionDir)\generate-vcxproj.sh'

write_exe bootloader bootloader {20B3434C-7FA3-4585-B923-3939F4B8CC04} .
write_exe bootp-deploy-win32 bootp-deploy-win32 {E127B728-760A-4F60-B3FF-8F499968A95D} .

write_solution bootloader.sln
