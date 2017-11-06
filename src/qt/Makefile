.PHONY: FORCE
all: FORCE
	$(MAKE) -C .. bitcoin_qt test_umkoin_qt
clean: FORCE
	$(MAKE) -C .. bitcoin_qt_clean test_umkoin_qt_clean
check: FORCE
	$(MAKE) -C .. test_umkoin_qt_check
umkoin-qt umkoin-qt.exe: FORCE
	 $(MAKE) -C .. bitcoin_qt
