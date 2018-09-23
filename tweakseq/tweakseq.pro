MOC_DIR = moc

OBJECTS_DIR = obj

INCLUDEPATH += include

DEPENDPATH=$$INCLUDEPATH

HEADERS       =  include/AboutDialog.h \
								 include/AlignmentTool.h \
								 include/AminoAcids.h \
								 include/Application.h \
								 include/Clipboard.h \
								 include/ClustalFile.h \
								 include/ClustalO.h \
								 include/DNA.h \
								 include/DebuggingInfo.h \
								 include/EditSequencePropertiesCmd.h \
								 include/FASTAFile.h \
								 include/GoToTool.h \
								 include/ImportDialog.h \
								 include/MAFFT.h \
								 include/MessageWin.h \
								 include/Muscle.h \
								 include/PDB.h \
								 include/PDBFile.h \
								 include/Project.h \
								 include/Propertied.h \
								 include/PropertiesDialog.h \
								 include/ResidueLockGroup.h \
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
								 include/Structure.h \
								 include/Property.h \
								 include/BoolProperty.h \
								 include/DoubleProperty.h \
								 include/IntProperty.h \
								 include/FloatProperty.h \
								 include/FileProperty.h \
								 include/StringProperty.h \
								 include/DoubleInput.h \
								 include/DoublePropertyInput.h \
								 include/EnumIntPropertyInput.h \
								 include/FilePropertyInput.h \
								 include/FloatInput.h \
								 include/FloatPropertyInput.h \
								 include/FloatRangePropertyInput.h \
								 include/IntInput.h \
								 include/IntPropertyInput.h \
								 include/OptionPropertyInput.h \
								 include/PropertyInput.h \
								 include/PropertyWidget.h \
								 include/StringPropertyInput.h \
								 include/SuperSlider.h \
								 include/Command.h \
								 include/AddInsertionsCmd.h \
								 include/AlignmentCmd.h \
								 include/CutResiduesCmd.h \
								 include/CutSequencesCmd.h \
								 include/GroupCmd.h \
								 include/PasteCmd.h \
								 include/ImportCmd.h \
								 include/LockResiduesCmd.h \
								 include/MoveCmd.h \
								 include/RenameCmd.h \
								 include/TrimCmd.h \
								 include/UngroupCmd.h \
								 include/UnlockResiduesCmd.h \
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
									Core/Propertied.cpp \
									Core/ResidueLockGroup.cpp \
									Core/ResidueSelection.cpp \
									Core/Sequence.cpp \
									Core/Sequences.cpp \
									Core/SequenceFile.cpp\
									Core/SequenceGroup.cpp\
									Core/SequenceSelection.cpp \
									Core/Structure.cpp \
									Core/Utility.cpp \
									Core/XMLHelper.cpp
									
SOURCES				+=  Core/UndoCommand/Command.cpp \
									Core/UndoCommand/AddInsertionsCmd.cpp \
									Core/UndoCommand/AlignmentCmd.cpp \
									Core/UndoCommand/CutResiduesCmd.cpp \
									Core/UndoCommand/CutSequencesCmd.cpp \
									Core/UndoCommand/EditSequencePropertiesCmd.cpp \
									Core/UndoCommand/ExcludeResiduesCmd.cpp \
									Core/UndoCommand/GroupCmd.cpp \
									Core/UndoCommand/ImportCmd.cpp \
									Core/UndoCommand/LockResiduesCmd.cpp \
									Core/UndoCommand/MoveCmd.cpp \
									Core/UndoCommand/PasteCmd.cpp \
									Core/UndoCommand/RenameCmd.cpp \
									Core/UndoCommand/TrimCmd.cpp \
									Core/UndoCommand/UngroupCmd.cpp \
									Core/UndoCommand/UnlockResiduesCmd.cpp \
									
SOURCES					+= Core/Properties/Property.cpp \
								 Core/Properties/BoolProperty.cpp \
								 Core/Properties/DoubleProperty.cpp \
								 Core/Properties/IntProperty.cpp \
								 Core/Properties/FloatProperty.cpp \
								 Core/Properties/FileProperty.cpp \
								 Core/Properties/StringProperty.cpp
								 
SOURCES				+=  Core/Annotations/Consensus.cpp

SOURCES       +=  UI/GoToTool.cpp \
									UI/MessageWin.cpp \
									UI/SearchTool.cpp \
									UI/SequenceEditor.cpp \
									UI/SeqEditMainWin.cpp \
									UI/SetupWizard.cpp
									
SOURCES				 += UI/Dialogs/AboutDialog.cpp \
									UI/Dialogs/ImportDialog.cpp \
									UI/Dialogs/PropertiesDialog.cpp \
									UI/Dialogs/SequencePropertiesDialog.cpp

SOURCES					+= 	UI/Widgets/DoubleInput.cpp \
									UI/Widgets/DoublePropertyInput.cpp \
									UI/Widgets/EnumIntPropertyInput.cpp \
									UI/Widgets/FilePropertyInput.cpp \
									UI/Widgets/FloatInput.cpp \
									UI/Widgets/FloatPropertyInput.cpp \
									UI/Widgets/FloatRangePropertyInput.cpp \
									UI/Widgets/IntInput.cpp \
									UI/Widgets/IntPropertyInput.cpp \
									UI/Widgets/OptionPropertyInput.cpp \
									UI/Widgets/PropertyWidget.cpp \
									UI/Widgets/StringPropertyInput.cpp \
									UI/Widgets/SuperSlider.cpp
									
RESOURCES = UI/Resources/application.qrc
									
QT           += core gui xml widgets printsupport

#DEFINES      += QT_NO_DEBUG_OUTPUT 

LIBS += -lz
