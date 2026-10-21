# Builds v3dshader against a built Mesa source tree (see README):
#   make MESA=/path/to/mesa
# and the shader headers for src/v3d/, for V3D 4.2 (shader_NAME.h) and
# V3D 7.1 (shader71_NAME.h):
#   make MESA=/path/to/mesa headers

MESA ?= mesa
BUILD = $(MESA)/build

CFLAGS = -std=gnu11 -O1 -g -D_GNU_SOURCE -DMESA_DEBUG=0 -DHAVE_ENDIAN_H \
	-DHAVE_PTHREAD -DHAVE_STRUCT_TIMESPEC -DHAVE_SECURE_GETENV -DHAVE___BUILTIN_CLZ \
	-DHAVE___BUILTIN_CLZLL -DHAVE___BUILTIN_CTZ -DHAVE___BUILTIN_FFS \
	-DHAVE___BUILTIN_FFSLL -DHAVE___BUILTIN_POPCOUNT \
	-DHAVE___BUILTIN_POPCOUNTLL -DHAVE___BUILTIN_EXPECT \
	-DHAVE___BUILTIN_UNREACHABLE -DHAVE_FUNC_ATTRIBUTE_PACKED \
	-DHAVE_FUNC_ATTRIBUTE_UNUSED -DHAVE_FUNC_ATTRIBUTE_FORMAT \
	-DHAVE_FUNC_ATTRIBUTE_NORETURN -DUSE_GCC_ATOMIC_BUILTINS \
	-I$(MESA)/include -I$(MESA)/src -I$(BUILD)/src \
	-I$(MESA)/src/compiler -I$(BUILD)/src/compiler \
	-I$(MESA)/src/compiler/nir -I$(BUILD)/src/compiler/nir \
	-I$(MESA)/src/gallium/include -I$(MESA)/src/broadcom \
	-I$(BUILD)/src/broadcom

LIBS = $(BUILD)/src/broadcom/compiler/libbroadcom_compiler.a \
	$(BUILD)/src/broadcom/qpu/libbroadcom_qpu.a \
	$(BUILD)/src/broadcom/libbroadcom_v3d.a \
	$(BUILD)/src/compiler/spirv/libvtn.a \
	$(BUILD)/src/compiler/nir/libnir.a \
	$(BUILD)/src/compiler/libcompiler.a \
	$(BUILD)/src/util/libmesa_util.a \
	$(BUILD)/src/util/libmesa_util_simd.a \
	$(BUILD)/src/c11/impl/libmesa_util_c11.a \
	$(BUILD)/src/util/blake3/libblake3.a \
	-lSPIRV-Tools -lstdc++ -lm -lpthread -lz

SHADERS = copy crt crt_smooth crt_curve crt_curve_smooth

v3dshader: v3dshader.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

rect.vert.spv: rect.vert
	glslangValidator -V $< -o $@

copy.frag.spv: copy.frag
	glslangValidator -V $< -o $@

crt.frag.spv: crt.frag
	glslangValidator -V $< -o $@

crt_smooth.frag.spv: crt.frag
	glslangValidator -V -DSMOOTH $< -o $@

crt_curve.frag.spv: crt.frag
	glslangValidator -V -DCURVATURE $< -o $@

crt_curve_smooth.frag.spv: crt.frag
	glslangValidator -V -DCURVATURE -DSMOOTH $< -o $@

../../src/v3d/shader_%.h: rect.vert.spv %.frag.spv v3dshader
	./v3dshader $* rect.vert.spv $*.frag.spv > $@

../../src/v3d/shader71_%.h: rect.vert.spv %.frag.spv v3dshader
	./v3dshader -71 $* rect.vert.spv $*.frag.spv > $@

headers: $(SHADERS:%=../../src/v3d/shader_%.h) $(SHADERS:%=../../src/v3d/shader71_%.h)

clean:
	rm -f v3dshader *.spv

.PHONY: headers clean
