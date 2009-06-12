#!/bin/python
# coding=UTF-8
import os, tempfile, shutil, stat

pngdir = None

def getFigureFiles():
	ret = []
	for f in os.listdir('figures'):
		if (f.endswith('.tex')):
			ret += [f]
	return ret

def rmDir(top):
	for root, dirs, files in os.walk(top):
		for name in files:
			os.remove(os.path.join(root, name))
		for name in dirs:
			os.rmdir(os.path.join(root, name))
	os.rmdir(top)

# Get a temporary directory to do our shit
tmpdir = tempfile.mkdtemp()
curwd = os.getcwd()

figsdir = os.path.join(curwd, 'figures')
os.putenv("TEXINPUTS", curwd + ":" + figsdir + ":");

files = getFigureFiles()

os.chdir(tmpdir)
for f in files:
	sourcefile = os.path.join(figsdir, f)
	epsfile = os.path.join(figsdir, f)[:-4] + ".eps"
	pdffile = os.path.join(figsdir, f)[:-4] + ".pdf"
	
	sourcefilestats = os.stat(sourcefile)
	buildeps = False
	buildpdf = False
	buildpng = False
	
	if not os.path.exists(epsfile): buildeps = True
	else:
		epsfilestats = os.stat(epsfile)
		if sourcefilestats[stat.ST_MTIME] > epsfilestats[stat.ST_MTIME]: buildeps = True
	if not os.path.exists(pdffile): buildpdf = True
	else:
		pdffilestats = os.stat(pdffile)
		if sourcefilestats[stat.ST_MTIME] > pdffilestats[stat.ST_MTIME]: buildpdf = True
	
	includedsourcefile = os.path.join(figsdir, f)[:-4]
	figfile = os.path.join(tmpdir, 'figure.tex')
		
	outfile = open(figfile, 'w')
	outfile.write('\\documentclass{article}\n\\usepackage{tikz}\n\\usetikzlibrary{shapes}\n\\usetikzlibrary{snakes}\n\\begin{document}\n\\pagestyle{empty}\n\\begin{tikzpicture}\n\\input{' + includedsourcefile + '}\n\\end{tikzpicture}\n\\end{document}\n')
	#outfile.write('\\documentclass[trans]{beamer}\n\\usepackage{tikz}\n\\usetikzlibrary{shapes}\n\\usetikzlibrary{snakes}\n\\beamertemplatenavigationsymbolsempty\n\\begin{document}\n\\pagestyle{empty}\n\\begin{frame}\n\\begin{tikzpicture}\n\\input{' + includedsourcefile + '}\n\\end{tikzpicture}\n\\end{frame}\n\\end{document}\n')
	outfile.close()
	
	# Build EPS?
	if buildeps:
		os.spawnlp(os.P_WAIT, 'latex', 'latex', figfile)
		os.spawnlp(os.P_WAIT, 'dvips', 'dvips', '-E', '-o',  'figure-temp.eps', 'figure.dvi')
		os.spawnlp(os.P_WAIT, 'epstool', 'epstool', '--copy', '--bbox', 'figure-temp.eps', 'figure.eps')
		shutil.copyfile(os.path.join(tmpdir, 'figure.eps'), epsfile)

	# Build PDF?
	if buildpdf:
		os.spawnlp(os.P_WAIT, 'pdflatex', 'pdflatex', figfile)
		os.spawnlp(os.P_WAIT, 'pdfcrop', 'pdfcrop', 'figure.pdf', 'figure2.pdf')
		shutil.copyfile(os.path.join(tmpdir, 'figure2.pdf'), pdffile)
	
	if pngdir != None:
		pngfile = os.path.join(pngdir, f)[:-4] + ".png"
		if not os.path.exists(pngfile): buildpng = True
		else:
			pngfilestats = os.stat(pngfile)
			if sourcefilestats[stat.ST_MTIME] > pngfilestats[stat.ST_MTIME]: buildpng = True
		
		# Build PNG?
		if buildpng:
			os.spawnlp(os.P_WAIT, 'convert', 'convert', '-density', '125', pdffile, pngfile)
rmDir(tmpdir)
