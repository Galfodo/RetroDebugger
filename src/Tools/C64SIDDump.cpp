#include "CDebugInterfaceVice.h"
#include "C64SIDDump.h"

// this dumps CSidData history in siddump format, note this is not async, mutex must be locked elsewhere
// the code below is heavily based on siddump print to file code by Cadaver
// https://github.com/cadaver/siddump/blob/master/siddump.c

typedef struct
{
  unsigned short freq;
  unsigned short pulse;
  unsigned short adsr;
  unsigned char wave;
  int note;
} CHANNEL;

typedef struct
{
  unsigned short cutoff;
  unsigned char ctrl;
  unsigned char type;
} FILTER;

// int basefreq = 0;
// int basenote = 0xb0;
// int timeseconds = 0;	// show time as seconds or frames
// int oldnotefactor = 1;
// int pattspacing = 0;
// int spacing = 0;
// int lowres = 0;

void C64SIDHistoryToByteBuffer(std::list<CSidData *> *sidDataHistory, CByteBuffer *byteBuffer, u8 format,
		int basefreq, int basenote, int spacing, int oldnotefactor, int pattspacing, int timeseconds, int lowres)
{
	CHANNEL chn[3];
	CHANNEL prevchn[3];
	CHANNEL prevchn2[3];
	FILTER filt;
	FILTER prevfilt;

	const char *notename[] =
	 {"C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
	  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
	  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
	  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
	  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
	  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
	  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
	  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7"};

	const char *filtername[] =
	 {"Off", "Low", "Bnd", "L+B", "Hi ", "L+H", "B+H", "LBH"};

	unsigned char freqtbllo[] = {
	  0x17,0x27,0x39,0x4b,0x5f,0x74,0x8a,0xa1,0xba,0xd4,0xf0,0x0e,
	  0x2d,0x4e,0x71,0x96,0xbe,0xe8,0x14,0x43,0x74,0xa9,0xe1,0x1c,
	  0x5a,0x9c,0xe2,0x2d,0x7c,0xcf,0x28,0x85,0xe8,0x52,0xc1,0x37,
	  0xb4,0x39,0xc5,0x5a,0xf7,0x9e,0x4f,0x0a,0xd1,0xa3,0x82,0x6e,
	  0x68,0x71,0x8a,0xb3,0xee,0x3c,0x9e,0x15,0xa2,0x46,0x04,0xdc,
	  0xd0,0xe2,0x14,0x67,0xdd,0x79,0x3c,0x29,0x44,0x8d,0x08,0xb8,
	  0xa1,0xc5,0x28,0xcd,0xba,0xf1,0x78,0x53,0x87,0x1a,0x10,0x71,
	  0x42,0x89,0x4f,0x9b,0x74,0xe2,0xf0,0xa6,0x0e,0x33,0x20,0xff};

	unsigned char freqtblhi[] = {
	  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,
	  0x02,0x02,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x03,0x04,
	  0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,0x08,
	  0x08,0x09,0x09,0x0a,0x0a,0x0b,0x0c,0x0d,0x0d,0x0e,0x0f,0x10,
	  0x11,0x12,0x13,0x14,0x15,0x17,0x18,0x1a,0x1b,0x1d,0x1f,0x20,
	  0x22,0x24,0x27,0x29,0x2b,0x2e,0x31,0x34,0x37,0x3a,0x3e,0x41,
	  0x45,0x49,0x4e,0x52,0x57,0x5c,0x62,0x68,0x6e,0x75,0x7c,0x83,
	  0x8b,0x93,0x9c,0xa5,0xaf,0xb9,0xc4,0xd0,0xdd,0xea,0xf8,0xff};
	
	if (format == SID_HISTORY_FORMAT_SIDDUMP)
	{
		byteBuffer->printf("Middle C frequency is $%04X\n\n", freqtbllo[48] | (freqtblhi[48] << 8));
		byteBuffer->printf("| Frame | Freq Note/Abs WF ADSR Pul | Freq Note/Abs WF ADSR Pul | Freq Note/Abs WF ADSR Pul | FCut RC Typ V |");
		//not supported (yet): if (profiling)
		//{ // CPU cycles, Raster lines, Raster lines with badlines on every 8th line, first line included
		//	printf(" Cycl RL RB |");
		//}
		byteBuffer->printf("\n");
		byteBuffer->printf("+-------+---------------------------+---------------------------+---------------------------+---------------+");
	//	 if (profiling)
	//	 {
	//	   printf("------------+");
	//	 }
		byteBuffer->printf("\n");
	}
	else if (format == SID_HISTORY_FORMAT_CSV)
	{
		byteBuffer->printf("Frame, Freq, Note/Abs, WF, ADSR, Pul, Freq, Note/Abs, WF, ADSR, Pul, Freq, Note/Abs, WF, ADSR, Pul, FCut, RC, Typ, V\n");
	}

	//
	int c;

	// configuration
	int frames = 0;
	int firstframe = 0;
	int counter = 0;
	int rows = 0;
	int sidNum = 0;

	// Recalibrate frequencytable
	if (basefreq)
	{
		basenote &= 0x7f;
		if ((basenote < 0) || (basenote > 96))
		{
			LOGError("Warning: Calibration note out of range. Aborting recalibration.\n");
		}
		else
		{
			for (c = 0; c < 96; c++)
			{
				double note = c - basenote;
				double freq = (double)basefreq * pow(2.0, note/12.0);
				int f = freq;
				if (freq > 0xffff) freq = 0xffff;
				freqtbllo[c] = f & 0xff;
				freqtblhi[c] = f >> 8;
			}
		}
	}
	
	// Check other parameters for correctness
	if ((lowres) && (!spacing)) lowres = 0;

	
	for (std::list<CSidData *>::reverse_iterator it = sidDataHistory->rbegin(); it != sidDataHistory->rend(); it++)
	{
		// Get SID parameters from each channel and the filter
		CSidData *sidData = *it;
		u8 *sidRegs =  sidData->sidRegs[sidNum];
		for (c = 0; c < 3; c++)
		{
			chn[c].freq = sidRegs[0x00 + 7*c] | (sidRegs[0x01 + 7*c] << 8);
			chn[c].pulse = (sidRegs[0x02 + 7*c] | (sidRegs[0x03 + 7*c] << 8)) & 0xfff;
			chn[c].wave = sidRegs[0x04 + 7*c];
			chn[c].adsr = sidRegs[0x06 + 7*c] | (sidRegs[0x05 + 7*c] << 8);
		}
		filt.cutoff = (sidRegs[0x15] << 5) | (sidRegs[0x16] << 8);
		filt.ctrl = sidRegs[0x17];
		filt.type = sidRegs[0x18];
		
		// Frame display
		if (frames >= firstframe)
		{
			char output[512];
			int time = frames - firstframe;
			output[0] = 0;
			
			if (format == SID_HISTORY_FORMAT_SIDDUMP)
			{
				if (!timeseconds)
					sprintf(&output[strlen(output)], "| %5d | ", time);
				else
					sprintf(&output[strlen(output)], "|%01d:%02d.%02d| ", time/3000, (time/50)%60, time%50);
			}
			else
			{
				if (!timeseconds)
					sprintf(&output[strlen(output)], "%5d, ", time);
				else
					sprintf(&output[strlen(output)], "%01d:%02d.%02d, ", time/3000, (time/50)%60, time%50);
			}
			
			// Loop for each channel
			for (c = 0; c < 3; c++)
			{
				int newnote = 0;
				
				// Keyoff-keyon sequence detection
				if (chn[c].wave >= 0x10)
				{
					if ((chn[c].wave & 1) && ((!(prevchn2[c].wave & 1)) || (prevchn2[c].wave < 0x10)))
						prevchn[c].note = -1;
				}
				
				// Frequency
				if ((frames == firstframe) || (prevchn[c].note == -1) || (chn[c].freq != prevchn[c].freq))
				{
					int d;
					int dist = 0x7fffffff;
					int delta = ((int)chn[c].freq) - ((int)prevchn2[c].freq);
					
					if (format == SID_HISTORY_FORMAT_SIDDUMP)
					{
						sprintf(&output[strlen(output)], "%04X ", chn[c].freq);
					}
					else
					{
						sprintf(&output[strlen(output)], "%04X, ", chn[c].freq);
					}
					
					if (chn[c].wave >= 0x10)
					{
						// Get new note number
						for (d = 0; d < 96; d++)
						{
							int cmpfreq = freqtbllo[d] | (freqtblhi[d] << 8);
							int freq = chn[c].freq;
							
							if (abs(freq - cmpfreq) < dist)
							{
								dist = abs(freq - cmpfreq);
								// Favor the old note
								if (d == prevchn[c].note) dist /= oldnotefactor;
								chn[c].note = d;
							}
						}
						
						// Print new note
						if (chn[c].note != prevchn[c].note)
						{
							if (format == SID_HISTORY_FORMAT_SIDDUMP)
							{
								if (prevchn[c].note == -1)
								{
									if (lowres) newnote = 1;
									sprintf(&output[strlen(output)], " %s %02X  ", notename[chn[c].note], chn[c].note | 0x80);
								}
								else
									sprintf(&output[strlen(output)], "(%s %02X) ", notename[chn[c].note], chn[c].note | 0x80);
							}
							else if (format == SID_HISTORY_FORMAT_CSV)
							{
								if (prevchn[c].note == -1)
								{
									if (lowres) newnote = 1;
									sprintf(&output[strlen(output)], " %s %02X,  ", notename[chn[c].note], chn[c].note | 0x80);
								}
								else
									sprintf(&output[strlen(output)], "(%s %02X), ", notename[chn[c].note], chn[c].note | 0x80);
							}
						}
						else
						{
							if (format == SID_HISTORY_FORMAT_SIDDUMP)
							{
								// If same note, print frequency change (slide/vibrato)
								if (delta)
								{
									if (delta > 0)
										sprintf(&output[strlen(output)], "(+ %04X) ", delta);
									else
										sprintf(&output[strlen(output)], "(- %04X) ", -delta);
								}
								else sprintf(&output[strlen(output)], " ... ..  ");
							}
							else if (format == SID_HISTORY_FORMAT_CSV)
							{
								// If same note, print frequency change (slide/vibrato)
								if (delta)
								{
									if (delta > 0)
										sprintf(&output[strlen(output)], "(+ %04X), ", delta);
									else
										sprintf(&output[strlen(output)], "(- %04X), ", -delta);
								}
								else sprintf(&output[strlen(output)], ",  ");

							}
						}
					}
					else
					{
						if (format == SID_HISTORY_FORMAT_SIDDUMP)
						{
							sprintf(&output[strlen(output)], " ... ..  ");
						}
						else if (format == SID_HISTORY_FORMAT_CSV)
						{
							sprintf(&output[strlen(output)], ", ");
						}
					}
				}
				else
				{
					if (format == SID_HISTORY_FORMAT_SIDDUMP)
					{
						sprintf(&output[strlen(output)], "....  ... ..  ");
					}
					else
					{
						sprintf(&output[strlen(output)], ", , ");
					}
				}
				
				// Waveform
				if (format == SID_HISTORY_FORMAT_SIDDUMP)
				{
					if ((frames == firstframe) || (newnote) || (chn[c].wave != prevchn[c].wave))
						sprintf(&output[strlen(output)], "%02X ", chn[c].wave);
					else sprintf(&output[strlen(output)], ".. ");
					
					// ADSR
					if ((frames == firstframe) || (newnote) || (chn[c].adsr != prevchn[c].adsr)) sprintf(&output[strlen(output)], "%04X ", chn[c].adsr);
					else sprintf(&output[strlen(output)], ".... ");
					
					// Pulse
					if ((frames == firstframe) || (newnote) || (chn[c].pulse != prevchn[c].pulse)) sprintf(&output[strlen(output)], "%03X ", chn[c].pulse);
					else sprintf(&output[strlen(output)], "... ");
					
					sprintf(&output[strlen(output)], "| ");
				}
				else if (format == SID_HISTORY_FORMAT_CSV)
				{
					if ((frames == firstframe) || (newnote) || (chn[c].wave != prevchn[c].wave))
						sprintf(&output[strlen(output)], "%02X, ", chn[c].wave);
					else sprintf(&output[strlen(output)], ", ");
					
					// ADSR
					if ((frames == firstframe) || (newnote) || (chn[c].adsr != prevchn[c].adsr)) sprintf(&output[strlen(output)], "%04X, ", chn[c].adsr);
					else sprintf(&output[strlen(output)], ", ");
					
					// Pulse
					if ((frames == firstframe) || (newnote) || (chn[c].pulse != prevchn[c].pulse)) sprintf(&output[strlen(output)], "%03X, ", chn[c].pulse);
					else sprintf(&output[strlen(output)], ", ");
				}
			}
			
			if (format == SID_HISTORY_FORMAT_SIDDUMP)
			{
				// Filter cutoff
				if ((frames == firstframe) || (filt.cutoff != prevfilt.cutoff)) sprintf(&output[strlen(output)], "%04X ", filt.cutoff);
				else sprintf(&output[strlen(output)], ".... ");
				
				// Filter control
				if ((frames == firstframe) || (filt.ctrl != prevfilt.ctrl))
					sprintf(&output[strlen(output)], "%02X ", filt.ctrl);
				else sprintf(&output[strlen(output)], ".. ");
				
				// Filter passband
				if ((frames == firstframe) || ((filt.type & 0x70) != (prevfilt.type & 0x70)))
					sprintf(&output[strlen(output)], "%s ", filtername[(filt.type >> 4) & 0x7]);
				else sprintf(&output[strlen(output)], "... ");
				
				// Mastervolume
				if ((frames == firstframe) || ((filt.type & 0xf) != (prevfilt.type & 0xf))) sprintf(&output[strlen(output)], "%01X ", filt.type & 0xf);
				else sprintf(&output[strlen(output)], ". ");
				
				//			  // Rasterlines / cycle count
				//			  if (profiling)
				//			  {
				//				int cycles = cpucycles;
				//				int rasterlines = (cycles + 62) / 63;
				//				int badlines = ((cycles + 503) / 504);
				//				int rasterlinesbad = (badlines * 40 + cycles + 62) / 63;
				//				sprintf(&output[strlen(output)], "| %4d %02X %02X ", cycles, rasterlines, rasterlinesbad);
				//			  }
				
				// End of frame display, print info so far and copy SID registers to old registers
				sprintf(&output[strlen(output)], "|\n");
				if ((!lowres) || (!((frames - firstframe) % spacing)))
				{
					byteBuffer->printf("%s", output);
					for (c = 0; c < 3; c++)
					{
						prevchn[c] = chn[c];
					}
					prevfilt = filt;
				}
				for (c = 0; c < 3; c++) prevchn2[c] = chn[c];
				
				// Print note/pattern separators
				if (spacing)
				{
					counter++;
					if (counter >= spacing)
					{
						counter = 0;
						if (pattspacing)
						{
							rows++;
							if (rows >= pattspacing)
							{
								rows = 0;
								byteBuffer->printf("+=======+===========================+===========================+===========================+===============+\n");
							}
							else
								if (!lowres)
								{
									byteBuffer->printf("+-------+---------------------------+---------------------------+---------------------------+---------------+\n");
								}
						}
						else
							if (!lowres)
							{
								byteBuffer->printf("+-------+---------------------------+---------------------------+---------------------------+---------------+\n");
							}
					}
				}
			}
			else if (format == SID_HISTORY_FORMAT_CSV)
			{
				// Filter cutoff
				if ((frames == firstframe) || (filt.cutoff != prevfilt.cutoff)) sprintf(&output[strlen(output)], "%04X, ", filt.cutoff);
				else sprintf(&output[strlen(output)], ", ");
				
				// Filter control
				if ((frames == firstframe) || (filt.ctrl != prevfilt.ctrl))
					sprintf(&output[strlen(output)], "%02X, ", filt.ctrl);
				else sprintf(&output[strlen(output)], ", ");
				
				// Filter passband
				if ((frames == firstframe) || ((filt.type & 0x70) != (prevfilt.type & 0x70)))
					sprintf(&output[strlen(output)], "%s, ", filtername[(filt.type >> 4) & 0x7]);
				else sprintf(&output[strlen(output)], ", ");
				
				// Mastervolume
				if ((frames == firstframe) || ((filt.type & 0xf) != (prevfilt.type & 0xf))) sprintf(&output[strlen(output)], "%01X, ", filt.type & 0xf);
				else sprintf(&output[strlen(output)], ", ");
				
				//			  // Rasterlines / cycle count
				//			  if (profiling)
				//			  {
				//				int cycles = cpucycles;
				//				int rasterlines = (cycles + 62) / 63;
				//				int badlines = ((cycles + 503) / 504);
				//				int rasterlinesbad = (badlines * 40 + cycles + 62) / 63;
				//				sprintf(&output[strlen(output)], "| %4d %02X %02X ", cycles, rasterlines, rasterlinesbad);
				//			  }
				
				// End of frame display, print info so far and copy SID registers to old registers
				sprintf(&output[strlen(output)], "\n");
				if ((!lowres) || (!((frames - firstframe) % spacing)))
				{
					byteBuffer->printf("%s", output);
					for (c = 0; c < 3; c++)
					{
						prevchn[c] = chn[c];
					}
					prevfilt = filt;
				}
				for (c = 0; c < 3; c++) prevchn2[c] = chn[c];
			}

		}
		
		// Advance to next frame
		frames++;
	}
}

