gcc -fPIC -shared tencentsm.c -o tencentsm.so -I /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -lpython2.7 ../release/mac/Release/libTencentSM.a
rm -rf ./product/macOS4py_x64/tencentsm.so
cp ./tencentsm.so ./product/macOS4py_x64/tencentsm.so


gcc -D PYTHON3  -fPIC -shared tencentsm.c -o tencentsm.so -I /Library/Frameworks/Python.framework/Versions/3.7/Headers/ $(python3-config --ldflags) ../release/mac/Release/libTencentSM.a
rm -rf ./product/macOS4py3_x64/tencentsm.so
cp ./tencentsm.so ./product/macOS4py3_x64/tencentsm.so