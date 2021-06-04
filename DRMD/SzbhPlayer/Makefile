

#=============DIR======================================================================

#=============TOOL CHAINS===================================================================
AR=$(CFG_HI_ARM_TOOLCHAINS_NAME)-ar
AS=$(CFG_HI_ARM_TOOLCHAINS_NAME)-as
LD=$(CFG_HI_ARM_TOOLCHAINS_NAME)-ld
CPP=$(CFG_HI_ARM_TOOLCHAINS_NAME)-cpp
CC=$(CFG_HI_ARM_TOOLCHAINS_NAME)-gcc
NM=$(CFG_HI_ARM_TOOLCHAINS_NAME)-nm
STRIP=$(CFG_HI_ARM_TOOLCHAINS_NAME)-strip
OBJCOPY=$(CFG_HI_ARM_TOOLCHAINS_NAME)-objcopy
OBJDUMP=$(CFG_HI_ARM_TOOLCHAINS_NAME)-objdump

#==============COMPILE OPTIONS================================================================
CFLAGS := -Werror -Wall

ifneq ($(findstring $(CFG_HI_CHIP_TYPE), hi3798mv100 hi3796mv100 hi3716dv100),)
CFLAGS += -mcpu=cortex-a7
else ifneq ($(findstring $(CFG_HI_CHIP_TYPE), hi3798cv200 hi3798mv200 hi3798mv200_a),)
CFLAGS += -mcpu=cortex-a53
else
CFLAGS += -mcpu=cortex-a9
endif

CFLAGS+= -D_GNU_SOURCE -Wall -O2 -ffunction-sections -fdata-sections -Wl,--gc-sections -g
CFLAGS+= -DCHIP_TYPE_$(CFG_HI_CHIP_TYPE) -DCFG_HI_SDK_VERSION=$(CFG_HI_SDK_VERSION)
CFLAGS+= $(CFG_HI_BOARD_CONFIGS)

ifeq ($(CFG_HI_HDMI_RX_SUPPORT),y)
CFLAGS += -DHI_HDMI_RX_INSIDE
endif
ifeq ($(CFG_HI_ADVCA_FUNCTION),FINAL)
    CFLAGS += -DHI_ADVCA_FUNCTION_RELEASE
else
    CFLAGS += -DHI_ADVCA_FUNCTION_$(CFG_HI_ADVCA_FUNCTION)
endif


LFLAGS := -L$(HI_OUT_DIR)/lib/external -L$(HI_OUT_DIR)/lib/share

SYS_LIBS := -lpthread -lrt -lm -ldl -lstdc++

HI_LIBS := -lhi_common -lfreetype -lharfbuzz -lz -lpng -lhigo -lhigoadp -ljpeg

DEPEND_DRM_LIBS := -ldrmengine -lutils -lcutils -llog -lbinder_ipc -ldrm -ldrm_common

HI_LIBS += -lhi_msp -lhi_so -lhi_charsetMgr -lsubdec -lplayer -lhiplayer_utils $(DEPEND_DRM_LIBS)


#================== ffmpeg ====================================
#CFLAGS += -I../VehicleServer/ffmpeg-4.1/install/include
#LFLAGS += -L../VehicleServer/ffmpeg-4.1/install/lib -lavformat -lavcodec -lavutil -lswscale -lswresample



#=============INCLUDE_DIR==================================================================
SZBH_API_DIR := $(SDK_DIR)/myapp/out/install

CFLAGS += -I$(SZBH_API_DIR)/include
LFLAGS += -L$(SZBH_API_DIR)/lib -lSzbhApi

TARGET := szbhplayer
OUT_DIR := out


SRC_OBJS := XTsDemux.o XTsFfmpegDemux.o XSectionFile.o XConfigFile.o XStationText.o  XScreenSnapshot.o XMenuShow.o XSetupMenu.o XPlayer.o  XScrollText.o XControlProtocol.o XDeviceState.o XPTUControl.o XProcessDrmd.o XPiscProc.o XColorCheck.o XLocalPlayer.o main.o
OUT_OBJS := $(addprefix $(OUT_DIR)/, $(SRC_OBJS))


all:$(TARGET)


$(SRC_OBJS):%.o:%.c
	if [ ! -d $(OUT_DIR) ]; then mkdir -p $(OUT_DIR); fi;\
	$(CC) -fPIC -c $(CFLAGS) $< -o $(OUT_DIR)/$@

$(TARGET):$(SRC_OBJS)
	$(CC) $(CFLAGS) $(OUT_OBJS) -o $(OUT_DIR)/$@ $(LFLAGS) $(SYS_LIBS) $(HI_LIBS)
	
clean:
	rm -rf $(OUT_DIR)/*
