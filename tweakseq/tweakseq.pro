MOC_DIR = moc

OBJECTS_DIR = obj

INCLUDEPATH += include

HEADERS       =  include/AboutDialog.h \
								 include/AlignmentTool.h \
								 include/Application.h \
								 include/ClustalFile.h \
								 include/ClustalO.h \
								 include/DebuggingInfo.h \
								 include/FASTAFile.h \
								 include/MessageWin.h \
								 include/Project.h \
								 include/ResidueSelection.h \
								 include/SeqEdit.h \
								 include/SeqEditMainWin.h \
								 include/Sequence.h \
								  include/Sequences.h \
								 include/SequenceFile.h \
								 include/SequenceGroup.h \
								 include/SequenceSelection.h \
								 include/UndoCommand.h \
								 include/UndoAlignmentCommand.h \
								 include/Version.h \
								 include/XMLHelper.h
								 
SOURCES				 =  Core/AlignmentTool.cpp \
									Core/Application.cpp \
									Core/ClustalFile.cpp \
									Core/ClustalO.cpp \
									Core/FASTAFile.cpp \
									Core/Main.cpp \
									Core/Project.cpp \
									Core/ResidueSelection.cpp \
									Core/Sequence.cpp \
									Core/Sequences.cpp \
									Core/SequenceFile.cpp\
									Core/SequenceGroup.cpp\
									Core/SequenceSelection.cpp \
									Core/XMLHelper.cpp
									
SOURCES				+=  Core/UndoCommand/UndoCommand.cpp \
									Core/UndoCommand/UndoAlignmentCommand.cpp
									
SOURCES       +=  UI/AboutDialog.cpp \
									UI/MessageWin.cpp \
									UI/SeqEdit.cpp \
									UI/SeqEditMainWin.cpp 

RESOURCES = UI/Resources/application.qrc
									
QT           += core gui xml qt3support
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#DEFINES      += QT_NO_DEBUG_OUTPUT 

LIBS += 
