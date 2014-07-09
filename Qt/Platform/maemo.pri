DEFINES += MAEMO __GL_EXPORTS
contains(MEEGO_EDITION,harmattan): DEFINES += "_SYS_UCONTEXT_H=1"

equals(TARGET, PPSSPPQt) {
	# Packaging
	target.path = /opt/PPSSPP/bin
	desktopfile.files = PPSSPP.desktop
	desktopfile.path = /usr/share/applications
	INSTALLS += target desktopfile
	# Booster
	QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
	QMAKE_LFLAGS += -pie -rdynamic
	CONFIG += qt-boostable
}
