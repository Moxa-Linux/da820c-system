-include $(ROOTDIR)/.config
CONFIG_PRODUCT=DA-720
PKG_ARCH=amd64
PKG_VERSION=1.0
MOXA_PACKAGE_DIR=fakeroot

#KDIR:=$(ROOTDIR)/$(LINUXDIR)
KDIR:=/lib/modules/`uname -r`/build

#KERNEL_VERSION=`cat $(ROOTDIR)/$(LINUXDIR)/include/config/kernel.release`
KERNEL_VERSION=`uname -r`

TARGET_MODULE = moxa_irigb
obj-m += $(TARGET_MODULE).o


all: modules


modules: $(TARGET_MODULE).ko


$(TARGET_MODULE).ko: $(TARGET_MODULE).c
	@echo "Making modules $(TARGET_MODULE).ko ..."
	$(MAKE) V=1 -C $(KDIR) M=`pwd` modules

clean:
	$(MAKE) -C $(KDIR) M=`pwd` clean
	rm -rf $(MOXA_PACKAGE_DIR)/lib \
	$(MOXA_PACKAGE_DIR)/DEBIAN/control \
	$(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-driver_$(PKG_VERSION)_$(PKG_ARCH).deb

package:
	mkdir -p $(MOXA_PACKAGE_DIR)/lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc/
	cp $(TARGET_MODULE).ko $(MOXA_PACKAGE_DIR)/lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc/
	:> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo -n "Package: " > $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "$(CONFIG_PRODUCT)-irigb-driver" | tr '[A-Z]' '[a-z]' >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Version: $(PKG_VERSION)" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Section: base" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Priority: required" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Architecture: $(PKG_ARCH)" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
ifeq ("$(CONFIG_PRODUCT)","DA-820")
	echo "Depends:linux-image-3.14-0.bpo.2-amd64" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
else ifeq ("$(CONFIG_PRODUCT)","DA-720")
	echo "Depends:linux-image-4.6.0-0.bpo.1-amd64" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
else
	echo "Depends:linux-image-3.2.0-4-amd64" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
endif
	echo "Maintainer: Jared Wu <jared.wu@moxa.com>" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	echo "Description: Moxa $(CONFIG_PRODUCT) IRIG-B module device driver" >> $(MOXA_PACKAGE_DIR)/DEBIAN/control
	dpkg -b $(MOXA_PACKAGE_DIR) $(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-driver_$(PKG_VERSION)_$(PKG_ARCH).deb
	rm -rf ../$(MOXA_PACKAGE_DIR)/lib/modules/$(KERNEL_VERSION)
	# Sign the package
	#dpkg-sig -v -k F7F3CD9E --sign builder $(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-driver_$(PKG_VERSION)_$(PKG_ARCH).deb
	#scp $(DPKGDIR)/$(CONFIG_PRODUCT)-irigb-driver_$(PKG_VERSION)_$(PKG_ARCH).deb moxa@220.135.161.42:/dev/shm/

romfs:
	mkdir -p ${OUTPUTDIR}/lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc
	cp -a ./${TARGET_MODULE}.ko ${OUTPUTDIR}/lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc/

install:
	mkdir -v -p "/lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc"
	cp -a $(TARGET_MODULE).ko "/lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc"
	depmod -a
	if [ -f /etc/modules ]; then \
		if [ "`grep -c $(TARGET_MODULE) /etc/modules`" == "0" ]; then \
			echo "$(TARGET_MODULE).ko" >> /etc/modules; \
		fi \
	elif [ -d /etc/sysconfig/modules ]; then \
		:> /etc/sysconfig/modules/$(TARGET_MODULE).modules; \
		echo "#!/bin/sh" >> /etc/sysconfig/modules/$(TARGET_MODULE).modules; \
		echo "modprobe $(TARGET_MODULE)" >> /etc/sysconfig/modules/$(TARGET_MODULE).modules; \
		chmod a+x /etc/sysconfig/modules/$(TARGET_MODULE).modules; \
	fi

uninstall:
	rm -f /lib/modules/$(KERNEL_VERSION)/kernel/drivers/misc/$(TARGET_MODULE).ko \
	/etc/sysconfig/modules/$(TARGET_MODULE).modules
	depmod -a
