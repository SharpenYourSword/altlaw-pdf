//========================================================================
//
// Altlaw.h
//
//========================================================================

#ifndef ALTLAW_H
#define ALTLAW_H

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <string.h>
#include "goo/gtypes.h"
#include "goo/GooList.h"
#include "goo/GooString.h"
#include "GfxFont.h"
#include "OutputDev.h"


// This should really be a function of the chosen DPI
// Assume 100dpi
#define ALTFUDGEY 10.0
#define ALTFUDGEX 8.0

class GfxState;
class GooString;

class AltlawDoc;
class AltLine;
class AltWord;
class AltString;

class AltDataPoint {
  public:
    AltDataPoint() { count = 0; };
    AltDataPoint(double d) { data = d; count = 1; };
    ~AltDataPoint() {};

    double data;
    int count;

};

static int DataCmp(const void *ptr1, const void *ptr2) {
  AltDataPoint* d1 = *((AltDataPoint**) ptr1);
  AltDataPoint* d2 = *((AltDataPoint**) ptr2);
  double diff = d1->data - d2->data;
  double epsilon = 0.05;

  return diff >= epsilon ? 1 : diff <= -epsilon ? -1 : 0;

};

static int CountCmp(const void *ptr1, const void *ptr2) {
  AltDataPoint* d1 = *((AltDataPoint**) ptr1);
  AltDataPoint* d2 = *((AltDataPoint**) ptr2);

  return d1->count > d2->count ? 1 : d1->count < d2->count ? -1 : 0;

};

#define NUM_CAPS_TYPES 5
enum CapMode {
  CAPS_LOWER,
  CAPS_FIRST,
  CAPS_UPPER,
  CAPS_NOALPHA,
  CAPS_UNKNOWN
};

/* AltString */
class AltString: public GooString {
  public:
  AltString(GooString *s, int page,
  			double x1, double y1, double x2, double y2, double yDraw,
			GfxFont *font, double fontSize, GBool bold, GBool italics);
  AltString(AltString* s);
  AltString() { _page = 0; _x1 = _y1 = _x2 = _y2 = _yDraw = _fontSize = _overlap = 0.0; _bold = _italics = _underline = gFalse; _font = NULL; };
  ~AltString() {};

  // acts like cmp(str, arg)
  int cmpPYX(int page, double x1, double y1, double x2, double y2);
  GBool isNum();
  double yOverlap(AltString *str) { return this->yOverlap(str->_y1,str->_y2); };
  double yOverlap(double y1, double y2);
  GBool looksSimilar(AltString *str);
  GBool looksLikeFootnote(double lineY, double lineFont);

  GBool hasAlpha();
  GBool isUpper();
  GBool isLower();
  GBool isFirstCap();
  CapMode capMode();

  static char* _lower;
  static char* _upper;

  void print(GBool full=gTrue);

  int page() { return _page; };
  double X() { return _x1; };
  double Y() { return _yDraw; };
  double dX() { return _x2 - _x1; };
  double yMin() { return _y1; };
  double yMax() { return _y2; };
  GfxFont *font() { return _font; };
  double fontSize() { return _fontSize; };
  GBool bold() { return _bold; };
  GBool italics() { return _italics; };
  GBool underline() { return _underline; };

  protected:
  int _page;
  double _x1,_y1,_x2,_y2,_yDraw,_overlap;
  double _fontSize;
  GfxFont *_font;
  GBool _bold,_italics,_underline;

  friend class AltLine;
  friend class AltlawDoc;
};

static int Xsorter(const void *ptr1, const void *ptr2) {
  AltString *s1 = *((AltString **) ptr1);
  AltString *s2 = *((AltString **) ptr2);
  return s1->X() > s2->X() ? 1 : s1->X() < s2->X() ? -1 : 0;
};

static int PYXsorter(const void *ptr1, const void *ptr2) {
  AltString *s1;
  AltString *s2;
  s1 = *((AltString**) ptr1);
  s2 = *((AltString**) ptr2);
  return s1->cmpPYX(s2->page(),s2->X(),s2->yMin(),s2->X()+s2->dX(),s2->yMax());
};

class AltWord: public AltString  {
  public:
  AltWord(double x1, double y1, double x2, double y2);
  AltWord(AltString *s);
  ~AltWord() {};

  void print(GBool full=gTrue);

  GBool sup;
  int numChars;

  AltLine *_line;

  friend class AltLine;
};

enum AltLineType {
  Header,
  Body,
  Footnote,
  Repeat,
  Unknown,
  null,
  Pstart,
  Paragraph,
  Pend,
  BlockQuote,
  SectionHeading,
  Separator,
  BQstart,
  BQend,
  Psingle,
};

class AltLine {
  public:
  AltLine(AltlawDoc *doc);
  ~AltLine() {
    deleteGooList(_strings, AltString);
    deleteGooList(_words, AltWord);
  };

  void add(AltString *str);
  void chomp();
  AltWord* startNewWord(AltString *str);
  void parseWords();
  void sort() { _strings->sort(Xsorter); }

