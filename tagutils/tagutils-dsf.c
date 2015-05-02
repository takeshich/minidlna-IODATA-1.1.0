//
//  tagutils-dsf.c
//  
//
//  Created by inXtron,Inc  on 13/10/3.
//
//


static int
_get_dsftags(char *file, struct song_metadata *psong)
{

	FILE *infile; 
	FILE *outfile;
    
	char *buffer = NULL;
    
	unsigned long long id3TagPoint = 0;

	buffer = (char *)malloc(sizeof(char)*1024);
	memset(buffer , 0, sizeof(buffer));

	//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"File : %s\n",file);
	if(!(infile = fopen(file, "rb")))
	{
		DPRINTF(E_DEBUG, L_SCANNER, "Could not open %s for reading\n", file);
		return -1;
	}

	outfile = fopen("/tmp/id3.tag", "wb");
	
	// Find the ID3 Tag Position of File
	fseek(infile, 20  , SEEK_SET);
	fread(buffer, 1, 8, infile );

	id3TagPoint = (buffer[0] & 0xffULL) | (buffer[1] & 0xffULL) << 8ULL | (buffer[2] & 0xffULL) << 16ULL | (buffer[3] & 0xffULL) << 24ULL| (buffer[4] & 0xffULL) << 32ULL | (buffer[5] & 0xffULL) << 40ULL |(buffer[6] & 0xffULL) << 48ULL | (buffer[7] & 0xffULL) << 56ULL ;

	//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"id3TagPoint : %lld\n",id3TagPoint);
	//DEBUG	DPRINTF(E_DEBUG,L_SCANNER,"id3TagPoint : 0x%08x\n",id3TagPoint);

	// Jump to the Position of ID3 Tag & read
	if (id3TagPoint > 0) {
		fseeko(infile, id3TagPoint  , SEEK_SET);
		memset(buffer , 0, sizeof(buffer));
		//Dump out to /tmp/id3.tag
    
		while (fread(buffer, 1, 1024, infile ) !=0 ){
			fwrite(buffer, 1, 1024, outfile);
		}
	}

	fclose(outfile);
	fclose(infile);
    
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
    
	pid3file = id3_file_open("/tmp/id3.tag", ID3_FILE_MODE_READONLY);

	if(!pid3file)
	{
		DPRINTF(E_ERROR, L_SCANNER, "Cannot open %s\n", file);
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
        
		if(!strcmp(pid3frame->id, "YTCP"))   /* for id3v2.2 */
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

			//DEBUG DPRINTF(E_DEBUG,L_SCANNER,"native_text %s\n",(char*)native_text); 
			if(native_text)
			{
				have_utf8 = 1;
				if(lang_index >= 0)
					utf8_text = _get_utf8_text(native_text); // through iconv
				else
					utf8_text = (unsigned char*)id3_ucs4_utf8duplicate(native_text);


				//DEBUG	DPRINTF(E_DEBUG, L_SCANNER, "%s %s\n",pid3frame->id, (char*)utf8_text);
				
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
#if 1	// New Version 20131209		// 2013-08-05T22:40
					if (strrchr((char*)utf8_text, 'T') > 0)
						psong->time = atoi((char*)strrchr((char*)utf8_text, 'T')+1)*100 + atoi((char*)strrchr((char*)utf8_text, ':')+1);
					if ((char*)strchr((char*)utf8_text, '-') > 0)
						psong->date = atoi((char*)strchr((char*)utf8_text, '-')+1)*100 + atoi((char*)strrchr((char*)utf8_text, '-')+1);
					psong->year = atoi((char*)utf8_text);
//					DPRINTF(E_DEBUG, L_SCANNER, "TDRC => %04d,%04d,%04d\n", psong->year, psong->date, psong->time);
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

	free(buffer);
    
	id3_file_close(pid3file);
    
	remove("/tmp/id3.tag");
    
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "Got id3 tag successfully for file=%s\n", file);

    return 0;
	
}


static int _get_dsffileinfo(char *file, struct song_metadata *psong)
{
	FILE * infile;

	char *buffer = NULL;

	unsigned long long FileSize=0;
//	int FileSize_H=0;
//	int ChannelType = 0;
	int ChannelNum = 0;
	unsigned long long SongSize = 0;
//	int SongSize_H = 0;
	long length = 0 ;
	long SampleFreq = 0;
	
    buffer = (char *)malloc(sizeof(char)*1024);
	memset(buffer , 0, sizeof(buffer));

//	if (strstr(file,"._*")>=0) return -1;
	
	if (!(infile = fopen(file, "rb")))
	{
		DPRINTF(E_ERROR, L_SCANNER, "Could not open %s for reading\n", file);
		return -1;
	}
	
	fread(buffer, 1, 28, infile);
	FileSize = (buffer[12]&0xffULL)|((buffer[13]&0xffULL)<< 8ULL )|((buffer[14]&0xffULL)<< 16ULL)|((buffer[15]&0xffULL)<<24ULL)|(buffer[16]&0xffULL)<<32ULL|((buffer[17]&0xffULL)<< 40ULL )|((buffer[18]&0xffULL)<< 48ULL)|((buffer[19]&0xffULL)<<56ULL);
//	FileSize_H = (buffer[16]&0xff)|((buffer[17]&0xff)<< 8 )|((buffer[18]&0xff)<< 16)|((buffer[19]&0xff)<<24);

	fseek(infile, 28, SEEK_SET);
	// Read FMT Chunk
	fread(buffer, 1, 36, infile);

//	ChannelType = (buffer[20]&0xff);
	ChannelNum  = (buffer[24]&0xff);
	SampleFreq  = (buffer[28]&0xff)|((buffer[29]&0xff)<< 8 )|((buffer[30]&0xff)<< 16)|((buffer[31]&0xff)<<24);

	fseek(infile, 60, SEEK_SET);

	fread(buffer, 1, 12, infile);
	SongSize = (buffer[4]&0xffULL)|((buffer[5]&0xffULL)<< 8ULL )|((buffer[6]&0xffULL)<< 16ULL)|((buffer[7]&0xffULL)<< 24ULL)|(buffer[8]&0xffULL)<< 32ULL|((buffer[9]&0xffULL)<< 40ULL )|((buffer[10]&0xffULL)<< 48ULL)|((buffer[11]&0xffULL)<<56ULL);
//	SongSize_H = (buffer[8]&0xff)|((buffer[9]&0xff)<< 8 )|((buffer[10]&0xff)<< 16)|((buffer[11]&0xff)<<24);

	length = SongSize / SampleFreq;

//	DPRINTF(E_DEBUG, L_SCANNER, "FileSize %d\n", FileSize);
//	DPRINTF(E_DEBUG, L_SCANNER, "ChannelType %d\n", ChannelType);
//	DPRINTF(E_DEBUG, L_SCANNER, "ChannelNum %d\n", ChannelNum);
//	DPRINTF(E_DEBUG, L_SCANNER, "SampleFreq %d\n", SampleFreq);
//	DPRINTF(E_DEBUG, L_SCANNER, "SongSize %d\n", SongSize);
//	DPRINTF(E_DEBUG, L_SCANNER, "length %d\n", length);

	psong->file_size = FileSize;
	psong->channels = ChannelNum;
	psong->samplerate = SampleFreq ;
//	psong->bitrate = SampleFreq ;
	psong->song_length= length * 1000 ;

//	DPRINTF(E_DEBUG, L_SCANNER, "FileSize %d\n", psong->file_size);
//	DPRINTF(E_DEBUG, L_SCANNER, "ChannelNum %d\n", psong->channels);
//	DPRINTF(E_DEBUG, L_SCANNER, "SampleFreq %d\n", psong->samplerate);
//	DPRINTF(E_DEBUG, L_SCANNER, "length %d\n", psong->song_length);
	
	fclose(infile);
	free(buffer);

//	xasprintf(&(psong->dlna_pn), "DSD");	// Disable on Beta VI 20131230

	return 0;
}




