mkdir tmpbin
mkdir tmpbin\x64
mkdir tmpbin\x64\Release
mkdir tmpbin\x64\Debug
copy Bin\x64\Release\*.dll tmpbin\x64\Release
copy Bin\x64\Debug\*.dll tmpbin\x64\Debug


mkdir tmpbin\x86
mkdir tmpbin\x86\Release
mkdir tmpbin\x86\Debug
copy Bin\x86\Release\*.dll tmpbin\x86\Release
copy Bin\x86\Debug\*.dll tmpbin\x86\Debug


mkdir tmplib
mkdir tmplib\x64
mkdir tmplib\x64\Release
mkdir tmplib\x64\Debug

copy Bin\x64\Release\*.lib tmplib\x64\Release
copy Bin\x64\Debug\*.lib tmplib\x64\Debug

mkdir tmplib\x86
mkdir tmplib\x86\Release
mkdir tmplib\x86\Debug
copy Bin\x86\Release\*.lib tmplib\x86\Release
copy Bin\x86\Debug\*.lib tmplib\x86\Debug

rmdir .vs /s /q
rmdir bif /s /q
rmdir Bin /s /q
rmdir Debug /s /q
rmdir lib /s /q
rmdir man /s /q
rmdir obj /s /q
rmdir pch /s /q
rmdir Release /s /q
rmdir tlog /s /q
rmdir pdb /s /q
rmdir x64 /s /q

ren tmpbin Bin
ren tmplib lib