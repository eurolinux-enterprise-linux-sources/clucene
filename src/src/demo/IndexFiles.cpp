/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "stdafx.h"

#include "CLucene.h"
#include "CLucene/util/Reader.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/dirent.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;

Document* FileDocument(const char* f){
	// make a new, empty document
	Document* doc = _CLNEW Document();

	// Add the path of the file as a field named "path".  Use a Tex t field, so
	// that the index stores the path, and so that the path is searchable
   TCHAR tf[CL_MAX_DIR];
   STRCPY_AtoT(tf,f,CL_MAX_DIR);
   doc->add( *_CLNEW Field(_T("path"), tf, Field::STORE_YES | Field::INDEX_UNTOKENIZED ) );

	// Add the last modified date of the file a field named "modified".  Use a
	// Keyword field, so that it's searchable, but so that no attempt is made
	// to tokenize the field into words.
	//doc->add( *Field.Keyword("modified", DateField.timeToString(f->lastModified())));

	// Add the contents of the file a field named "contents".  Use a Text
	// field, specifying a Reader, so that the text of the file is tokenized.

    //read the data without any encoding. if you want to use special encoding
    //see the contrib/jstreams - they contain various types of stream readers
    FILE* fh = fopen(f,"r");
	if ( fh != NULL ){
		StringBuffer str;
		// use fstat for portability
		int fn = fileno(fh);
		struct stat filestat;
		fstat(fn, &filestat);
		str.reserve(filestat.st_size);
		//str.reserve(fileSize(fh->_file));
		char abuf[1024];
		TCHAR tbuf[1024];
		size_t r;
		do{
			r = fread(abuf,1,1023,fh);
			abuf[r]=0;
			STRCPY_AtoT(tbuf,abuf,r);
			tbuf[r]=0;
			str.append(tbuf);
		}while(r>0);
		fclose(fh);

		doc->add( *_CLNEW Field(_T("contents"),str.getBuffer(), Field::STORE_YES | Field::INDEX_TOKENIZED) );
	}

	//_tprintf(_T("%s\n"),doc->toString());
	// return the document
	return doc;
}

void indexDocs(IndexWriter* writer, char* directory) {
	DIR* dir = opendir(directory);
	if ( dir != NULL ){
		struct dirent* fl;
		
		struct fileStat buf;

		char path[CL_MAX_DIR];
		strcpy(path,directory);
		strcat(path,PATH_DELIMITERA);
		char* pathP = path + strlen(path);

		fl = readdir(dir);
		while ( fl != NULL ){
			if ( (strcmp(fl->d_name, ".")) && (strcmp(fl->d_name, "..")) ) {
			pathP[0]=0;
			strcat(pathP,fl->d_name);
			int32_t ret = fileStat(path,&buf);
			if ( buf.st_mode & S_IFDIR ) {
				indexDocs(writer, path );
			}else{
				printf( "adding: %s\n", fl->d_name );

				Document* doc = FileDocument( path );
				writer->addDocument( doc );
				_CLDELETE(doc);
			}
		}
		fl = readdir(dir);

		}
		closedir(dir);
	}else{
		    printf( "adding: %s\n", directory);

		    Document* doc = FileDocument( directory );
		    writer->addDocument( doc );
		    _CLDELETE(doc);
	}
}
void IndexFiles(char* path, char* target, const bool clearIndex){
	IndexWriter* writer = NULL;
	//lucene::analysis::SimpleAnalyzer* an = *_CLNEW lucene::analysis::SimpleAnalyzer();
	lucene::analysis::standard::StandardAnalyzer an;
	
	if ( !clearIndex && IndexReader::indexExists(target) ){
		if ( IndexReader::isLocked(target) ){
			printf("Index was locked... unlocking it.\n");
			IndexReader::unlock(target);
		}

		writer = _CLNEW IndexWriter( target, &an, false);
	}else{
		writer = _CLNEW IndexWriter( target ,&an, true);
	}
	writer->setMaxFieldLength(IndexWriter::DEFAULT_MAX_FIELD_LENGTH);
	/*printf("Set MaxFieldLength: ");
	char mfl[250];
	fgets(mfl,250,stdin);
	mfl[strlen(mfl)-1] = 0;
	if ( mfl[0] != 0 )
		writer->setMaxFieldLength(atoi(mfl));*/
	//writer->infoStream = cout; //TODO: infoStream - unicode

	uint64_t str = lucene::util::Misc::currentTimeMillis();

	indexDocs(writer, path);
	writer->optimize();
	writer->close();
	_CLDELETE(writer);

	printf("Indexing took: %d ms.\n\n", lucene::util::Misc::currentTimeMillis() - str);
}
