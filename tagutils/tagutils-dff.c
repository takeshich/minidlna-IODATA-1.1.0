
//  tagutils-dff.c
//  
//
//  Created by inXtro, Inc on 13/10/3.
//
//


static int
_get_dfftags(char *file, struct song_metadata *psong)
{
	FILE *infile;
	
	char *buffer = NULL;
	char *ID = NULL, *DataSize = NULL, *Data=NULL;

	long Size = 0, Offset = 0;
	int HasID3Tag=0;

#if 1	// DSD Support	// Disable on Bata 6 For IODATA	// Paul Liao inXtron 20131230
		// DSD Support	// Enable on Bata 7 For IODATA	// Paul Liao inXtron 20140303
	char **m;
#endif
	
	//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"File : %s\n",file);

	// DFF TAG
 	buffer = (char *)malloc(sizeof(char)*256);
	memset(buffer , 0, sizeof(buffer));
	
	ID = (char *)malloc(sizeof(char)*4);	
	memset(ID , 0, sizeof(ID) );
	
	DataSize = (char *)malloc(sizeof(char)*8);	
	memset(DataSize , 0, sizeof(DataSize));

 	HasID3Tag = Get_DffID3_Info (file);

	//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"HasID3 : %d\n",HasID3Tag);
 
	if (!(infile = fopen(file, "rb")))
	{
		DPRINTF(E_ERROR, L_SCANNER, "Could not open %s for reading\n", file);
		return -1;
	}
	
	// Seek to Sample Rate Chunk
	
	fseek(infile, 0x30, SEEK_SET);	// 0x30	
	
	while( fread(ID, 1, 4, infile) == 4 ){
		memset(DataSize , 0, sizeof(DataSize) );
		memset(buffer , 0, sizeof(buffer) );
		
		
		//DEBUG DPRINTF(E_DEBUG,L_SCANNER,"Get ID is [%s]\n",ID);
		
		if (!strncmp(ID, "DIIN", 4) ){
			fread(DataSize, 1, 8, infile);
		}
		else if (!strncmp(ID, "DIAR", 4) ){		
			fread(DataSize, 1, 8, infile);
			Size = ((DataSize[4]& 0xff) << 24)|((DataSize[5]& 0xff) << 16)|((DataSize[6] & 0xff )<< 8)|(DataSize[7] & 0xff);
			fread(buffer, 1, Size, infile );
 			//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"ID %s\n",ID);
#if 0	// DSD Support	// Disable on Bata 6 For IODATA	// Paul Liao inXtron 20131230
			if (!HasID3Tag){	
				char *Artist = buffer+4;
 				m = &psong->contributor[ROLE_ARTIST];
				*m = malloc(strlen(Artist) + 1);
				strncpy(*m, Artist, strlen(Artist));
				(*m)[strlen(Artist)] = '\0';
			}
#endif
#if 1	// DSD Support	// Enable on Bata 7 For IODATA	// Paul Liao inXtron 20140303
			if (!HasID3Tag){
				char *Artist = buffer+4;
				char *utf8_text = NULL;
				char *iconv_buf;
				iconv_result rc;
				int u8textlen = 0;
				
				m = &psong->contributor[ROLE_ARTIST];
  				iconv_buf = (char*)calloc(MAX_ICONV_BUF, sizeof(char));
				rc = do_iconv( "UTF-8" , "CP932", Artist, strlen(Artist), iconv_buf, MAX_ICONV_BUF);
			
				if(rc == ICONV_OK)
				{	
					utf8_text = (char*)iconv_buf;
					u8textlen = strlen(utf8_text);
					//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "[%s]:(u8) [%s][%d]\n", ID, utf8_text, u8textlen);
					*m = malloc( u8textlen + 1 );
					strncpy(*m, utf8_text, u8textlen);
					(*m)[u8textlen] = '\0';
				}
				free(iconv_buf);	
			}
#endif
 		}
		else if (!strncmp(ID, "DITI", 4)){
			fread(DataSize, 1, 8, infile);
			Size = ((DataSize[4]& 0xff) << 24)|((DataSize[5]& 0xff) << 16)|((DataSize[6] & 0xff )<< 8)|(DataSize[7] & 0xff);
			fread(buffer, 1, Size, infile );
			//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"ID %s\n",ID); 			
#if 0	// DSD Support	// Disable on Bata 6 for IODATA	// Paul Liao inXtron 20131230 
			if (!HasID3Tag){
				char *Title = buffer+4;
				m = &psong->title;
				*m = malloc(strlen(Title) + 1);
				strncpy(*m, Title, strlen(Title));
				(*m)[strlen(Title)] = '\0';	
			}
#endif
#if 1	// DSD Support	// Enable on Bata 7 for IODATA    // Paul Liao inXtron 20140303
			if (!HasID3Tag){
				char *Title = buffer+4;
				char *utf8_text = NULL;
				char *iconv_buf;
				iconv_result rc;
				int u8textlen = 0;

				m = &psong->title;
				iconv_buf = (char*)calloc(MAX_ICONV_BUF, sizeof(char));
				rc = do_iconv( "UTF-8" , "CP932", Title, strlen(Title), iconv_buf, MAX_ICONV_BUF);

				if(rc == ICONV_OK)
				{
					utf8_text = (char*)iconv_buf;
					u8textlen = strlen(utf8_text);
					//DEBUG	DPRINTF(E_DEBUG, L_SCANNER, "[%s]:(u8) [%s][%d]\n", ID, utf8_text, u8textlen);
					*m = malloc( u8textlen + 1 );
					strncpy(*m, utf8_text, u8textlen);
					(*m)[u8textlen] = '\0';
				}
				free(iconv_buf);
			}
#endif
		}
		else if (!strncmp(ID, "FS  ", 4)){
			fread(DataSize, 1, 8, infile);
			Size = ((DataSize[4]& 0xff) << 24)|((DataSize[5]& 0xff) << 16)|((DataSize[6] & 0xff )<< 8)|(DataSize[7] & 0xff);
			fread(buffer, 1, Size, infile );
			// DEBUG DPRINTF(E_DEBUG,L_SCANNER,"ID %s = %d\n",ID, buffer[1]);
			psong->samplerate = ((buffer[0]& 0xff) << 24)|((buffer[1]& 0xff) << 16)|((buffer[2] & 0xff )<< 8)|(buffer[3] & 0xff);
		}
		else if (!strncmp(ID, "CHNL", 4)){
			fread(DataSize, 1, 8, infile);
			Size = ((DataSize[4]& 0xff) << 24)|((DataSize[5]& 0xff) << 16)|((DataSize[6] & 0xff )<< 8)|(DataSize[7] & 0xff);
			fread(buffer, 1, Size, infile );
			// DEBUG DPRINTF(E_DEBUG,L_SCANNER,"ID %s = %d\n",ID, buffer[1]);
			psong->channels =  buffer[1] & 0xFF;
		}
		else {
			fread(DataSize, 1, 8, infile);
			Size = ((DataSize[4]& 0xff) << 24)|((DataSize[5]& 0xff) << 16)|((DataSize[6] & 0xff )<< 8)|(DataSize[7] & 0xff);
			Offset = Size;
			//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"ID %s = %d\n",ID, Size);
			fseek(infile, Offset, SEEK_CUR);
		}
	}
	
	fclose(infile);
	// DEBUG DPRINTF(E_DEBUG,L_SCANNER,"fclose infile [%s]\n",file);
	free(buffer);
	free(ID);
	free(DataSize);
	free(Data);

//	ID3 TAG if it Has
 	if ( HasID3Tag )
 	{
		//	Read ID3 Tag
		struct id3_file *pid3file;
		struct id3_tag *pid3tag;
		struct id3_frame *pid3frame;
		int err;
		int index;
		int used;
		unsigned char *utf8_text;
		int genre = WINAMP_GENRE_UNKNOWN;
		int have_utf8;
		int have_text;
		id3_ucs4_t const *native_text;
		char *tmp;
		int got_numeric_genre;
		id3_byte_t const *image;
		id3_length_t image_size = 0;
	
		pid3file = id3_file_open("/tmp/dffid3.tag", ID3_FILE_MODE_READONLY);
		
		if(!pid3file)
		{
			DPRINTF(E_ERROR, L_SCANNER, "Cannot open /tmp/dffid3.tag\n");
			return -1;
		}
			
		pid3tag = id3_file_tag(pid3file);
		
		if(!pid3tag)
		{
			err = errno;
			id3_file_close(pid3file);
			errno = err;
			DPRINTF(E_WARN, L_SCANNER, "Cannot get ID3 tag for %s\n", file);
			return -1;
		}
		
		index = 0;
		while((pid3frame = id3_tag_findframe(pid3tag, "", index)))
		{
			used = 0;
			utf8_text = NULL;
			native_text = NULL;
			have_utf8 = 0;
			have_text = 0;

			if(!strcmp(pid3frame->id, "YTCP"))	 // for id3v2.2 
			{
				psong->compilation = 1;
				DPRINTF(E_DEBUG, L_SCANNER, "Compilation: %d [%s]\n", psong->compilation, basename(file));
			}
			else if(!strcmp(pid3frame->id, "APIC") && !image_size)
			{
				if( (strcmp((char*)id3_field_getlatin1(&pid3frame->fields[1]), "image/jpeg") == 0) ||
					(strcmp((char*)id3_field_getlatin1(&pid3frame->fields[1]), "image/jpg") == 0) ||
					(strcmp((char*)id3_field_getlatin1(&pid3frame->fields[1]), "jpeg") == 0) )
				{
					image = id3_field_getbinarydata(&pid3frame->fields[4], &image_size);
					if( image_size )
					{
						psong->image = malloc(image_size);
						memcpy(psong->image, image, image_size);
						psong->image_size = image_size;
						//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "Found thumbnail: %d\n", psong->image_size);
					}
				}
			}
				
			if(((pid3frame->id[0] == 'T') || (strcmp(pid3frame->id, "COMM") == 0)) &&
			   (id3_field_getnstrings(&pid3frame->fields[1])))
				have_text = 1;
				
			if(have_text)
			{
				native_text = id3_field_getstrings(&pid3frame->fields[1], 0);
				//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"native_text %s\n",(char*)native_text); 	
				
				if(native_text)
				{
					have_utf8 = 1;
					if(lang_index >= 0)
						utf8_text = _get_utf8_text(native_text); // through iconv
					else
						utf8_text = (unsigned char*)id3_ucs4_utf8duplicate(native_text);

					//DEBUG	DPRINTF(E_DEBUG, L_SCANNER, "%s \n",pid3frame->id);	
					if(!strcmp(pid3frame->id, "TIT2"))
					{
						used = 1;
						psong->title = (char*)utf8_text;
					}
					else if(!strcmp(pid3frame->id, "TPE1"))
					{
						used = 1;
						psong->contributor[ROLE_ARTIST] = (char*)utf8_text;
					}
					else if(!strcmp(pid3frame->id, "TALB"))
					{
						used = 1;
						psong->album = (char*)utf8_text;
					}
					else if(!strcmp(pid3frame->id, "TCOM"))
					{
						used = 1;
						psong->contributor[ROLE_COMPOSER] = (char*)utf8_text;
					}
					else if(!strcmp(pid3frame->id, "TIT1"))
					{
						used = 1;
						psong->grouping = (char*)utf8_text;
					}
					else if(!strcmp(pid3frame->id, "TPE2"))
					{
						used = 1;
						psong->contributor[ROLE_BAND] = (char*)utf8_text;				
					}
					else if(!strcmp(pid3frame->id, "TPE3"))
					{
						used = 1;
						psong->contributor[ROLE_CONDUCTOR] = (char*)utf8_text;
					}
					else if(!strcmp(pid3frame->id, "TCON"))
					{
						used = 1;
						psong->genre = (char*)utf8_text;
						got_numeric_genre = 0;
						if(psong->genre)
						{
							if(!strlen(psong->genre))
							{
								genre = WINAMP_GENRE_UNKNOWN;
								got_numeric_genre = 1;
							}
							else if(isdigit(psong->genre[0]))
							{
								genre = atoi(psong->genre);
								got_numeric_genre = 1;
							}
							else if((psong->genre[0] == '(') && (isdigit(psong->genre[1])))
							{
								genre = atoi((char*)&psong->genre[1]);
								got_numeric_genre = 1;
							}
							
							if(got_numeric_genre)
							{
								if((genre < 0) || (genre > WINAMP_GENRE_UNKNOWN))
									genre = WINAMP_GENRE_UNKNOWN;
								free(psong->genre);
								psong->genre = strdup(winamp_genre[genre]);
							}
						}
					}
	
					else if(!strcmp(pid3frame->id, "COMM"))
					{
						used = 1;
						psong->comment = (char*)utf8_text;
					}
					else if(!strcmp(pid3frame->id, "TPOS"))
					{
						tmp = (char*)utf8_text;
						strsep(&tmp, "/");
						if(tmp)
						{
							psong->total_discs = atoi(tmp);
						}
						psong->disc = atoi((char*)utf8_text);
					}
					else if(!strcmp(pid3frame->id, "TRCK"))
					{
						tmp = (char*)utf8_text;
						strsep(&tmp, "/");
						if(tmp)
						{
							psong->total_tracks = atoi(tmp);
						}
						psong->track = atoi((char*)utf8_text);
					}
					else if(!strcmp(pid3frame->id, "TDRC"))
					{
#if 1	// New Version 20131118			// 2013-08-05T22:40
						if (strrchr((char*)utf8_text, 'T') > 0)
							psong->time = atoi((char*)strrchr((char*)utf8_text, 'T')+1)*100 + atoi((char*)strrchr((char*)utf8_text, ':')+1);
						if ((char*)strchr((char*)utf8_text, '-') > 0)
							psong->date = atoi((char*)strchr((char*)utf8_text, '-')+1)*100 + atoi((char*)strrchr((char*)utf8_text, '-')+1);
						psong->year = atoi((char*)utf8_text);

						//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "TDRC => %04d,%04d,%04d\n", psong->year, psong->date, psong->time);
#else	// Old Version	
						psong->year = atoi((char*)utf8_text);
#endif					
					}
					else if(!strcmp(pid3frame->id, "TLEN"))
					{
						psong->song_length = atoi((char*)utf8_text);
					}
					else if(!strcmp(pid3frame->id, "TBPM"))
					{
						psong->bpm = atoi((char*)utf8_text);
					}
					else if(!strcmp(pid3frame->id, "TCMP"))
					{
						psong->compilation = (char)atoi((char*)utf8_text);
					}
				}
			}
				
			// check if text tag
			if((!used) && (have_utf8) && (utf8_text))
				free(utf8_text);
				
			// v2 COMM
			if((!strcmp(pid3frame->id, "COMM")) && (pid3frame->nfields == 4))
			{
				native_text = id3_field_getstring(&pid3frame->fields[2]);
				if(native_text)
				{
					utf8_text = (unsigned char*)id3_ucs4_utf8duplicate(native_text);
					if((utf8_text) && (strncasecmp((char*)utf8_text, "iTun", 4) != 0))
					{
						// read comment
						free(utf8_text);
						
						native_text = id3_field_getfullstring(&pid3frame->fields[3]);
						if(native_text)
						{
							utf8_text = (unsigned char*)id3_ucs4_utf8duplicate(native_text);
							if(utf8_text)
							{
								free(psong->comment);
								psong->comment = (char*)utf8_text;
							}
						}
					}
					else
					{
						free(utf8_text);
					}
				}
			}
			
			index++;
		}
	
		id3_file_close(pid3file);
	
		remove("/tmp/dffid3.tag");

	}

    return 0;
}


