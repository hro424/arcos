
TEX=		latex
BIBTEX=		bibtex
DVIVIEWER=	xdvi
PDFLATEX=	pdflatex
PDFVIEWER=	acroread

SRC=		00_ngc.tex
DVI=		$(SRC:.tex=.dvi)
PDF=		$(SRC:.tex=.pdf)
BIB=		reference.bib

%.dvi:%.tex
	$(TEX) $(^:.tex=)
	$(BIBTEX) $(^:.tex=)
	$(TEX) $(^:.tex=)
	$(TEX) $(^:.tex=)

%.pdf:%.tex
	$(TEX) $^
	$(BIBTEX) $(^:.tex=)
	$(TEX) $^
	$(PDFLATEX) $^

all: pdf

view: $(DVI)
	$(DVIVIEWER) $^ &

pdf: $(DVI)
	dvipdfm $^

.PHONY: clean
clean:
	rm -fr *.log *.dvi *.pbm *.aux *.pdf *.bbl *blg

