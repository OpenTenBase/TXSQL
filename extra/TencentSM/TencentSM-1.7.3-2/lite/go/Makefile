
linux64:
	mkdir -p /usr/local/lib
	mkdir -p /usr/local/include/tencentsm
	cp ../release/linux64/libTencentSM.a /usr/local/lib/
	cp ../gmp/lib/linux64/libgmp.a /usr/local/lib/
	cp ../gmp/include/gmp.h /usr/local/include/
	cp ../source/sm.h /usr/local/include/tencentsm/
	cp ../source/tc.h /usr/local/include/tencentsm/
	cp ../source/tc_sm2.h /usr/local/include/tencentsm/
	cp ../source/tc_utils.h /usr/local/include/tencentsm/
	cp ../source/tc_asn1.h /usr/local/include/tencentsm/
	cp -r ./tencentsm ${GOPATH}/src/tencentsm
	
linux64-clean:
	rm -rf /usr/local/include/tencentsm
	rm -rf /usr/local/include/gmp.h
	rm -f /usr/local/lib/libTencentSM.a
	rm -f /usr/local/lib/libgmp.a
	rm -rf ${GOPATH}/src/tencentsm