static int _get_dfffileinfo(char *file, struct song_metadata *psong)
{
	FILE *infile;

	char *buffer = NULL;

	long FileSize=0;
	long SongSize = 0;
	long SongLength = 0 ;
	long SampleRate = 0;
	int Channels = 0 ;

	buffer = (char *)malloc(sizeof(char)*16);
	memset(buffer , 0, sizeof(buffer));

	//DEBUG	DPRINTF(E_DEBUG, L_SCANNER, "open %s for reading\n", file);

	if (!(infile = fopen(file, "rb")))
	{
		DPRINTF(E_ERROR, L_SCANNER, "Could not open %s for reading\n", file);
		return -1;
	}


	// File Size
	fseek(infile, 0, SEEK_END);
	FileSize = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	// Simple Rate	
	
	fseek(infile, 0x30, SEEK_SET);	// 0x30
	fread(buffer, 1, 16, infile);
	SampleRate = ((buffer[12]& 0xff) << 24)|((buffer[13]& 0xff) << 16)|((buffer[14] & 0xff )<< 8)|(buffer[15] & 0xff);

        // Channels
        fread(buffer, 1, 16, infile);
        Channels = buffer[13] & 0xff;

	SongSize = FileSize;
	SongLength = SongSize / ( (SampleRate * Channels / 8 ) );
	// SongLength = SongSize / (Bitrate / 8); Bitrate = SampleRate * Channelsl;
	
	// COMMENTS

	psong->file_size = FileSize;
	psong->song_length = SongLength * 1000;

	fclose(infile);

//	xasprintf(&(psong->dlna_pn), "DSD");	// Disable on Bata VI 20131230

	return 0;
}


