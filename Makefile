#
# media-server 组件 Makefile（框架库）
#
include $(TOPDIR)/rules.mk

PKG_NAME:=media-server
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/media-server
  CATEGORY:=Oray
  SECTION:=utils
  TITLE:=Oray media-server framework
  DEPENDS:=+libstdcpp
endef

define Package/media-server/description
  MPI/stream/media 抽象框架，支持多路流、监听者、硬件检测。
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_CXXFLAGS += -I$(STAGING_DIR)/usr/include

MEDIA_SERVER_SRC := $(PKG_BUILD_DIR)/src
MEDIA_SERVER_INC := $(MEDIA_SERVER_SRC)

define Build/Prepare
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		AR="$(TARGET_AR)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)"
endef

define Package/media-server/install
	$(INSTALL_DIR) $(1)/usr/include/media-server
	$(CP) $(PKG_BUILD_DIR)/src/mpi/*.h $(1)/usr/include/media-server/ 2>/dev/null || true
	$(CP) $(PKG_BUILD_DIR)/src/mpi_ctx/*.h $(1)/usr/include/media-server/ 2>/dev/null || true
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libmedia_server.a $(1)/usr/lib/ 2>/dev/null || true
endef

$(eval $(call BuildPackage,media-server))
