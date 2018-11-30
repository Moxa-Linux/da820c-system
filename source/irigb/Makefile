-include $(ROOTDIR)/.config

CONFIG_PRODUCT=DA-720
PKG_ARCH=amd64
PKG_VERSION=1.3

MOXA_PACKAGE_DIR=fakeroot
CXX=g++

.PHONY: release_src

all: 
	make -C mxIrigb
	# Make the source released for customer
	make release_src

clean:
	make -C mxIrigb clean
	rm -rf $(MOXA_PACKAGE_DIR)/usr \
	$(MOXA_PACKAGE_DIR)/DEBIAN/control \
	$(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-timesync-daemon_$(PKG_VERSION)_$(PKG_ARCH).deb \
	mxIrigb_example mxIrigb_example.tar.gz

romfs:
	mkdir -p $(ROMFSDIR)/usr/sbin
	$(ROMFSINST) mxIrigb/mxSyncTimeSvc/ServiceSyncTime  /usr/sbin
	$(ROMFSINST) $(MOXA_PACKAGE_DIR)/etc/init.d/mx_irigb.sh  /etc/init.d/
	$(STRIP) -s $(ROMFSDIR)/usr/sbin/ServiceSynctime

package:
	mkdir -p $(MOXA_PACKAGE_DIR)/usr/sbin
	cp -a mxIrigb/mxSyncTimeSvc/ServiceSyncTime $(MOXA_PACKAGE_DIR)/usr/sbin/
	cp -a mxIrigb/mxIrigUtil/mxIrigUtil $(MOXA_PACKAGE_DIR)/usr/sbin/
	chmod a+x $(MOXA_PACKAGE_DIR)/etc/init.d/mx_irigb.sh
	chmod 755 $(MOXA_PACKAGE_DIR)/DEBIAN/*
	#cp -a mxIrigb/unitest/unitest $(MOXA_PACKAGE_DIR)/usr/sbin/
	strip -s $(MOXA_PACKAGE_DIR)/usr/sbin/*
	:> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo -n "Package: " > $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "$(CONFIG_PRODUCT)-irigb-timesync-daemon" | tr '[A-Z]' '[a-z]' >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Version: $(PKG_VERSION)" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Section: base" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Priority: required" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Architecture: $(PKG_ARCH)" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo -n "Depends: " >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "$(CONFIG_PRODUCT)-irigb-driver"  | tr '[A-Z]' '[a-z]' >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Maintainer: Jared Wu <jared.wu@moxa.com>" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Description: Moxa $(CONFIG_PRODUCT) IRIG-B time sync daemon" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	dpkg -b $(MOXA_PACKAGE_DIR) $(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-timesync-daemon_$(PKG_VERSION)_$(PKG_ARCH).deb
	rm -rf ../$(MOXA_PACKAGE_DIR)/usr
	# Sign the package
	#dpkg-sig -v -k F7F3CD9E --sign builder $(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-timesync-daemon_$(PKG_VERSION)_$(PKG_ARCH).deb
#	scp $(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-timesync-daemon_$(PKG_VERSION)_$(PKG_ARCH).deb moxa@220.135.161.42:/dev/shm/

release_src:
	mkdir -p mxIrigb_example/mxirig \
	mxIrigb_example/unitest \
	mxIrigb_example/mxSyncTimeSvc \
	mxIrigb_example/mxIrigUtil
	# Copy the released file
	cp -a mxIrigb/Makefile mxIrigb_example/
	cp -a mxIrigb/README mxIrigb_example/
	cp -a mxIrigb/mxirig/Makefile mxIrigb_example/mxirig/
	sed -i 's/rm/#rm/g' mxIrigb_example/mxirig/Makefile
	sed -i 's/DIR = mxirig/DIR = /g' mxIrigb_example/Makefile
	cp -a mxIrigb/mxirig/*.h mxIrigb_example/mxirig/
	cp -a mxIrigb/mxirig/libmxirig*.a mxIrigb_example/mxirig/
	#cp -a mxIrigb/unitest/Makefile mxIrigb_example/unitest/
	#cp -a mxIrigb/unitest/*.h mxIrigb_example/unitest/
	#cp -a mxIrigb/unitest/unitest.cpp mxIrigb_example/unitest/
	cp -a mxIrigb/mxSyncTimeSvc/Makefile mxIrigb_example/mxSyncTimeSvc/
	cp -a mxIrigb/mxSyncTimeSvc/ServiceSyncTime.cpp mxIrigb_example/mxSyncTimeSvc/
	cp -a mxIrigb/mxIrigUtil/Makefile mxIrigb_example/mxIrigUtil/
	#cp -a mxIrigb/mxIrigUtil/*.h mxIrigb_example/mxIrigUtil/
	cp -a mxIrigb/mxIrigUtil/mxIrigUtil.cpp mxIrigb_example/mxIrigUtil/
	# Compress the released files
	tar czvf mxIrigb_example.tar.gz mxIrigb_example
	rm -rf mxIrigb_example