int Get_DffID3_Info(char *file)
{	
	
	FILE *infile;
	FILE *outfile;
	
	char *buffer = NULL;
	char *ID = NULL;
	char *DataSize = NULL;

	long Size = 0;
	long Offset = 0;
	long tagSize = 0;
	int ReadSize = 0;	
	int ret = 0;

 	buffer = (char *)malloc(sizeof(char)*1024);
	memset(buffer , 0, sizeof(buffer) );
	
	ID = (char *)malloc(sizeof(char)*5);	
	memset(ID , 0, sizeof(ID) );
	
	DataSize = (char *)malloc(sizeof(char)*8);	
	memset(DataSize , 0, sizeof(DataSize) );

	//DEBUG	DPRINTF(E_DEBUG, L_SCANNER, "Get ID3 Tag Info : [%s]\n",file);

	if (!(infile = fopen(file, "rb")))
	{
		DPRINTF(E_ERROR, L_SCANNER, "Could not open %s for reading\n", file);
		return -1;
	}
	
	// Seek to Sample Rate Chunk
	
	fseek(infile, 0x30, SEEK_SET);	// 0x30	
	
	while(fread(ID, 1, 4, infile) == 4 ){
		//DEBUG	DPRINTF(E_DEBUG, L_SCANNER, "TAG ID [%s]\n",ID);
		memset(DataSize , 0, sizeof(DataSize));
		memset(buffer , 0, sizeof(buffer));
		
		if (!strncmp(ID, "ID3 ", 4)) {
			//DEBUG	DPRINTF(E_DEBUG, L_SCANNER, "File ID[%s]\n", ID);
			fread(DataSize, 1, 8, infile);
			Size = ((DataSize[4]& 0xff) << 24)|((DataSize[5]& 0xff) << 16)|((DataSize[6] & 0xff )<< 8)|(DataSize[7] & 0xff);

			outfile = fopen("/tmp/dffid3.tag", "wb");

			//Dump out to /tmp/dffid3.tag
			tagSize = Size;
					
			while (tagSize > 0 ){
				ReadSize = 0;
				ReadSize = fread(buffer, 1, 1024, infile);
				fwrite(buffer, 1, 1024, outfile);
				tagSize -= ReadSize;
			}
			ret = 1;
			fclose(outfile);
			// DEBUG DPRINTF(E_DEBUG, L_SCANNER, "fclose outfile /tmp/dffid3.tag\n");
			break;
		}
		else {
			fread(DataSize, 1, 8, infile);
			Size = ((DataSize[4]& 0xff) << 24)|((DataSize[5]& 0xff) << 16)|((DataSize[6] & 0xff )<< 8)|(DataSize[7] & 0xff);
			 //DEBUG DPRINTF(E_DEBUG, L_SCANNER, "Size [%ld]\n",Size);	
			Offset = Size;
			fseek(infile, Offset, SEEK_CUR);
		}
		memset(ID , 0, sizeof(ID) );
	}
	
	fclose(infile);
	
	free(buffer);
	free(ID);
	free(DataSize);

	return ret;
}


