include ../Makefile.defs

DDEB=dpkg-deb
MD=mkdir -p
CP=cp
RM=rm -rf
EH=echo

LNAME=lib$(NAME)
DDIR=$(LNAME)_$(VER)_$(LARCH)
DEB=$(DDIR).deb
CNT=$(DDIR)/DEBIAN/control

build: ../$(LNAME).so ../vulmat.h $(CNT)
	$(MD) $(DDIR)/usr/lib
	$(CP) ../libvulmat.so $(DDIR)/usr/lib/
	$(MD) $(DDIR)/usr/include
	$(CP) ../vulmat.h $(DDIR)/usr/include/
	$(DDEB) --build --root-owner-group $(DDIR)

clean:
	$(RM) $(DDIR) $(DDIR).deb

$(CNT): Makefile
	$(MD) $(DDIR)/DEBIAN/
	$(EH) 'Package: $(LNAME)' > $(CNT)
	$(EH) 'Version: $(VER)' >> $(CNT)
	$(EH) 'Architecture: $(LARCH)' >> $(CNT)
	$(EH) 'Maintainer: $(MNAME) <$(MEMAIL)>' >> $(CNT)
	$(EH) 'Depends: libvulcmp' >> $(CNT)
	$(EH) 'Description: $(DESC)' >> $(CNT)
