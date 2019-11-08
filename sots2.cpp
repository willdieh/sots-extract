#include "sots2.h"

#include <fstream>
#include <string>
#include <iostream>
#include <cstring>

int ProcessSots2File(std::ifstream& iFile, std::ofstream& oFile, std::string& oName)
{
	//Create a structure to hold our header data
	SOTS2_Header_struct sots2;

	//Create a DDS structure to hold our data
	DDS_Header_struct dds;

	//Create a BC3 structure to hold (compressed) image data
	BC3_struct bc3;

	//used for reading individual bytes in the file
	//unsigned char c = ' ';
	char c = ' ';

	//Was a valid header detected?
	bool valid_file = false;

	//Search the beginning of the SOTS2 header data (search for "SHYL").
	//20 byte sanity check so we don't parse the entire file
	while (iFile.tellg() < 20)
	{
		ReadBinary(iFile, c);

		if (c == 'S' && c != EOF && iFile.tellg() < 20)
		{
			ReadBinary(iFile, c);
			if (c == 'H' && c != EOF && iFile.tellg() < 20)
			{
				ReadBinary(iFile, c);
				if (c == 'Y' && c != EOF && iFile.tellg() < 20)
				{
					ReadBinary(iFile, c);
					if (c == 'L' && c != EOF && iFile.tellg() < 20)
					{
						std::cout << "** SOTS2 Texture File Found! **" << std::endl;
						valid_file = true;
						break;
					}
				}
			}
		}
	}

	//If we found a valid SOTS2 header, store relevant data in local vars for writing out to the target
	if (valid_file)
	{

		//Read in the mysterious/unknown magic number (DWORD)
		if (ReadBinary(iFile, sots2.SOTS2_magic_number))
		{
			std::cout << "SOTS2 Magic Number = " << sots2.SOTS2_magic_number << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read SOTS2 Magic Number" << std::endl;
			return -1;
		}


		//Read in the byte holding the file type string length
		if (!ReadBinary(iFile, sots2.string_len))
		{
			std::cout << "Error: Unable to read File Type Desc String Length" << std::endl;
			return -1;
		}

		//Store the file type string, adding a terminating char
		for (int i = 0; i < (int)sots2.string_len; i++)
		{
			if (!ReadBinary(iFile, c))
			{
				std::cout << "Error: Unable to read File Type Desc String" << std::endl;
				return -1;
			}
			else {
				sots2.file_type += c;
			}
		}
		std::cout << "SOTS2 File Type Desc: " << sots2.file_type << std::endl;


		//Get the file type code
		if(ReadBinary(iFile, sots2.type_code))
		{
			std::cout << "SOTS2 File Type Code: " << sots2.type_code << " (See MSDN for DXGI_FORMAT info)" << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read File Type Code" << std::endl;
			return -1;
		}


		//Get the width and height
		if(ReadBinary(iFile, sots2.width))
		{
			std::cout << "Width = " << sots2.width << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read Width" << std::endl;
			return -1;
		}

		if (ReadBinary(iFile, sots2.height))
		{
			std::cout << "Height = " << sots2.height << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read Height" << std::endl;
			return -1;
		}


		//Get the number of mipmaps
		if(ReadBinary(iFile, sots2.MIP_map_count))
		{
			std::cout << sots2.MIP_map_count << " MIP Maps Found" << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read MIP Map Count" << std::endl;
			return -1;
		}


		//read two unknown flag values
		if(!ReadBinary(iFile, sots2.flag1) || !ReadBinary(iFile, sots2.flag2))
		{
			std::cout << "Error: Unable to read SOTS2 flags" << std::endl;
			return -1;
		}


		//parse through the MIP Map info
		for (unsigned int i = 1; i <= sots2.MIP_map_count; i++)
		{
			if (i == 1) //First MIP definition holds pitch (bytes per line) of main image. Pick up the main image size too.
			{
				int data = 0; //Throw away first word? Hmmm, my original logic is lost during this refactoring
				if(!ReadBinary(iFile, data) || !ReadBinary(iFile, sots2.pitch) || !ReadBinary(iFile, sots2.main_size))
				{
					std::cout << "Error: Unable to read MIP Map " << i << " of " << sots2.MIP_map_count << std::endl;
					return -1;
				}
				else
				{
					std::cout << "Pitch (bytes per line) = " << sots2.pitch << std::endl;

					if (sots2.MIP_map_count == 1)
					{
						sots2.total_size = sots2.main_size; //If there's only one MIP, main_size == total_size
						std::cout << "Total Image Size (bytes) = " << sots2.total_size << std::endl;
					}
				}

			}
			else if (i == sots2.MIP_map_count) //Pull the total file size from last MIP (only get here if more than one MIP)
			{
				int data = 0; //Throw away first two words? Hmmm, my original logic is lost during this refactoring
				if (!ReadBinary(iFile, data) || !ReadBinary(iFile, data) || !ReadBinary(iFile, sots2.total_size))
				{
					std::cout << "Error: Unable to read MIP Map " << i << " of " << sots2.MIP_map_count << std::endl;
					return -1;
				}
				else
				{
					std::cout << "Total Image Size (bytes) = " << sots2.total_size << std::endl; //Use total_size from the last MIP instead (if multi)
				}
			}
			else //Otherwise just pull in the three DWORD data for each MIP
			{
				int data = 0; //Throw away three words? Hmmm, my original logic is lost during this refactoring
				if(!ReadBinary(iFile, data) || !ReadBinary(iFile, data) || !ReadBinary(iFile, data))
				{
					std::cout << "Error: Unable to read MIP Map " << i << " of " << sots2.MIP_map_count << std::endl;
					return -1;
				}
			}

		}

	}
	else
	{
		std::cout << "No SOTS2 Header Found in Input File" << std::endl;
		return -1;
	}


	//Write out to the appropriate file type
	//Type Code: 77 - DirectDraw Surface (DDS) in Block Compression 3 (BC3/DXT5) format
	//Type Code: 28 - DirectDraw Surface (DDS) in Uncompressed 4 byte RGBA format
	if (sots2.type_code == 77)
	{

		//Initialize our DDS structure
		//We could have just written straight to the output file from the values in the SOTS2 header, but this is clearer to understand [maybe]

		if (sots2.MIP_map_count > 1)
		{
			dds.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE;
		}
		else
		{
			dds.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
		}

		dds.width = sots2.width;        //width
		dds.height = sots2.height;      //height

		dds.pitch = sots2.total_size;    //pitch for compressed images is the total image size
		dds.depth = 0;                  //not a depth texture, so zero
		dds.MIP_map_count = sots2.MIP_map_count;    //Number of MIP Maps

													//Define pixel format info
		dds.pf.flags = DDPF_FOURCC;         //Set the DDS Pixel Format flags. Assumes this is a compressed format. todo: logic for uncompressed images
		dds.pf.fourCC[0] = 'D';             //Set the 4 char code compression type
		dds.pf.fourCC[1] = 'X';
		dds.pf.fourCC[2] = 'T';
		dds.pf.fourCC[3] = '5';
		dds.pf.RGBbitcount = 0;             //Not an uncompressed RGB image. todo: logic for uncompressed images
		dds.pf.Rbitmask = 0;                //Not an uncompressed RGB image. todo: logic for uncompressed images
		dds.pf.Gbitmask = 0;                //Not an uncompressed RGB image. todo: logic for uncompressed images
		dds.pf.Bbitmask = 0;                //Not an uncompressed RGB image. todo: logic for uncompressed images
		dds.pf.Abitmask = 0;                //Not an uncompressed RGB image. todo: logic for uncompressed images

											//set capabilities flags
		if (sots2.MIP_map_count > 0)
		{
			dds.caps = DDSCAPS_COMPLEX | DDSCAPS_MIPMAP | DDSCAPS_TEXTURE;   //has mipmaps
		}
		else
		{
			dds.caps = DDSCAPS_TEXTURE; //no mipmaps
		}


		if (!oFile.is_open())
		{
		    std::string filename;
            if(oName.find(".tga~") != std::string::npos){
                filename = oName.substr(0, oName.find(".tga~"));
		    }
		    else if(oName.find(".bmp~") != std::string::npos){
                filename = oName.substr(0, oName.find(".bmp~"));
		    }
			filename.append(".dds");

			std::cout << "===============================================================" << std::endl;
			std::cout << "OUTPUT File Name: " << filename << std::endl;

			oFile.open(filename, std::ostream::out | std::ostream::binary);
			if (oFile.fail())
			{
				std::cout << "Error: Unable to open OUTPUT filename " << filename << std::endl;
				return -1;
			}
		}
	}
	else if (sots2.type_code == 28) //Uncompressed RGBA image
	{
		//Initialize our DDS structure
		//We could have just written straight to the output file from the values in the SOTS2 header, but this is clearer to understand [maybe]

		if (sots2.MIP_map_count > 1)
		{
			dds.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT;
		}
		else
		{
			dds.flags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH | DDSD_PIXELFORMAT;
		}

		dds.width = sots2.width;        //width
		dds.height = sots2.height;      //height

		dds.pitch = sots2.pitch;        //pitch for uncompressed images is the bytes per line
		dds.depth = 0;                  //not a depth texture, so zero
		dds.MIP_map_count = sots2.MIP_map_count;    //Number of MIP Maps

													//Define pixel format info
		dds.pf.flags = DDPF_ALPHAPIXELS | DDPF_RGB;         //Set the DDS Pixel Format flags. Assumes this is a compressed format. todo: logic for uncompressed images
		dds.pf.fourCC[0] = 0;             //Set the 4 char code compression type (N/A for uncompressed RGBA files)
		dds.pf.fourCC[1] = 0;
		dds.pf.fourCC[2] = 0;
		dds.pf.fourCC[3] = 0;
		dds.pf.RGBbitcount = 32;            //Number of bits in an RGB format
		dds.pf.Rbitmask = 0x000000ff;       //Red Color Bitmask (little endian)
		dds.pf.Gbitmask = 0x0000ff00;       //Green Color Bitmask (little endian)
		dds.pf.Bbitmask = 0x00ff0000;       //Blue Color Bitmask (little endian)
		dds.pf.Abitmask = 0xff000000;       //Alpha Color Bitmask (little endian)

											//set capabilities flags
		if (sots2.MIP_map_count > 1)
		{
			dds.caps = DDSCAPS_COMPLEX | DDSCAPS_MIPMAP | DDSCAPS_TEXTURE;   //has mipmaps
		}
		else
		{
			dds.caps = DDSCAPS_TEXTURE; //no mipmaps
		}

		if (!oFile.is_open())
		{
		    std::string filename;
		    if(oName.find(".tga~") != std::string::npos){
                filename = oName.substr(0, oName.find(".tga~"));
		    }
		    else if(oName.find(".bmp~") != std::string::npos){
                filename = oName.substr(0, oName.find(".bmp~"));
		    }

			filename.append(".dds");

			std::cout << "===============================================================" << std::endl;
			std::cout << "OUTPUT File Name: " << filename << std::endl;

			oFile.open(filename, std::ostream::out | std::ostream::binary);
			if (oFile.fail())
			{
				std::cout << "Error: Unable to open OUTPUT filename " << filename << std::endl;
				return -1;
			}

		}
	}
	else
	{
		std::cout << "INPUT file is not a valid SOTS2 resource file " << std::endl;
		return -1;
	}

	//Write out the DDS header
	WriteBinary(oFile, dds);

	//Read in and Write out the DDS image data
	unsigned int count = 0;
	while (count < sots2.total_size && !iFile.eof())
	{
		ReadBinary(iFile, bc3);
		WriteBinary(oFile, bc3);

		count += sizeof(BC3_struct);
	}

	oFile.close();
	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

int ProcessDDSFile(std::ifstream& iFile, std::ofstream& oFile, std::string& oName)
{
	//Create a structure to hold our header data
	SOTS2_Header_struct sots2;

	//Create a DDS structure to hold our data
	DDS_Header_struct dds;

	//used for reading individual bytes / DWORDS in the file
	char c = ' ' ;
	unsigned int dword = 0;

	//Was a valid header detected?
	bool valid_file = false;

	//Search the beginning of the DDS header data (search for "DDS ").
	//5 byte sanity check so we don't parse the entire file
	while (iFile.tellg() < 5)
	{
		ReadBinary(iFile, c);
		if (c == 'D' && c != EOF && iFile.tellg() < 5)
		{
			ReadBinary(iFile, c);
			if (c == 'D' && c != EOF && iFile.tellg() < 5)
			{
				ReadBinary(iFile, c);
				if (c == 'S' && c != EOF && iFile.tellg() < 5)
				{
					ReadBinary(iFile, c);
					if (c == ' ' && c != EOF && iFile.tellg() < 5)
					{
						std::cout << "** DirectDraw Surface (DDS) File Found! **" << std::endl;
						valid_file = true;
						break;
					}
				}
			}
		}
	}

	//If we found a valid DDS header, store relevant data in local vars for writing out to the target
	if (valid_file)
	{
		ReadBinary(iFile, dword);

		//Read the DDS structure size. Should always be 124 (0x7c 0x00 0x00 0x00, little endian)
		if ((int)dword == EOF || dword != 124)
		{
			std::cout << "Error: Unable to read DDS structure size" << std::endl;
			return -1;
		}
		else
		{
			dds.structure_size = dword;
		}

		//Read in the DDS type flags
		ReadBinary(iFile, dword);

		if (!(dword ^ (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_PITCH)))
		{
			std::cout << "Uncompressed DDS File (No MIPMAPS)" << std::endl;
			dds.flags = dword;
			sots2.type_code = 28; //See DXGI_FORMAT
		}
		else if (!(dword ^ (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE)))
		{
			std::cout << "Compressed DDS File (No MIPMAPS)" << std::endl;
			dds.flags = dword;
			sots2.type_code = 77; //See DXGI_FORMAT
		}
		else if (!(dword ^ (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE | DDSD_MIPMAPCOUNT)))
		{
			std::cout << "Compressed DDS File (With MIPMAPS)" << std::endl;
			dds.flags = dword;
			sots2.type_code = 77; //See DXGI_FORMAT
		}
		else
		{
			std::cout << "No valid DDS File Type Found" << std::endl;
			return -1;
		}


		sots2.string_len = 9;
		sots2.file_type = "Texture2D";

		//Read in the height
		if (ReadBinary(iFile, sots2.height))
		{
			std::cout << "DDS Height:\t" << sots2.height << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read DDS Height" << std::endl;
			return -1;
		}

		//Read in the width
		if(ReadBinary(iFile, sots2.width))
		{
			std::cout << "DDS Width:\t" << sots2.width << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read DDS Width" << std::endl;
			return -1;
		}

		//Read in the pitch
		if(ReadBinary(iFile, sots2.pitch))
		{
			std::cout << "DDS Pitch:\t" << sots2.pitch << " bytes" << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read DDS Pitch" << std::endl;
			return -1;
		}

		//Read in the Depth (unused)
		if (!ReadBinary(iFile, dword))
		{
			std::cout << "Error: Unable to read DDS Depth" << std::endl;
			return -1;
		}

		//Read in the MIP Map Count
		if (ReadBinary(iFile, sots2.MIP_map_count))
		{
			std::cout << "DDS MIPs:\t" << sots2.MIP_map_count << std::endl;
		}
		else
		{
			std::cout << "Error: Unable to read DDS MIP Count" << std::endl;
			return -1;
		}

		//Read Reserved/unused DDS bytes (11 DWORDS)
		for (int i = 0; i < 11; i++) ReadBinary(iFile, dword);

		//Read Pixel Format bytes (8 Dwords)
		for (int i = 0; i < 8; i++) ReadBinary(iFile, dword);

		//Read the capabilities bytes (4 DWORD) + unused (1 DWORD)
		for (int i = 0; i < 5; i++) ReadBinary(iFile, dword);

		//unknown flags. seem to always have the same value though.
		sots2.flag1 = 1;
		sots2.flag2 = 0;

		if (!oFile.is_open())
		{
			std::string filename = oName.substr(0, oName.find(".dds"));
			filename.append(".tga~");

			std::cout << "===============================================================" << std::endl;
			std::cout << "OUTPUT File Name: " << filename << std::endl;

			oFile.open(filename, std::ostream::out | std::ostream::binary);
			if (oFile.fail())
			{
				std::cout << "Error: Unable to open OUTPUT filename " << filename << std::endl;
				return -1;
			}

		}

		//Begin writing out the SOTS2 header
		WriteBinary(oFile, "SHYL");

		//This is the SOTS2 "magic" number (no idea what it is... trying just an empty value)
		dword = 0x00000000;
		WriteBinary(oFile, dword);

		//File Type string length and string
		WriteBinary(oFile, sots2.string_len);
		for (int i = 0; i<sots2.string_len; i++)
		{
			WriteBinary(oFile, sots2.file_type.at(i));
		}

		//File Type Code
		WriteBinary(oFile, sots2.type_code);

		//width
		WriteBinary(oFile, sots2.width);

		//height
		WriteBinary(oFile, sots2.height);

		//MIP Map Count
		WriteBinary(oFile, sots2.MIP_map_count);

		//unknown flags
		WriteBinary(oFile, sots2.flag1);
		WriteBinary(oFile, sots2.flag2);

		//loop through each writing mipmap section
		int filepos = 0;
		for (unsigned int i = 1; i <= sots2.MIP_map_count; i++)
		{
			std::cout << "\tProcessing MIP Map Definition " << i << " of " << sots2.MIP_map_count << std::endl;
			if (sots2.type_code == 28)
			{
				//Image Size in bytes (uncompressed RGBA = 4 bytes per pixel)
				dword = sots2.width * sots2.height * 4;
				WriteBinary(oFile, dword);

				std::cout << "\t" << dword << " ";

				//Pitch (bytes per line)
				dword = sots2.width * 4;
				WriteBinary(oFile, dword);

				std::cout << "\t" << dword << " ";

				//File Offset to next MIP Map Data Section
				filepos = filepos + ((sots2.width / i) * (sots2.height / i) * 4);
				WriteBinary(oFile, filepos);

				std::cout << "\t" << filepos << std::endl;

			}
			else if (sots2.type_code == 77)
			{
				//Calculate the MIP Map divisor (powers of four)
				unsigned int divisor = 1;
				for (unsigned int x = 1; x<i; x++) divisor = divisor * 4;

				//Image Size in bytes (compressed using BC3 = 1 byte per pixel)
				dword = (sots2.width * sots2.height) / divisor;
				dword > 16 ? dword : dword = 16; //16 is the min size of a MIP map
				WriteBinary(oFile, dword);

				filepos = filepos + dword;

				std::cout << "\t" << dword << " ";

				//Calculate Pitch divisor (powers of 2)
				divisor = 1;
				for (unsigned int x = 1; x<i; x++) divisor = divisor * 2;

				//Pitch (bytes per line)
				dword = (sots2.width / divisor) * 4;
				dword > 16 ? dword : dword = 16; //16 is the min size of a MIP map
				WriteBinary(oFile, dword);

				std::cout << "\t" << dword << " ";

				//File Offset to next MIP Map Data Section
				WriteBinary(oFile, filepos);

				std::cout << "\t" << filepos << std::endl;

			}

		}

		std::cout << "Copying Image Data..." << std::endl;

		//Dump the rest of the file stream buffer into our output file.
		oFile << iFile.rdbuf();

		//Write Footer
		c = 0x00;
		for (int i = 0; i < 108; i++) WriteBinary(oFile, c);

		oFile.close();
	}
	 else {
		 std::cout << "Error: DDS Header Not Found" << std::endl;
		 return -1;
	}

	return 1;
}

template<typename T> bool WriteBinary(std::ofstream& oFile, T srcData) {
	//std::cout << "Writing " << sizeof(srcData) << " bytes\n";
	oFile.write(reinterpret_cast<char*>(std::addressof(srcData)), sizeof(srcData));
	oFile.flush();
	return oFile.good();

};

template<> bool WriteBinary<std::string>(std::ofstream& oFile, std::string string) {
	oFile.write(string.c_str(), string.size());
	oFile.flush();
	return oFile.good();
}

template<> bool WriteBinary<const char*>(std::ofstream& oFile, const char* string) {
	oFile.write(string, strlen(string));
	oFile.flush();
	return oFile.good();
}


template<typename T> bool ReadBinary(std::ifstream& iFile, T& tgtData) {
	//std::cout << "Reading " << sizeof(tgtData) << " bytes at position " << iFile.tellg() << "\n";
	iFile.read(reinterpret_cast<char*>(std::addressof(tgtData)), sizeof(tgtData));
	return iFile.good();
}
