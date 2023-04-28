BUILDDIR := build

rebuild: | $(BUILDDIR)
	cmake --build $(BUILDDIR) --config RelWithDebInfo
	./uimg_header.py build/mp153/RelWithDebInfo/mp153.bin build/mp153/RelWithDebInfo/mp153.uimg

debug:
	cmake --build $(BUILDDIR) --config Debug
	./uimg_header.py build/mp153/Debug/mp153.bin build/mp153/Debug/mp153.uimg

release:
	cmake --build $(BUILDDIR) --config RelWithDebInfo
	./uimg_header.py build/mp153/RelWithDebInfo/mp153.bin build/mp153/RelWithDebInfo/mp153.uimg

$(BUILDDIR):
	cmake -B $(BUILDDIR) -G"Ninja Multi-Config" 

clean:
	rm -rf $(BUILDDIR)

