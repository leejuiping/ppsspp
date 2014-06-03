VERSION = 0.9.8
DEFINES += USING_QT_UI USE_FFMPEG
unix:!qnx:!symbian:!mac: CONFIG += linux
maemo5|contains(MEEGO_EDITION,harmattan): CONFIG += maemo
arm:!symbian: CONFIG += armv7

# Global specific
win32:CONFIG(release, debug|release): CONFIG_DIR = $$join(OUT_PWD,,,/release)
else:win32:CONFIG(debug, debug|release): CONFIG_DIR = $$join(OUT_PWD,,,/debug)
else:CONFIG_DIR=$$OUT_PWD
OBJECTS_DIR = $$CONFIG_DIR/.obj/$$TARGET
MOC_DIR = $$CONFIG_DIR/.moc/$$TARGET
UI_DIR = $$CONFIG_DIR/.ui/$$TARGET
P = $$_PRO_FILE_PWD_/..
INCLUDEPATH += $$P/ext/zlib $$P/Common

exists($$P/.git): GIT_VERSION = '\\"$$system(git describe --always)\\"'
isEmpty(GIT_VERSION): GIT_VERSION = '\\"$$VERSION\\"'
DEFINES += PPSSPP_GIT_VERSION=\"$$GIT_VERSION\"

win32-msvc* {
	QMAKE_CXXFLAGS_RELEASE += /O2 /arch:SSE2 /fp:fast
	DEFINES += _MBCS GLEW_STATIC _CRT_SECURE_NO_WARNINGS
	DEFINES += "_VARIADIC_MAX=10"
	contains(DEFINES,UNICODE): DEFINES+=_UNICODE
	PRECOMPILED_HEADER = $$P/Windows/stdafx.h
	PRECOMPILED_SOURCE = $$P/Windows/stdafx.cpp
	INCLUDEPATH += .. $$P/ffmpeg/Windows/$${QMAKE_TARGET.arch}/include
} else {
	DEFINES += __STDC_CONSTANT_MACROS
	QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-variable -Wno-multichar -Wno-uninitialized -Wno-ignored-qualifiers -Wno-missing-field-initializers -Wno-unused-parameter
	QMAKE_CXXFLAGS += -ffast-math -fno-strict-aliasing
	greaterThan(QT_MAJOR_VERSION,4): CONFIG+=c++11
	else {
		maemo: QMAKE_CXXFLAGS += -std=gnu++0x
		else: QMAKE_CXXFLAGS += -std=c++0x
	}
	QMAKE_CFLAGS_RELEASE -= -O2
	QMAKE_CFLAGS_RELEASE += -O3
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -O3
}
# Arch specific
xarch = $$find(QT_ARCH, "86")
contains(QT_ARCH, windows)|count(xarch, 1) {
	!win32-msvc*: QMAKE_CXXFLAGS += -msse2
	CONFIG += x86
} else { # Assume ARM
	DEFINES += ARM
	CONFIG += arm
}

gleslib = $$find(QT_CONFIG, "opengles")
count(gleslib,0) { # OpenGL
	INCLUDEPATH += $$P/native/ext/glew
} else {
	DEFINES += USING_GLES2 MOBILE_DEVICE
	CONFIG += mobile_platform
}

# Platform specific
contains(MEEGO_EDITION,harmattan): DEFINES += MEEGO_EDITION_HARMATTAN "_SYS_UCONTEXT_H=1"
maemo5: DEFINES += MAEMO

macx: INCLUDEPATH += $$P/ffmpeg/macosx/x86_64/include
ios: INCLUDEPATH += $$P/ffmpeg/ios/universal/include

linux:!android {
	arm: INCLUDEPATH += $$P/ffmpeg/linux/arm/include
	x86 {
		contains(QT_ARCH, x86_64): QMAKE_TARGET.arch = x86_64
		else: QMAKE_TARGET.arch = x86
		INCLUDEPATH += $$P/ffmpeg/linux/$${QMAKE_TARGET.arch}/include
	}
}
qnx {
	# Use mkspec: unsupported/qws/qnx-armv7-g++
	DEFINES += BLACKBERRY "_QNX_SOURCE=1" "_C99=1"
	INCLUDEPATH += $$P/ffmpeg/blackberry/armv7/include
}
symbian {
	# Does not seem to be a way to change to armv6 compile so just override in variants.xml (see Wiki)
	DEFINES += "BOOST_COMPILER_CONFIG=\"$$EPOCROOT/epoc32/include/stdapis/boost/mpl/aux_/config/gcc.hpp\""
	QMAKE_CXXFLAGS += -marm -Wno-parentheses -Wno-comment
	INCLUDEPATH += $$EPOCROOT/epoc32/include/stdapis
	INCLUDEPATH += $$P/ffmpeg/symbian/armv6/include
}
android {
	DEFINES += ANDROID ARMEABI_V7A ARMV7
	QMAKE_CXXFLAGS += -march=armv7
	INCLUDEPATH += $$P/native/ext/libzip
	INCLUDEPATH += $$P/ffmpeg/android/armv7/include
}