  void print(GBool full=gTrue);

  GooList *strings() { return _strings; };
  GooList *words() { return _words; };

  GBool match(char *str);
  GBool looksLikeFootnote();
  GBool looksLikeSectionHeading();
  GBool isBold();

  // basic accessors
  AltLineType type() { return _type; };
  void setType(AltLineType type) { _type = type; };
  int page() { return _page; };
  double X() { return _x; };
  double Y() { return _y; };
  double rightX() { return _x + _dx; };
  double length() { return _dx; };
  double height() { return _dy; };
  double fontSize() { return _fontSize; };
  int chars() { return _chars; };
  void calcFeatures();

  /* features */
  CapMode capMode;
  double avgWordHeight;
  double avgCharWidth;
  double normAvgWordHeight;
  double normAvgCharWidth;
  double align;
  double vertPos;
  int pageLineId;
  int docLineId;
  int pageLines;
  int docLines;
  double prevDistance;
  double nextDistance;
  int wordCount;
  double normLength;

  private:
  AltlawDoc *_doc;
  GooList *_strings;
  GooList *_words;
  int _page, _chars;
  double _x,_y,_dx,_dy,_fontSize;
  AltLineType _type;

};

//------------------------------------------------------------------------
// AltlawDoc
//------------------------------------------------------------------------

class AltlawDoc: public OutputDev {
public:

  AltlawDoc(PDFDoc *pdf);
  virtual ~AltlawDoc();

  void sort() { strings.sort(PYXsorter); }
  void print(GBool full=gFalse);

  void parse();
  void calcFeatures();
  void normalizeLineFeatures();
  void calcLineTypes();
  int findOpinionStart();

  void addUnderline(int page, double x1, double y1, double x2, double y2);
  AltWord* getWordAt(int page, double x1, double y1, double x2, double y2);

  GBool isRepeatedWord( AltWord *word );
  GBool isRepeatedLine( AltLine *line );
  GBool looksLikeBody( AltLine *line);
  GBool onLeftMargin( AltLine *line );
  GBool onRightMargin( AltLine *line );
  GBool onTabStop( AltLine *line, int tab );

  int pages() { return _pages; };
  double pageHeight() { return _pageHeight; };
  double pageWidth() { return _pageWidth; };
  double leftMargin() { return _leftMargin; };
  double rightMargin() { return _rightMargin; };

  // Check if file was successfully created.
  virtual GBool isOk() { return gTrue; }

  //---- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown() { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() { return gFalse; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() { return gFalse; }

  // Does this device need non-text content?
  virtual GBool needNonText() { return gFalse; }

  // Start a page.
  virtual void startPage(int pg, GfxState *state);

  // End a page.
  virtual void endPage();

  //----- update text state
  virtual void updateFont(GfxState *state);
  virtual void updateTextMat(GfxState *state) { /* printf("updateTextMat\n"); */ }
  virtual void updateCharSpace(GfxState *state) { /* printf("updateCharSpace(%f)\n",state->getCharSpace()); */ }
  virtual void updateRender(GfxState *state) { /* printf("updateRender\n"); */ } // found in some CA9 ops
  virtual void updateRise(GfxState *state) { /* printf("updateRise\n"); */ } // found in some CA9 ops
  virtual void updateWordSpace(GfxState *state) { /* printf("updateWordSpace(%f)\n",state->getWordSpace()); */ }
  virtual void updateHorizScaling(GfxState *state) { /* printf("updateHorizScaling\n"); */ } // found in some CA9 ops
  virtual void updateTextPos(GfxState *state) { /* printf("->(%f,%f)\n",state->getCurY(),state->getCurX()); */ }
  virtual void updateTextShift(GfxState *state, double shift) { /* printf("updateTextShift(%f)\n",shift / -100.0); */ }

  //----- text drawing
  virtual void beginString(GfxState *state, GooString *s);
  virtual void endString(GfxState *state);
  virtual void drawString(GfxState *state, GooString *s);
  virtual void drawChar(GfxState *state, double x, double y,
			double dx, double dy,
			double originX, double originY,
			CharCode code, int nBytes, Unicode *u, int uLen);
  
  virtual void drawImageMask(GfxState *state, Object *ref, 
			     Stream *str,
			     int width, int height, GBool invert,
			     GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			  int width, int height, GfxImageColorMap *colorMap,
			 int *maskColors, GBool inlineImg);
  virtual void stroke(GfxState * /*state*/);
  virtual void fill(GfxState * /*state*/);
  virtual void eoFill(GfxState * /*state*/);

private:
  GooList strings;		// list of all strings we get
  GooList underlines;		// track drawn lines during render - will merge with words during parse
  GooList words;
  GooList lines;		// grouped into lines
  GooList header;
  GooList body;
  GooList footnotes;
  GooList repeats;

  double _pageHeight, _pageWidth;

  int pageNum;
  int _pages;
  double _leftMargin;
  double _rightMargin;

  // stats to help us parse
  double tab_x;
  double linespace_y;

};

#endif
