#!/usr/bin/python

import glob
import os

includedir = './include'

if not os.path.exists(includedir):
	os.makedirs(includedir)
else:
	for f in os.listdir(includedir):
		fpath = os.path.join(includedir,f)
		try:
			if os.path.isfile(fpath):
				os.unlink(fpath)
		except Exception as e:
			print(e)

os.chdir(includedir)				
searchpaths = ['../Core','../UI']

for sp in searchpaths:
	pattern=os.path.join(sp,'*.h')
	headers=glob.glob(pattern)
	for hdr in headers:
		os.symlink(hdr,os.path.basename(hdr))
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	