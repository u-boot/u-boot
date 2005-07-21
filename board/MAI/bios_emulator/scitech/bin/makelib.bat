call wc11-d32.bat

cd c:\private\src\license
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\pm
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\console
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\nucleus
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\zlib
dmake clean
dmake depend
dmake -u install

cd c:\private\src\graphics\ref2d
dmake clean
dmake depend
dmake -u install
cd c:\private\src\drvlib
dmake clean
dmake depend
dmake -u install

call wc11-w32.bat

cd c:\private\src\license
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\pm
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\console
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\nucleus
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\zlib
dmake clean
dmake depend
dmake -u install

cd c:\private\src\graphics\ref2d
dmake clean
dmake depend
dmake -u install
cd c:\private\src\drvlib
dmake clean
dmake depend
dmake -u install

call wc10-d32.bat

cd c:\private\src\license
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\pm
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\console
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\nucleus
dmake clean
dmake depend
dmake -u install
cd c:\scitech\src\zlib
dmake clean
dmake depend
dmake -u install

cd c:\private\src\graphics\ref2d
dmake clean
dmake depend
dmake -u install
cd c:\private\src\drvlib
dmake clean
dmake depend
dmake -u install

cd \private\src\graphics\drivers
