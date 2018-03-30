# tweakseq
A sequence editor

Building
--------

From the top level

	cd tweakseq

	./makeinclude.py

	qmake-qt4 tweakseq.pro

	make

(the preceding steps should be run after every git pull)

Editor
------

Multiple sequences can be selected in a continuous block by clicking and dragging the mouse.
Individual sequences are added to the current selection with CTRL-click.

Insertions are added by selecting a block of residues and then using the space bar. Multiple insertions
can be made by typing the number of insertions required, prior to using the space bar. By convention,
insertions are made after the selected residues. If the SHIFT key is used with the space bar, insertions
are made before the selected residues. Insertions can be removed by selecting them and then using "cut'.

The current selection can be grouped.
All members of a group can be selected with double-click.
A group can be locked. When locked, insertions apply to all members of the group.

Members of a group can be hidden by selecting the group members that you want to display and then
selecting the 'Hide non-selected' menu item. 
This is useful to reduce the number of sequences displayed.
When locked, insertions are applied to hidden sequences group too.

