MOD_NAME="tvip"
export BDL_CXX="g++"
export BDL_CXXFLAG="-O3 -flto -Wl,-z,relro,-z,now"
export BDL_ADDLINK="../libleveldb.a"
${BDL_CXX} ${BDL_CXXFLAG} -fvisibility=hidden -shared -fPIC -std=gnu++17 -I ../../include main.cpp ${BDL_ADDLINK} -o ../../out/${MOD_NAME}.so