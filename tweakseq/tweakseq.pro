MOC_DIR = moc

OBJECTS_DIR = obj

INCLUDEPATH += include

DEPENDPATH=$$INCLUDEPATH

HEADERS       =  include/AboutDialog.h \
								 include/AlignmentTool.h \
								 include/AlignmentToolDlg.h \
								 include/Application.h \
								 include/Clipboard.h \
								 include/ClustalFile.h \
								 include/ClustalO.h \
								 include/DebuggingInfo.h \
								 include/FASTAFile.h \
								 include/MessageWin.h \
								 include/Muscle.h \
								 include/Project.h \
								 include/ResidueSelection.h \
								 include/SeqEditor.h \
								 include/SeqResidueView.h \
								 include/SeqInfoView.h \
								 include/SeqEditMainWin.h \
								 include/SeqPreview.h \
								 include/SeqPreviewDlg.h \
								 include/Sequence.h \
								 include/Sequences.h \
								 include/SequenceFile.h \
								 include/SequenceGroup.h \
								 include/SequenceSelection.h \
								 include/SetupWizard.h \
								 include/UndoCommand.h \
								 include/UndoAlignmentCommand.h \
								 include/Utility.h \
								 include/Version.h \
								 include/XMLHelper.h
								 
SOURCES				 =  Core/AlignmentTool.cpp \
									Core/Application.cpp \
									Core/Clipboard.cpp \
									Core/ClustalFile.cpp \
									Core/ClustalO.cpp \
									Core/FASTAFile.cpp \
									Core/Main.cpp \
									Core/Muscle.cpp \
									Core/Project.cpp \
									Core/ResidueSelection.cpp \
									Core/Sequence.cpp \
									Core/Sequences.cpp \
									Core/SequenceFile.cpp\
									Core/SequenceGroup.cpp\
									Core/SequenceSelection.cpp \
									Core/Utility.cpp \
									Core/XMLHelper.cpp
									
SOURCES				+=  Core/UndoCommand/UndoCommand.cpp \
									Core/UndoCommand/UndoAlignmentCommand.cpp
									
SOURCES       +=  UI/AboutDialog.cpp \
									UI/AlignmentToolDlg.cpp \
									UI/MessageWin.cpp \
									UI/SeqInfoView.cpp \
									UI/SeqResidueView.cpp \
									UI/SeqEditor.cpp \
									UI/SeqEditMainWin.cpp \
									UI/SeqPreview.cpp \
									UI/SeqPreviewDlg.cpp \
									UI/SetupWizard.cpp

RESOURCES = UI/Resources/application.qrc
									
QT           += core gui xml widgets printsupport

#DEFINES      += QT_NO_DEBUG_OUTPUT 

LIBS += 
