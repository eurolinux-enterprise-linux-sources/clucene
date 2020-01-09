/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

  void assertAnalyzersTo(CuTest *tc,Analyzer* a, const TCHAR* input, TCHAR* output){
   Reader* reader = _CLNEW StringReader(input);
	TokenStream* ts = a->tokenStream(_T("dummy"), reader );

	int32_t i = 0;
	const TCHAR* pos = output;
	TCHAR buffer[80];
	const TCHAR* last = output;
	Token t;
	while( (pos = _tcsstr(pos+1, _T(";"))) != NULL ) {
		int32_t len = (int32_t)(pos-last);
		_tcsncpy(buffer,last,len);
		buffer[len]=0;

	  CLUCENE_ASSERT(ts->next(&t));
	  CLUCENE_ASSERT(_tcscmp( t.termText(),buffer) == 0 );
		
    last = pos+1;
  }
  CLUCENE_ASSERT(!ts->next(&t)); //Test failed, more fields than expected.
  
	 ts->close();
    _CLDELETE(reader);
    _CLDELETE(ts);
  }

  void testSimpleAnalyzer(CuTest *tc){
    Analyzer* a = _CLNEW SimpleAnalyzer();
	 assertAnalyzersTo(tc,a, _T("foo bar FOO BAR"), _T("foo;bar;foo;bar;") );
    assertAnalyzersTo(tc,a, _T("foo      bar .  FOO <> BAR"), _T("foo;bar;foo;bar;"));
    assertAnalyzersTo(tc,a, _T("foo.bar.FOO.BAR"), _T("foo;bar;foo;bar;"));
    assertAnalyzersTo(tc,a, _T("U.S.A."), _T("u;s;a;") );
    assertAnalyzersTo(tc,a, _T("C++"), _T("c;") );
    assertAnalyzersTo(tc,a, _T("B2B"), _T("b;b;"));
    assertAnalyzersTo(tc,a, _T("2B"), _T("b;"));
    assertAnalyzersTo(tc,a, _T("\"QUOTED\" word"), _T("quoted;word;"));
    
    _CLDELETE(a);
  }
  
   void testStandardAnalyzer(CuTest *tc){
    Analyzer* a = _CLNEW StandardAnalyzer();
    
    //todo: check this
	 assertAnalyzersTo(tc,a, _T("[050-070]"), _T("050;-070;") );
    
    _CLDELETE(a);
   }

   
   void testPerFieldAnalzyerWrapper(CuTest *tc){
        const TCHAR* text = _T("Qwerty");
        PerFieldAnalyzerWrapper analyzer(_CLNEW WhitespaceAnalyzer());

        analyzer.addAnalyzer(_T("special"), _CLNEW SimpleAnalyzer());

        StringReader reader(text);
        TokenStream* tokenStream = analyzer.tokenStream( _T("field"), &reader);
        Token token;

        CLUCENE_ASSERT( tokenStream->next(&token) );
        CuAssertStrEquals(tc,_T("token.termText()"), _T("Qwerty"),
                    token.termText());
        _CLDELETE(tokenStream);

        StringReader reader2(text);
        tokenStream = analyzer.tokenStream(_T("special"), &reader2);
        CLUCENE_ASSERT( tokenStream->next(&token) );
        CuAssertStrEquals(tc, _T("token.termText()"), _T("qwerty"),
                    token.termText());
        _CLDELETE(tokenStream);
   }

  void testNullAnalyzer(CuTest *tc){
    Analyzer* a = _CLNEW WhitespaceAnalyzer();
    assertAnalyzersTo(tc,a, _T("foo bar FOO BAR"), _T("foo;bar;FOO;BAR;"));
    assertAnalyzersTo(tc,a, _T("foo      bar .  FOO <> BAR"), _T("foo;bar;.;FOO;<>;BAR;"));
    assertAnalyzersTo(tc,a, _T("foo.bar.FOO.BAR"), _T("foo.bar.FOO.BAR;"));
    assertAnalyzersTo(tc,a, _T("U.S.A."), _T("U.S.A.;"));
    assertAnalyzersTo(tc,a, _T("C++"), _T("C++;"));
    assertAnalyzersTo(tc,a, _T("B2B"), _T("B2B;"));
    assertAnalyzersTo(tc,a, _T("2B"), _T("2B;"));
    assertAnalyzersTo(tc,a, _T("\"QUOTED\" word"), _T("\"QUOTED\";word;") );
    
    _CLDELETE(a);
  }

  void testStopAnalyzer(CuTest *tc){
    Analyzer* a = _CLNEW StopAnalyzer();
    assertAnalyzersTo(tc,a, _T("foo bar FOO BAR"), _T("foo;bar;foo;bar;"));
    assertAnalyzersTo(tc,a, _T("foo a bar such FOO THESE BAR"), _T("foo;bar;foo;bar;"));
    
    _CLDELETE(a);
  }

  void testISOLatin1AccentFilter(CuTest *tc){
	  TCHAR str[200];
	  _tcscpy(str, _T("Des mot cl\xe9s \xc0 LA CHA\xceNE \xc0 \xc1 \xc2 ") //Des mot cl�s � LA CHA�NE � � � 
						_T("\xc3 \xc4 \xc5 \xc6 \xc7 \xc8 \xc9 \xca \xcb \xcc \xcd \xce \xcf") //� � � � � � � � � � � � � 
						_T(" \xd0 \xd1 \xd2 \xd3 \xd4 \xd5 \xd6 \xd8 \xde \xd9 \xda \xdb") //� � � � � � � � � � � � � 
						_T(" \xdc \xdd \xe0 \xe1 \xe2 \xe3 \xe4 \xe5 \xe6 \xe7 \xe8 \xe9 ") //� � � � � � � � � � � 
						_T("\xea \xeb \xec \xed \xee \xef \xf0 \xf1 \xf2 \xf3 \xf4 \xf5 \xf6 ") //� � � � � � � � � � � � � 
						_T("\xf8 \xdf \xfe \xf9 \xfa \xfb \xfc \xfd \xff") //� � � � � � � � �
						_T("      ") ); //room for extra latin stuff
	#ifdef _UCS2
		int p = _tcslen(str)-6;
		str[p+1] = 0x152;// �
		str[p+3] = 0x153;// � 
		str[p+5] = 0x178;//� 
	#endif
	
	StringReader reader(str);
	WhitespaceTokenizer ws(&reader);
	ISOLatin1AccentFilter filter(&ws,false);
	Token token;

	
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("Des"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("mot"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("cles"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("A"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("LA"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("CHAINE"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("A"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("A"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("A"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("A"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("A"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("A"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("AE"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("C"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("E"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("E"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("E"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("E"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("I"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("I"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("I"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("I"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("D"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("N"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("O"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("O"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("O"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("O"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("O"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("O"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("TH"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("U"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("U"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("U"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("U"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("Y"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("a"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("a"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("a"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("a"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("a"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("a"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("ae"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("c"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("e"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("e"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("e"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("e"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("i"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("i"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("i"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("i"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("d"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("n"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("o"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("o"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("o"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("o"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("o"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("o"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("ss"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("th"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("u"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("u"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("u"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("u"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("y"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("y"), token.termText());
	
	#ifdef _UCS2
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("OE"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("oe"), token.termText());
	CLUCENE_ASSERT(filter.next(&token)); CuAssertStrEquals(tc, _T("Token compare"), _T("Y"), token.termText());
	#endif
	
	
	CLUCENE_ASSERT(filter.next(&token)==false);
  }


  
CuSuite *testanalyzers(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene Analyzers Test"));

    SUITE_ADD_TEST(suite, testISOLatin1AccentFilter);
    SUITE_ADD_TEST(suite, testStopAnalyzer);
    SUITE_ADD_TEST(suite, testNullAnalyzer);
    SUITE_ADD_TEST(suite, testSimpleAnalyzer);
    SUITE_ADD_TEST(suite, testPerFieldAnalzyerWrapper);
    return suite; 
}
// EOF
