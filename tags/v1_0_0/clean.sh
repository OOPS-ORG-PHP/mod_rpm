if [ -f "./Makefile" ]; then
  make clean
fi
rm -rf auto* build include modules Make* ac* config* install-sh missing mkinstalldirs ltmain* libtool .deps
cp -af rpmconfig.m4 config.m4
