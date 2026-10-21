#
# pi45_libs.mk: the Raspberry Pi 4 and 5 builds link without the VideoCore
# service libraries (see pi45.mk). Included from Makefile after LIBS.
#
# This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
# See LICENSE for licensing details.
#

ifneq ($(filter 4 5,$(RASPPI)),)
LIBS := $(filter-out %/libvchiq.a %/libbcm_host.a %/libkhrn_client.a %/libvcos.a \
                     %/libvmcs_host.a %/liblinuxemu.a,$(LIBS))
endif
