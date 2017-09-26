MOC_DIR = moc

OBJECTS_DIR = obj

INCLUDEPATH += include

HEADERS       =  include/AboutDialog.h \
								 include/Application.h \
								 include/ClustalFile.h \
								 include/DebuggingInfo.h \
								 include/FASTAFile.h \
								 include/MessageWin.h \
								 include/SeqEdit.h \
								 include/SeqEditMainWin.h \
								 include/Sequence.h \
								 include/SequenceFile.h 
								 
SOURCES				 =  Core/Application.cpp \
									Core/ClustalFile.cpp \
									Core/FASTAFile.cpp \
									Core/Main.cpp \
									Core/Sequence.cpp \
									Core/SequenceFile.cpp
									
SOURCES       +=  UI/AboutDialog.cpp \
									UI/MessageWin.cpp \
									UI/SeqEdit.cpp \
									UI/SeqEditMainWin.cpp 

RESOURCES = UI/Resources/application.qrc
									
QT           += core gui xml qt3support
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#DEFINES      += QT_NO_DEBUG_OUTPUT 

LIBS += 
