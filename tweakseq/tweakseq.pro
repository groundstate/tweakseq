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
								 include/GoToTool.h \
								 include/ImportDialog.h \
								 include/MAFFT.h \
								 include/MessageWin.h \
								 include/Muscle.h \
								 include/PDB.h \
								 include/PDBFile.h \
								 include/Project.h \
								 include/ResidueSelection.h \
								 include/SearchTool.h \
								 include/SequenceEditor.h \
								 include/SeqEditMainWin.h \
								 include/Sequence.h \
								 include/Sequences.h \
								 include/SequenceFile.h \
								 include/SequenceGroup.h \
								 include/SequenceSelection.h \
								 include/SequencePropertiesDialog.h \
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
								 include/RenameCmd.h \
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
									Core/MAFFT.cpp \
									Core/Muscle.cpp \
									Core/PDB.cpp \
									Core/PDBFile.cpp \
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
									Core/UndoCommand/RenameCmd.cpp \
									Core/UndoCommand/UngroupCmd.cpp

SOURCES				+=  Core/Annotations/Consensus.cpp

SOURCES       +=  UI/GoToTool.cpp \
									UI/MessageWin.cpp \
									UI/SearchTool.cpp \
									UI/SequenceEditor.cpp \
									UI/SeqEditMainWin.cpp \
									UI/SetupWizard.cpp
									
SOURCES				 += UI/Dialogs/AboutDialog.cpp \
									UI/Dialogs/AlignmentToolDlg.cpp \
									UI/Dialogs/ImportDialog.cpp \
									UI/Dialogs/SequencePropertiesDialog.cpp
									
RESOURCES = UI/Resources/application.qrc
									
QT           += core gui xml widgets printsupport

#DEFINES      += QT_NO_DEBUG_OUTPUT 

LIBS += -lz
