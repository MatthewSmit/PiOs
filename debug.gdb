add-symbol-file ./cmake-build-debug/PiOs.exe 0x80000
display/i $pc
target remote tcp::9000