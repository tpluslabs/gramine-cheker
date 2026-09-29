EDMM ?= 0

all: hello.manifest.sgx

hello: hello.c
	$(CC) -O2 -Wall -o $@ $<

hello.manifest: hello.manifest.template hello
	gramine-manifest -Dedmm=$(EDMM) $< $@

hello.manifest.sgx: hello.manifest
	gramine-sgx-sign --manifest $< --output $@

clean:
	rm -f hello hello.manifest hello.manifest.sgx hello.sig

.PHONY: all clean
