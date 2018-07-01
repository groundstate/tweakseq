# tweakseq
A sequence editor

Building
--------

The current version is based on Qt5 and is currently being compiled with Qt5.5

At the top level

	cd tweakseq

	./makeinclude.py

	qmake tweakseq.pro

	make

(the preceding steps should be run after every git pull)

Editor
------

A context menu (right-click) is available for most editor actions.

Multiple sequences can be selected in a continuous block by clicking and dragging the mouse.
Sequences can be dragged by selecting and then clicking again to drag
Individual sequences are added to the current selection with CTRL-click and SHIFT-click.

Insertions are added by selecting a block of residues and then using the space bar. Multiple insertions
can be made by typing the number of insertions required, prior to using the space bar. By convention,
insertions are made after the selected residues. If the SHIFT key is used with the space bar, insertions
are made before the selected residues. Insertions can be removed by selecting them and then using "Cut'.

The current selection can be grouped.
All members of a group can be selected with double-click.
A group can be locked. When locked, insertions apply to all members of the group.

Members of a group can be hidden by selecting the group members that you want to display and then
selecting the 'Hide non-selected' menu item. 
This is useful to reduce the number of sequences displayed.
When locked, insertions are also applied to hidden sequences.

Bookmarks can be added to help with navigating large alignments. The search box in the toolbar can also be used for navigation.

Undo/redo works for most things but is a work in progress.

Alignment
---------

ClustalO and MUSCLE are supported.

Alignment output appears in the bottom area of the window.
