gcc -fPIC -shared tencentsm.c -o tencentsm.so  -I/usr/include/python2.7/ -lpython2.7 ../release/linux64/Release/libTencentSM.a
rm -rf ./product/linux4py_x64/tencentsm.so
cp ./tencentsm.so ./product/linux4py_x64/tencentsm.so

gcc -D PYTHON3 -fPIC -shared tencentsm.c -o tencentsm.so  -I/usr/include/python3.6m/ $(python3-config --ldflags) ../release/linux64/Release/libTencentSM.a
rm -rf ./product/linux4py3_x64/tencentsm.so
cp ./tencentsm.so ./product/linux4py3_x64/tencentsm.so