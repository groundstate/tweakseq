MOC_DIR = moc

OBJECTS_DIR = obj

INCLUDEPATH += include

DEPENDPATH=$$INCLUDEPATH

HEADERS       =  include/AboutDialog.h \
								 include/AlignmentTool.h \
								 include/AlignmentToolDlg.h \
								 include/AminoAcids.h \
								 include/Application.h \
								 include/Clipboard.h \
								 include/ClustalFile.h \
								 include/ClustalO.h \
								 include/DNA.h \
								 include/DebuggingInfo.h \
								 include/FASTAFile.h \
								 include/FindTool.h \
								 include/ImportDialog.h \
								 include/MessageWin.h \
								 include/Muscle.h \
								 include/Project.h \
								 include/ResidueSelection.h \
								 include/SequenceEditor.h \
								 include/SeqEditMainWin.h \
								 include/SeqPreview.h \
								 include/SeqPreviewDlg.h \
								 include/Sequence.h \
								 include/Sequences.h \
								 include/SequenceFile.h \
								 include/SequenceGroup.h \
								 include/SequenceSelection.h \
								 include/SetupWizard.h \
								 include/Command.h \
								 include/AddInsertionsCmd.h \
								 include/AlignmentCmd.h \
								 include/CutResiduesCmd.h \
								 include/CutSequencesCmd.h \
								 include/GroupCmd.h \
								 include/PasteCmd.h \
								 include/ImportCmd.h \
								 include/MoveCmd.h \
								 include/UngroupCmd.h \
								 include/Utility.h \
								 include/Version.h \
								 include/XMLHelper.h \
								 include/Consensus.h
								 
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
									
SOURCES				+=  Core/UndoCommand/Command.cpp \
									Core/UndoCommand/AddInsertionsCmd.cpp \
									Core/UndoCommand/AlignmentCmd.cpp \
									Core/UndoCommand/CutResiduesCmd.cpp \
									Core/UndoCommand/CutSequencesCmd.cpp \
									Core/UndoCommand/ExcludeResiduesCmd.cpp \
									Core/UndoCommand/GroupCmd.cpp \
									Core/UndoCommand/ImportCmd.cpp \
									Core/UndoCommand/MoveCmd.cpp \
									Core/UndoCommand/PasteCmd.cpp \
									Core/UndoCommand/UngroupCmd.cpp

SOURCES				+=  Core/Annotations/Consensus.cpp

SOURCES       +=  UI/AboutDialog.cpp \
									UI/AlignmentToolDlg.cpp \
									UI/FindTool.cpp \
									UI/ImportDialog.cpp \
									UI/MessageWin.cpp \
									UI/SequenceEditor.cpp \
									UI/SeqEditMainWin.cpp \
									UI/SeqPreview.cpp \
									UI/SeqPreviewDlg.cpp \
									UI/SetupWizard.cpp

RESOURCES = UI/Resources/application.qrc
									
QT           += core gui xml widgets printsupport

#DEFINES      += QT_NO_DEBUG_OUTPUT 

LIBS += 
