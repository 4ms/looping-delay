BUILDDIR := build

rebuild: | $(BUILDDIR)
	cmake --build $(BUILDDIR) --config Debug 
	cmake --build $(BUILDDIR) --config RelWithDebInfo

debug:
	cmake --build $(BUILDDIR) --config Debug

release:
	cmake --build $(BUILDDIR) --config RelWithDebInfo

$(BUILDDIR):
	cmake -B $(BUILDDIR) -G"Ninja Multi-Config" 

clean:
	rm -rf $(BUILDDIR)
