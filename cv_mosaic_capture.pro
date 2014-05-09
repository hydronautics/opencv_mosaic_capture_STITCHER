TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH += C:\OpenCV249\build\include C:\OpenCV249\build\include\opencv
LIBS += -LC:\OpenCV249\build\x86\vc10\lib


CONFIG(debug, debug|release) {
    LIBS += -lopencv_core249d \
    -lopencv_highgui249d \
} else {
    LIBS += -lopencv_core249 \
    -lopencv_highgui249 \

}
